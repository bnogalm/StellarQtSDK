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
    KeyPair *m_keyPair;
    quint64 m_sequenceNumber;
public:

    /**
     * Class constructor.
     * @param keypair KeyPair associated with this Account
     * @param sequenceNumber Current sequence number of the account (can be obtained using java-stellar-sdk or horizon server)
     */
    Account(KeyPair* keypair, quint64 sequenceNumber);


    KeyPair* getKeypair();

    quint64 getSequenceNumber();
    quint64 getIncrementedSequenceNumber();

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
