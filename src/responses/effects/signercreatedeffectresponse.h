#ifndef SIGNERCREATEDEFFECTRESPONSE_H
#define SIGNERCREATEDEFFECTRESPONSE_H

#include "signereffectresponse.h"

/**
 * Represents signer_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class SignerCreatedEffectResponse : public SignerEffectResponse
{
    Q_OBJECT
public:
    SignerCreatedEffectResponse(QNetworkReply *reply  = nullptr);
    virtual ~SignerCreatedEffectResponse();
};
Q_DECLARE_METATYPE(SignerCreatedEffectResponse*)
#endif // SIGNERCREATEDEFFECTRESPONSE_H
