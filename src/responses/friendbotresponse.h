#ifndef FRIENDBOTRESPONSE_H
#define FRIENDBOTRESPONSE_H
#include "response.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * What a friendbot answers once it funded an account (see
 * Server::friendbot()): the transaction it submitted, as Horizon reports a
 * submission. ready() only for a 200 whose body is an object with
 * "successful": true and a "hash" of 64 hex digits, as QML's
 * Horizon.fundAccount() reads it; any other 2xx answer, or a body that says
 * otherwise, emits error() with an UnknownResponseException. An HTTP error
 * (friendbot answers 400 with a problem document for an account it funded
 * already) is the error() every request gives: its status, the Problem with
 * its detail, and the exception of its kind (BadRequestException,
 * TooManyRequestsException, BadResponseException, ...). js's friendbot()
 * returns whatever 2xx body comes back.
 */
class FriendbotResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(bool successful MEMBER m_successful)
    Q_PROPERTY(QString hash MEMBER m_hash)
    Q_PROPERTY(qint64 ledger MEMBER m_ledger)
    Q_PROPERTY(QString envelope_xdr MEMBER m_envelopeXdr)
    Q_PROPERTY(QString result_xdr MEMBER m_resultXdr)

    bool m_successful;
    QString m_hash;
    qint64 m_ledger;
    QString m_envelopeXdr;
    QString m_resultXdr;
public:
    Q_INVOKABLE explicit FriendbotResponse(QNetworkReply* reply = nullptr);
    virtual ~FriendbotResponse();

    /** Reads friendbot's answer; throws std::runtime_error for one that does not say the account was funded. */
    void loadFromJson(QByteArray data) override;

    /** The hash of the transaction that funded the account, 64 hex digits. */
    QString getHash() const;

    /**
     * The ledger the transaction was applied in, when friendbot waited for
     * it and says (1 to 2^32 - 1); 0 when it does not, as today's
     * friendbot.stellar.org answers.
     */
    qint64 getLedger() const;

    /** The funding transaction's envelope, base64 XDR, as friendbot sent it; empty when it did not. */
    QString getEnvelopeXdr() const;

    /** Its result, base64 XDR, as friendbot sent it; empty when it did not. */
    QString getResultXdr() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(FriendbotResponse)

#endif // FRIENDBOTRESPONSE_H
