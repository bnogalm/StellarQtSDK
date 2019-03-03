#include "rootresponse.h"

RootResponse::RootResponse(QNetworkReply *reply):Response(reply)
  ,m_historyLatestLedger(0),m_historyElderLedger(0),m_coreLatestLedger(0),m_protocolVersion(0)
  ,m_currentProtocolVersion(0)
  ,m_coreSupportedProtocolVersion(0)
{

}

RootResponse::~RootResponse()
{

}

QString RootResponse::getHorizonVersion() const{
    return m_horizonVersion;
}

QString RootResponse::getStellarCoreVersion() const{
    return m_coreVersion;
}

qint32 RootResponse::getHistoryLatestLedger() const{
    return m_historyLatestLedger;
}

qint32 RootResponse::getHistoryElderLedger() const{
    return m_historyElderLedger;
}

qint32 RootResponse::getCoreLatestLedger() const{
    return m_coreLatestLedger;
}

QString RootResponse::getNetworkPassphrase() const{
    return m_networkPassphrase;
}

qint32 RootResponse::getProtocolVersion() const{
    return m_protocolVersion;
}

qint32 RootResponse::getCurrentProtocolVersion() const
{
    return m_currentProtocolVersion;
}

qint32 RootResponse::getCoreSupportedProtocolVersion() const
{
    return m_coreSupportedProtocolVersion;
}
