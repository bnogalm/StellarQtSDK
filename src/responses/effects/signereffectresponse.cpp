#include "signereffectresponse.h"

QSTELLAR_BEGIN_NS


SignerEffectResponse::SignerEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_weight(0)
{

}

SignerEffectResponse::~SignerEffectResponse()
{

}

qint32 SignerEffectResponse::getWeight() const{
    return m_weight;
}

QString SignerEffectResponse::getPublicKey() const{
    return m_publicKey;
}
QSTELLAR_END_NS
