#ifndef QSTELLAR_TXBUILDERQML_H
#define QSTELLAR_TXBUILDERQML_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class AssetRegistry;

/**
 * Builds a transaction from a plain JavaScript object and returns everything a
 * laboratory UI wants to show about it.
 *
 * The shape is deliberate. A QML-friendly builder could have been a stateful
 * object with a property per field and a list of operation types, but that
 * means mirroring the whole operation hierarchy into QML types and keeping the
 * two in step forever. A declarative spec in, a result map out, is one function
 * that covers every operation and stays honest when the SDK grows:
 *
 *     var r = Tx.build({
 *         sourceAccount: "G...",
 *         sequence: "123456",
 *         baseFee: 100,
 *         timeout: 180,
 *         memo: { type: "text", value: "hello" },
 *         operations: [
 *             { type: "payment", destination: "G...", asset: "native", amount: "10.5" }
 *         ]
 *     })
 *     // r.xdr, r.txrep, r.hash, r.fee   — or r.error
 *
 * Amounts are decimal text ("10.5"). A JS number is read only while it is
 * exact: below 2^29 units (536870912) any seven-decimal amount, above it
 * whole numbers only; anything else is refused ("pass it as text"), never
 * signed as a nearby amount. Integers (sequence, memo id, ...) likewise, up
 * to 2^53 - 1 as numbers and at any size as text.
 *
 * Rebuilding on every keystroke is the intended use: it is pure, it touches no
 * network, and the SDK's own validation is what produces `error`.
 */
class TxBuilderQml : public QObject
{
    Q_OBJECT
public:
    explicit TxBuilderQml(AssetRegistry* assets, QObject* parent = nullptr);

    /**
     * Builds and returns { xdr, txrep, hash, fee, operationCount }, or
     * { error } with the SDK's own message when the spec does not describe a
     * valid transaction. Never throws.
     *
     * `network` is "testnet" (default) or "public". `sequence` is a string
     * because a JavaScript number cannot hold an int64 sequence exactly, and it
     * is the sequence that ends up in the envelope — not the account's current
     * one. Horizon.loadAccount hands out `nextSequence` ready to drop in here.
     *
     * `baseFee` (stroops per operation, 0 to 4294967295; default 100: under 100 only for a fee bump's inner transaction) and
     * `timeout` (seconds; default 180; 0 means the transaction never expires)
     * must be whole numbers when given. Anything else — negative, fractional,
     * blank — is an error, never a silent default. So is a total fee that the
     * envelope's uint32 fee field cannot hold.
     */
    Q_INVOKABLE QVariantMap build(const QVariantMap& spec) const;

    /** The operation types `build()` understands, for a UI to offer. */
    Q_INVOKABLE QStringList supportedOperations() const;

    /**
     * What each operation type takes, as a list of
     * { name, kind, label, required, note } maps — enough for a form to draw
     * itself without keeping its own copy of the field list.
     *
     * `kind` names the editor to use; see operationtable.h for the vocabulary.
     * Numbers are carried as strings throughout, because several of these
     * fields are int64 and a JavaScript number is not.
     */
    Q_INVOKABLE QVariantList fieldsFor(const QString& operationType) const;

    /** One line describing an operation, for a form to show under the picker. */
    Q_INVOKABLE QString describeOperation(const QString& operationType) const;

    /** A plausible testnet value for one field, or an empty string. */
    Q_INVOKABLE QString demoValueFor(const QString& operationType,
                                     const QString& fieldName) const;

    /**
     * Adds signatures to an envelope. Offline: signing sends nothing anywhere.
     *
     *     Tx.sign({ xdr: "AAAA...", seeds: ["S...", "S..."], network: "testnet" })
     *     // { xdr, hash, signatureCount }   — or { error }
     *
     * Signing is separated from building because on Stellar they genuinely are
     * separate acts: an envelope can be built on one machine, signed on
     * another, and submitted by a third. Keeping them apart in the API is what
     * lets a UI show that.
     *
     * The network matters even though nothing is transmitted: the signature is
     * over a payload that starts with the hash of the network passphrase, so a
     * signature made for the wrong network is silently worthless. Pass either
     * `network` ("testnet" / "public") or an explicit `networkPassphrase`.
     */
    Q_INVOKABLE QVariantMap sign(const QVariantMap& args) const;

    /**
     * Reads an envelope without changing it: { hash, txrep, envelopeType,
     * sourceAccount, fee, sequence, operationCount, signatureCount,
     * isFeeBump }, or { error }.
     *
     * This is the "someone sent me an XDR blob" case — the one place where a
     * user is asked to approve bytes they did not produce.
     *
     * `txrep` is the envelope's own SEP-11 document, signatures included:
     * a V0 one's says ENVELOPE_TYPE_TX_V0, and a fee bump's is the fee
     * bump's. When SEP-11 has no form for it (the Soroban operations) it is
     * empty and `txrepUnavailable` says why. A fee bump has no sourceAccount
     * or sequence of its own: it gives { feeSource, fee, signatureCount } as
     * its own, and the transaction it wraps under inner names only:
     * { innerHash, innerSourceAccount, innerSequence, innerFee,
     * innerSignatureCount, operationCount }.
     */
    Q_INVOKABLE QVariantMap inspect(const QVariantMap& args) const;

    /**
     * The id of the liquidity pool for two assets and a fee.
     *
     * Unlike an offer id, which the network assigns and hands back, a pool id
     * is SHA-256 over the pool's own parameters — so it can be worked out
     * before the pool exists, and there is no way to deposit into one without
     * working it out. The two assets must be in XDR lexicographic order and
     * the SDK throws if they are not, which is the error worth surfacing:
     * "native" is always first, and beyond that the ordering is not the
     * alphabetical one anybody guesses.
     *
     * Returns { poolId, assetA, assetB, fee } or { error }.
     */
    /**
     * The footprint for a contract's own instance entry, as base64
     * SorobanTransactionData ready to hand back to build().
     *
     * A Soroban transaction declares every ledger entry it will touch. For the
     * two operations that do nothing but manage entries — extendFootprintTtl
     * and restoreFootprint — there is no host function to run, so a simulation
     * has nothing to infer the footprint FROM: the caller has to say which
     * entries it means, and without one the RPC cannot even parse the envelope
     * ("Could not unmarshal transaction").
     *
     * Deliberately narrow. A general ledger-key vocabulary in QML would be a
     * large surface for one use, so this covers the case that comes up: the
     * instance entry of a contract you have deployed, which is what a client
     * extends to stop its contract expiring.
     *
     * `durability` is "persistent" or "temporary". The resource fee defaults
     * to zero: TransactionBuilder adds a declared resource fee to the
     * transaction fee, and a later simulation replaces it with its own. Given,
     * it is read as baseFee is: whole stroops, 0 to 4294967295, a number or
     * text, no '+', no -0.
     *
     * Returns { sorobanData, contractId, resourceFee, durability } or { error }.
     */
    Q_INVOKABLE QVariantMap footprintForContract(
        const QString& contractId,
        const QString& durability = QStringLiteral("persistent"),
        const QVariant& resourceFee = QVariant()) const;

    Q_INVOKABLE QVariantMap liquidityPoolId(const QString& assetA,
                                            const QString& assetB,
                                            const QVariant& fee = QVariant(30)) const;

private:
    AssetRegistry* m_assets;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TxBuilderQml)

#endif // QSTELLAR_TXBUILDERQML_H
