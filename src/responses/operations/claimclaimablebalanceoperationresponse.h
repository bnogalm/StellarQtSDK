#ifndef CLAIMCLAIMABLEBALANCEOPERATIONRESPONSE_H
#define CLAIMCLAIMABLEBALANCEOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents ClaimClaimableBalance operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ClaimClaimableBalanceOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)
    Q_PROPERTY(QString claimant READ getClaimant WRITE setClaimant NOTIFY claimantChanged)
    // The muxed (M...) form and id Horizon adds when the account was
    // given muxed. They were dropped.
    Q_PROPERTY(QString claimant_muxed MEMBER m_claimantMuxed)
    Q_PROPERTY(QString claimant_muxed_id MEMBER m_claimantMuxedId)


    QString m_balanceID;
    QString m_claimant;

    QString m_claimantMuxed;
    QString m_claimantMuxedId;
public:
    /** The muxed (M...) address when the account was given muxed; empty otherwise. */
    QString getClaimantMuxed() const { return m_claimantMuxed; }
    /** Its muxed id (a uint64, as text); empty otherwise. */
    QString getClaimantMuxedId() const { return m_claimantMuxedId; }
    ClaimClaimableBalanceOperationResponse(QNetworkReply* reply = nullptr);
    virtual ~ClaimClaimableBalanceOperationResponse();
    QString getBalanceID() const;
    QString getClaimant() const;

public slots:
    void setBalanceID(QString balanceID);
    void setClaimant(QString claimant);

signals:
    void balanceIDChanged();
    void claimantChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClaimClaimableBalanceOperationResponse)




#endif // CLAIMCLAIMABLEBALANCEOPERATIONRESPONSE_H
