#ifndef TRUSTLINEUPDATEDEFFECTRESPONSE_H
#define TRUSTLINEUPDATEDEFFECTRESPONSE_H

#include "trustlinecudresponse.h"

/**
 * Represents trustline_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineUpdatedEffectResponse : public TrustlineCUDResponse
{
    Q_OBJECT
public:
    TrustlineUpdatedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineUpdatedEffectResponse();
};
Q_DECLARE_METATYPE(TrustlineUpdatedEffectResponse*)
#endif // TRUSTLINEUPDATEDEFFECTRESPONSE_H
