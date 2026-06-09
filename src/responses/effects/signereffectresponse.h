#ifndef SIGNEREFFECTRESPONSE_H
#define SIGNEREFFECTRESPONSE_H
#include "effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(SignerEffectResponse)



Q_DECLARE_METATYPE(qstellar::SignerEffectResponse*)


#endif // SIGNEREFFECTRESPONSE_H
