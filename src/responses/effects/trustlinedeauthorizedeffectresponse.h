#ifndef TRUSTLINEDEAUTHORIZEDEFFECTRESPONSE_H
#define TRUSTLINEDEAUTHORIZEDEFFECTRESPONSE_H

#include "trustlineauthorizationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * @deprecated As of release 0.24.0, replaced by {@link TrustlineFlagsUpdatedEffectResponse}
 *
 * Represents trustline_deauthorized effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */

class Q_DECL_DEPRECATED TrustlineDeauthorizedEffectResponse : public TrustlineAuthorizationResponse
{
    Q_OBJECT
public:
    TrustlineDeauthorizedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineDeauthorizedEffectResponse();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineDeauthorizedEffectResponse)



Q_DECLARE_METATYPE(qstellar::TrustlineDeauthorizedEffectResponse*)
#endif // TRUSTLINEDEAUTHORIZEDEFFECTRESPONSE_H
