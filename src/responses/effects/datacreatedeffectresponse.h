#ifndef DATACREATEDEFFECTRESPONSE_H
#define DATACREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(DataCreatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::DataCreatedEffectResponse*)
#endif // DATACREATEDEFFECTRESPONSE_H
