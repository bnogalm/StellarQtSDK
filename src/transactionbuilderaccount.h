#ifndef TRANSACTIONBUILDERACCOUNT_H
#define TRANSACTIONBUILDERACCOUNT_H
#include <QtGlobal>

#include "keypair.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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
     * The address the transaction is sent from: G..., or M... for a muxed
     * source. The default is the key pair's G address; Account returns the
     * address it was made from, so Account("M...", n) sends from M... (it
     * used to be signed as its G account, the muxed id dropped).
     */
    virtual QString getAccountId() const { return const_cast<TransactionBuilderAccount*>(this)->getKeypair()->getAccountId(); }

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(TransactionBuilderAccount)

#endif // TRANSACTIONBUILDERACCOUNT_H
