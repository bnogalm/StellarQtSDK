#ifndef TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
#define TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
#include "trustlineauthorizationresponse.h"
/**
 * Represents trustline_authorized_to_maintain_liabilities effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 */
class TrustlineAuthorizedToMaintainLiabilitiesEffectResponse : public TrustlineAuthorizationResponse
{
public:
    TrustlineAuthorizedToMaintainLiabilitiesEffectResponse();
    virtual ~TrustlineAuthorizedToMaintainLiabilitiesEffectResponse();
};

#endif // TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
