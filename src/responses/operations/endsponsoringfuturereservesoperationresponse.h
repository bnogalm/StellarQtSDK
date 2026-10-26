#ifndef ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
#define ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents EndSponsoringFutureReserves operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class EndSponsoringFutureReservesOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString begin_sponsor READ getBeginSponsor WRITE setBeginSponsor NOTIFY beginSponsorChanged)
    // The muxed (M...) form and id Horizon adds when the account was
    // given muxed. They were dropped.
    Q_PROPERTY(QString begin_sponsor_muxed MEMBER m_beginSponsorMuxed)
    Q_PROPERTY(QString begin_sponsor_muxed_id MEMBER m_beginSponsorMuxedId)

    QString m_beginSponsor;

    QString m_beginSponsorMuxed;
    QString m_beginSponsorMuxedId;
public:
    /** The muxed (M...) address when the account was given muxed; empty otherwise. */
    QString getBeginSponsorMuxed() const { return m_beginSponsorMuxed; }
    /** Its muxed id (a uint64, as text); empty otherwise. */
    QString getBeginSponsorMuxedId() const { return m_beginSponsorMuxedId; }
    EndSponsoringFutureReservesOperationResponse(QNetworkReply* reply=nullptr);
    QString getBeginSponsor() const;
public slots:
    void setBeginSponsor(QString beginSponsor);
signals:
    void beginSponsorChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(EndSponsoringFutureReservesOperationResponse)




#endif // ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
