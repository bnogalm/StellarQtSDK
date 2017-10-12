#ifndef NETWORK_H
#define NETWORK_H
#include <QString>
#include "util.h"
#include <exception>

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
    static const QString PUBLIC;
    static const QString TESTNET;
    static Network *s_current;
    QString m_networkPassphrase;
public:

    /**
     * Creates a new Network object to represent a network with a given passphrase
     * @param networkPassphrase
     */
    Network(QString networkPassphrase);
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
     * Use Stellar Public Network
     */
    static void usePublicNetwork();

    /**
     * Use Stellar Test Network.
     */
    static void useTestNetwork();
};

#endif // NETWORK_H
