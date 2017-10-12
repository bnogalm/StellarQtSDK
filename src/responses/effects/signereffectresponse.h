#ifndef SIGNEREFFECTRESPONSE_H
#define SIGNEREFFECTRESPONSE_H
#include "effectresponse.h"


class SignerEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(qint32 weight MEMBER m_weight)
    Q_PROPERTY(QString public_key MEMBER m_publicKey)
    qint32 m_weight;
    QString m_publicKey;
public:
    SignerEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~SignerEffectResponse();
    qint32 getWeight() const;

    QString getPublicKey() const;
};
Q_DECLARE_METATYPE(SignerEffectResponse*)


#endif // SIGNEREFFECTRESPONSE_H
