# Explainer for Multicast in Direct Sockets

## Table of Contents

<!-- Update this table of contents by running `npx doctoc README.md` -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Gloal](#gloal)
- [What is Multicast?](#what-is-multicast)
- [Similar functionality](#similar-functionality)
- [Use cases](#use-cases)
- [Sending datagrams example](#sending-datagrams-example)
- [Receiving datagrams example](#receiving-datagrams-example)
- [IDL Definitions](#idl-definitions)
  - [UDPSocketOptions](#udpsocketoptions)
  - [MulticastController](#multicastcontroller)
- [Which network interface will it use?](#which-network-interface-will-it-use)
- [Security considerations](#security-considerations)
- [Privacy considerations](#privacy-considerations)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Gloal
Allow [Isolated Web Apps](https://github.com/WICG/isolated-web-apps) to join/leave multicast groups and to specify multicast settings like time to live for sending datagrams using [Direct Sockets API](https://github.com/WICG/direct-sockets/blob/main/docs/explainer.md).
Only on [Managed by Admin devices](https://wicg.github.io/WebApiDevice/managed_config/).

## What is Multicast?
Multicast sockets are a way of one to many network communication. It is possible only for UDP datagram packets to be sent to a multicast group, and all subscribers would receive it.

In IPv4, any address between 224.0.0.0 to 239.255.255.255 can be used as a multicast address.
In IPv6 multicast addresses, the first 8 bits are all ones, i.e. FF00::/8. Further, bit 113-116 represents the scope of the address, which can be either one of the following 4: Global, Site-local, Link-local, Node-local.

Node-Local : Same device

Link-Local : Link-local scope confines packets to the local network segment or link. This means the packet will reach all multicast listeners on the same physical or logical network (like a single Ethernet LAN or Wi-Fi network), but it will not be forwarded by routers to other network segments. This is commonly used for service discovery and autoconfiguration protocols, such as Neighbor Discovery Protocol (NDP).

Site-local scope is larger than link-local. It is intended for use within a specific site or organization. Packets sent with this scope can be forwarded by routers, but they should not leave the boundaries of the organization's network. This is useful for enterprise-wide services where you want to reach all nodes within a company's network but prevent the traffic from leaking onto the public internet.

Global : The whole world.

## Use cases

1. Synchronous video stream from one device to the whole network of devices. For example, imagine in a shopping mall there would be multiple displays which show videos simultaneously on all devices.
2. Device discovery on a local network. mDNS protocol uses multicast sockets under the hood. In short, when a device joins a network, it sends a message: “Hello, I am headphones-10, my ip is ‘20.20.20.14’ . Who else is here?”, other devices reply and in such a manner all devices can know about each other without a central server.

## Similar functionality

ChromeApp api [sockets.udp](https://developer.chrome.com/docs/apps/reference/sockets/udp) already has support for all necessary functions for multicast socket. The proposal is basically to port those api’s to open web (IWA).<br>
The reason why it is restricted to IWA, is because the whole api is part of the DirectSockets which are IWA only. More about threats can be found in [DirectSocket explainer](https://github.com/WICG/direct-sockets/blob/main/docs/explainer.md), in short, this api is considered dangerous primarily because it allows for unencrypted, unsecured connection via any unknown protocol with potential vulnerabilities, and more importantly it allows access to private network, for example, it can be used to scan all possible devices on private network for fingerprinting.

## Sending datagrams example

Everything is very similar to how direct sockets are used for sending data to a remote. The only difference is multicast options.

```javascript
// Params that sender and receiver are agreed upon.
var PORT = 12345;
var MULTICAST_GROUP_ADDR = "239.0.0.1";

// Can be bound or connected socket as you like.
// This is example of connected one.
const socket = new UDPSocket({
    remoteAddress: MULTICAST_GROUP_ADDR,
    remotePort: PORT,
    // How much network hops are allowed until the packet is discarded.
    multicastTimeToLive: 5,
    // For debug purposes, send this datagram also back to this machine.
    multicastLoopback: true
});

const encoder = new TextEncoder();
const { writable, remoteAddress, remotePort, localAddress, localPort } = await socket.opened;

const writer = writable.getWriter();
await writer.ready;

for (let i = 0; i < 3; i++) { 
    await writer.write({data: encoder.encode(`message from CLIENT = ${i}`)});
}

writer.releaseLock();
await reader.cancel();
await socket.close();
```

## Receiving datagrams example

```javascript
// Params that sender and receiver are agreed upon.
var PORT = 12345;
var MULTICAST_GROUP_ADDR = "239.0.0.1";
// This must be bound socket. Cannot be connected.
const socket = new UDPSocket({
    // This address acts as a filter in case of multicast.
    localAddress: '0.0.0.0',
    // The port must be same as remotePort in sender.
    localPort: PORT,
    // Other multicast options are only for sender.
    multicastAllowAddressSharing: true
});

const { readable, multicastController } = await socket.opened;

await multicastController.joinGroup(MULTICAST_GROUP_ADDR);

for (group in multicastController.joinedGroups) {
   console.log("joined multicast group=" + group);
}

const reader = readable.getReader()
readStream(reader, (message) => { 
    const messageStr = decoder.decode(message.data);
    console.log('processing response= ' + messageStr);
});

// stop execution after reading messages for 5s.
await delay(5000);

await reader.cancel();
// Not necessary. The socket will leave all joined groups on calling close.
await multicastController.leaveGroup(MULTICAST_GROUP_ADDR);
await socket.close();


export async function readStream(
  reader, // :ReadableStreamDefaultReader
  cb, // : (value: Uint8Array) => void
) { 
  // Read from the socket until it's closed
  while (reader) {
    // Wait for the next chunk
    const { value, done } = await reader.read();

    // Send the chunk to the callback
    if (value) {
      cb(value);
    }

    // Release the reader if we're done
    if (done) {
      console.log('readStream finish');
      reader.releaseLock();
      break;
    }
  }
}

function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}
```

## IDL Definitions

### UDPSocketOptions
```java
dictionary UDPSocketOptions : SocketOptions {
  .. // old fields.
  [EnforceRange] octet multicastTimeToLive;
  boolean multicastLoopback;
  boolean multicastAllowAddressSharing;
};
```

* **multicastAllowAddressSharing** - Whether address reuse is allowed.
This comes in handy for device discovery use cases. When multiple applications need to listen to the same address and port to be able to catch messages that a new device has joined the network.<br>
Value can only be provided if the app has permission to use Multicast subscribing.<br>
The default is false.<br>
It corresponds to SO_REUSEADDR / SO_REUSEPORT in Unix.

* **multicastTimeToLive** - This option controls how far your multicast packets can travel across a network. Each time a packet passes through a router, its TTL value is decremented. If the TTL reaches zero, the packet is discarded.<br>
The default is 1.<br>
It corresponds to IP_MULTICAST_TTL / IPV6_MULTICAST_HOPS in Unix.

* **multicastLoopback** - Sets whether multicast packets sent from the host to the multicast group will be looped back to the host.<br>
The default is true.<br>
Note: the behavior of loopback is slightly different between Windows and Unix-like systems. The inconsistency happens only when there is more than one application on the same host joined to the same multicast group while having different settings on multicast loopback mode. On Windows, the applications with loopback off will not RECEIVE the loopback packets; while on Unix-like systems, the applications with loopback off will not SEND the loopback packets to other applications on the same host. See [MSDN](https://learn.microsoft.com/en-us/windows/win32/winsock/ip-multicast-2)<br>
It corresponds to IP_MULTICAST_LOOP / IPV6_MULTICAST_LOOP in Unix.


### MulticastController

```java
dictionary UDPSocketOpenInfo : SocketOpenInfo {
  MulticastController multicastController;
};

[
    ActiveScriptWrappable,
    Exposed(
      Window DirectSockets,
      DedicatedWorker DirectSockets,
      SharedWorker DirectSocketsInSharedWorkers,
      ServiceWorker DirectSocketsInServiceWorkers
    ),
    SecureContext,
    IsolatedContext
]
interface MulticastController {

  [CallWith=ScriptState, RaisesException, MeasureAs=MulticastControllerJoinGroupFunction]
  Promise<undefined> joinGroup(DOMString ipAddress);
  [CallWith=ScriptState, RaisesException, MeasureAs=MulticastControllerLeaveGroupFunction]
  Promise<undefined> leaveGroup(DOMString ipAddress);

  [CallWith=ScriptState, MeasureAs=MulticastControllerJoinedGroups]
  readonly attribute FrozenArray<DOMString> joinedGroups;
};
```
* **MulticastController** is an object returned if the app has permission to use Multicast subscribing.

* **joinGroup(DOMString ipAddress)**<br>
Joins the multicast group and starts to receive packets from that group. The socket must be bound to a local port before calling this method.<br>
The ipAddress param must be a valid ip address, not a domain name. Otherwise TypeError is thrown.<br>
The function returns a promise, which resolves with undefined in case of success. In case of a network error, the promise is rejected with a NetworkError DOMException. The message property may contain an implementation-defined string describing the error condition in more detail.<br>
Corresponds to IP_ADD_MEMBERSHIP / IP6_ADD_MEMBERSHIP in Unix.


* **leaveGroup(DOMString ipAddress)**<br>
Leaves the multicast group previously joined using joinGroup. This is only necessary to call if you plan to keep using the socket afterwards, since clean up of resources will be done automatically by the OS when the socket is closed.
Leaving the group will prevent the router from sending multicast datagrams to the local host, presuming no other process on the host is still joined to the group.<br>
The ipAddress param must be a valid ip address, not a domain name. Otherwise TypeError is thrown.<br>
The function returns a promise, which resolves with undefined in case of success. In case of a network error, the promise is rejected with a NetworkError DOMException. The message property may contain an implementation-defined string describing the error condition in more detail.<br>
Corresponds to IP_DROP_MEMBERSHIP / IP6_DROP_MEMBERSHIP in Unix.

* **FrozenArray<DOMString> joinedGroups**<br>
Gets the multicast group addresses the socket is currently joined to. The groups that are in process of joining are not returned.


## Which network interface will it use?

It is not planned to add an ability to pick which network interface to use. It would complicate the api.
Besides, right now there’s no way to get a networking interface ip address (in chrome apps, it was possible with https://developer.chrome.com/docs/apps/reference/system/network).
ChromeApp https://developer.chrome.com/docs/apps/reference/sockets/udp api also does not give an ability to use a specific networking interface.

For the reference, the C unix api gives such options: 
group.imr_interface.s_addr = inet_addr("127.0.0.1");

DirectSocket implementation will use group.imr_interface.s_addr = INADD_ANY; 
Which lets the OS pick the current active network interface.


## Security considerations

> This api gives the opportunity to flood the network with packets.

For now, this functionality is supported only on Managed devices, meaning that Admins have complete control over the software that is installed, and only allowlisted by google IWA’s can be installed.
However, in the future it is planned to make this api available also for Unmanaged use cases.

Additionally, routers have control, whether they send multicast packets or drop them. In many setups dropping multicast packets is the default setting.

## Privacy considerations
The api allows to list all devices on a private network if they use device discovery protocol like mDNS. Further, this can be used for fingerprinting.

## Security/Privacy mitigations
As a mitigation, the new permission policy `direct-sockets-multicast` is required to use Multicast capabilities.
If it is not provided, then specifying multicast parameters on UDPSocketOptions
or calling joinGroup/leaveGroup will result in SecurityError.

* In a similar fashion, apple requires that apps with multicast declare [multicast entitlment](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.developer.networking.multicast). 
