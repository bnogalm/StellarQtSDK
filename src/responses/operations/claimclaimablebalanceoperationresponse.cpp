#include "claimclaimablebalanceoperationresponse.h"

ClaimClaimableBalanceOperationResponse::ClaimClaimableBalanceOperationResponse(QNetworkReply* reply)
    :OperationResponse(reply)
{

}

ClaimClaimableBalanceOperationResponse::~ClaimClaimableBalanceOperationResponse()
{

}

QString ClaimClaimableBalanceOperationResponse::getBalanceID() const
{
    return m_balanceID;
}

QString ClaimClaimableBalanceOperationResponse::getClaimant() const
{
    return m_claimant;
}

void ClaimClaimableBalanceOperationResponse::setBalanceID(QString balanceID)
{
    if (m_balanceID == balanceID)
        return;

    m_balanceID = balanceID;
    emit balanceIDChanged();
}

void ClaimClaimableBalanceOperationResponse::setClaimant(QString claimant)
{
    if (m_claimant == claimant)
        return;

    m_claimant = claimant;
    emit claimantChanged();
}
