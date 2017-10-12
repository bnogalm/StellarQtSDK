#ifndef OFFERCREATEDEFFECTRESPONSE_H
#define OFFERCREATEDEFFECTRESPONSE_H
#include "effectresponse.h"
/**
 * Represents offer_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class OfferCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    OfferCreatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~OfferCreatedEffectResponse();
};
Q_DECLARE_METATYPE(OfferCreatedEffectResponse*)
#endif // OFFERCREATEDEFFECTRESPONSE_H
