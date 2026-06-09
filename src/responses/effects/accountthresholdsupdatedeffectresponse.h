#ifndef ACCOUNTTHRESHOLDSUPDATEDEFFECTRESPONSE_H
#define ACCOUNTTHRESHOLDSUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents account_thresholds_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountThresholdsUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(qint32 low_threshold MEMBER m_lowThreshold)
    Q_PROPERTY(qint32 med_threshold MEMBER m_medThreshold)
    Q_PROPERTY(qint32 high_threshold MEMBER m_highThreshold)
    qint32 m_lowThreshold;
    qint32 m_medThreshold;
    qint32 m_highThreshold;
public:
    AccountThresholdsUpdatedEffectResponse(QNetworkReply * reply=nullptr);

    qint32 getLowThreshold() const;
    qint32 getMedThreshold() const;
    qint32 getHighThreshold() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountThresholdsUpdatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::AccountThresholdsUpdatedEffectResponse*)
#endif // ACCOUNTTHRESHOLDSUPDATEDEFFECTRESPONSE_H


