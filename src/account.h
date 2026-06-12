#ifndef ACCOUNT_H
#define ACCOUNT_H
#include "transactionbuilderaccount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents an account in Stellar network with it's sequence number.
 * Account object is required to build a {@link Transaction}.
 * @see org.stellar.sdk.Transaction.Builder
 */
class Account : public TransactionBuilderAccount
{
    stellar::MuxedAccount m_accountId;
    KeyPair *m_keyPair;
    qint64 m_sequenceNumber;
public:

    /**
     * Class constructor.
     * @param keypair KeyPair associated with this Account
     * @param sequenceNumber Current sequence number of the account (can be obtained using java-stellar-sdk or horizon server)
     */
    Account(KeyPair* keypair, qint64 sequenceNumber);    
    Account(QString accountId, qint64 sequenceNumber);
    ~Account();

    KeyPair* getKeypair();

    QString getAccountId() const;

    qint64 getSequenceNumber();
    qint64 getIncrementedSequenceNumber();

    /**
     * Increments sequence number in this object by one.
     */
    void incrementSequenceNumber();

    /**
     * Decrements the local sequence number by one — use it to roll back after a
     * `TransactionBuilder::build()` whose transaction was NOT submitted, or whose
     * submission is *confirmed* failed, so later builds don't get `tx_bad_seq`.
     * Do NOT call it on a submit timeout — see `SubmitTransactionResponse::isTimeout()`.
     */
    void decrementSequenceNumber();


    enum class AccountFlag{
        AUTH_REQUIRED = 0x01
        ,AUTH_REVOCABLE = 0x02
        ,AUTH_IMMUTABLE = 0x04
    };
private:


};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Account)

#endif // ACCOUNT_H
