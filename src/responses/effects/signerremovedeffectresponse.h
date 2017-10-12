#ifndef SIGNERREMOVEDEFFECTRESPONSE_H
#define SIGNERREMOVEDEFFECTRESPONSE_H
#include "signereffectresponse.h"


/**
 * Represents signer_removed effect response.
 */
class SignerRemovedEffectResponse : public SignerEffectResponse
{
    Q_OBJECT
public:
    SignerRemovedEffectResponse(QNetworkReply * reply=nullptr);
    virtual ~SignerRemovedEffectResponse();
};
Q_DECLARE_METATYPE(SignerRemovedEffectResponse*)
#endif // SIGNERREMOVEDEFFECTRESPONSE_H
