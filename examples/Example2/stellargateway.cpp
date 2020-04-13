#include "stellargateway.h"
#include "network.h"

StellarGateway::StellarGateway(QObject *parent) : QObject(parent)
  , m_server(new Server("https://horizon-testnet.stellar.org"))
{
    Network::useTestNetwork();//our example only works on test network
}

Server *StellarGateway::server() const
{
    return m_server;
}

