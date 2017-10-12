#include "accountcreatedeffectresponse.h"

AccountCreatedEffectResponse::AccountCreatedEffectResponse(QNetworkReply *reply):EffectResponse(reply)
{

}

AccountCreatedEffectResponse::~AccountCreatedEffectResponse()
{

}

QString AccountCreatedEffectResponse::getStartingBalance() const{
    return m_startingBalance;
}
