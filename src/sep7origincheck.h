#ifndef SEP7ORIGINCHECK_H
#define SEP7ORIGINCHECK_H

#include <QObject>
#include <QString>

#include "qstellar_namespace.h"

class QNetworkAccessManager;

QSTELLAR_BEGIN_NS

/**
 * SEP-7's origin check, network included: reads the stellar.toml of the
 * request's origin_domain (StellarTomlResolver: https, no redirect, a
 * timeout, 100 KB at most) and checks the request against it with
 * Sep7::verifyOrigin().
 *
 * @code
 * auto* check = Sep7OriginCheck::verify(uri);
 * QObject::connect(check, &Sep7OriginCheck::verified, [](const QString& key) { ... });
 * QObject::connect(check, &Sep7OriginCheck::failed, [](const QString& why) { ... });
 * @endcode
 *
 * It deletes itself once it has emitted one of its signals; deleting it
 * earlier cancels it, and neither comes.
 */
class Sep7OriginCheck : public QObject
{
    Q_OBJECT
public:
    /**
     * Starts the check of `uri`. Throws std::runtime_error at once, before
     * any request, for what is not a `pay` or `tx` request, one that names
     * no origin_domain, or an origin_domain that is not a fully qualified
     * domain name (Sep7::isFullyQualifiedDomainName: not an IP address, not
     * a single label such as "localhost").
     */
    static Sep7OriginCheck* verify(const QString& uri, QNetworkAccessManager* network = nullptr,
                                   int timeoutMs = STELLAR_QT_REPLY_TIMEOUT);

signals:
    /**
     * The request is signed by its origin's URI_REQUEST_SIGNING_KEY,
     * `signingKey`. The origin to show is Sep7::originDomain(uri), in its
     * ACE form.
     */
    void verified(const QString& signingKey);
    void failed(const QString& reason);

private:
    Sep7OriginCheck() = default;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep7OriginCheck)

#endif // SEP7ORIGINCHECK_H
