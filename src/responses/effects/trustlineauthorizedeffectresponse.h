#ifndef TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H
#define TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H

#include "trustlineauthorizationresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

/**
 * @deprecated As of release 0.24.0, replaced by {@link TrustlineFlagsUpdatedEffectResponse}
 *
 * Represents trustline_authorized effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */

class Q_DECL_DEPRECATED TrustlineAuthorizedEffectResponse : public TrustlineAuthorizationResponse
{
    Q_OBJECT
public:
    TrustlineAuthorizedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineAuthorizedEffectResponse();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineAuthorizedEffectResponse)



Q_DECLARE_METATYPE(qstellar::TrustlineAuthorizedEffectResponse*)
#endif // TRUSTLINEAUTHORIZEDEFFECTRESPONSE_H
