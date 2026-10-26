#include "sep7origincheck.h"
#include "sep7.h"
#include "stellartomlresolver.h"

#include <stdexcept>

QSTELLAR_BEGIN_NS

Sep7OriginCheck* Sep7OriginCheck::verify(const QString& uri, QNetworkAccessManager* network, int timeoutMs)
{
    const QString origin = Sep7::originDomain(uri);
    if (origin.isEmpty())
        throw std::runtime_error("Sep7: the request names no origin_domain");
    // Refused before a request is made, as verifyOrigin() would refuse it.
    if (!Sep7::isFullyQualifiedDomainName(origin))
        throw std::runtime_error("Sep7: the origin_domain is not a fully qualified domain name");
    StellarTomlResolver* resolver = StellarTomlResolver::resolve(origin, network, timeoutMs);
    auto* check = new Sep7OriginCheck();
    // The check's: deleting it cancels the request.
    resolver->setParent(check);
    connect(resolver, &StellarTomlResolver::resolved, check, [check, uri](const QVariantMap& toml) {
        check->deleteLater();
        QString key;
        try {
            key = Sep7::verifyOrigin(uri, toml);
        } catch (const std::exception& e) {
            emit check->failed(QString::fromUtf8(e.what()));
            return;
        }
        emit check->verified(key);
    });
    connect(resolver, &StellarTomlResolver::failed, check, [check](const QString& reason) {
        check->deleteLater();
        emit check->failed(QStringLiteral("Sep7: the origin_domain's ") + reason.mid(reason.indexOf(QLatin1String("stellar.toml"))));
    });
    return check;
}

QSTELLAR_END_NS
