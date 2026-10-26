#ifndef STELLARTOMLRESOLVER_H
#define STELLARTOMLRESOLVER_H

#include <QObject>
#include <QString>
#include <QVariantMap>

#include "qstellar_namespace.h"

class QNetworkAccessManager;
class QNetworkReply;

QSTELLAR_BEGIN_NS

/**
 * SEP-1 — fetches and reads a domain's `stellar.toml`, as js's
 * `StellarToml.Resolver.resolve` does:
 * `https://<domain>/.well-known/stellar.toml`, over https only, with no
 * redirect followed (js's default), given up when the host sends nothing
 * for a while (the timeout), and refused when it is larger than 100 KB
 * (js's `STELLAR_TOML_MAX_SIZE`). py's `fetch_stellar_toml` is not the
 * same: it follows redirects, reads up to 512 KB, and refuses a 404 only
 * (another error page is parsed as TOML).
 *
 * @code
 * auto* toml = StellarTomlResolver::resolve("example.com");
 * QObject::connect(toml, &StellarTomlResolver::resolved, [](const QVariantMap& t) {
 *     qDebug() << t.value("SIGNING_KEY");
 * });
 * QObject::connect(toml, &StellarTomlResolver::failed, [](const QString& why) { qWarning() << why; });
 * @endcode
 *
 * The resolver deletes itself once it has emitted one of its signals.
 * Deleting it earlier cancels the request, and neither signal comes.
 */
class StellarTomlResolver : public QObject
{
    Q_OBJECT
public:
    /** The largest stellar.toml read, in bytes (js: STELLAR_TOML_MAX_SIZE). */
    static constexpr qint64 MaxSize = 100 * 1024;

    /**
     * Starts reading `domain`'s stellar.toml. `domain` is a host name, with
     * an optional port as js and py take one ("anchor.example:8443"), and
     * nothing else (no scheme, path or user); surrounding whitespace and a
     * trailing '/' are dropped, as js drops them. Anything else throws
     * std::runtime_error, since it is pasted into the URL and
     * "good.example@evil.example" would be fetched from evil.example.
     * `network` is the manager to use (one of the SDK's own when null);
     * `timeoutMs` is how long the host may send nothing before the request
     * is given up (QNetworkRequest::setTransferTimeout): a host that keeps
     * sending is read until it ends or passes MaxSize, so the size bounds
     * the wait, not the clock. js's timeout is a deadline for the whole
     * answer.
     */
    static StellarTomlResolver* resolve(const QString& domain,
                                        QNetworkAccessManager* network = nullptr,
                                        int timeoutMs = STELLAR_QT_REPLY_TIMEOUT);

    /**
     * The request resolve() makes, for a caller that reads the reply itself:
     * https, no redirect, the timeout (for a host sending nothing), and
     * aborted once more than MaxSize bytes are on their way. Read it with
     * read().
     */
    static QNetworkReply* request(const QString& domain, QNetworkAccessManager& network,
                                  int timeoutMs = STELLAR_QT_REPLY_TIMEOUT);

    /**
     * The toml of a finished reply. Throws std::runtime_error saying why
     * there is none: the request failed (a redirect is not followed), the
     * answer is not HTTP 2xx, it is larger than MaxSize, or it is not TOML.
     */
    static QVariantMap read(QNetworkReply* reply);

    /**
     * True when `domain` is a host name, with an optional port (1 to 65535),
     * and nothing else; nothing is trimmed. An IPv4 address and an IDN name
     * are host names, as py and js's StellarToml.Resolver take them (js's
     * federation check refuses them, and every label that starts with a
     * digit, such as 3m.com); an IPv6 literal is not.
     */
    static bool isHostName(const QString& domain);

    /**
     * The manager resolve() uses when given none, and FederationServer
     * always: the calling thread's own. A QNetworkAccessManager belongs to
     * the thread it was made in and is not thread-safe, so each thread gets
     * one, made on its first call and deleted when the thread ends (the main
     * thread's when the application object is). Call it, and resolve()
     * without a manager, from a thread with an event loop.
     */
    static QNetworkAccessManager& defaultNetworkAccessManager();

    QString domain() const { return m_domain; }

signals:
    void resolved(const QVariantMap& toml);
    void failed(const QString& reason);

private:
    explicit StellarTomlResolver(const QString& domain) : m_domain(domain) {}
    void finish(QNetworkReply* reply);

    QString m_domain;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(StellarTomlResolver)

#endif // STELLARTOMLRESOLVER_H
