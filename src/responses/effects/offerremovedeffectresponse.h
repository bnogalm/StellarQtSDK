#ifndef OFFERREMOVEDEFFECTRESPONSE_H
#define OFFERREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"


/**
 * Represents offer_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class OfferRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    OfferRemovedEffectResponse(QNetworkReply * reply=nullptr);
    virtual ~OfferRemovedEffectResponse();
};
Q_DECLARE_METATYPE(OfferRemovedEffectResponse*)
#endif // OFFERREMOVEDEFFECTRESPONSE_H
