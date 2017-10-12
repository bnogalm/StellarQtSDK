#ifndef TRUSTLINECREATEDEFFECTRESPONSE_H
#define TRUSTLINECREATEDEFFECTRESPONSE_H

#include "trustlinecudresponse.h"

/**
 * Represents trustline_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */

class TrustlineCreatedEffectResponse : public TrustlineCUDResponse
{
    Q_OBJECT
public:
    TrustlineCreatedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineCreatedEffectResponse();
};
Q_DECLARE_METATYPE(TrustlineCreatedEffectResponse*)
#endif // TRUSTLINECREATEDEFFECTRESPONSE_H
