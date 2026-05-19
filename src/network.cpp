#include "network.h"

// Silence self-use: the deprecated legacy globals call each other.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

const QString Network::PUBLIC_S = "Public Global Stellar Network ; September 2015";
const QString Network::TESTNET_S = "Test SDF Network ; September 2015";

Network* Network::s_current = 0;
QMap<QString, Network*> Network::s_usedNetworks;

Network::Network(QString networkPassphrase)
{
    m_networkPassphrase = checkNotNull(networkPassphrase, "networkPassphrase cannot be null");
}

const QString& Network::PUBLIC_NETWORK_PASSPHRASE()  { return PUBLIC_S; }
const QString& Network::TESTNET_NETWORK_PASSPHRASE() { return TESTNET_S; }

QString Network::getNetworkPassphrase() const {
    return m_networkPassphrase;
}

QByteArray Network::getNetworkId() const {
    // FIX §6.1: hash this instance's passphrase, not the singleton's.
    return Util::hash(m_networkPassphrase.toLatin1());
}

bool Network::isPublicNetwork() const  { return m_networkPassphrase == PUBLIC_S; }
bool Network::isTestNetwork()   const  { return m_networkPassphrase == TESTNET_S; }

Network* Network::publicNetwork()
{
    if (auto p = s_usedNetworks.value(PUBLIC_S))
        return p;
    auto net = new Network(PUBLIC_S);
    s_usedNetworks.insert(PUBLIC_S, net);
    return net;
}

Network* Network::testnetNetwork()
{
    if (auto p = s_usedNetworks.value(TESTNET_S))
        return p;
    auto net = new Network(TESTNET_S);
    s_usedNetworks.insert(TESTNET_S, net);
    return net;
}

// -------- Legacy deprecated API (kept for source-compat through 0.x) -------

Network* Network::current() {
    if (!s_current)
        throw std::runtime_error("Not selected network");
    return s_current;
}

void Network::use(Network* network) {
    s_current = network;
}

void Network::usePublicNetwork() {
    use(publicNetwork());
}

void Network::useTestNetwork() {
    use(testnetNetwork());
}

Network* Network::PUBLIC()  { return publicNetwork(); }
Network* Network::TESTNET() { return testnetNetwork(); }

Network* checkNotNull(Network* network, const char *error)
{
    if (!network || network->m_networkPassphrase.isEmpty()) {
        throw std::runtime_error(error);
    }
    return network;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif
