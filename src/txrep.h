#ifndef TXREP_H
#define TXREP_H

#include <QString>

#include "network.h"

#include "qstellar_namespace.h"
QSTELLAR_FWD(Transaction)
QSTELLAR_BEGIN_NS


/**
 * SEP-11 — Txrep, the human-readable text representation of Stellar
 * transactions.
 *
 * Supports all memo types (NONE / TEXT / ID / HASH / RETURN), all
 * precondition variants (NONE / TIME / V2 — ledgerBounds, minSeqNum,
 * minSeqAge, minSeqLedgerGap, extraSigners), and every classic operation
 * (CreateAccount, Payment, PathPayment strict-receive/send, the offer ops,
 * SetOptions, ChangeTrust, AllowTrust, AccountMerge, ManageData,
 * BumpSequence, the sponsorship begin/end ops, Clawback, ClawbackClaimableBalance,
 * SetTrustLineFlags, Claim/CreateClaimableBalance with nested predicate trees,
 * the liquidity-pool deposit/withdraw ops, and RevokeSponsorship in all its
 * ledger-key / signer variants).
 *
 * The native asset is rendered as `native` and field paths follow the
 * SEP-11 / stc convention for cross-tool interoperability.
 *
 * The Soroban operations (InvokeHostFunction / ExtendFootprintTTL /
 * RestoreFootprint) are intentionally NOT supported — txrep is a pre-Soroban
 * format and no mainstream SDK encodes them (py-stellar-sdk raises, the JS
 * SDK has no txrep), so there is no interoperable representation to target.
 * They throw an `unsupported` runtime_error.
 *
 * The format follows the dotted-path notation of the SEP-11 spec:
 *   type: ENVELOPE_TYPE_TX
 *   tx.sourceAccount: GC...
 *   tx.fee: 100
 *   tx.operations.len: 1
 *   ...
 */
class Txrep
{
public:
    /** Renders a Transaction's V1 envelope as a SEP-11 Txrep string. */
    static QString toTxrep(Transaction* transaction);

    /**
     * Parses a SEP-11 Txrep string back into a Transaction. The caller
     * takes ownership of the returned pointer. The `network` parameter is
     * required because SEP-11 doesn't encode the network passphrase.
     */
    static Transaction* fromTxrep(const QString& txrep, Network* network);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Txrep)

#endif // TXREP_H
