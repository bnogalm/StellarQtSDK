#ifndef ACCOUNTREMOVEDEFFECTRESPONSE_H
#define ACCOUNTREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"

/**
 * Represents account_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    AccountRemovedEffectResponse(QNetworkReply * reply=nullptr);
    virtual ~AccountRemovedEffectResponse();
};
Q_DECLARE_METATYPE(AccountRemovedEffectResponse*)
#endif // ACCOUNTREMOVEDEFFECTRESPONSE_H
