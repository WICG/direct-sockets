# Explainer for Multicast in Direct Sockets

## Table of Contents

<!-- Update this table of contents by running `npx doctoc README.md` -->
<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [What is Multicast?](#what-is-multicast)
- [Use cases](#use-cases)
- [Similar functionality](#similar-functionality)
- [Permissions Policy integration](#permissions-policy-integration)
- [Sending datagrams example](#sending-datagrams-example)
- [Receiving datagrams example](#receiving-datagrams-example)
- [IDL Definitions](#idl-definitions)
  - [UDPSocketOptions](#udpsocketoptions)
  - [MulticastController](#multicastcontroller)
- [Which network interface will it use?](#which-network-interface-will-it-use)
- [Security considerations](#security-considerations)
- [Privacy considerations](#privacy-considerations)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## What is Multicast?
Multicast sockets are a way of one to many network communication. It is possible only for UDP datagram packets to be sent to a multicast group, and all subscribers would receive it.

In IPv4, any address between 224.0.0.0 to 239.255.255.255 can be used as a multicast address.
In IPv6 multicast addresses, the first 8 bits are all ones, i.e. FF00::/8. Further, bit 113-116 represents the scope of the address, which can be either one of the following 4: Global, Site-local, Link-local, Node-local.

Node-Local 
: Same device

Link-Local 
: This scope confines packets to the local network segment or link. This means the packet will reach all multicast listeners on the same physical or logical network (like a single Ethernet LAN or Wi-Fi network), but it will not be forwarded by routers to other network segments. This is commonly used for service discovery and autoconfiguration protocols, such as Neighbor Discovery Protocol (NDP).

Site-local
: This scope is larger than link-local. It is intended for use within a specific site or organization. Packets sent with this scope can be forwarded by routers, but they should not leave the boundaries of the organization's network. This is useful for enterprise-wide services where you want to reach all nodes within a company's network but prevent the traffic from leaking onto the public internet.

Global
: The whole world.

## Use cases

1. Synchronous video stream from one device to the whole network of devices. For example, imagine in a shopping mall there would be multiple displays which show videos simultaneously on all devices.
2. Device discovery on a local network. mDNS protocol uses multicast sockets under the hood. In short, when a device joins a network, it sends a message: “Hello, I am headphones-10, my ip is ‘20.20.20.14’ . Who else is here?”, other devices reply and in such a manner all devices can know about each other without a central server.

## Similar functionality

ChromeApp API [sockets.udp](https://developer.chrome.com/docs/apps/reference/sockets/udp) already has support for all necessary functions for multicast socket. The proposal is basically to port those API’s to [Isolated Web Apps](https://github.com/WICG/isolated-web-apps/blob/main/README.md).
The reason why it is restricted to Isolated Web Apps is because it is part of the Direct Sockets API which is also restricted to Isolated Web Apps. More about threats can be found in [Direct Sockets explainer](https://github.com/WICG/direct-sockets/blob/main/docs/explainer.md), in short, this API is considered high-risk primarily because it allows unencrypted, unsecured connection via arbitrary protocols (which might have vulnerabilities) and, more importantly, enables web applications to interact with (and potentially control) devices on private networks.

## Permissions Policy integration

This specification defines a policy-controlled permission identified by the string `direct-sockets-multicast`.

```
Permissions-Policy: direct-sockets-multicast=(self)
```
This [`Permissions-Policy`](https://chromestatus.com/feature/5745992911552512) header determines whether 
`multicastController.joinGroup()`, `multicastController.leaveGroup()` call or providing multicast params to
`new UDPSocket(..)` immediately rejects with SecurityError.

* In a similar fashion, Apple requires that apps with multicast declare [multicast entitlment](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.developer.networking.multicast). 

## Sending datagrams example

Everything is very similar to how `UDPSocket` is used to send data to a remote. The only difference is additional options to control multicast behavior.

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
Note: the behavior of loopback is slightly different between Windows and Unix-like systems. The inconsistency happens only when there is more than one application on the same host joined to the same multicast group while having different settings on multicast loopback mode. On Windows, the applications with loopback off will not RECEIVE the loopback packets; while on Unix-like systems, the applications with loopback off will not SEND the loopback packets to other applications on the same host. See [MSDN](https://learn.microsoft.com/en-us/windows/win32/winsock/ip-multicast-2).
It corresponds to IP_MULTICAST_LOOP / IPV6_MULTICAST_LOOP in Unix.


### MulticastController

```java
dictionary UDPSocketOpenInfo : SocketOpenInfo {
  MulticastController multicastController;
};

[
    RuntimeEnabled=MulticastInDirectSockets,
    Exposed=(Window, DedicatedWorker),
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

It is not planned to add an ability to pick which network interface to use. It would complicate the API.
Besides, right now there’s no way to get a networking interface ip address (in chrome apps, it was possible with https://developer.chrome.com/docs/apps/reference/system/network).
The [sockets.udp](https://developer.chrome.com/docs/apps/reference/sockets/udp) API also does not give an ability to use a specific networking interface.

For the reference, the C unix API gives such options: 
```c
group.imr_interface.s_addr = inet_addr("127.0.0.1");
```

Direct Sockets implementations should set `group.imr_interface.s_addr` to `INADDR_ANY`
which lets the OS pick the current active network interface.


## Security considerations

> This API gives the opportunity to flood the network with packets.

By only providing this API to Isolated Web Apps the user agent can be more confident that the application has not been compromised. This allows users and system administrators to confidently grant this capability to trusted applications.

Additionally, routers have control, whether they send multicast packets or drop them. In many setups dropping multicast packets is the default setting.

## Privacy considerations
The API allows an application to list all devices on a private network if they use a device discovery protocol like mDNS. This can be used for fingerprinting.