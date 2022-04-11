#include "clawbackclaimablebalanceoperationresponse.h"

ClawbackClaimableBalanceOperationResponse::ClawbackClaimableBalanceOperationResponse(QNetworkReply *reply):OperationResponse(reply)
{

}

ClawbackClaimableBalanceOperationResponse::~ClawbackClaimableBalanceOperationResponse()
{

}

const QString ClawbackClaimableBalanceOperationResponse::getBalanceID() const
{
    return m_balanceId;
}

void ClawbackClaimableBalanceOperationResponse::setBalanceID(const QString newBalance_id)
{
    if (m_balanceId == newBalance_id)
        return;
    m_balanceId = newBalance_id;
    emit balanceIDChanged();
}
