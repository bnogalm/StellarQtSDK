#ifndef DATACREATEDEFFECTRESPONSE_H
#define DATACREATEDEFFECTRESPONSE_H

#include "effectresponse.h"

/**
* Represents data_created effect response.
* @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
* @see org.stellar.sdk.requests.EffectsRequestBuilder
* @see org.stellar.sdk.Server#effects()
*/
class DataCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    DataCreatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~DataCreatedEffectResponse();
signals:

public slots:
};
Q_DECLARE_METATYPE(DataCreatedEffectResponse*)
#endif // DATACREATEDEFFECTRESPONSE_H
