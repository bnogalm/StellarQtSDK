#ifndef REVOKESPONSORSHIPOPERATIONRESPONSE_H
#define REVOKESPONSORSHIPOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
/**
 * Represents RevokeSponsorship operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class RevokeSponsorshipOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString account_id READ getAccountID WRITE setAccountID NOTIFY accountIDChanged)
    Q_PROPERTY(QString claimable_balance_id READ getClaimableBalanceID WRITE setClaimableBalanceID NOTIFY claimableBalanceIDChanged)
    Q_PROPERTY(QString data_account_id READ getDataAccountID WRITE setDataAccountID NOTIFY dataAccountIDChanged)
    Q_PROPERTY(QString data_name READ getDataName WRITE setDataName NOTIFY dataNameChanged)
    Q_PROPERTY(QString offer_id READ getOfferID WRITE setOfferID NOTIFY offerIDChanged)
    Q_PROPERTY(QString trustline_account_id READ getTrustlineAccountID WRITE setTrustlineAccountID NOTIFY trustlineAccountIDChanged)
    Q_PROPERTY(QString trustline_asset READ getTrustlineAsset WRITE setTrustlineAsset NOTIFY trustlineAssetChanged)
    Q_PROPERTY(QString signer_account_id READ getSignerAccountID WRITE setSignerAccountID NOTIFY signerAccountIDChanged)
    Q_PROPERTY(QString signer_key READ getSignerKey WRITE setSignerKey NOTIFY signerKeyChanged)

    QString m_accountID;
    QString m_claimableBalanceID;
    QString m_dataAccountID;
    QString m_dataName;
    QString m_offerID;
    QString m_trustlineAccountID;
    QString m_trustlineAsset;
    QString m_signerAccountID;
    QString m_signerKey;

public:
    RevokeSponsorshipOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~RevokeSponsorshipOperationResponse();
    QString getAccountID() const;
    QString getClaimableBalanceID() const;
    QString getDataAccountID() const;
    QString getDataName() const;
    QString getOfferID() const;
    QString getTrustlineAccountID() const;
    QString getTrustlineAsset() const;
    QString getSignerAccountID() const;
    QString getSignerKey() const;

public slots:
    void setAccountID(QString accountID);
    void setClaimableBalanceID(QString claimableBalanceID);
    void setDataAccountID(QString dataAccountID);
    void setDataName(QString dataName);
    void setOfferID(QString offerID);
    void setTrustlineAccountID(QString trustlineAccountID);
    void setTrustlineAsset(QString trustlineAsset);
    void setSignerAccountID(QString signerAccountID);
    void setSignerKey(QString signerKey);

signals:
    void accountIDChanged();
    void claimableBalanceIDChanged();
    void dataAccountIDChanged();
    void dataNameChanged();
    void offerIDChanged();
    void trustlineAccountIDChanged();
    void trustlineAssetChanged();
    void signerAccountIDChanged();
    void signerKeyChanged();
};

#endif // REVOKESPONSORSHIPOPERATIONRESPONSE_H
