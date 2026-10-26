#ifndef SCV_H
#define SCV_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "xdr/scval.h"
#include "../stellaraddress.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS

namespace scvdetail {
/** An integer that is not bool: what the checked factories take. */
template <typename T>
struct IsInteger : std::integral_constant<bool, std::is_integral<T>::value && !std::is_same<T, bool>::value> {};
/** ...or bytes, for the factories that also take a big-endian QByteArray. */
template <typename T>
struct IsIntegerOrBytes : std::integral_constant<bool, IsInteger<T>::value || std::is_convertible<T, QByteArray>::value> {};
}

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
    //
    // Each takes its exact type, and any other integer type range-checked:
    // a value the SCVal cannot hold throws std::out_of_range (as java's
    // Scv does) instead of being narrowed by C++'s implicit conversion —
    // qint64 4294967296 passed to toUint32 used to be signed as u32 0, and -1
    // passed to toUint64 as 18446744073709551615, without a warning. Floating
    // point arguments do not compile, and neither do bool, enums or classes
    // that convert (qfloat16, std::atomic...): they narrowed silently too.
    static stellar::SCVal toUint32(quint32 n);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toUint32(T n) { return toUint32(checkedNarrow<quint32>(n, "u32")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toUint32(T n) = delete;
    static quint32 fromUint32(const stellar::SCVal& v);

    static stellar::SCVal toInt32(qint32 n);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toInt32(T n) { return toInt32(checkedNarrow<qint32>(n, "i32")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toInt32(T n) = delete;
    static qint32 fromInt32(const stellar::SCVal& v);

    static stellar::SCVal toUint64(quint64 n);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toUint64(T n) { return toUint64(checkedNarrow<quint64>(n, "u64")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toUint64(T n) = delete;
    static quint64 fromUint64(const stellar::SCVal& v);

    static stellar::SCVal toInt64(qint64 n);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toInt64(T n) { return toInt64(checkedNarrow<qint64>(n, "i64")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toInt64(T n) = delete;
    static qint64 fromInt64(const stellar::SCVal& v);

    static stellar::SCVal toTimePoint(quint64 t);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toTimePoint(T t) { return toTimePoint(checkedNarrow<quint64>(t, "timepoint")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toTimePoint(T t) = delete;
    static quint64 fromTimePoint(const stellar::SCVal& v);

    static stellar::SCVal toDuration(quint64 d);
    template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toDuration(T d) { return toDuration(checkedNarrow<quint64>(d, "duration")); }
    template <typename T, typename std::enable_if<!std::is_integral<T>::value || std::is_same<T, bool>::value, int>::type = 0>
    static stellar::SCVal toDuration(T d) = delete;
    static quint64 fromDuration(const stellar::SCVal& v);

    /** `n` as `Target`, or std::out_of_range when `Target` cannot hold it. */
    template <typename Target, typename T>
    static Target checkedNarrow(T n, const char* what)
    {
        // The checks below go through long long: a wider integer (GCC's
        // __int128) would be cut down before it is compared.
        static_assert(sizeof(T) <= sizeof(long long), "integer wider than 64 bits");
        const bool negative = std::is_signed<T>::value && n < T(0);
        bool fits;
        if (negative) {
            fits = std::is_signed<Target>::value
                   && static_cast<long long>(n) >= static_cast<long long>(std::numeric_limits<Target>::min());
        } else {
            fits = static_cast<unsigned long long>(n)
                   <= static_cast<unsigned long long>(std::numeric_limits<Target>::max());
        }
        if (!fits)
            throw std::out_of_range(std::string(what) + " cannot hold " + std::to_string(n));
        return static_cast<Target>(n);
    }

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
    //
    // toInt128(n) / toUint128(n) take a 64-bit (or narrower) integer as a
    // VALUE: a negative i128 is sign-extended into hi, and a negative u128 is
    // refused. They are the way to build a token amount; the Parts factories
    // take the two raw words and are range-checked like toUint32 (a negative
    // `lo` or u128 `hi` throws std::out_of_range, as js refuses it, instead
    // of being signed as a number near 2^64). Floating point, bool and enum
    // arguments do not compile anywhere here: 19.99 * 1e7 tokens used to be
    // signed one base unit short.

    static stellar::SCVal toUint128(const QByteArray& bigEndian16);
    static stellar::SCVal toUint128(quint64 n);
    template <typename T, typename std::enable_if<scvdetail::IsInteger<T>::value, int>::type = 0>
    static stellar::SCVal toUint128(T n) { return toUint128(checkedNarrow<quint64>(n, "u128")); }
    template <typename T, typename std::enable_if<!scvdetail::IsIntegerOrBytes<T>::value, int>::type = 0>
    static stellar::SCVal toUint128(T n) = delete;
    static stellar::SCVal toUint128Parts(quint64 hi, quint64 lo);
    template <typename H, typename L, typename std::enable_if<scvdetail::IsInteger<H>::value && scvdetail::IsInteger<L>::value, int>::type = 0>
    static stellar::SCVal toUint128Parts(H hi, L lo)
    {
        return toUint128Parts(checkedNarrow<quint64>(hi, "u128 hi"), checkedNarrow<quint64>(lo, "u128 lo"));
    }
    template <typename H, typename L, typename std::enable_if<!(scvdetail::IsInteger<H>::value && scvdetail::IsInteger<L>::value), int>::type = 0>
    static stellar::SCVal toUint128Parts(H hi, L lo) = delete;
    static QByteArray fromUint128(const stellar::SCVal& v);

    static stellar::SCVal toInt128(const QByteArray& bigEndian16);
    static stellar::SCVal toInt128(qint64 n);
    template <typename T, typename std::enable_if<scvdetail::IsInteger<T>::value, int>::type = 0>
    static stellar::SCVal toInt128(T n)
    {
        // Every 64-bit value fits an i128, including a u64 above 2^63-1.
        if (std::is_signed<T>::value)
            return toInt128(checkedNarrow<qint64>(n, "i128"));
        return toInt128Parts(qint64(0), checkedNarrow<quint64>(n, "i128"));
    }
    template <typename T, typename std::enable_if<!scvdetail::IsIntegerOrBytes<T>::value, int>::type = 0>
    static stellar::SCVal toInt128(T n) = delete;
    static stellar::SCVal toInt128Parts(qint64 hi, quint64 lo);
    template <typename H, typename L, typename std::enable_if<scvdetail::IsInteger<H>::value && scvdetail::IsInteger<L>::value, int>::type = 0>
    static stellar::SCVal toInt128Parts(H hi, L lo)
    {
        return toInt128Parts(checkedNarrow<qint64>(hi, "i128 hi"), checkedNarrow<quint64>(lo, "i128 lo"));
    }
    template <typename H, typename L, typename std::enable_if<!(scvdetail::IsInteger<H>::value && scvdetail::IsInteger<L>::value), int>::type = 0>
    static stellar::SCVal toInt128Parts(H hi, L lo) = delete;
    static QByteArray fromInt128(const stellar::SCVal& v);

    static stellar::SCVal toUint256(const QByteArray& bigEndian32);
    static QByteArray fromUint256(const stellar::SCVal& v);

    static stellar::SCVal toInt256(const QByteArray& bigEndian32);
    static QByteArray fromInt256(const stellar::SCVal& v);

    /**
     * A u128 / i128 / u256 / i256 as exact decimal text, e.g.
     * "-170141183460469231731687303715884105728": the words are read as one
     * big-endian two's-complement integer, with no floating point anywhere.
     * Throws std::runtime_error for any other type.
     */
    static QString decimalText(const stellar::SCVal& v);

    // ─── Vec / Map (CAP-46) ──────────────────────────────────────────

    /** Builds an SCV_VEC. Elements are stored as-is (no ordering enforced). */
    static stellar::SCVal toVec(const QList<stellar::SCVal>& elements);
    static QList<stellar::SCVal> fromVec(const stellar::SCVal& v);

    /**
     * Builds an SCV_MAP. Entries are sorted by key in the order the Soroban
     * host requires (see compare()) — it refuses a map that is not — so two
     * semantically equal maps produce identical XDR. Duplicate keys throw.
     */
    static stellar::SCVal toMap(const QList<stellar::SCMapEntry>& entries);
    /** Convenience: builds the SCMapEntry list from parallel key/value lists. */
    static stellar::SCVal toMap(const QList<stellar::SCVal>& keys,
                                const QList<stellar::SCVal>& values);
    static QList<stellar::SCMapEntry> fromMap(const stellar::SCVal& v);

    /**
     * The Soroban host's order on SCVal: Rust's #[derive(Ord)] on ScVal, which
     * is what the host checks an SCMap's keys against (py stellar_sdk's
     * _compare_sc_val is the same rule). First by type; then signed integers
     * as numbers, bytes / string / symbol as raw bytes with no length prefix,
     * vec and map element by element (a prefix first, an absent one before
     * any present one). That is NOT the order of the XDR bytes: those put a
     * length before a symbol and store -1 as ffffffff, so {amount, to} and
     * {-1, 1} used to sort backwards and be refused on chain, fee paid.
     * Returns <0 if a<b, 0 if a==b (same XDR), >0 otherwise.
     */
    static int compare(const stellar::SCVal& a, const stellar::SCVal& b);

    // ─── Address / Error / NonceKey / ContractInstance (CAP-46) ─────

    /** Builds SCV_ADDRESS from any SEP-23 strkey (G / M / C / B / L). */
    static stellar::SCVal toAddress(const StellarAddress& addr);
    static stellar::SCVal toAddress(const QString& strkey);
    static stellar::SCVal toAddress(const stellar::SCAddress& xdr);
    static stellar::SCAddress fromAddressXdr(const stellar::SCVal& v);
    static StellarAddress fromAddress(const stellar::SCVal& v);

    /**
     * SCError factories. `contractCodeOrEnum` is any uint32 for SCE_CONTRACT
     * and an SCErrorCode for the other types. Throws std::runtime_error for a
     * type or code stellar-xdr has no arm for, which could not be encoded.
     */
    static stellar::SCVal toError(stellar::SCErrorType type, quint32 contractCodeOrEnum);
    static stellar::SCError fromError(const stellar::SCVal& v);

    /** SCV_LEDGER_KEY_NONCE — wraps an int64 nonce. Any other integer type is
     *  range-checked like toInt64: quint64 2^63 used to become INT64_MIN. */
    static stellar::SCVal toLedgerKeyNonce(qint64 nonce);
    template <typename T, typename std::enable_if<scvdetail::IsInteger<T>::value, int>::type = 0>
    static stellar::SCVal toLedgerKeyNonce(T n) { return toLedgerKeyNonce(checkedNarrow<qint64>(n, "nonce")); }
    template <typename T, typename std::enable_if<!scvdetail::IsInteger<T>::value, int>::type = 0>
    static stellar::SCVal toLedgerKeyNonce(T n) = delete;
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
