<img src="https://wicg.github.io/direct-sockets/logo-socket.svg" height="100" align=right>

# Direct Sockets

## Background

This Direct Sockets API proposal relates to the Discourse post [Filling the remaining gap between WebSocket, WebRTC and WebTransport](https://discourse.wicg.io/t/filling-the-remaining-gap-between-websocket-webrtc-and-webtranspor/4366).

This API is currently planned as a part of the [Isolated Web Apps](https://github.com/WICG/isolated-web-apps/blob/main/README.md) proposal - check out [Telnet Client Demo](https://github.com/GoogleChromeLabs/telnet-client) for a showcase of the API's capabilities.

## Use cases

The initial motivating use case is to support creating a web app that talks to servers and devices that have their own protocols incompatible with what’s available on the web. The web app should be able to talk to a legacy system, without requiring users to change or replace that system.

- [Secure Shell](https://en.wikipedia.org/wiki/Secure_Shell)
- [Remote Desktop Protocol](https://en.wikipedia.org/wiki/Remote_Desktop_Protocol)
- [printer protocols](https://en.wikipedia.org/wiki/List_of_printing_protocols)
- Mail
- [IRC](https://en.wikipedia.org/wiki/Internet_Relay_Chat)
- [IOT](https://en.wikipedia.org/wiki/Internet_of_things) smart devices
- [Distributed Hash Tables for P2P systems](https://discourse.wicg.io/t/filling-the-remaining-gap-between-websocket-webrtc-and-webtranspor/4366/3)
- [Resilient collaboration using IPFS](https://discourse.wicg.io/t/filling-the-remaining-gap-between-websocket-webrtc-and-webtranspor/4366/5)
- [Virtual Desktop Infrastructure (VDI)](https://en.wikipedia.org/wiki/Desktop_virtualization)

## Alternatives

Web apps can already establish data communications using [XMLHttpRequest](https://xhr.spec.whatwg.org/), [WebSocket](https://html.spec.whatwg.org/multipage/web-sockets.html) and [WebRTC](https://w3c.github.io/webrtc-pc/).

These facilities are designed to be tightly constrained ([example design requirements](https://lists.w3.org/Archives/Public/public-whatwg-archive/2008Jun/0165.html)), and don't allow raw TCP or UDP communication. Such facilities were [requested](https://lists.whatwg.org/pipermail/whatwg-whatwg.org/2008-September/058530.html) but not provided due to the potential for abuse.

If the web server has network access to the required server or device, it can act as relay. For example, a web mail application might use XMLHttpRequest for communication between the browser and the web server, and use SMTP and IMAP between the web server and a mail server.

In scenarios like the following, a vendor would require their customers to deploy a relay web server on each site where devices/servers are in use:
- The target devices might be behind firewalls, not accessible to a central server.
- Communication might be bandwidth-intensive or latency-sensitive.
- There might be legal constraints on where data can be sent.
- On-site communication may need to be resilient to ISP outages.

Developers have used browser plugins - such as Java applets, ActiveX, [Adobe Flex](https://www.adobe.com/au/products/flex.html) or [Microsoft Silverlight](https://www.microsoft.com/silverlight/) - to establish raw TCP or UDP communication from the browser, without relaying through a web server.

With the shift away from browser plugins, native apps now provide the main alternative. Widely used APIs include [POSIX sockets](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_socket.h.html), [Winsock](https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-functions), [java.net](https://docs.oracle.com/javase/7/docs/api/java/net/package-summary.html) and [System.Net.Sockets](https://docs.microsoft.com/en-us/dotnet/api/system.net.sockets).

JavaScript APIs for socket communication have been developed for B2G OS ([TCP](https://developer.mozilla.org/en-US/docs/Archive/B2G_OS/API/TCPSocket), [UDP](https://developer.mozilla.org/en-US/docs/Archive/B2G_OS/API/UDPSocket)) and Chrome Apps ([TCP](https://developer.chrome.com/apps/sockets_tcp), [UDP](https://developer.chrome.com/apps/sockets_udp)). An earlier proposed API for the web platform is the [TCP and UDP Socket API](https://www.w3.org/TR/tcp-udp-sockets/), which the System Applications Working Group published as an informative Working Group Note and is no longer progressing.

## Permissions Policy integration

This specification defines a policy-controlled permission identified by the string `direct-sockets`. Its default allowlist is `self`.

```
Permissions-Policy: direct-sockets=(self)
```

This [`Permissions-Policy`](https://chromestatus.com/feature/5745992911552512) header determines whether a `new TCPSocket(...)`, `new UDPSocket(...)` or `new TCPServerSocket(...)` call immediately rejects with a `NotAllowedError` `DOMException`.

## Security Considerations

The API is planned to be available only in
[Isolated Web Apps](https://github.com/WICG/isolated-web-apps/blob/main/README.md)
which themselves provide a decent level of security thanks to a transparent update model and strict Content Security Policy. Nevertheless, user agents will need to carefully consider when to make the Direct Sockets API available to web applications, and what UI will be shown to the user.

### Threat

Third party iframes (such as ads) might initiate connections.

#### Mitigation

The `direct-sockets` permissions policy will control access, preventing third party use by default. To further safeguard from potential third-party attacks, IWAs employ a strict Content Security Policy that makes using external resources (i.e. the ones not originating from the Web Bundle itself) difficult and enforce [cross-origin-isolation](https://web.dev/why-coop-coep/).

### Threat

Use of the API may violate organization policies, that control which protocols may be used.

#### Mitigation

User agents may restrict use of the API when enterprise software policies are in effect. For example, user agents might by default not allow use of this API unless the user has permission to install new binaries.

### Threat

MITM attackers may hijack plaintext connections created using the API.

#### Mitigation

User agents should reject connection attempts when [Content Security Policy](https://w3c.github.io/webappsec-csp/) allows the `unsafe-eval` source expression. This prevents sites from executing [eval()](https://tc39.es/ecma262/#sec-eval-x) on data retrieved using this API.

We should also facilitate use of TLS on TCP connections.

One option would be to allow TLS to be requested when opening a connection, like the [TCP and UDP Socket API](https://www.w3.org/TR/tcp-udp-sockets/)'s [useSecureTransport](https://www.w3.org/TR/tcp-udp-sockets/#widl-TCPOptions-useSecureTransport).

Another option would be to provide a method that upgrades an existing TCP connection to use TLS. Use cases would include SMTP [STARTTLS](https://tools.ietf.org/html/rfc3207#section-4), IMAP [STARTTLS](https://tools.ietf.org/html/rfc2595#section-3.1) and POP [STLS](https://tools.ietf.org/html/rfc2595#section-4).

## TCPSocket

Applications will be able to request a TCP socket by creating a `TCPSocket` class using the `new` operator and then waiting for the connection to be established. Refer to the snippets below for a deeper dive.

### IDL Definitions
```java
enum SocketDnsQueryType { "ipv4", "ipv6" };

dictionary TCPSocketOptions {
  boolean noDelay = false;
  [EnforceRange] unsigned long keepAliveDelay;
  [EnforceRange] unsigned long sendBufferSize;
  [EnforceRange] unsigned long receiveBufferSize;
  SocketDnsQueryType dnsQueryType;
};

dictionary TCPSocketOpenInfo {
  ReadableStream readable;
  WritableStream writable;

  DOMString remoteAddress;
  unsigned short remotePort;

  DOMString localAddress;
  unsigned short localPort;
};

interface TCPSocket {
  constructor(
    DOMString remoteAddress,
    unsigned short remotePort,
    optional TCPSocketOptions options = {});

  readonly attribute Promise<TCPSocketOpenInfo> opened;
  readonly attribute Promise<void> closed;

  Promise<void> close();
};
```

### Examples

<details>
<summary>Learn more about using TCPSocket.</summary>

### Opening/Closing the socket

```javascript
const remoteAddress = 'example.com';
const remotePort = 7;

const options = {
  noDelay: false,
  keepAlive: true,
  keepAliveDelay: 720_000
};

let tcpSocket = new TCPSocket(remoteAddress, remotePort, options);
// If rejected by permissions-policy...
if (!tcpSocket) {
  return;
}

// Wait for the connection to be established...
let { readable, writable } = await tcpSocket.opened;

// do stuff with the socket
...

// Close the socket. Note that this operation will succeeed if and only if neither readable not writable streams are locked.
tcpSocket.close();
```

### IO operations

The TCP socket can be used for reading and writing.
- Writable stream accepts [`BufferSource`](https://developer.mozilla.org/en-US/docs/Web/API/BufferSource)
- Readable stream returns [`Uint8Array`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array)

#### Reading

See [`ReadableStream`](https://streams.spec.whatwg.org/#rs-intro) spec for more examples.
`TCPSocket` supports both [`ReadableStreamDefaultReader`](https://streams.spec.whatwg.org/#default-reader-class) and [`ReadableStreamBYOBReader`](https://streams.spec.whatwg.org/#readablestreambyobreader).

```javascript
let tcpSocket = new TCPSocket(...);

let { readable } = await tcpSocket.opened;
let reader = readable.getReader();

let { value, done } = await reader.read();
if (done) {
  // stream is exhausted...
  return;
}

const decoder = new TextDecoder();
let message = decoder.decode(value);
...

// Don't forget to call releaseLock() or cancel() on the reader once done.
```

#### Writing

See [`WritableStream`](https://streams.spec.whatwg.org/#ws-intro) spec for more examples.

```javascript
let tcpSocket = new TCPSocket(...);

let { writable } = await tcpSocket.opened;
let writer = writable.getWriter();

const encoder = new TextEncoder();
let message = "Some user-created tcp data";

await writer.ready;
writer.write(
  encoder.encode(message)
).catch(err => console.log(err));
...

// Don't forget to call releaseLock() or cancel()/abort() on the writer once done.
```

</details>

## UDPSocket

Applications will be able to request a UDP socket by creating a `UDPSocket` class using the `new` operator and then waiting for the socket to be opened.

`UDPSocket` operates in different modes depending on the provided set of options:
* In `bound` mode the socket is bound to a specific local IP endpoint (defined by `localAddress` and optionally `localPort`) and is capable of sending/receiving datagrams to/from arbitrary destinations.
  * `localAddress` must be a valid IP address.
  * `localPort` can be omitted to let the OS pick one on its own.
  * `remoteAddress` and `remotePort` must be specified on a per-packet basis as part of `UDPMessage`.
  * `remoteAddress` (in `UDPMessage`) can either be an IP address or a domain name when sending.
* In `connected` mode the socket is bound to an arbitrary local IP endpoint and sends/receives datagrams from/to a single destination (defined by `remoteAddress` and `remotePort`).
  *`remoteAddress` can either be an IP address or a domain name.

`remoteAddress`/`remotePort` and `localAddress`/`localPort` pairs cannot be specified together.

### IDL Definitions
```java
enum SocketDnsQueryType { "ipv4", "ipv6" };

dictionary UDPSocketOptions {
  DOMString remoteAddress;
  [EnforceRange] unsigned short remotePort;

  DOMString localAddress;
  [EnforceRange] unsigned short localPort;

  SocketDnsQueryType dnsQueryType;

  [EnforceRange] unsigned long sendBufferSize;
  [EnforceRange] unsigned long receiveBufferSize;
};

dictionary UDPSocketOpenInfo {
  ReadableStream readable;
  WritableStream writable;

  DOMString remoteAddress;
  unsigned short remotePort;

  DOMString localAddress;
  unsigned short localPort;
};

interface UDPSocket {
  constructor(UDPSocketOptions options);

  readonly attribute Promise<UDPSocketOpenInfo> opened;
  readonly attribute Promise<void> closed;

  Promise<void> close();
};
```

### Examples

<details>
<summary>Learn more about using UDPSocket.</summary>

### Opening/Closing the socket

#### Connected version
```javascript
const remoteAddress = 'example.com'; // could be a raw IP address too
const remotePort = 7;

let udpSocket = new UDPSocket({ remoteAddress, remotePort });
// If rejected by permissions-policy...
if (!udpSocket) {
  return;
}

// Wait for the connection to be established...
let { readable, writable } = await udpSocket.opened;

// do stuff with the socket
...

// Close the socket. Note that this operation will succeeed if and only if neither readable not writable streams are locked.
udpSocket.close();
```

#### Bound version
```javascript
const localAddress = '127.0.0.1';

// Omitting |localPort| allows the OS to pick one on its own.
let udpSocket = new UDPSocket({ localAddress });
// If rejected by permissions-policy...
if (!udpSocket) {
  return;
}

// Wait for the connection to be established...
let { readable, writable } = await udpSocket.opened;

// do stuff with the socket
...

// Close the socket. Note that this operation will succeeed if and only if neither readable not writable streams are locked.
udpSocket.close();
```

### IO operations

The UDP socket can be used for reading and writing. Both streams operate on the `UDPMessage` object which is defined as follows (idl):
```javascript
dictionary UDPMessage {
  BufferSource   data;
  DOMString      remoteAddress;
  unsigned short remotePort;
};
```
- Writable stream accepts `data` as [`BufferSource`](https://developer.mozilla.org/en-US/docs/Web/API/BufferSource)
- Readable stream returns `data` as [`Uint8Array`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Uint8Array)

#### Reading

See [`ReadableStream`](https://streams.spec.whatwg.org/#rs-intro) spec for more examples.

##### Connected mode

```javascript
let udpSocket = new UDPSocket({ remoteAddress, remotePort });
let { readable } = await udpSocket.opened;

let reader = readable.getReader();

let { value, done } = await reader.read();
if (done) {
  // stream is exhausted...
  // happens either on socket errors or explicit socket.close();
  return;
}

const decoder = new TextDecoder();

// |value| is a UDPMessage object.
// |remoteAddress| and |remotePort| members of UDPMessage are guaranteed to be null.
let { data } = value;
let message = decoder.decode(data);
...

// Don't forget to call releaseLock() or cancel() on the reader once done.
```

##### Bound mode

```javascript
let udpSocket = new UDPSocket({ localAddress });
let { readable } = await udpSocket.opened;

let reader = readable.getReader();

let { value, done } = await reader.read();
if (done) {
  // stream is exhausted...
  // happens either on socket errors or explicit socket.close();
  return;
}

const decoder = new TextDecoder();

// |value| is a UDPMessage object.
// |remoteAddress| and |remotePort| members of UDPMessage indicate the remote host
// where the datagram came from.
let { data, remoteAddress, remoetPort } = value;
let message = decoder.decode(data);
...

// Don't forget to call releaseLock() or cancel() on the reader once done.
```

#### Writing

See [`WritableStream`](https://streams.spec.whatwg.org/#ws-intro) spec for more examples.

##### Connected mode

```javascript
let udpSocket = new UDPSocket({ remoteAddress, remotePort });
let { writable } = await udpSocket.opened;

let writer = writable.getWriter();

const encoder = new TextEncoder();
let message = "Some user-created datagram";

// Sends a UDPMessage object where |data| is a Uint8Array.
// Note that |remoteAddress| and |remotePort| must not be specified.
await writer.ready;
writer.write({
    data: encoder.encode(message)
}).catch(err => console.log(err));

// Sends a UDPMessage object where |data| is an ArrayBuffer.
// Note that |remoteAddress| and |remotePort| must not be specified.
await writer.ready;
writer.write({
    data: encoder.encode(message).buffer
}).catch(err => console.log(err));
...

// Don't forget to call releaseLock() or cancel()/abort() on the writer once done.
```

##### Bound mode

```javascript
let udpSocket = new UDPSocket({ localAddress });
let { writable } = await udpSocket.opened;

let writer = writable.getWriter();

const encoder = new TextEncoder();
let message = "Some user-created datagram";

// Sends a UDPMessage object where |data| is a Uint8Array.
// Note that both |remoteAddress| and |remotePort| must be specified in this case.
await writer.ready;
writer.write({
    data: encoder.encode(message),
    remoteAddress: 'example.com',
    remotePort: 7
}).catch(err => console.log(err));

// Sends a UDPMessage object where |data| is an ArrayBuffer.
// Note that both |remoteAddress| and |remotePort| must be specified in this case.
await writer.ready;
writer.write({
    data: encoder.encode(message).buffer,
    remoteAddress: '98.76.54.32',
    remotePort: 18
}).catch(err => console.log(err));
...

// Don't forget to call releaseLock() or cancel()/abort() on the writer once done.
```

</details>

## TCPServerSocket

Applications will be able to request a TCP server socket by creating a `TCPServerSocket` class using the `new` operator and then waiting for the socket to be opened.
  * `localPort` can be omitted to let the OS pick one on its own.
  * `backlog` sets the size of the OS accept queue; if not specified, will be substituted by platform default.

### IDL Definitions

```java
dictionary TCPServerSocketOptions {
  [EnforceRange] unsigned short localPort,
  [EnforceRange] unsigned long backlog;
};

dictionary TCPServerSocketOpenInfo {
  ReadableStream readable;

  DOMString localAddress;
  unsigned short localPort;
};

interface TCPServerSocket {
  constructor(
    DOMString localAddress,
    optional TCPServerSocketOptions options = {});

  readonly attribute Promise<TCPServerSocketOpenInfo> opened;
  readonly attribute Promise<void> closed;

  Promise<void> close();
};
```

### Examples

<details>
<summary>Learn more about TCPServerSocket.</summary>

### Opening/Closing the socket

```javascript
let tcpServerSocket = new TCPServerSocket('::');
// If rejected by permissions-policy...
if (!tcpServerSocket) {
  return;
}

// Wait for the connection to be established...
let { readable } = await tcpServerSocket.opened;

// do stuff with the socket
...

// Close the socket. Note that this operation will succeeed if and only if readable stream is not locked.
tcpServerSocket.close();
```

### Accepting connections

Connection accepted by `TCPServerSocket` are delivered via its `ReadableStream` in the form of ready-to-use `TCPSocket` objects.

See [`ReadableStream`](https://streams.spec.whatwg.org/#rs-intro) spec for more examples.

```javascript
let tcpServerSocket = new TCPServerSocket('::');
let { readable: tcpServerSocketReadable } = await udpSocket.opened;

let tcpServerSocketReader = tcpServerSocketReadable.getReader();

// |value| is an accepted TCPSocket.
let { value: tcpSocket, done } = await tcpServerSocketReader.read();
if (done) {
  // stream is exhausted...
  // happens either on socket errors or explicit socket.close();
  return;
}
tcpServerSocketReader.releaseLock();

// Send a packet using the newly accepted socket.
const { writable: tcpSocketWritable } = await tcpSocket.opened;

const tcpSocketWriter = tcpSocketWritable.getWriter();

const encoder = new TextEncoder();
const message = "Some user-created tcp data";

await tcpSocketWriter.ready;
await tcpSocketWriter.write(encoder.encode(message));

// Don't forget to call releaseLock() or close()/abort() on the writer once done.
```
</details>
