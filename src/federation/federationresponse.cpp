#include "federationresponse.h"


FederationResponse::FederationResponse(QNetworkReply *reply):Response(reply) {
}

FederationResponse::FederationResponse(QString stellarAddress, QString accountId, QString memoType, QString memo) {
    m_stellarAddress = stellarAddress;
    m_accountId = accountId;
    m_memoType = memoType;
    m_memo = memo;
}

QString FederationResponse::getStellarAddress() {
    return m_stellarAddress;
}

QString FederationResponse::getAccountId() {
    return m_accountId;
}

QString FederationResponse::getMemoType() {
    return m_memoType;
}

QString FederationResponse::getMemo() {
    return m_memo;
}
