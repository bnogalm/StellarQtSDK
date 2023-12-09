#ifndef NETWORK_H
#define NETWORK_H
#include <QString>
#include "util.h"
#include <exception>
#include <QMap>

/**
 * Indicates that no network was selected.
 */
class NoNetworkSelectedException :public std::exception {
    public:
    ~NoNetworkSelectedException() throw(){

    }

    const char *what() const throw ()
    {
        return "No network selected. Use `Network.use`, `Network.usePublicNetwork` or `Network.useTestNetwork` helper methods to select network.";
    }
};

/**
 * Network class is used to specify which Stellar network you want to use.
 * Each network has a <code>networkPassphrase</code> which is hashed to
 * every transaction id.
 * There is no default network. You need to specify network when initializing your app by calling
 * {@link Network#use(Network)}, {@link Network#usePublicNetwork()} or {@link Network#useTestNetwork()}.
 */

class Network
{
    static const QString PUBLIC_S;
    static const QString TESTNET_S;
    static Network *s_current;
    static QMap<QString, Network*> s_usedNetworks;
    QString m_networkPassphrase;


    friend Network* checkNotNull(Network* network, const char *error);
    /**
     * Creates a new Network object to represent a network with a given passphrase
     * @param networkPassphrase
     */
    Network(QString networkPassphrase);
public:


    /**
     * Returns network passphrase
     */
    QString getNetworkPassphrase();

    /**
     * Returns network id (SHA-256 hashed <code>networkPassphrase</code>).
     */
    QByteArray getNetworkId();

    /**
     * Returns currently used Network object.
     */
    static Network *current();

    /**
     * Use <code>network</code> as a current network.
     * @param network Network object to set as current network
     */
    static void use(Network *network);

    /**
     * Use Stellar Public Network as default network
     */
    static void usePublicNetwork();

    /**
     * Use Stellar Test Network as default network
     */
    static void useTestNetwork();

    ///
    /// \brief PUBLIC
    /// to keep same API than other SDKs
    /// \return
    ///
    static Network* PUBLIC();
    ///
    /// \brief TESTNET
    /// to keep same API than other SDKs
    /// \return
    ///
    static Network* TESTNET();
};

Network* checkNotNull(Network* network, const char *error);


#endif // NETWORK_H
