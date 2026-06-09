#include "accounthomedomainupdatedeffectresponse.h"

QSTELLAR_BEGIN_NS


AccountHomeDomainUpdatedEffectResponse::AccountHomeDomainUpdatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

AccountHomeDomainUpdatedEffectResponse::~AccountHomeDomainUpdatedEffectResponse()
{

}

QString AccountHomeDomainUpdatedEffectResponse::getHomeDomain() const{
    return m_homeDomain;
}
QSTELLAR_END_NS
