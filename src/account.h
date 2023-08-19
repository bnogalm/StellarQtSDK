#ifndef ACCOUNT_H
#define ACCOUNT_H
#include "transactionbuilderaccount.h"

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


    enum class AccountFlag{
        AUTH_REQUIRED = 0x01
        ,AUTH_REVOCABLE = 0x02
        ,AUTH_IMMUTABLE = 0x04
    };
private:


};

#endif // ACCOUNT_H
