#include "claimablebalanceclawedbackeffectresponse.h"

QSTELLAR_BEGIN_NS


ClaimableBalanceClawedBackEffectResponse::ClaimableBalanceClawedBackEffectResponse(QNetworkReply *reply):EffectResponse(reply)
{

}

ClaimableBalanceClawedBackEffectResponse::~ClaimableBalanceClawedBackEffectResponse()
{

}

const QString ClaimableBalanceClawedBackEffectResponse::getBalanceID() const
{
    return m_balance_id;
}

void ClaimableBalanceClawedBackEffectResponse::setBalanceID(const QString newBalance_id)
{
    if (m_balance_id == newBalance_id)
        return;
    m_balance_id = newBalance_id;
    emit balanceIDChanged();
}
QSTELLAR_END_NS
