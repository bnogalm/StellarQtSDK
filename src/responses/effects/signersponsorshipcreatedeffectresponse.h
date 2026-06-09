#ifndef SIGNERSPONSORSHIPCREATEDEFFECTRESPONSE_H
#define SIGNERSPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents signer_sponsorship_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class SignerSponsorshipCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    Q_PROPERTY(QString signer READ getSigner WRITE setSigner NOTIFY signerChanged)

    QString m_sponsor;
    QString m_signer;

public:
    SignerSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~SignerSponsorshipCreatedEffectResponse();;
    QString getSponsor() const;
    QString getSigner() const;

public slots:
    void setSponsor(QString sponsor);
    void setSigner(QString signer);

signals:
    void sponsorChanged();
    void signerChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SignerSponsorshipCreatedEffectResponse)




#endif // SIGNERSPONSORSHIPCREATEDEFFECTRESPONSE_H
