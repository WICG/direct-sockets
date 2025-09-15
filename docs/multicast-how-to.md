# How to use the Multicast in Direct Sockets API

The Multicast In Direct Sockets API is currently available in Chrome 142 and later on Desktop platforms (ChromeOS, Windows, Linux, MacOS) behind a flag.

1) Ensure Chrome is updated to 142.
2) Navigate to `chrome://flags` and enable `Multicast in Direct Sockets` (remember to restart the browser as prompted).
3) Add new permission policy `direct-sockets-multicast` to Isolated Web App manifest. `direct-sockets`, `direct-sockets-private` are necessary as well.
4) The API should be available for newly installed Isolated Web Apps now.
