#include "accountthresholdsupdatedeffectresponse.h"

AccountThresholdsUpdatedEffectResponse::AccountThresholdsUpdatedEffectResponse(QNetworkReply * reply)
    :EffectResponse(reply)
    ,m_lowThreshold(0)
    ,m_medThreshold(0)
    ,m_highThreshold(0)
{
}

qint32 AccountThresholdsUpdatedEffectResponse::getLowThreshold() const{
    return m_lowThreshold;
}

qint32 AccountThresholdsUpdatedEffectResponse::getMedThreshold() const{
    return m_medThreshold;
}

qint32 AccountThresholdsUpdatedEffectResponse::getHighThreshold() const{
    return m_highThreshold;
}
