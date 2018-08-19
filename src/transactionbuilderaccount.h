#ifndef TRANSACTIONBUILDERACCOUNT_H
#define TRANSACTIONBUILDERACCOUNT_H
#include <QtGlobal>

#include "keypair.h"



/**
 * Specifies interface for Account object used in {@link org.stellar.sdk.Transaction.Builder}
 */
class TransactionBuilderAccount
{
public:
    virtual ~TransactionBuilderAccount(){}
    /**
     * Returns keypair associated with this Account
     */
    virtual KeyPair *getKeypair() = 0;

    /**
     * Returns current sequence number ot this Account.
     */
    virtual qint64 getSequenceNumber() = 0;

    /**
     * Returns sequence number incremented by one, but does not increment internal counter.
     */
    virtual qint64 getIncrementedSequenceNumber() = 0;

    /**
     * Increments sequence number in this object by one.
     */
    virtual void incrementSequenceNumber() = 0;
};

inline TransactionBuilderAccount * checkNotNull(TransactionBuilderAccount *t, const char *error)
{
    if(!t)
        throw std::runtime_error(error);
    return t;
}
#endif // TRANSACTIONBUILDERACCOUNT_H
