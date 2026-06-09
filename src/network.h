#ifndef NETWORK_H
#define NETWORK_H
#include <QString>
#include "util.h"
#include <exception>
#include <QMap>
#include "stellardeprecated.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Indicates that no network was selected.
 *
 * @deprecated Only thrown by the legacy `Network::current()` global. New code
 * should always pass `Network*` explicitly and never relies on global state.
 */
class NoNetworkSelectedException : public std::exception {
public:
    ~NoNetworkSelectedException() throw(){}
    const char *what() const throw () {
        return "No network selected. Use `Network.use`, `Network.usePublicNetwork` or `Network.useTestNetwork` helper methods to select network.";
    }
};

/**
 * Network class is used to specify which Stellar network you want to use.
 * Each network has a <code>networkPassphrase</code> which is hashed into
 * every transaction id.
 *
 * Starting with 0.4.0 the public API is:
 *
 *   Network testnet = Network(Network::TESTNET_NETWORK_PASSPHRASE);
 *   // or, equivalent:
 *   Network* testnet = Network::testnetNetwork();
 *
 * The legacy global API (`Network::current()`, `Network::use(...)`,
 * `usePublicNetwork()`, `useTestNetwork()`, `PUBLIC()`, `TESTNET()`) is
 * deprecated. It will be removed in 1.0.0. Pass `Network*` explicitly to
 * transactions and SDK calls.
 */
class Network
{
    static const QString PUBLIC_S;
    static const QString TESTNET_S;
    static Network *s_current;
    static QMap<QString, Network*> s_usedNetworks;
    QString m_networkPassphrase;

    friend Network* checkNotNull(Network* network, const char *error);

public:
    /** Canonical passphrase for the Stellar public network. */
    static const QString& PUBLIC_NETWORK_PASSPHRASE();
    /** Canonical passphrase for the Stellar test network. */
    static const QString& TESTNET_NETWORK_PASSPHRASE();

    /**
     * Creates a new Network object for a given passphrase.
     *
     * As of 0.4.0 this constructor is **public**. It used to be private and
     * only reachable via the singleton factories below.
     */
    explicit Network(QString networkPassphrase);

    /** Returns network passphrase. */
    QString getNetworkPassphrase() const;

    /**
     * Returns network id (SHA-256 hashed networkPassphrase).
     * FIX §6.1: uses `this->m_networkPassphrase` (not the singleton).
     */
    QByteArray getNetworkId() const;

    /** True if this is the Stellar public network. */
    bool isPublicNetwork() const;
    /** True if this is the Stellar test network. */
    bool isTestNetwork() const;

    /**
     * Returns a Network* for the public network. Does NOT touch the legacy
     * singleton. The pointer is owned by the SDK and lives for the lifetime
     * of the process.
     */
    static Network* publicNetwork();

    /**
     * Returns a Network* for the test network. Does NOT touch the legacy
     * singleton. The pointer is owned by the SDK and lives for the lifetime
     * of the process.
     */
    static Network* testnetNetwork();

    // -------- Legacy global API (deprecated, kept for source-compat) -------

    /**
     * @deprecated Use `Network::publicNetwork()` / `Network::testnetNetwork()`
     * or construct your own Network and pass it explicitly. Removed in 1.0.0.
     */
    STELLAR_DEPRECATED("Use Network* passed explicitly. Removed in 1.0.0")
    static Network *current();

    /**
     * @deprecated Pass `Network*` explicitly to SDK calls. Removed in 1.0.0.
     */
    STELLAR_DEPRECATED("Pass Network* explicitly. Removed in 1.0.0")
    static void use(Network *network);

    /**
     * @deprecated Use `Network::publicNetwork()` (no global state). Removed in 1.0.0.
     */
    STELLAR_DEPRECATED("Use Network::publicNetwork(). Removed in 1.0.0")
    static void usePublicNetwork();

    /**
     * @deprecated Use `Network::testnetNetwork()` (no global state). Removed in 1.0.0.
     */
    STELLAR_DEPRECATED("Use Network::testnetNetwork(). Removed in 1.0.0")
    static void useTestNetwork();

    /**
     * @deprecated Use `Network::publicNetwork()`. Same return value; the
     * legacy name is kept for downstream that copies from other SDKs. Removed
     * in 1.0.0.
     */
    STELLAR_DEPRECATED("Use Network::publicNetwork(). Removed in 1.0.0")
    static Network* PUBLIC();

    /**
     * @deprecated Use `Network::testnetNetwork()`. Removed in 1.0.0.
     */
    STELLAR_DEPRECATED("Use Network::testnetNetwork(). Removed in 1.0.0")
    static Network* TESTNET();
};

Network* checkNotNull(Network* network, const char *error);

QSTELLAR_END_NS

QSTELLAR_ALIAS(NoNetworkSelectedException)
QSTELLAR_ALIAS(Network)
// `checkNotNull(Network*, const char*)` lives in the namespace too; bring it
// to global so legacy calls keep resolving without ADL surprises.
#ifndef STELLAR_QT_LEGACY_GLOBAL_NS
using ::qstellar::checkNotNull;
#endif

#endif // NETWORK_H
