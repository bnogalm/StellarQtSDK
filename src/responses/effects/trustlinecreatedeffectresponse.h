#ifndef TRUSTLINECREATEDEFFECTRESPONSE_H
#define TRUSTLINECREATEDEFFECTRESPONSE_H

#include "trustlinecudresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineCreatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::TrustlineCreatedEffectResponse*)
#endif // TRUSTLINECREATEDEFFECTRESPONSE_H
