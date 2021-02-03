#ifndef SIGNERSPONSORSHIPREMOVEDEFFECTRESPONSE_H
#define SIGNERSPONSORSHIPREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents signer_sponsorship_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class SignerSponsorshipRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT

    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString signer READ getSigner WRITE setSigner NOTIFY signerChanged)

    QString m_formerSponsor;
    QString m_signer;

public:
    SignerSponsorshipRemovedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~SignerSponsorshipRemovedEffectResponse();;
    QString getFormerSponsor() const;
    QString getSigner() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setSigner(QString signer);

signals:
    void formerSponsorChanged();
    void signerChanged();
};

#endif // SIGNERSPONSORSHIPREMOVEDEFFECTRESPONSE_H
