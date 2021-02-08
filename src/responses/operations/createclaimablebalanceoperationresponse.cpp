#include "createclaimablebalanceoperationresponse.h"

CreateClaimableBalanceOperationResponse::CreateClaimableBalanceOperationResponse(QNetworkReply* reply)
    :OperationResponse(reply),m_asset(nullptr)
{

}

CreateClaimableBalanceOperationResponse::~CreateClaimableBalanceOperationResponse()
{
    if(m_asset)
        delete m_asset;
}

QString CreateClaimableBalanceOperationResponse::asset() const
{
    return m_assetString;
}

Asset *CreateClaimableBalanceOperationResponse::getAsset()
{
    if(!m_asset)
        m_asset = Asset::create(m_assetString);
    return m_asset;
}

QString CreateClaimableBalanceOperationResponse::getAmount() const
{
    return m_amount;
}

QList<Claimant> CreateClaimableBalanceOperationResponse::getClaimants() const
{
    return m_claimants;
}

void CreateClaimableBalanceOperationResponse::setAsset(QString assetString)
{
    if (m_assetString == assetString)
        return;
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetString = assetString;
    emit assetChanged();
}

void CreateClaimableBalanceOperationResponse::setAmount(QString amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    emit amountChanged();
}

void CreateClaimableBalanceOperationResponse::setClaimants(QList<Claimant> claimants)
{
    if (m_claimants == claimants)
        return;

    m_claimants = claimants;
    emit claimantsChanged();
}
