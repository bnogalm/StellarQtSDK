#include "txbuilderqml.h"

#include <QHash>
#include <cstring>
#include <QScopedPointer>
#include <QVariantList>
#include <limits>
#include <stdexcept>

#include "assetregistry.h"
#include "operationfactory.h"
#include "operationtable.h"
#include "account.h"
#include "asset.h"
#include "changetrustoperation.h"
#include "createaccountoperation.h"
#include "keypair.h"
#include "liquiditypool.h"
#include "strkey.h"
#include "sorobandatabuilder.h"
#include "managesellofferoperation.h"
#include "memo.h"
#include "network.h"
#include "operation.h"
#include "paymentoperation.h"
#include "price.h"
#include "transaction.h"
#include "abstracttransaction.h"
#include "feebumptransaction.h"
#include "transactionbuilder.h"
#include "txrep.h"

QSTELLAR_BEGIN_NS

namespace {

QString str(const QVariantMap& m, const char* key)
{
    return m.value(QString::fromLatin1(key)).toString();
}

/** Required field, with a message that names what is missing. */
QString need(const QVariantMap& m, const char* key)
{
    const QString v = str(m, key);
    if (v.isEmpty())
        throw std::runtime_error(QStringLiteral("missing field: %1")
                                     .arg(QString::fromLatin1(key)).toStdString());
    return v;
}

Memo* memoFrom(const QVariantMap& spec)
{
    // A memo is { type, value } or nothing. Anything else — "12345", 12345,
    // { id: "12345" }, a memo with no type — used to read as "no memo": an
    // exchange deposit sent without the memo that says whose it is.
    const QVariant given = spec.value(QStringLiteral("memo"));
    if (spec.contains(QStringLiteral("memo")) && !given.isValid())
        throw std::runtime_error("memo cannot be read; leave it out, or give { type, value }");
    if (isMissing(given))
        return nullptr;
    if (given.userType() != QMetaType::QVariantMap)
        throw std::runtime_error("memo must be { type, value }");
    const QVariantMap m = given.toMap();
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
        if (it.key() != QLatin1String("type") && it.key() != QLatin1String("value"))
            throw std::runtime_error(QStringLiteral("memo has no field \"%1\": it is { type, value }")
                                         .arg(it.key()).toStdString());
    }
    const QString type = str(m, "type").toLower();
    if (type.isEmpty() || type == QLatin1String("none")) {
        const QVariant value = m.value(QStringLiteral("value"));
        const bool hasValue = !isMissing(value)
                              && !(value.userType() == QMetaType::QString
                                   && value.toString().trimmed().isEmpty());
        if (hasValue)
            throw std::runtime_error("a memo with a value needs a type: text, id, hash or return");
        return nullptr;
    }
    const QString value = str(m, "value");
    if (type == QLatin1String("text")) {
        // A number is written out exactly, or refused. QVariant::toString()
        // prints a JS double in shortest form, so a deposit reference of
        // 1000000 went out as the text "1e+06" (and 100000 as "1e+05" on Qt 6
        // but not on Qt 5).
        const QString text = exactText(m.value(QStringLiteral("value")),
                                       QStringLiteral("memo text"));
        // Empty is refused on both Qt versions: Qt 5 hands JS "" over as a
        // null string, which Memo::text refused, while Qt 6 signed an empty
        // MEMO_TEXT. A memo with nothing in it is memo type none.
        if (text.isEmpty())
            throw std::runtime_error("memo text is empty: use memo type none for no memo");
        return Memo::text(text);
    }
    if (type == QLatin1String("id")) {
        // Read exactly or refused. Discarding toULongLong()'s failure flag
        // once turned anything that was not a plain uint64 into memo id 0;
        // reading a JS number above 2^53 turned it into a different id. Either
        // way a deposit to an exchange is credited to nobody, or to somebody
        // else. Memo ids that large travel as text.
        return Memo::id(exactUnsigned(m.value(QStringLiteral("value")),
                                      QStringLiteral("memo id"),
                                      std::numeric_limits<quint64>::max()));
    }
    if (type == QLatin1String("hash") || type == QLatin1String("return")) {
        // Exactly 32 bytes (Memo::hash refuses anything else too; the QML
        // messages here name the field). An empty field is no hash.
        const QString hex = value.trimmed();
        if (hex.isEmpty())
            throw std::runtime_error("memo hash is empty: use memo type none for no memo");
        if (hex.size() != 64)
            throw std::runtime_error("memo hash must be 64 hex characters (32 bytes)");
        return type == QLatin1String("hash") ? static_cast<Memo*>(Memo::hash(hex))
                                             : static_cast<Memo*>(Memo::returnHash(hex));
    }
    throw std::runtime_error(QStringLiteral("unknown memo type: %1").arg(type).toStdString());
}

/**
 * The network a spec refers to.
 *
 * An explicit `networkPassphrase` wins over the friendly `network` name so a
 * caller working against a standalone core is not stuck with the two
 * well-known ones.
 */
Network* networkFrom(const QVariantMap& spec)
{
    return qmlNetwork(spec.value(QStringLiteral("network")).toString(),
                      str(spec, "networkPassphrase"));
}

/**
 * An optional whole-number field of the transaction: left out (absent, JS
 * undefined or null) means `fallback`; given means exactly a whole number from
 * `low` to `high` (see exactSigned), or an error.
 *
 * Dropping QVariant's failure flags is how a base fee of -5 became 4294967291
 * stroops per operation and a blank timeout became TIMEOUT_INFINITE — a
 * transaction that never expires. A value the caller did give is never
 * replaced by a default: a blank field is an error, not a request for one,
 * on Qt 5 (where JS "" arrives as a null QString) as on Qt 6.
 */
qint64 optionalWhole(const QVariantMap& spec, const char* key, qint64 fallback,
                     qint64 low, qint64 high)
{
    const QVariant raw = spec.value(QString::fromLatin1(key));
    // Present but unreadable — a JS function on Qt 5 — is not "left out".
    if (spec.contains(QString::fromLatin1(key)) && !raw.isValid())
        throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                     .arg(QString::fromLatin1(key)).toStdString());
    if (isMissing(raw))
        return fallback;
    return exactSigned(raw, QString::fromLatin1(key), low, high);
}

/** Runs `fn` and reports failure the way QML can consume: a map with "error". */
template <typename Fn>
QVariantMap guarded(Fn fn)
{
    try {
        return fn();
    } catch (const std::exception& e) {
        QVariantMap out;
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
        return out;
    } catch (...) {
        QVariantMap out;
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
        return out;
    }
}

/** The envelope's type, by its XDR name: a V0 one is not a V1 one. */
QString envelopeTypeName(AbstractTransaction* tx)
{
    switch (tx->toEnvelopeXdr().type) {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
        return QStringLiteral("ENVELOPE_TYPE_TX_V0");
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
        return QStringLiteral("ENVELOPE_TYPE_TX");
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        return QStringLiteral("ENVELOPE_TYPE_TX_FEE_BUMP");
    default:
        throw std::runtime_error("not a transaction envelope");
    }
}

} // namespace

TxBuilderQml::TxBuilderQml(AssetRegistry* assets, QObject* parent)
    : QObject(parent), m_assets(assets)
{
}

QStringList TxBuilderQml::supportedOperations() const
{
    QStringList types;
    const QVector<OpSpec>& table = operationTable();
    types.reserve(table.size());
    for (const OpSpec& spec : table)
        types.append(QString::fromLatin1(spec.type));
    return types;
}

QVariantList TxBuilderQml::fieldsFor(const QString& operationType) const
{
    QVariantList out;
    const OpSpec* spec = operationSpec(operationType);
    if (!spec)
        return out;

    for (const FieldSpec& field : spec->fields) {
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), QString::fromLatin1(field.name));
        entry.insert(QStringLiteral("kind"), QString::fromLatin1(field.kind));
        entry.insert(QStringLiteral("label"), QString::fromLatin1(field.label));
        entry.insert(QStringLiteral("required"), field.required);
        entry.insert(QStringLiteral("note"), QString::fromUtf8(field.note));
        out.append(entry);
    }
    return out;
}

QString TxBuilderQml::describeOperation(const QString& operationType) const
{
    const OpSpec* spec = operationSpec(operationType);
    if (!spec)
        return QString();
    QString summary = QString::fromUtf8(spec->summary);
    if (spec->deprecated)
        summary += QStringLiteral(" (deprecated \u2014 kept so old transactions can be read)");
    return summary;
}

QString TxBuilderQml::demoValueFor(const QString& operationType,
                                   const QString& fieldName) const
{
    const OpSpec* spec = operationSpec(operationType);
    if (!spec)
        return QString();
    for (const FieldSpec& field : spec->fields) {
        if (fieldName == QLatin1String(field.name))
            return QString::fromUtf8(field.demo);
    }
    return QString();
}

QVariantMap TxBuilderQml::build(const QVariantMap& spec) const
{
    QVariantMap out;
    try {
        // Every key must be one this reads. An unknown one used to be ignored,
        // so a fee under the wrong name ("fee", "base_fee") left the default
        // in place and was never mentioned.
        static const QStringList known{
            QStringLiteral("network"), QStringLiteral("networkPassphrase"),
            QStringLiteral("sourceAccount"), QStringLiteral("sequence"),
            QStringLiteral("baseFee"), QStringLiteral("timeout"), QStringLiteral("memo"),
            QStringLiteral("operations"), QStringLiteral("sorobanData")};
        for (auto it = spec.constBegin(); it != spec.constEnd(); ++it) {
            if (!known.contains(it.key()))
                throw std::runtime_error(QStringLiteral("a transaction spec has no field \"%1\"")
                                             .arg(it.key()).toStdString());
        }

        Network* network = networkFrom(spec);

        // G... or a muxed M..., which is sent as the muxed source (as js does;
        // the builder used to drop the muxed id, so it was refused here).
        const QString source = need(spec, "sourceAccount");
        // A JS number cannot hold an int64 exactly, so the sequence should
        // arrive as text; a number is accepted only while it is still exact.
        const qint64 sequence = exactSigned(spec.value(QStringLiteral("sequence")),
                                            QStringLiteral("sequence"), 1,
                                            std::numeric_limits<qint64>::max());

        // The sequence handed in is the one that must appear in the envelope.
        //
        // TransactionBuilder takes an Account and calls
        // getIncrementedSequenceNumber() on it, which is right for a caller
        // holding a live Account but wrong for this API: here the number came
        // from a text field, or straight from Horizon.nextSequence, and a user
        // who types 42 and reads 43 back out of the XDR has been lied to.
        // Feeding the builder sequence-1 makes what you pass and what you get
        // the same number.
        if (sequence < 1)
            throw std::runtime_error("sequence must be 1 or greater");
        Account account(source, sequence - 1);
        TransactionBuilder builder(&account, network);

        // The fee per operation, in stroops. The XDR field is a uint32, and so
        // is the total: TransactionBuilder refuses a total that does not fit
        // rather than let the envelope carry a different fee from the one
        // reported here.
        // 0 up: a fee bump's inner transaction carries less than the
        // network minimum (commonly 0), and js and py build it.
        const qint64 baseFee = optionalWhole(spec, "baseFee", TransactionBuilder::BASE_FEE, 0,
                                             std::numeric_limits<quint32>::max());
        builder.setBaseFee(static_cast<quint32>(baseFee));

        // Seconds until the transaction expires. 0 is TIMEOUT_INFINITE — no
        // expiry at all — so it has to be asked for explicitly, as 0 or "0";
        // only a missing key gets the default.
        const qint64 timeout = optionalWhole(spec, "timeout", 180, 0,
                                             std::numeric_limits<qint32>::max());
        builder.setTimeout(timeout);

        // A Soroban footprint, when the caller has one. Only the two
        // entry-management operations need to supply their own — for
        // everything else the simulation works it out and prepare() installs
        // it, which is why this is optional rather than part of the shape.
        const QString sorobanData = str(spec, "sorobanData");
        if (!sorobanData.isEmpty()) {
            const auto decoded = QByteArray::fromBase64Encoding(
                sorobanData.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
            if (!decoded)
                throw std::runtime_error("sorobanData is not valid base64");
            QDataStream stream(*decoded);
            stellar::SorobanTransactionData data;
            stream >> data;
            if (stream.status() != QDataStream::Ok || !stream.atEnd())
                throw std::runtime_error("sorobanData is not a SorobanTransactionData");
            builder.setSorobanData(data);
        }

        if (Memo* memo = memoFrom(spec))
            builder.addMemo(memo);          // builder takes ownership

        const QVariantList ops = spec.value(QStringLiteral("operations")).toList();
        if (ops.isEmpty())
            throw std::runtime_error("a transaction needs at least one operation");

        for (const QVariant& entry : ops) {
            // One place builds an operation, and it is not this file: see
            // operationfactory.cpp. Keeping them apart is what lets this
            // function stay about sequence numbers, fees and memos while the
            // operation list grows with the protocol.
            builder.addOperation(makeOperation(entry.toMap(), m_assets));
        }

        QScopedPointer<Transaction> tx(builder.build());
        out.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("hash"), tx->hashHex());

        // TxRep is a VIEW, so its absence must not fail the build.
        //
        // SEP-11 does not cover every operation the network does — the Soroban
        // ones — and the emitter throws on those. Letting that throw out of
        // here would mean an operation the SDK can build correctly, whose XDR
        // and hash are right in front of us, reported to the user as if it
        // could not be built at all.
        try {
            out.insert(QStringLiteral("txrep"), Txrep::toTxrep(tx.data()));
        } catch (const std::exception& e) {
            out.insert(QStringLiteral("txrep"), QString());
            out.insert(QStringLiteral("txrepUnavailable"), QString::fromUtf8(e.what()));
        }
        out.insert(QStringLiteral("fee"), static_cast<qint64>(tx->getFee()));
        out.insert(QStringLiteral("operationCount"), ops.size());
    } catch (const std::exception& e) {
        // The SDK's own message, verbatim: it is what the lab exists to show.
        out.clear();
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
    } catch (...) {
        // A pointer-to-exception, which this SDK threw in six places until
        // recently and which `catch (const std::exception&)` does not match.
        out.clear();
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
    }
    return out;
}

QVariantMap TxBuilderQml::sign(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"xdr", "seeds", "network", "networkPassphrase"}, QStringLiteral("Tx.sign"));
        const QString xdr = need(args, "xdr");
        Network* network = networkFrom(args);

        //! [signSign.cpp]
        QScopedPointer<AbstractTransaction> tx(
            AbstractTransaction::fromEnvelopeXdr(xdr, network));
        //! [signSign.cpp]
        if (!tx)
            throw std::runtime_error("could not read the envelope XDR");

        const QVariantList seeds = args.value(QStringLiteral("seeds")).toList();
        if (seeds.isEmpty())
            throw std::runtime_error("no seeds to sign with");

        const int before = tx->getSignatures().size();
        for (const QVariant& entry : seeds) {
            const QString seed = entry.toString();
            if (seed.isEmpty())
                continue;
            // fromSecretSeed throws on a malformed seed, which guarded() turns
            // into an error map; the envelope is left untouched because the
            // signatures were being added to a copy that is about to die.
            QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(seed));
            // A key that has already signed is not added again: a second copy
            // of the same signature is an extra signature, and the network
            // refuses the transaction for it (txBAD_AUTH_EXTRA).
            const QByteArray hash = tx->hash();
            bool already = false;
            const stellar::SignatureHint hint = signer->getSignatureHint();
            for (const stellar::DecoratedSignature& s : tx->getSignatures()) {
                // The hint as well: core matches a signature to its signer by
                // the hint, so one under another key's hint does not count.
                if (std::memcmp(s.hint.signatureHint, hint.signatureHint, 4) == 0
                        && signer->verify(hash, s.signature.binary())) {
                    already = true;
                    break;
                }
            }
            if (!already)
                tx->sign(signer.data());
        }

        QVariantMap out;
        out.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("hash"), tx->hashHex());
        out.insert(QStringLiteral("signatureCount"), tx->getSignatures().size());
        out.insert(QStringLiteral("added"), tx->getSignatures().size() - before);
        return out;
    });
}

QVariantMap TxBuilderQml::inspect(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"xdr", "network", "networkPassphrase"}, QStringLiteral("Tx.inspect"));
        const QString xdr = need(args, "xdr");
        Network* network = networkFrom(args);

        QScopedPointer<AbstractTransaction> tx(
            AbstractTransaction::fromEnvelopeXdr(xdr, network));
        if (!tx)
            throw std::runtime_error("could not read the envelope XDR");

        QVariantMap out;
        out.insert(QStringLiteral("hash"), tx->hashHex());
        out.insert(QStringLiteral("signatureCount"), tx->getSignatures().size());
        out.insert(QStringLiteral("envelopeType"), envelopeTypeName(tx.data()));

        // A fee-bump envelope wraps another transaction; the distinction is the
        // first thing a reviewer needs, because the fee source is a different
        // account from the one that signed the inner operations.
        auto* plain = dynamic_cast<Transaction*>(tx.data());
        out.insert(QStringLiteral("isFeeBump"), plain == nullptr);

        // The document of the envelope itself, whatever its type, signatures
        // included: for a fee bump, the fee bump's (its fee source and fee,
        // the transaction it wraps, both signature lists), never the inner
        // transaction's, which is not what is signed. A fee bump used to have
        // none at all.
        //
        // The emitter may still refuse: SEP-11 does not describe the Soroban
        // operations. That is a missing VIEW of a perfectly readable
        // envelope, so it must not fail the inspection — the hash and the
        // operation count are exactly what a reviewer needs most when the
        // human-readable form is the thing that is unavailable.
        try {
            out.insert(QStringLiteral("txrep"), Txrep::toTxrep(tx.data()));
        } catch (const std::exception& e) {
            out.insert(QStringLiteral("txrep"), QString());
            out.insert(QStringLiteral("txrepUnavailable"), QString::fromUtf8(e.what()));
        }
        if (plain) {
            out.insert(QStringLiteral("sourceAccount"), plain->getSourceAccount());
            out.insert(QStringLiteral("sequence"),
                       QString::number(plain->getSequenceNumber()));
            out.insert(QStringLiteral("fee"), static_cast<qint64>(plain->getFee()));
            out.insert(QStringLiteral("operationCount"), plain->getOperations().size());
        } else if (auto* feeBump = dynamic_cast<FeeBumpTransaction*>(tx.data())) {
            // The account that pays and what it pays, then what it pays for,
            // each under its own name: the inner transaction's source is not
            // the envelope's, and its fee is not what is charged.
            Transaction* inner = feeBump->getInnerTransaction();
            out.insert(QStringLiteral("feeSource"), feeBump->getFeeAccount());
            out.insert(QStringLiteral("fee"), feeBump->getFee());
            out.insert(QStringLiteral("innerHash"), inner->hashHex());
            out.insert(QStringLiteral("innerSourceAccount"), inner->getSourceAccount());
            out.insert(QStringLiteral("innerSequence"),
                       QString::number(inner->getSequenceNumber()));
            out.insert(QStringLiteral("innerFee"), static_cast<qint64>(inner->getFee()));
            out.insert(QStringLiteral("operationCount"), inner->getOperations().size());
            out.insert(QStringLiteral("innerSignatureCount"), inner->getSignatures().size());
        }
        return out;
    });
}

QVariantMap TxBuilderQml::footprintForContract(const QString& contractId,
                                               const QString& durability,
                                               const QVariant& resourceFee) const
{
    QVariantMap out;
    try {
        const QByteArray raw = StrKey::decodeContract(contractId.trimmed());
        if (raw.size() != 32)
            throw std::runtime_error("a contract id is a C-address, 56 characters");

        stellar::SCAddress address;
        address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        std::memcpy(address.contractId, raw.constData(), 32);

        // SCV_LEDGER_KEY_CONTRACT_INSTANCE carries no payload: it is the
        // discriminator alone that names "this contract's own instance entry".
        stellar::SCVal key;
        key.type = stellar::SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE;

        stellar::SorobanLedgerKey entry;
        entry.type = stellar::LedgerEntryType::CONTRACT_DATA;
        entry.contractData.contract = address;
        entry.contractData.key = key;
        // One of two words: anything else ("tempory") used to mean persistent,
        // and the wrong entry was then extended or restored.
        const QString d = durability.trimmed().toLower();
        if (d != QLatin1String("temporary") && d != QLatin1String("persistent"))
            throw std::runtime_error("durability is temporary or persistent");
        entry.contractData.durability = d == QLatin1String("temporary")
                                            ? stellar::ContractDataDurability::TEMPORARY
                                            : stellar::ContractDataDurability::PERSISTENT;

        // The resource fee is the caller's to declare here, and for the two
        // entry-management operations it has to be: they run no host function,
        // so there is nothing for a simulation to measure — the RPC answers a
        // simulate request for one with "Could not unmarshal transaction",
        // which sounds like a malformed envelope and is not. Leave it at zero
        // for anything that WILL be simulated, since simulation adds the
        // minimum on top and a non-zero value here would be counted twice.
        //
        // Read as baseFee is: toLongLong() let "+5000" and "-0" through. No
        // transaction fee can be past the uint32 maximum, so neither can this.
        const qint64 fee = !resourceFee.isValid() || resourceFee.isNull()
            ? 0
            : exactSigned(resourceFee, QStringLiteral("resourceFee"), 0,
                          std::numeric_limits<quint32>::max());

        // Read-only: extending a lifetime does not change what the entry says.
        const stellar::SorobanTransactionData data = SorobanDataBuilder()
            .setReadOnly({ entry })
            .setResourceFee(fee)
            .build();

        // Whole or an error: this is handed to build() and signed, and a write
        // that failed part-way used to be handed over all the same.
        const QByteArray encoded = xdr::encodeOrThrow(data, "the footprint");

        out.insert(QStringLiteral("sorobanData"),
                   QString::fromLatin1(encoded.toBase64()));
        out.insert(QStringLiteral("contractId"), contractId.trimmed());
        out.insert(QStringLiteral("resourceFee"), QString::number(fee));
        out.insert(QStringLiteral("durability"),
                   entry.contractData.durability
                           == stellar::ContractDataDurability::TEMPORARY
                       ? QStringLiteral("temporary")
                       : QStringLiteral("persistent"));
    } catch (const std::exception& e) {
        out.clear();
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
    } catch (...) {
        out.clear();
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
    }
    return out;
}

QVariantMap TxBuilderQml::liquidityPoolId(const QString& assetA,
                                          const QString& assetB, const QVariant& feeValue) const
{
    QVariantMap out;
    try {
        // Exactly: an int parameter took 30.5 or 2^32 + 30 as 30.
        const qint64 fee = exactSigned(feeValue, QStringLiteral("fee"), std::numeric_limits<qint64>::min(),
                                       std::numeric_limits<qint64>::max());
        // The registry keeps ownership of these; getLiquidityPoolID only reads
        // them to serialise the parameters it hashes.
        // 30 is the only pool fee there is; an id for any other names no pool.
        if (fee != 30)
            throw std::runtime_error("the pool fee is 30 (the only fee a pool has)");
        Asset* a = m_assets->assetFor(assetA.trimmed());
        Asset* b = m_assets->assetFor(assetB.trimmed());
        const QByteArray id = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            a, b, qint32(fee));

        out.insert(QStringLiteral("poolId"), QString::fromLatin1(id.toHex()));
        out.insert(QStringLiteral("assetA"), assetA.trimmed());
        out.insert(QStringLiteral("assetB"), assetB.trimmed());
        out.insert(QStringLiteral("fee"), int(fee));
    } catch (const std::exception& e) {
        // Almost always the ordering rule, and the SDK's own wording says so
        // better than anything invented here would.
        out.clear();
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
    } catch (...) {
        out.clear();
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
    }
    return out;
}

QSTELLAR_END_NS
