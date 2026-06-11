#ifndef SCV_H
#define SCV_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include "xdr/scval.h"
#include "../stellaraddress.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS


/**
 * CAP-46 — factories and accessors for `stellar::SCVal`. Mirrors the
 * Java SDK `org.stellar.sdk.scval.Scv` static helpers.
 *
 * Covers the full SCVal surface: primitives (Bool, Void, U32/I32, U64/I64,
 * TimePoint, Duration, Bytes, String, Symbol), wide integers (128/256),
 * Vec / Map, Address (SCV_ADDRESS), Error, ContractInstance, NonceKey and the
 * LedgerKey* variants.
 *
 * Symmetric API:
 *   stellar::SCVal v = Scv::toUint32(42);
 *   quint32 n = Scv::fromUint32(v);
 *
 * `fromX(v)` throws `std::runtime_error` if `v.type` is not the expected
 * variant.
 */
class Scv
{
public:
    // ─── Void / Boolean ──────────────────────────────────────────────
    static stellar::SCVal toVoid();
    static stellar::SCVal toBoolean(bool b);
    static bool fromBoolean(const stellar::SCVal& v);

    // ─── Fixed-width integers ────────────────────────────────────────
    static stellar::SCVal toUint32(quint32 n);
    static quint32 fromUint32(const stellar::SCVal& v);

    static stellar::SCVal toInt32(qint32 n);
    static qint32 fromInt32(const stellar::SCVal& v);

    static stellar::SCVal toUint64(quint64 n);
    static quint64 fromUint64(const stellar::SCVal& v);

    static stellar::SCVal toInt64(qint64 n);
    static qint64 fromInt64(const stellar::SCVal& v);

    static stellar::SCVal toTimePoint(quint64 t);
    static quint64 fromTimePoint(const stellar::SCVal& v);

    static stellar::SCVal toDuration(quint64 d);
    static quint64 fromDuration(const stellar::SCVal& v);

    // ─── Variable-length byte payloads ───────────────────────────────
    static stellar::SCVal toBytes(const QByteArray& data);
    static QByteArray fromBytes(const stellar::SCVal& v);

    /** UTF-8 string variant. */
    static stellar::SCVal toString(const QString& s);
    static stellar::SCVal toString(const QByteArray& raw);
    static QString fromString(const stellar::SCVal& v);

    /** Symbol — ASCII / restricted charset, max 32 bytes. */
    static stellar::SCVal toSymbol(const QString& s);
    static QString fromSymbol(const stellar::SCVal& v);

    // ─── 128 / 256-bit integers ──────────────────────────────────────
    //
    // QByteArray representation is canonical big-endian, two's complement
    // for signed types: 16 bytes for 128-bit, 32 bytes for 256-bit.
    // Inputs shorter than the expected length are sign-extended (signed)
    // or zero-extended (unsigned) from the left.

    static stellar::SCVal toUint128(const QByteArray& bigEndian16);
    static stellar::SCVal toUint128Parts(quint64 hi, quint64 lo);
    static QByteArray fromUint128(const stellar::SCVal& v);

    static stellar::SCVal toInt128(const QByteArray& bigEndian16);
    static stellar::SCVal toInt128Parts(qint64 hi, quint64 lo);
    static QByteArray fromInt128(const stellar::SCVal& v);

    static stellar::SCVal toUint256(const QByteArray& bigEndian32);
    static QByteArray fromUint256(const stellar::SCVal& v);

    static stellar::SCVal toInt256(const QByteArray& bigEndian32);
    static QByteArray fromInt256(const stellar::SCVal& v);

    // ─── Vec / Map (CAP-46) ──────────────────────────────────────────

    /** Builds an SCV_VEC. Elements are stored as-is (no ordering enforced). */
    static stellar::SCVal toVec(const QList<stellar::SCVal>& elements);
    static QList<stellar::SCVal> fromVec(const stellar::SCVal& v);

    /**
     * Builds an SCV_MAP. Entries are reordered into the canonical Soroban
     * ordering (by serialized key, ascending) so two semantically equal
     * maps produce identical XDR.
     */
    static stellar::SCVal toMap(const QList<stellar::SCMapEntry>& entries);
    /** Convenience: builds the SCMapEntry list from parallel key/value lists. */
    static stellar::SCVal toMap(const QList<stellar::SCVal>& keys,
                                const QList<stellar::SCVal>& values);
    static QList<stellar::SCMapEntry> fromMap(const stellar::SCVal& v);

    /**
     * Canonical comparator for SCVal: by XDR-byte lexicographic order.
     * Returns <0 if a<b, 0 if a==b, >0 otherwise. Matches the comparator
     * Soroban core uses to canonicalize maps.
     */
    static int compare(const stellar::SCVal& a, const stellar::SCVal& b);

    // ─── Address / Error / NonceKey / ContractInstance (CAP-46) ─────

    /** Builds SCV_ADDRESS from any SEP-23 strkey (G / M / C / B / L). */
    static stellar::SCVal toAddress(const StellarAddress& addr);
    static stellar::SCVal toAddress(const QString& strkey);
    static stellar::SCVal toAddress(const stellar::SCAddress& xdr);
    static stellar::SCAddress fromAddressXdr(const stellar::SCVal& v);
    static StellarAddress fromAddress(const stellar::SCVal& v);

    /** SCError factories. */
    static stellar::SCVal toError(stellar::SCErrorType type, quint32 contractCodeOrEnum);
    static stellar::SCError fromError(const stellar::SCVal& v);

    /** SCV_LEDGER_KEY_NONCE — wraps an int64 nonce. */
    static stellar::SCVal toLedgerKeyNonce(qint64 nonce);
    static qint64 fromLedgerKeyNonce(const stellar::SCVal& v);

    /** SCV_LEDGER_KEY_CONTRACT_INSTANCE has no payload. */
    static stellar::SCVal toLedgerKeyContractInstance();

    /** SCV_CONTRACT_INSTANCE. `storage` may be a SCV_MAP or default-SCV_VOID
     *  to omit the optional. */
    static stellar::SCVal toContractInstance(const stellar::ContractExecutable& executable,
                                             const stellar::SCVal& storage = stellar::SCVal());
    static stellar::SCContractInstance fromContractInstance(const stellar::SCVal& v);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Scv)

#endif // SCV_H
