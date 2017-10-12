#ifndef TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H
#define TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H

#include "trustlineauthorizationresponse.h"


/**
 * Represents trustline_authorized effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineAuthorizedEffectResponse : public TrustlineAuthorizationResponse
{
    Q_OBJECT
public:
    TrustlineAuthorizedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineAuthorizedEffectResponse();
};
Q_DECLARE_METATYPE(TrustlineAuthorizedEffectResponse*)
#endif // TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H
