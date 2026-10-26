#ifndef CHANGETRUSTOPERATIONRESPONSE_H
#define CHANGETRUSTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

class Asset;

/**
 * Represents ChangeTrust operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ChangeTrustOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString trustor READ trustor WRITE setTrustor)
    // The muxed (M...) form and id Horizon adds when the account was
    // given muxed. They were dropped.
    Q_PROPERTY(QString trustor_muxed MEMBER m_trustorMuxed)
    Q_PROPERTY(QString trustor_muxed_id MEMBER m_trustorMuxedId)
    Q_PROPERTY(QString trustee READ trustee WRITE setTrustee)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    Q_PROPERTY(QString limit MEMBER m_limit)

    QString m_trustor;
    QString m_trustee;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    KeyPair* m_trustorKeypair;
    KeyPair* m_trusteeKeypair;
    Asset *m_asset;
    QString m_liquidityPoolId; // hex pool ID when asset_type=="liquidity_pool_shares"
    QString m_limit;

    QString m_trustorMuxed;
    QString m_trustorMuxedId;
public:
    /** The muxed (M...) address when the account was given muxed; empty otherwise. */
    QString getTrustorMuxed() const { return m_trustorMuxed; }
    /** Its muxed id (a uint64, as text); empty otherwise. */
    QString getTrustorMuxedId() const { return m_trustorMuxedId; }
    ChangeTrustOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~ChangeTrustOperationResponse();
    KeyPair& getTrustor();
    KeyPair& getTrustee();
    Asset* getAsset();
    QString trustor() const;
    QString trustee() const;
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;
    /** Pool ID hex string when `asset_type=="liquidity_pool_shares"`. Empty otherwise. */
    QString getLiquidityPoolId() const { return m_liquidityPoolId; }
    QString getLimit() const;
public slots:
    void setTrustor(QString trustor);
    void setTrustee(QString trustee);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ChangeTrustOperationResponse)



Q_DECLARE_METATYPE(qstellar::ChangeTrustOperationResponse*)
#endif // CHANGETRUSTOPERATIONRESPONSE_H


