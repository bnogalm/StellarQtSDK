#ifndef SIGNERUPDATEDEFFECTRESPONSE_H
#define SIGNERUPDATEDEFFECTRESPONSE_H
#include "signereffectresponse.h"



/**
 * Represents signed_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class SignerUpdatedEffectResponse : public SignerEffectResponse
{
    Q_OBJECT
public:
    SignerUpdatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~SignerUpdatedEffectResponse();
};
Q_DECLARE_METATYPE(SignerUpdatedEffectResponse*)
#endif // SIGNERUPDATEDEFFECTRESPONSE_H
