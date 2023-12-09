#include "network.h"

const QString Network::PUBLIC_S = "Public Global Stellar Network ; September 2015";
const QString Network::TESTNET_S = "Test SDF Network ; September 2015";

Network* Network::s_current = 0;

QMap<QString, Network*> Network::s_usedNetworks;

Network::Network(QString networkPassphrase)
{
    m_networkPassphrase = checkNotNull(networkPassphrase, "networkPassphrase cannot be null");
}

QString Network::getNetworkPassphrase() {
    return m_networkPassphrase;
}

QByteArray Network::getNetworkId() {
    return Util::hash(s_current->getNetworkPassphrase().toLatin1());
}

Network* Network::current() {
    if(!s_current)
        throw std::runtime_error("Not selected network");
    return s_current;
}

void Network::use(Network* network) {
    s_current = network;
}

void Network::usePublicNetwork() {
    Network::use(Network::PUBLIC());
}

void Network::useTestNetwork() {
    Network::use(Network::TESTNET());
}

Network *Network::PUBLIC()
{
    if(auto publicNetwork = s_usedNetworks.value(PUBLIC_S))
    {
        return publicNetwork;
    }
    auto net = new Network(PUBLIC_S);
    s_usedNetworks.insert(PUBLIC_S, net);
    return net;
}

Network *Network::TESTNET()
{
    if(auto testNetwork = s_usedNetworks.value(TESTNET_S))
    {
        return testNetwork;
    }
    auto net = new Network(TESTNET_S);
    s_usedNetworks.insert(TESTNET_S, net);
    return net;
}

Network* checkNotNull(Network* network, const char *error)
{
    if(!network|| network->m_networkPassphrase.isEmpty()){
        throw std::runtime_error(error);
    }
    return network;
}
