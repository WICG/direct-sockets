## Responses to the [Self-Review Questionnaire: Security and Privacy](https://w3ctag.github.io/security-questionnaire/) for the [Direct Sockets API](https://github.com/WICG/direct-sockets/)


1. **What information might this feature expose to Web sites or other parties, and for what purposes is that exposure necessary?**

    When the client establishes a socket connection with a remote endpoint, and the connection succeeds, that system's IP address and port, and any information it sends to the remote endpoint, becomes available to the client, and vice versa.

2. **Do features in your specification expose the minimum amount of information necessary to enable their intended uses?**

    Yes. The specification provides no more information than is needed for communication with arbitrary systems directly over TCP and UDP.

3. **How do the features in your specification deal with personal information, personally-identifiable information (PII), or information derived from them?**

    This specification deals with generic information.

    This specification allows devices to use their existing protocols for communication. These may or may not be secure. We intend to [facilitate](https://github.com/WICG/direct-sockets/blob/master/docs/explainer.md#mitigation-7) the use of TLS at some point.

4. **How do the features in your specification deal with sensitive information?**

    Same as in #3.

5. **Do the features in your specification introduce new state for an origin that persists across browsing sessions?**

    No.

6. **Do the features in your specification expose information about the underlying platform to origins?**

    No.

7. **Does this specification allow an origin to send data to the underlying platform?**

    Yes -- TCP and UDP connections are handled by the underlying platform.

8. **Do features in this specification enable access to device sensors?**

    Only if the sensors provide a TCP- or UDP- based API.

9. **Do features in this specification enable new script execution/loading mechanisms?**

    No.

10. **Do features in this specification allow an origin to access other devices?**

    Yes -- if they speak TCP or UDP.

11. **Do features in this specification allow an origin some measure of control over a user agent’s native UI?**

    No.

12. **What temporary identifiers do the features in this specification create or expose to the web?**

    None.

13. **How does this specification distinguish between behavior in first-party and third-party contexts?**

    By default, permissions policy prevents use from third-party contexts.

14. **How do the features in this specification work in the context of a browser’s Private Browsing or Incognito mode?**

    Not applicable -- this API is intended to be used in Isolated Web Apps (which do not support either mode).

15. **Does this specification have both "Security Considerations" and "Privacy Considerations" sections?**

    The specification has an extensive Security Considerations section. It includes various mitigations.

    The specification has a Privacy Considerations section.

16. **Do features in your specification enable origins to downgrade default security protections?**

    Yes -- for instance, the feature is not subject to CORS restrictions.

17. **How does your feature handle non-"fully active" documents?**

    The existing socket connections will be invalidated.

18. **What should this questionnaire have asked?**

    N/A.
