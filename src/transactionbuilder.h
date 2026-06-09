#ifndef TRANSACTIONBUILDER_H
#define TRANSACTIONBUILDER_H

#include <QtGlobal>
#include <QVector>
#include "accountconverter.h"
#include "transactionbuilderaccount.h"
#include "timebounds.h"
#include "ledgerbounds.h"
#include "transactionpreconditions.h"
#include "signerkey.h"
#include "memo.h"
#include "network.h"
#include "operation.h"
#include "xdr/sorobantxdata.h"
#include <QSharedPointer>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Transaction;

/**
 * @brief Free TransactionBuilder class (introduced in 0.4.0).
 *
 * Equivalent to the legacy `Transaction::Builder` nested class — which now
 * lives on as a deprecated `using` alias for source compatibility. Builders
 * are owners of the operations / memo / timeBounds passed in; on a successful
 * `build()` ownership is transferred to the resulting `Transaction`.
 *
 * 0.7.0 (CAP-21): the legacy TimeBounds slot has been generalized into a
 * full `TransactionPreconditions` bundle. `addTimeBounds(TimeBounds*)` and
 * `setTimeout(...)` continue to work; the new setters below let callers
 * populate the V2-only fields (ledger bounds, minSeqNumber, minSeqAge,
 * minSeqLedgerGap, extraSigners).
 */
class TransactionBuilder
{
    AccountConverter m_accountConverter;
    TransactionBuilderAccount* m_sourceAccount;
    Network* m_network;
    Memo* m_memo;
    TransactionPreconditions m_preconditions;
    QVector<Operation*> m_operations;
    QSharedPointer<stellar::SorobanTransactionData> m_sorobanData;
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

    // ─── CAP-21 setters ──────────────────────────────────────────────────

    /** Replaces the entire preconditions bundle. Copies the input. */
    TransactionBuilder& addPreconditions(const TransactionPreconditions& preconditions);

    /** @param ledgerBounds ownership transferred. */
    TransactionBuilder& setLedgerBounds(LedgerBounds* ledgerBounds);

    /** Sets the minimum source account sequence number. */
    TransactionBuilder& setMinSeqNumber(qint64 seqNum);

    /** Sets the minimum source account sequence age (seconds). */
    TransactionBuilder& setMinSeqAge(quint64 minSeqAge);

    /** Sets the minimum source account sequence ledger gap. */
    TransactionBuilder& setMinSeqLedgerGap(quint32 gap);

    /** Adds an extra signer (max 2 per CAP-21). */
    TransactionBuilder& addExtraSigner(const SignerKey& key);

    /**
     * CAP-46 — attaches a `SorobanTransactionData` payload (footprint +
     * resources + resourceFee). When set, `build()` adds the
     * `resourceFee` to the transaction's total fee. Replaces any
     * previously set Soroban data.
     */
    TransactionBuilder& setSorobanData(const stellar::SorobanTransactionData& data);

    TransactionBuilder& setBaseFee(quint32 baseFee);

    /**
     * Builds the Transaction. Sequence number of the source account is
     * incremented after a successful build. Caller takes ownership.
     */
    Transaction* build();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TransactionBuilder)

#endif // TRANSACTIONBUILDER_H
