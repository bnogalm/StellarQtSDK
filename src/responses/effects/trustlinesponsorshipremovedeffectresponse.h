#ifndef TRUSTLINESPONSORSHIPREMOVEDEFFECTRESPONSE_H
#define TRUSTLINESPONSORSHIPREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
#include "asset.h"

/**
 * Represents trustline_sponsorship_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineSponsorshipRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)

    QString m_assetString;
    Asset* m_asset;

    QString m_formerSponsor;

public:
    TrustlineSponsorshipRemovedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~TrustlineSponsorshipRemovedEffectResponse();
    QString asset() const;
    Asset* getAsset();
    QString getFormerSponsor() const;

public slots:
    void setAsset(QString assetString);
    void setFormerSponsor(QString formerSponsor);

signals:
    void assetChanged();
    void formerSponsorChanged();
};

#endif // TRUSTLINESPONSORSHIPREMOVEDEFFECTRESPONSE_H
