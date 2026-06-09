#include "accountflagsupdatedeffectresponse.h"

QSTELLAR_BEGIN_NS


AccountFlagsUpdatedEffectResponse::AccountFlagsUpdatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply), m_authRequiredFlag(false),m_authRevokableFlag(false)
{

}

AccountFlagsUpdatedEffectResponse::~AccountFlagsUpdatedEffectResponse()
{

}

bool AccountFlagsUpdatedEffectResponse::getAuthRequiredFlag() const{
    return m_authRequiredFlag;
}

bool AccountFlagsUpdatedEffectResponse::getAuthRevokableFlag() const{
    return m_authRevokableFlag;
}
QSTELLAR_END_NS
