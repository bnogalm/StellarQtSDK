#ifndef TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H
#define TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "asset.h"

#include <QObject>
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
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)

    QString m_assetString;
    Asset* m_asset;

    QString m_sponsor;

public:
    TrustlineSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~TrustlineSponsorshipCreatedEffectResponse();
    QString asset() const;
    Asset* getAsset();
    QString getSponsor() const;

public slots:
    void setAsset(QString assetString);
    void setSponsor(QString sponsor);

signals:
    void assetChanged();
    void sponsorChanged();
};

#endif // TRUSTLINESPONSORSHIPCREATEDEFFECTRESPONSE_H
