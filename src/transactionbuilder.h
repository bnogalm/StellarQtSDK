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
    bool m_baseFeeSet = false;      // setBaseFee(0) is a fee, not "none given"
    static quint32 s_defaultOperationFee;

    // Txrep rebuilds a PRECOND_NONE document (or a V2 one without time bounds)
    // as it was written: setTimeout(TIMEOUT_INFINITE) would add {0, 0} and
    // change its hash.
    friend class Txrep;
    void markTimeoutHandled() { m_timeoutSet = true; }

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
    // Legacy convenience ctor: defaults to the process-wide current network.
    // Network::current() is deprecated (callers should pass a Network explicitly,
    // removal slated for 2.0) but this overload's whole purpose is that implicit
    // default, so the one intentional internal use is silenced right here.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    TransactionBuilder(AccountConverter accountConverter,
                       TransactionBuilderAccount* sourceAccount,
                       Network* network = Network::current());
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

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

    /**
     * Not assignable. The implicit one copied the operation and memo pointers,
     * so two builders owned them: `x = y` ended in a double free, or in x
     * signing whatever was allocated where y's operations had been.
     */
    TransactionBuilder& operator=(const TransactionBuilder&) = delete;

    ~TransactionBuilder();

    int getOperationsCount();

    /** Used only with STELLAR_QT_AUTOSET_BASE_FEE: 100 to 4294967295, or it throws. */
    static void setDefaultOperationFee(qint64 opFee);

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
     *
     * TIMEOUT_INFINITE (0) writes time bounds {0, 0} when none are set yet, as
     * js-stellar-sdk and java do, so the same transaction has the same hash in
     * each. After addTimeBounds(min, 0) with a min, it throws, as py does: js
     * would write {0, 0}, dropping that min, and this kept it, so the same
     * calls signed different transactions. A timeout of n seconds keeps the
     * min, as js does.
     */
    TransactionBuilder& setTimeout(qint64 timeout);

    // ─── CAP-21 setters ──────────────────────────────────────────────────

    /**
     * Replaces the entire preconditions bundle, time bounds included. Copies
     * the input. Without time bounds in it, a timeout set before no longer
     * applies: call setTimeout after this, or build() throws.
     */
    TransactionBuilder& addPreconditions(const TransactionPreconditions& preconditions);

    /** @param ledgerBounds ownership transferred. */
    TransactionBuilder& setLedgerBounds(LedgerBounds* ledgerBounds);

    /** Sets the minimum source account sequence number. */
    TransactionBuilder& setMinSeqNumber(qint64 seqNum);

    /**
     * Sets the minimum source account sequence age (seconds). Takes a qint64,
     * as setBaseFee does, so that a negative value reaches the check: as a
     * quint64, -1 arrived as 2^64 - 1 and was signed. js refuses it too
     * ("min_account_sequence_age cannot be negative"). An age of 2^63 or more
     * goes through TransactionPreconditions::setMinSeqAge.
     */
    TransactionBuilder& setMinSeqAge(qint64 minSeqAge);

    /**
     * Sets the minimum source account sequence ledger gap, 0 to 4294967295:
     * a negative gap was signed as 2^32 - 1 and is refused, as js refuses it.
     */
    TransactionBuilder& setMinSeqLedgerGap(qint64 gap);

    /** Adds an extra signer (max 2 per CAP-21). */
    TransactionBuilder& addExtraSigner(const SignerKey& key);

    /**
     * CAP-46 — attaches a `SorobanTransactionData` payload (footprint +
     * resources + resourceFee). When set, `build()` adds the
     * `resourceFee` to the transaction's total fee. Replaces any
     * previously set Soroban data.
     */
    TransactionBuilder& setSorobanData(const stellar::SorobanTransactionData& data);

    /**
     * The fee per operation, in stroops: 0 to 4294967295, or it throws.
     * Below the network's minimum (100) a transaction is refused when
     * submitted on its own; it is what the inner transaction of a fee bump
     * carries (commonly 0: the fee source pays), which js and py build too.
     * 100 and up was all this took.
     *
     * Takes a qint64 so that a negative value reaches the check. As a quint32
     * parameter, -5 from a caller holding a signed number arrived as
     * 4294967291 and was accepted.
     */
    TransactionBuilder& setBaseFee(qint64 baseFee);

    /**
     * Builds the Transaction. Sequence number of the source account is
     * incremented after a successful build. Caller takes ownership. Throws
     * when the account's sequence number is INT64_MAX: none follows it.
     *
     * The builder can build again, but each build() needs its own timeout
     * (setTimeout, addTimeBounds or preconditions with time bounds): the time
     * bounds go with the transaction built.
     */
    Transaction* build();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TransactionBuilder)

#endif // TRANSACTIONBUILDER_H
