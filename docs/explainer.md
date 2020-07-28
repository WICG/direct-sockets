# Raw Sockets



## Motivation

This Network Sockets API proposal related to the Discourse post [Filling the remaining gap between WebSocket, WebRTC and WebTransport](https://discourse.wicg.io/t/filling-the-remaining-gap-between-websocket-webrtc-and-webtranspor/4366).

The motivating use case is to support creating a web app that talks to servers and devices that have their own protocols incompatible with what’s available on the web, e.g. [SSH](https://en.wikipedia.org/wiki/Secure_Shell), [RDP](https://en.wikipedia.org/wiki/Remote_Desktop_Protocol) or [printer protocols](https://en.wikipedia.org/wiki/List_of_printing_protocols). The web app should be able to talk to a legacy system, without requiring users to change or replace that system.



## Security Considerations

User agents will need to carefully consider when to make the Raw Sockets API available to web applications,
and what UI will be shown to the user.

## TCP

Applications will be able to request a TCP socket using a method on `navigator`:

```javascript
const options = {
    remoteAddress: 'example.com',
    remotePort: 7,
    keepAlive: false,
    noDelay: false
};
navigator.openTCPSocket(options).then(tcpSocket => { ... }).else(error => { ... });
```

The `remoteAddress` member may be omitted or ignored - the user agent may invite the user to specify the address.

The TCP socket can be used for reading and writing:

```
let readableStream = tcpSocket.readable;
let writableStream = tcpSocket.writable;
...
tcpSocket.close();
```

## UDP
