#include "stellartomlresolver.h"
#include "stellartoml.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThreadStorage>
#include <QUrl>
#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

const char* const TooLarge = "qstellarTomlTooLarge";

// The domain as js reads it: surrounding whitespace and a trailing '/' (or
// '\') are not part of it.
QString hostOf(const QString& domain)
{
    QString name = domain.trimmed();
    while (name.endsWith(QLatin1Char('/')) || name.endsWith(QLatin1Char('\\')))
        name.chop(1);
    return name;
}

} // namespace

QNetworkAccessManager& StellarTomlResolver::defaultNetworkAccessManager()
{
    // One per thread. There was one for the process, parented to qApp and
    // made by whichever thread asked first: made from a worker thread (where
    // qApp cannot be a parent) it served the GUI thread's requests from the
    // wrong thread, and none at all once the worker had ended.
    static QThreadStorage<QNetworkAccessManager*> networks;
    if (!networks.hasLocalData())
        networks.setLocalData(new QNetworkAccessManager());
    return *networks.localData();
}

bool StellarTomlResolver::isHostName(const QString& domain)
{
    // An optional port, as js and py take one ("anchor.example:8443"): 1 to
    // 65535, in digits only. It was refused with the rest of ':'.
    QString host = domain;
    const auto colon = domain.lastIndexOf(QLatin1Char(':'));
    if (colon >= 0) {
        const QString port = domain.mid(colon + 1);
        if (port.isEmpty() || port.size() > 5)
            return false;
        for (const QChar c : port) {
            if (c < QLatin1Char('0') || c > QLatin1Char('9'))
                return false;
        }
        const int number = port.toInt();
        if (number < 1 || number > 65535)
            return false;
        host = domain.left(colon);
    }
    if (host.isEmpty())
        return false;
    // No user, path, query or fragment: "good.example@evil.example" would be
    // fetched from evil.example. No IPv6 literal either ('[', ']').
    for (const QChar c : host) {
        if (c.isSpace() || QStringLiteral("/\\?#@:%[]").contains(c))
            return false;
    }
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(host, QUrl::StrictMode);
    return url.isValid() && !url.host().isEmpty();
}

QNetworkReply* StellarTomlResolver::request(const QString& domain, QNetworkAccessManager& network,
                                            int timeoutMs)
{
    const QString name = hostOf(domain);
    if (!isHostName(name))
        throw std::runtime_error("a stellar.toml is asked for by a domain name, with an optional port, "
                                 "and nothing else (no scheme, path or user)");
    QNetworkRequest request(QUrl(QStringLiteral("https://%1/.well-known/stellar.toml").arg(name)));
    // A redirect is not followed, as js does not by default: the toml is the
    // domain's own word, read from the domain itself.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    // A host that accepted the connection and never answered is given up.
    request.setTransferTimeout(timeoutMs > 0 ? timeoutMs : STELLAR_QT_REPLY_TIMEOUT);
    QNetworkReply* reply = network.get(request);
    // Stopped as soon as more than MaxSize is announced or has arrived, not
    // read whole first: a host could otherwise send without end.
    QObject::connect(reply, &QNetworkReply::downloadProgress, reply, [reply](qint64 received, qint64 total) {
        if ((received > MaxSize || total > MaxSize) && !reply->property(TooLarge).toBool()) {
            reply->setProperty(TooLarge, true);
            reply->abort();
        }
    });
    return reply;
}

QVariantMap StellarTomlResolver::read(QNetworkReply* reply)
{
    if (reply->property(TooLarge).toBool())
        throw std::runtime_error("the stellar.toml is larger than 100 KB");
    if (reply->error() != QNetworkReply::NoError)
        throw std::runtime_error(("the stellar.toml could not be read: " + reply->errorString()).toStdString());
    const QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (status.isValid() && (status.toInt() < 200 || status.toInt() > 299)) {
        // A 3xx included: the redirect is not followed.
        throw std::runtime_error(QStringLiteral("the stellar.toml could not be read: HTTP %1")
                                     .arg(status.toInt()).toStdString());
    }
    const QByteArray data = reply->read(MaxSize + 1);
    if (data.size() > MaxSize)
        throw std::runtime_error("the stellar.toml is larger than 100 KB");
    try {
        return StellarToml::parse(QString::fromUtf8(data));
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("the stellar.toml is not valid TOML: ") + e.what());
    }
}

StellarTomlResolver* StellarTomlResolver::resolve(const QString& domain, QNetworkAccessManager* network,
                                                  int timeoutMs)
{
    QNetworkReply* reply = request(domain, network ? *network : defaultNetworkAccessManager(), timeoutMs);
    auto* resolver = new StellarTomlResolver(hostOf(domain));
    // The resolver's: deleting it deletes the reply, which aborts it.
    reply->setParent(resolver);
    QObject::connect(reply, &QNetworkReply::finished, resolver, [resolver, reply]() { resolver->finish(reply); });
    return resolver;
}

void StellarTomlResolver::finish(QNetworkReply* reply)
{
    deleteLater();
    QVariantMap toml;
    try {
        toml = read(reply);
    } catch (const std::exception& e) {
        emit failed(QString::fromUtf8(e.what()));
        return;
    }
    emit resolved(toml);
}

QSTELLAR_END_NS
