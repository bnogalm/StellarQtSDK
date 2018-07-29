#ifndef ACCOUNTINFLATIONDESTINATIONUPDATEDEFFECTRESPONSE_H
#define ACCOUNTINFLATIONDESTINATIONUPDATEDEFFECTRESPONSE_H
#include "effectresponse.h"


/**
* Represents account_inflation_destination_updated effect response.
* @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
* @see org.stellar.sdk.requests.EffectsRequestBuilder
* @see org.stellar.sdk.Server#effects()
*/
class AccountInflationDestinationUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    AccountInflationDestinationUpdatedEffectResponse(QNetworkReply * reply=nullptr);
    virtual ~AccountInflationDestinationUpdatedEffectResponse();
};

Q_DECLARE_METATYPE(AccountInflationDestinationUpdatedEffectResponse*)

#endif // ACCOUNTINFLATIONDESTINATIONUPDATEDEFFECTRESPONSE_H
