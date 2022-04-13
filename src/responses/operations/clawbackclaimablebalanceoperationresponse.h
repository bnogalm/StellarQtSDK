#ifndef CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H
#define CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H

#include "operationresponse.h"


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

#endif // CLAWBACKCLAIMABLEBALANCEOPERATIONRESPONSE_H
