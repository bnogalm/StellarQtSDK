#ifndef CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H
#define CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H

#include "operationresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

/**
 * Represents a Clawback Claimable Balance operation response.
 *
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ClawbackClaimableBalanceOperationResponse : public OperationResponse
{
    Q_OBJECT

    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)
    QString m_balanceId;

public:
    ClawbackClaimableBalanceOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~ClawbackClaimableBalanceOperationResponse();
    const QString getBalanceID() const;
    void setBalanceID(const QString newBalance_id);
signals:
    void balanceIDChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClawbackClaimableBalanceOperationResponse)




#endif // CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H
