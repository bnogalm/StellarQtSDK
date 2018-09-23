#ifndef FEDERATIONSERVER_H
#define FEDERATIONSERVER_H
#include <QNetworkAccessManager>
#include <exception>
#include <QObject>
#include "federationresponse.h"

/**
 * FederationServer handles a network connection to a
 * <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">federation server</a>
 * instance and exposes an interface for requests to that instance.
 *
 * For resolving a stellar address without knowing which federation server
 * to query use {@link Federation#resolve(String)}.
 *
 * @see <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation docs</a>
 */
class FederationServer : public QObject
{
    Q_OBJECT

    QUrl m_serverUri;
    QString m_domain;
    static QNetworkAccessManager* s_httpClient;
    static QHash<QString,FederationServer*>* s_cachedFederationServers;
    QNetworkReply * buildReply(const QString &address);
    static QNetworkAccessManager &httpClient();


public:
    static bool isCacheActive();

    static void activeCache(bool active);

    static void clearCached();

    /**
     * Creates a new <code>FederationServer</code> instance.
     * @param serverUri Federation Server URI
     * @param domain Domain name this federation server is responsible for
     * @throws FederationServerInvalidException Federation server is invalid (malformed URL, not HTTPS, etc.)
     */
    FederationServer(QUrl serverUri, QString domain);

    /**
     * Creates a new <code>FederationServer</code> instance.
     * @param serverUri Federation Server URI
     * @param domain Domain name this federation server is responsible for
     * @throws FederationServerInvalidException Federation server is invalid (malformed URL, not HTTPS, etc.)
     */
    FederationServer(QString serverUri, QString domain);

    FederationServer();


    /**
     * Creates a <code>FederationServer</code> instance for a given domain.
     * It tries to find a federation server URL in stellar.toml file.
     * @see <a href="https://www.stellar.org/developers/learn/concepts/stellar-toml.html" target="_blank">Stellar.toml docs</a>
     * @param domain Domain to find a federation server for
     *
     * @return FederationServer, you will have to wait until ready signal is emmited or error, as load happends asynchoronous.
     */
    static FederationServer* createForDomain(QString domain);

    bool valid() const;
    /**
     * Resolves a stellar address using a given federation server.
     * @param address Stellar address, like <code>bob*stellar.org</code>
     * @return FederationResponse
     */
    FederationResponse* resolveAddress(const QString& address);

    /**
     * Returns a federation server URI.
     * @return URI
     */
    QUrl getServerUri() {
      return m_serverUri;
    }

    /**
     * Returns a domain this server is responsible for.
     * @return InternetDomainName
     */
    QString getDomain() {
      return m_domain;
    }

private slots:
    void initialize();

signals:
    void ready();
    void error(QString reason);
};

#endif // FEDERATIONSERVER_H

