#ifndef ACCOUNTCREATEDEFFECTRESPONSE_H
#define ACCOUNTCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"


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
Q_DECLARE_METATYPE(AccountCreatedEffectResponse*)
#endif // ACCOUNTCREATEDEFFECTRESPONSE_H

