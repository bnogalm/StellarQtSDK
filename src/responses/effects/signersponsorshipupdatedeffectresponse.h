#ifndef SIGNERSPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define SIGNERSPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents signer_sponsorship_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class SignerSponsorshipUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString new_sponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)
    Q_PROPERTY(QString signer READ getSigner WRITE setSigner NOTIFY signerChanged)

    QString m_formerSponsor;
    QString m_newSponsor;
    QString m_signer;

public:
    SignerSponsorshipUpdatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~SignerSponsorshipUpdatedEffectResponse();;

    QString getFormerSponsor() const;
    QString getNewSponsor() const;

    QString getSigner() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setNewSponsor(QString newSponsor);

    void setSigner(QString signer);

signals:
    void formerSponsorChanged();
    void newSponsorChanged();
    void signerChanged();
};

#endif // SIGNERSPONSORSHIPUPDATEDEFFECTRESPONSE_H
