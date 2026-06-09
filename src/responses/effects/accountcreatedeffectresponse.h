#ifndef ACCOUNTCREATEDEFFECTRESPONSE_H
#define ACCOUNTCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

/**
 * Represents account_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString starting_balance MEMBER m_startingBalance)
    QString m_startingBalance;
public:
    AccountCreatedEffectResponse(QNetworkReply * reply = nullptr);
    virtual ~AccountCreatedEffectResponse();
    QString getStartingBalance() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountCreatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::AccountCreatedEffectResponse*)
#endif // ACCOUNTCREATEDEFFECTRESPONSE_H

