#include "accounthomedomainupdatedeffectresponse.h"

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
