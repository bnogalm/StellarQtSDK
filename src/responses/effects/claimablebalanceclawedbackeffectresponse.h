#ifndef CLAIMABLEBALANCECLAWEDBACKEFFECTRESPONSE_H
#define CLAIMABLEBALANCECLAWEDBACKEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents claimable_balance_clawed_back effect response.
 *
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceClawedBackEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)
    QString m_balance_id;

public:
    ClaimableBalanceClawedBackEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceClawedBackEffectResponse();
    const QString getBalanceID() const;
    void setBalanceID(const QString newBalance_id);
signals:
    void balanceIDChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClaimableBalanceClawedBackEffectResponse)




#endif // CLAIMABLEBALANCECLAWEDBACKEFFECTRESPONSE_H
