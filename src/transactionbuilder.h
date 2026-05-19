#ifndef TRANSACTIONBUILDER_H
#define TRANSACTIONBUILDER_H

#include <QtGlobal>
#include <QVector>
#include "accountconverter.h"
#include "transactionbuilderaccount.h"
#include "timebounds.h"
#include "memo.h"
#include "network.h"
#include "operation.h"

class Transaction;

/**
 * @brief Free TransactionBuilder class (introduced in 0.4.0).
 *
 * Equivalent to the legacy `Transaction::Builder` nested class — which now
 * lives on as a deprecated `using` alias for source compatibility. Builders
 * are owners of the operations / memo / timeBounds passed in; on a successful
 * `build()` ownership is transferred to the resulting `Transaction`.
 *
 * The API and ownership semantics are deliberately identical to the legacy
 * nested builder so that downstream code can switch over by simply renaming
 * `Transaction::Builder` → `TransactionBuilder`.
 *
 * Java SDK alignment: a constructor without `AccountConverter` (muxed always
 * on) will be added in 0.4.0 Iter 3 together with the desingletonization of
 * `Network`.
 */
class TransactionBuilder
{
    AccountConverter m_accountConverter;
    TransactionBuilderAccount* m_sourceAccount;
    Network* m_network;
    Memo* m_memo;
    TimeBounds* m_timeBounds;
    QVector<Operation*> m_operations;
    bool m_timeoutSet;
    quint32 m_baseFee;
    static quint32 s_defaultOperationFee;

public:
    static const quint32 BASE_FEE       = 100;
    static const qint64  TIMEOUT_INFINITE = 0;

    /**
     * Legacy constructor (compat with the old nested API).
     *
     * @param accountConverter how to encode source/destination accounts (muxed or not).
     * @param sourceAccount source account; its sequence number is incremented on build().
     * @param network network this transaction targets.
     */
    TransactionBuilder(AccountConverter accountConverter,
                       TransactionBuilderAccount* sourceAccount,
                       Network* network = Network::current());

    /**
     * Java-aligned constructor (introduced in 0.4.0).
     *
     * Equivalent to the legacy one with `AccountConverter().enableMuxed()`
     * — i.e. muxed accounts always enabled, which is what 0.5.0+ assumes
     * everywhere. Mirrors `new TransactionBuilder(account, Network.TESTNET)`
     * from `java-stellar-sdk`.
     *
     * @param sourceAccount source account; its sequence number is incremented on build().
     * @param network network this transaction targets (must not be null).
     */
    TransactionBuilder(TransactionBuilderAccount* sourceAccount, Network* network);

    /**
     * Move-like "copy" constructor — leaves `other` empty after the call.
     * Matches the historical nested API; this isn't a true copy.
     */
    TransactionBuilder(TransactionBuilder& other);

    ~TransactionBuilder();

    int getOperationsCount();

    static void setDefaultOperationFee(quint32 opFee);

    /**
     * @param operation, you lose ownership of the object — don't delete it.
     */
    TransactionBuilder& addOperation(Operation* operation);

    /**
     * @param memo, you lose ownership of the object — don't delete it.
     */
    TransactionBuilder& addMemo(Memo* memo);

    /**
     * @param timeBounds, you lose ownership of the object — don't delete it.
     */
    TransactionBuilder& addTimeBounds(TimeBounds* timeBounds);

    /**
     * Convenience: sets a relative timeout (in seconds) from now.
     */
    TransactionBuilder& setTimeout(qint64 timeout);

    TransactionBuilder& setBaseFee(quint32 baseFee);

    /**
     * Builds the Transaction. Sequence number of the source account is
     * incremented after a successful build. Caller takes ownership.
     */
    Transaction* build();
};

#endif // TRANSACTIONBUILDER_H
