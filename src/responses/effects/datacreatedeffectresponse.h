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
    // The entry's name and (base64) value, as Horizon sends them and js
    // types them. They were dropped: nothing said which entry changed.
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(QString value MEMBER m_value)
    QString m_name;
    QString m_value;
public:
    /** The data entry's name. */
    QString getName() const { return m_name; }
    /** Its value, base64 as Horizon sends it (empty for an empty value). */
    QString getValue() const { return m_value; }
    /** Its value, decoded. */
    QByteArray getValueBytes() const { return QByteArray::fromBase64(m_value.toLatin1()); }
    DataCreatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~DataCreatedEffectResponse();
signals:

public slots:
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(DataCreatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::DataCreatedEffectResponse*)
#endif // DATACREATEDEFFECTRESPONSE_H
