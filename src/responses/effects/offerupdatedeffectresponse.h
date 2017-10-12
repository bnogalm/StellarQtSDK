#ifndef OFFERUPDATEDEFFECTRESPONSE_H
#define OFFERUPDATEDEFFECTRESPONSE_H


#include "effectresponse.h"

/**
 * Represents offer_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class OfferUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    OfferUpdatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~OfferUpdatedEffectResponse();
};
Q_DECLARE_METATYPE(OfferUpdatedEffectResponse*)
#endif // OFFERUPDATEDEFFECTRESPONSE_H
