#include "revokesponsorshipoperationresponse.h"

RevokeSponsorshipOperationResponse::RevokeSponsorshipOperationResponse(QNetworkReply* reply)
    :OperationResponse(reply)
{

}

RevokeSponsorshipOperationResponse::~RevokeSponsorshipOperationResponse()
{

}

QString RevokeSponsorshipOperationResponse::getAccountID() const
{
    return m_accountID;
}

QString RevokeSponsorshipOperationResponse::getClaimableBalanceID() const
{
    return m_claimableBalanceID;
}

QString RevokeSponsorshipOperationResponse::getDataAccountID() const
{
    return m_dataAccountID;
}

QString RevokeSponsorshipOperationResponse::getDataName() const
{
    return m_dataName;
}

QString RevokeSponsorshipOperationResponse::getOfferID() const
{
    return m_offerID;
}

QString RevokeSponsorshipOperationResponse::getTrustlineAccountID() const
{
    return m_trustlineAccountID;
}

QString RevokeSponsorshipOperationResponse::getTrustlineAsset() const
{
    return m_trustlineAsset;
}

QString RevokeSponsorshipOperationResponse::getSignerAccountID() const
{
    return m_signerAccountID;
}

QString RevokeSponsorshipOperationResponse::getSignerKey() const
{
    return m_signerKey;
}

void RevokeSponsorshipOperationResponse::setAccountID(QString accountID)
{
    if (m_accountID == accountID)
        return;

    m_accountID = accountID;
    emit accountIDChanged();
}

void RevokeSponsorshipOperationResponse::setClaimableBalanceID(QString claimableBalanceID)
{
    if (m_claimableBalanceID == claimableBalanceID)
        return;

    m_claimableBalanceID = claimableBalanceID;
    emit claimableBalanceIDChanged();
}

void RevokeSponsorshipOperationResponse::setDataAccountID(QString dataAccountID)
{
    if (m_dataAccountID == dataAccountID)
        return;

    m_dataAccountID = dataAccountID;
    emit dataAccountIDChanged();
}

void RevokeSponsorshipOperationResponse::setDataName(QString dataName)
{
    if (m_dataName == dataName)
        return;

    m_dataName = dataName;
    emit dataNameChanged();
}

void RevokeSponsorshipOperationResponse::setOfferID(QString offerID)
{
    if (m_offerID == offerID)
        return;

    m_offerID = offerID;
    emit offerIDChanged();
}

void RevokeSponsorshipOperationResponse::setTrustlineAccountID(QString trustlineAccountID)
{
    if (m_trustlineAccountID == trustlineAccountID)
        return;

    m_trustlineAccountID = trustlineAccountID;
    emit trustlineAccountIDChanged();
}

void RevokeSponsorshipOperationResponse::setTrustlineAsset(QString trustlineAsset)
{
    if (m_trustlineAsset == trustlineAsset)
        return;

    m_trustlineAsset = trustlineAsset;
    emit trustlineAssetChanged();
}

void RevokeSponsorshipOperationResponse::setSignerAccountID(QString signerAccountID)
{
    if (m_signerAccountID == signerAccountID)
        return;

    m_signerAccountID = signerAccountID;
    emit signerAccountIDChanged();
}

void RevokeSponsorshipOperationResponse::setSignerKey(QString signerKey)
{
    if (m_signerKey == signerKey)
        return;

    m_signerKey = signerKey;
    emit signerKeyChanged();
}
