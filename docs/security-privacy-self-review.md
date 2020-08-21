

Responses to the [Self-Review Questionnaire: Security and Privacy](https://w3ctag.github.io/security-questionnaire/) for the [Raw Sockets API](https://github.com/WICG/raw-sockets/)




## 2.1 What information might this feature expose to Web sites or other parties, and for what purposes is that exposure necessary?

When the user agrees to establish a socket connection with an information system, and the connection succeeds, that system's IP address and port, and any information it sends to the user agent, becomes available to the site.

The destination system also receives the user's IP address and port, and any information the web app sends.


## 2.2 Is this specification exposing the minimum amount of information necessary to power the feature?

The specification provides no more information than is needed for communication with arbitrary systems directly over TCP and UDP.

Implementors will want to carefully consider what information is revealed in the error message when connection attempts are rejected.


## 2.3 How does this specification deal with personal information or personally-identifiable information or information derived thereof?

This specification deals with generic information.

This specification allows devices to use their existing protocols for communication. These may or may not be secure. We intend to [facilitate](https://github.com/WICG/raw-sockets/blob/master/docs/explainer.md#mitigation-7) use of TLS.


## 2.4 How does this specification deal with sensitive information?

See answer 2.3


## 2.5 Does this specification introduce new state for an origin that persists across browsing sessions?

A user agent may (at the user's direction) remember specific destinations that an origin may connect to, without recurring consent dialogs.

This is by design, to avoid consent fatigue.


## 2.6 What information from the underlying platform, e.g. configuration data, is exposed by this specification to an origin?

The origin will be able to make deductions from the presence or absence of the API. For example, a user agent might decide to support the API on specific platforms only.


## 2.7 Does this specification allow an origin access to sensors on a user’s device

A use case would be connect to sensors that provide a TCP or UDP API.

The user would first need to enter the IP address in a consent dialog. (The port would typically be pre-populated by the web app, and would either be shown numerically in a dialog, or could be described in words if the port is well known.)


## 2.8 What data does this specification expose to an origin? Please also document what data is identical to data exposed by other features, in the same or different contexts.

If the user agrees to establish a socket connection with an information system, that system's IP address and any information it sends to the user agent becomes available to the origin.


## 2.9 Does this specification enable new script execution/loading mechanisms?

No.


## 2.10 Does this specification allow an origin to access other devices?

This API allows access to any network device that the user agrees to establish communication with.


## 2.11 Does this specification allow an origin some measure of control over a user agent’s native UI?

No


## 2.12 What temporary identifiers might this this specification create or expose to the web?

When communication has been established, the destination address could be used as an identifier.


## 2.13 How does this specification distinguish between behavior in first-party and third-party contexts?

By default, permissions policy prevents use from third-party contexts.


## 2.14 How does this specification work in the context of a user agent’s Private Browsing or "incognito" mode?

A user agent should not record permitted destinations if the API is used in Private Browsing modes.


## 2.15 Does this specification have a "Security Considerations" and "Privacy Considerations" section?

The specification has an extensive Security Considerations section. It includes various mitigations.

The specification has a Privacy Considerations section.


## 2.16 Does this specification allow downgrading default security characteristics?

A site could use this API to talk over HTTP (or other non-secure protocol) to its own server from a page that was loaded over HTTPS.


## 2.17 What should this questionnaire have asked?


