#ifndef TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
#define TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
#include "trustlineauthorizationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * @deprecated As of release 0.24.0, replaced by {@link TrustlineFlagsUpdatedEffectResponse}
 *
 * Represents trustline_authorized_to_maintain_liabilities effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 */
class Q_DECL_DEPRECATED TrustlineAuthorizedToMaintainLiabilitiesEffectResponse : public TrustlineAuthorizationResponse
{
public:
    TrustlineAuthorizedToMaintainLiabilitiesEffectResponse();
    virtual ~TrustlineAuthorizedToMaintainLiabilitiesEffectResponse();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineAuthorizedToMaintainLiabilitiesEffectResponse)




#endif // TRUSTLINEAUTHORIZEDTOMAINTAINLIABILITIESEFFECTRESPONSE_H
