#ifndef ACCOUNTCREDITEDEFFECTRESPONSE_H
#define ACCOUNTCREDITEDEFFECTRESPONSE_H

#include "effectresponse.h"

class Asset;
/**
 * Represents account_credited effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountCreditedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)

    QString m_amount;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    Asset* m_asset;
public:
    AccountCreditedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~AccountCreditedEffectResponse();
    QString getAmount() const;

    Asset* getAsset();
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;

public slots:
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};
Q_DECLARE_METATYPE(AccountCreditedEffectResponse*)
#endif // ACCOUNTCREDITEDEFFECTRESPONSE_H
