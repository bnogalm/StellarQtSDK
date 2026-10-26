#ifndef TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H
#define TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "asset.h"

#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents trustline_sponsorship_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineSponsorshipCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT

    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    // A pool-share trustline has no "asset": Horizon names it by these two.
    Q_PROPERTY(QString asset_type MEMBER m_assetType)
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)

    QString m_assetString;
    QString m_assetType;
    QString m_liquidityPoolId;
    Asset* m_asset;

    QString m_sponsor;

public:
    TrustlineSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~TrustlineSponsorshipCreatedEffectResponse();
    QString asset() const;
    /** "liquidity_pool" for a pool-share trustline, empty for a credit one. */
    QString getAssetType() const { return m_assetType; }
    /** The pool (hex) of a pool-share trustline, empty for a credit one. */
    QString getLiquidityPoolId() const { return m_liquidityPoolId; }
    Asset* getAsset();
    QString getSponsor() const;

public slots:
    void setAsset(QString assetString);
    void setSponsor(QString sponsor);

signals:
    void assetChanged();
    void sponsorChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineSponsorshipCreatedEffectResponse)




#endif // TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H
