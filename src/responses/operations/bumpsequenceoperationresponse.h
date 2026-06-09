#ifndef BUMPSEQUENCEOPERATIONRESPONSE_H
#define BUMPSEQUENCEOPERATIONRESPONSE_H
#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents BumpSequence operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class BumpSequenceOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString bump_to READ bumpTo WRITE setBumpTo)
    qint64 m_bumpTo;
public:
    BumpSequenceOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~BumpSequenceOperationResponse();
    qint64 getBumpTo() const;
    QString bumpTo() const;
public slots:
    void setBumpTo(QString bump_to);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(BumpSequenceOperationResponse)



Q_DECLARE_METATYPE(qstellar::BumpSequenceOperationResponse*)
#endif // BUMPSEQUENCEOPERATIONRESPONSE_H
