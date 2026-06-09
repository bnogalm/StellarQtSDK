#ifndef OFFERREMOVEDEFFECTRESPONSE_H
#define OFFERREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(OfferRemovedEffectResponse)



Q_DECLARE_METATYPE(qstellar::OfferRemovedEffectResponse*)
#endif // OFFERREMOVEDEFFECTRESPONSE_H
