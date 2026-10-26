#ifndef DATAREMOVEDEFFECTRESPONSE_H
#define DATAREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

/**
* Represents data_removed effect response.
* @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
* @see org.stellar.sdk.requests.EffectsRequestBuilder
* @see org.stellar.sdk.Server#effects()
*/
class DataRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    // The entry's name and (base64) value, as Horizon sends them and js
    // types them. They were dropped: nothing said which entry changed.
    Q_PROPERTY(QString name MEMBER m_name)
    QString m_name;
public:
    /** The data entry's name. */
    QString getName() const { return m_name; }
    DataRemovedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~DataRemovedEffectResponse();
signals:

public slots:
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(DataRemovedEffectResponse)



Q_DECLARE_METATYPE(qstellar::DataRemovedEffectResponse*)
#endif // DATAREMOVEDEFFECTRESPONSE_H
