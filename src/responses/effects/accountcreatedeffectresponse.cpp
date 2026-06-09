#include "accountcreatedeffectresponse.h"

QSTELLAR_BEGIN_NS


AccountCreatedEffectResponse::AccountCreatedEffectResponse(QNetworkReply *reply):EffectResponse(reply)
{

}

AccountCreatedEffectResponse::~AccountCreatedEffectResponse()
{

}

QString AccountCreatedEffectResponse::getStartingBalance() const{
    return m_startingBalance;
}
QSTELLAR_END_NS
