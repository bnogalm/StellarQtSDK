#ifndef TRUSTLINECUDRESPONSE_H
#define TRUSTLINECUDRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

class Asset;class TrustlineCUDResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString limit MEMBER m_limit)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    // A pool-share trustline (asset_type liquidity_pool_shares) is named by
    // this, and has no code or issuer. It was dropped.
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)

    QString m_limit;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    QString m_liquidityPoolId;
    Asset* m_asset;

public:
    TrustlineCUDResponse(QNetworkReply *reply= nullptr);
    virtual ~TrustlineCUDResponse();
    QString getLimit() const;
    /**
     * The trustline's asset. A pool-share trustline has none: this throws
     * std::runtime_error for one (it threw on an empty issuer), and
     * getLiquidityPoolId() names its pool.
     */
    Asset* getAsset();
    /** The pool (hex) of a pool-share trustline; empty for any other. */
    QString getLiquidityPoolId() const { return m_liquidityPoolId; }
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;

public slots:
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineCUDResponse)



Q_DECLARE_METATYPE(qstellar::TrustlineCUDResponse*)
#endif // TRUSTLINECUDRESPONSE_H
