#ifndef OFFERUPDATEDEFFECTRESPONSE_H
#define OFFERUPDATEDEFFECTRESPONSE_H


#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(OfferUpdatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::OfferUpdatedEffectResponse*)
#endif // OFFERUPDATEDEFFECTRESPONSE_H
