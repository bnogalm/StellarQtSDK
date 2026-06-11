#ifndef CONTRACTCREDITEDEFFECTRESPONSE_H
#define CONTRACTCREDITEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Asset;/**
 * Represents contract_credited effect response (Soroban: a contract received an asset).
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ContractCreditedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString contract MEMBER m_contract)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)

    QString m_contract;
    QString m_amount;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    Asset* m_asset;
public:
    ContractCreditedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~ContractCreditedEffectResponse();
    /** C-strkey of the contract that was credited. */
    QString getContract() const { return m_contract; }
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(ContractCreditedEffectResponse)



Q_DECLARE_METATYPE(qstellar::ContractCreditedEffectResponse*)
#endif // CONTRACTCREDITEDEFFECTRESPONSE_H
