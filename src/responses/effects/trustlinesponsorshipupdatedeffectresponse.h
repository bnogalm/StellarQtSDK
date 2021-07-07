#ifndef TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
#include "asset.h"
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
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString newSponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)

    QString m_assetString;
    Asset* m_asset;
    QString m_formerSponsor;
    QString m_newSponsor;

public:
    TrustlineSponsorshipUpdatedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineSponsorshipUpdatedEffectResponse();
    QString asset() const;
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

#endif // TRUSTLINESPONSORSHIPUPDATEDEFFECTRESPONSE_H
