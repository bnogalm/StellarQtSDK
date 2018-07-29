#ifndef DATAREMOVEDEFFECTRESPONSE_H
#define DATAREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"


/**
* Represents data_removed effect response.
* @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
* @see org.stellar.sdk.requests.EffectsRequestBuilder
* @see org.stellar.sdk.Server#effects()
*/
class DataRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
public:
    DataRemovedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~DataRemovedEffectResponse();
signals:

public slots:
};
Q_DECLARE_METATYPE(DataRemovedEffectResponse*)
#endif // DATAREMOVEDEFFECTRESPONSE_H
