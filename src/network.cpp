#include "network.h"

const QString Network::PUBLIC = "Public Global Stellar Network ; September 2015";
const QString Network::TESTNET = "Test SDF Network ; September 2015";

Network* Network::s_current = 0;

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
    if(s_current)
        delete s_current;
    s_current = network;
}

void Network::usePublicNetwork() {
    Network::use(new Network(PUBLIC));
}

void Network::useTestNetwork() {
    Network::use(new Network(TESTNET));
}

Network* checkNotNull(Network* network, const char *error)
{
    if(!network|| network->m_networkPassphrase.isEmpty()){
        throw std::runtime_error(error);
    }
    return network;
}
