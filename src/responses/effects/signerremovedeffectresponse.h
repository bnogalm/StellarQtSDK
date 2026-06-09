#ifndef SIGNERREMOVEDEFFECTRESPONSE_H
#define SIGNERREMOVEDEFFECTRESPONSE_H
#include "signereffectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(SignerRemovedEffectResponse)



Q_DECLARE_METATYPE(qstellar::SignerRemovedEffectResponse*)
#endif // SIGNERREMOVEDEFFECTRESPONSE_H
