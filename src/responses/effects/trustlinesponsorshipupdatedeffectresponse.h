#ifndef TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
#include "asset.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents trustline_sponsorship_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineSponsorshipUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    // A pool-share trustline has no "asset": Horizon names it by these two.
    Q_PROPERTY(QString asset_type MEMBER m_assetType)
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString new_sponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)

    QString m_assetString;
    QString m_assetType;
    QString m_liquidityPoolId;
    Asset* m_asset;
    QString m_formerSponsor;
    QString m_newSponsor;

public:
    TrustlineSponsorshipUpdatedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineSponsorshipUpdatedEffectResponse();
    QString asset() const;
    /** "liquidity_pool" for a pool-share trustline, empty for a credit one. */
    QString getAssetType() const { return m_assetType; }
    /** The pool (hex) of a pool-share trustline, empty for a credit one. */
    QString getLiquidityPoolId() const { return m_liquidityPoolId; }
    Asset* getAsset();
    QString getFormerSponsor() const;

    QString getNewSponsor() const;

public slots:
    void setAsset(QString assetString);
    void setFormerSponsor(QString formerSponsor);

    void setNewSponsor(QString newSponsor);

signals:
    void assetChanged();
    void formerSponsorChanged();
    void newSponsorChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineSponsorshipUpdatedEffectResponse)




#endif // TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H
