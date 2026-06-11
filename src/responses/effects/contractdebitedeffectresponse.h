#ifndef CONTRACTDEBITEDEFFECTRESPONSE_H
#define CONTRACTDEBITEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Asset;/**
 * Represents contract_debited effect response (Soroban: a contract was debited an asset).
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ContractDebitedEffectResponse : public EffectResponse
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
    ContractDebitedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~ContractDebitedEffectResponse();
    /** C-strkey of the contract that was debited. */
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

QSTELLAR_ALIAS(ContractDebitedEffectResponse)



Q_DECLARE_METATYPE(qstellar::ContractDebitedEffectResponse*)
#endif // CONTRACTDEBITEDEFFECTRESPONSE_H
