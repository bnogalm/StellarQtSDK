#ifndef TXREP_H
#define TXREP_H

#include <QString>

#include "network.h"

#include "qstellar_namespace.h"
QSTELLAR_FWD(Transaction)
QSTELLAR_FWD(FeeBumpTransaction)
QSTELLAR_FWD(AbstractTransaction)
QSTELLAR_BEGIN_NS


/**
 * SEP-11 — Txrep, the human-readable text representation of Stellar
 * transactions.
 *
 * Supports all memo types (NONE / TEXT / ID / HASH / RETURN), all
 * precondition variants (NONE / TIME / V2 — ledgerBounds, minSeqNum,
 * minSeqAge, minSeqLedgerGap, extraSigners), and every classic operation
 * (CreateAccount, Payment, PathPayment strict-receive/send, the offer ops,
 * SetOptions, ChangeTrust — liquidity-pool shares included — AllowTrust,
 * AccountMerge, Inflation, ManageData, BumpSequence, the sponsorship
 * begin/end ops, Clawback, ClawbackClaimableBalance, SetTrustLineFlags,
 * Claim/CreateClaimableBalance with nested predicate trees, the
 * liquidity-pool deposit/withdraw ops, and RevokeSponsorship in all its
 * ledger-key / signer variants, a trustline of pool shares included).
 *
 * The native asset is rendered as `native` and field paths follow the
 * SEP-11 / stc convention for cross-tool interoperability, in the form
 * py-stellar-sdk writes and reads. Muxed accounts (the transaction and
 * operation sources, payment, path-payment and merge destinations, the
 * clawback `from`, a fee bump's fee source) are written as their M...
 * address followed by py-stellar-sdk's comment
 * `(accountID: G..., accountMuxedID: N)`; a comment of that form that names
 * another account or id is refused. A revoke-data name is written bare, as
 * py writes and reads it; a name py cannot read back bare has no form here.
 *
 * All three envelope types are read and written, as py-stellar-sdk writes
 * them: ENVELOPE_TYPE_TX, ENVELOPE_TYPE_TX_V0 and ENVELOPE_TYPE_TX_FEE_BUMP
 * (the fee source and fee under `feeBump.tx.`, the wrapped transaction under
 * `feeBump.tx.innerTx.tx.`, its signatures under `feeBump.tx.innerTx.` and
 * the fee bump's under `feeBump.`).
 *
 * A comment after a value is for the reader, who trusts it. On every field
 * the comment must state that field's value in a form that is checked, or be
 * left out; otherwise the document is refused. The forms are those
 * py-stellar-sdk and stc write: an amount (or a fee) in units ("(10.5)") or
 * in stroops with the exponent 7 ("(10.5e7)", "(1,000e7)", digits grouped
 * only as stc groups them), never as a plain number of stroops; a time as a
 * date in UTC, at an offset or at a zone's abbreviation that names one
 * offset in every tz release and wherever else it is used
 * ("(2023-11-14 22:13:20.000000+00:00 (UTC))", "(Tue Nov 14 22:13:20 UTC
 * 2023)", "(Wed Nov 15 02:13:20 +04 2023)", "(Tue Nov 14 23:13:20 CET
 * 2023)", "(Sat Sep  7 22:19:53 UTC 58712)"; not at one read at several,
 * such as EST, which was Sydney's too, GST, IST, CST, PST or BST) or
 * as the number; any other integer as the number; an M... address's account
 * and id as above; a signature hint the key that made the signature (checked
 * against it), or stc's note that it could not tell. Nothing else, words
 * included: a label such as "(the exchange)" names an account nothing in
 * the document confirms, and the list of names stc's editor writes after a
 * type states every other value as much as the one signed.
 *
 * stc's own forms are read too, each exactly: the native asset under the
 * name stc gives it for the network read for ("TestXLM" for the test
 * network), a ChangeTrust asset as the union's arm, a claimable balance id
 * as its union, V2 preconditions under `tx.cond.v2.`, a V0 transaction's
 * `tx.timeBounds`, and an empty data value or signature as "0 bytes". A
 * revoke-data name is read only bare, as py-stellar-sdk writes it: stc
 * quotes it, and py takes the quotes for part of the name.
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
    /**
     * Renders a Transaction's envelope as a SEP-11 Txrep string: its type
     * (ENVELOPE_TYPE_TX, or ENVELOPE_TYPE_TX_V0 for a V0 transaction) and its
     * signatures included. The document is written from the envelope the
     * transaction sends, and is refused (throws) unless it reads back as
     * that envelope, with the transaction's hash.
     */
    static QString toTxrep(Transaction* transaction);

    /**
     * Renders a fee bump's envelope (ENVELOPE_TYPE_TX_FEE_BUMP): its fee
     * source and fee, the transaction it wraps, and both signature lists.
     */
    static QString toTxrep(FeeBumpTransaction* transaction);

    /** Either of the above, by what the transaction is. */
    static QString toTxrep(AbstractTransaction* transaction);

    /**
     * Parses a SEP-11 Txrep string back into a Transaction. The caller
     * takes ownership of the returned pointer. The `network` parameter is
     * required because SEP-11 doesn't encode the network passphrase.
     *
     * An ENVELOPE_TYPE_TX or ENVELOPE_TYPE_TX_V0 document (the latter gives
     * a V0 transaction). A fee-bump document is refused: its inner
     * transaction is not the fee bump; read it with fromTxrepEnvelope(). The
     * document's signatures are checked and not carried over: the
     * transaction comes back to be signed.
     */
    static Transaction* fromTxrep(const QString& txrep, Network* network);

    /**
     * Parses a SEP-11 Txrep document into the envelope it describes: a
     * Transaction (ENVELOPE_TYPE_TX or ENVELOPE_TYPE_TX_V0) or a
     * FeeBumpTransaction, with its signatures. The caller takes ownership.
     */
    static AbstractTransaction* fromTxrepEnvelope(const QString& txrep, Network* network);

private:
    /** What fromTxrep and fromTxrepEnvelope share; `envelope` says which. */
    static AbstractTransaction* read(const QString& txrep, Network* network, bool envelope);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Txrep)

#endif // TXREP_H
