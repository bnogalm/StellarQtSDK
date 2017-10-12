#include "signereffectresponse.h"

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
