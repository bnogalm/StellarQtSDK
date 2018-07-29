#ifndef DATAUPDATEDEFFECTRESPONSE_H
#define DATAUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
/**
* Represents data_updatedeffect response.
* @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
* @see org.stellar.sdk.requests.EffectsRequestBuilder
* @see org.stellar.sdk.Server#effects()
*/
class DataUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    DataUpdatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~DataUpdatedEffectResponse();
signals:

public slots:
};
Q_DECLARE_METATYPE(DataUpdatedEffectResponse*)
#endif // DATAUPDATEDEFFECTRESPONSE_H
