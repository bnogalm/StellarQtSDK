#ifndef TRUSTLINEREMOVEDEFFECTRESPONSE_H
#define TRUSTLINEREMOVEDEFFECTRESPONSE_H

#include "trustlinecudresponse.h"
/**
 * Represents trustline_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineRemovedEffectResponse : public TrustlineCUDResponse
{
    Q_OBJECT
public:
    TrustlineRemovedEffectResponse(QNetworkReply * reply = nullptr);
    virtual ~TrustlineRemovedEffectResponse();
};
Q_DECLARE_METATYPE(TrustlineRemovedEffectResponse*)
#endif // TRUSTLINEREMOVEDEFFECTRESPONSE_H
