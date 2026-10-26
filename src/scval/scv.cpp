#include "scv.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include "../stellaraddress.h"
#include "../util.h"

QSTELLAR_BEGIN_NS


namespace {

void requireType(const stellar::SCVal& v, stellar::SCValType expected)
{
    if (v.type != expected) {
        throw std::runtime_error("SCVal type mismatch");
    }
}

} // namespace

// ─── Void / Boolean ──────────────────────────────────────────────────

stellar::SCVal Scv::toVoid()
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_VOID;
    return v;
}

stellar::SCVal Scv::toBoolean(bool b)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_BOOL;
    v.b = b;
    return v;
}

bool Scv::fromBoolean(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_BOOL);
    return v.b;
}

// ─── Fixed-width integers ────────────────────────────────────────────

stellar::SCVal Scv::toUint32(quint32 n)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_U32;
    v.u32 = n;
    return v;
}

quint32 Scv::fromUint32(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_U32);
    return v.u32;
}

stellar::SCVal Scv::toInt32(qint32 n)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_I32;
    v.i32 = n;
    return v;
}

qint32 Scv::fromInt32(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_I32);
    return v.i32;
}

stellar::SCVal Scv::toUint64(quint64 n)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_U64;
    v.u64 = n;
    return v;
}

quint64 Scv::fromUint64(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_U64);
    return v.u64;
}

stellar::SCVal Scv::toInt64(qint64 n)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_I64;
    v.i64 = n;
    return v;
}

qint64 Scv::fromInt64(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_I64);
    return v.i64;
}

stellar::SCVal Scv::toTimePoint(quint64 t)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_TIMEPOINT;
    v.timepoint = t;
    return v;
}

quint64 Scv::fromTimePoint(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_TIMEPOINT);
    return v.timepoint;
}

stellar::SCVal Scv::toDuration(quint64 d)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_DURATION;
    v.duration = d;
    return v;
}

quint64 Scv::fromDuration(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_DURATION);
    return v.duration;
}

// ─── Variable-length byte payloads ───────────────────────────────────

stellar::SCVal Scv::toBytes(const QByteArray& data)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_BYTES;
    v.bytes = data;
    return v;
}

QByteArray Scv::fromBytes(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_BYTES);
    return v.bytes;
}

stellar::SCVal Scv::toString(const QString& s)
{
    // Text UTF-8 cannot hold as given (an unpaired surrogate) is refused: it
    // was signed with the surrogate dropped on Qt 6 and as '?' on Qt 5.
    const QByteArray utf8 = s.toUtf8();
    if (Util::hasUnpairedSurrogate(s))
        throw std::runtime_error("string is not valid Unicode (an unpaired surrogate?)");
    return toString(utf8);
}

stellar::SCVal Scv::toString(const QByteArray& raw)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_STRING;
    v.str = raw;
    return v;
}

QString Scv::fromString(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_STRING);
    return QString::fromUtf8(v.str);
}

stellar::SCVal Scv::toSymbol(const QString& s)
{
    QByteArray utf8 = s.toUtf8();
    if (utf8.size() > 32) {
        throw std::runtime_error("SCSymbol exceeds 32 bytes");
    }
    // The host accepts only [a-zA-Z0-9_]; anything else would be signed and
    // then fail on chain, fee paid.
    for (const char c : utf8) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
            throw std::runtime_error("a symbol may contain only a-z, A-Z, 0-9 and _");
    }
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_SYMBOL;
    v.symbol = utf8;
    return v;
}

QString Scv::fromSymbol(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_SYMBOL);
    return QString::fromUtf8(v.symbol);
}

// ─── 128 / 256-bit integers ──────────────────────────────────────────

namespace {

// Pads `in` to `target` bytes from the left. `signed_` controls the fill
// byte for sign-extension (0xFF when the top bit of the input is set).
QByteArray padToBigEndian(const QByteArray& in, int target, bool signed_)
{
    // No bytes is not zero: it is what a failed QByteArray::fromHex gives, and
    // it used to become a zero amount.
    if (in.isEmpty())
        throw std::runtime_error("a 128/256-bit integer needs at least one byte");
    if (in.size() == target) return in;
    if (in.size() > target) {
        // Accept a leading 0 byte (or 0xFF for signed-extension cases) so
        // a 17-byte unsigned positive integer still fits a 128-bit slot.
        const int extra = in.size() - target;
        const bool extendingByte = (signed_ && !in.isEmpty()
            && (static_cast<unsigned char>(in.at(extra)) & 0x80));
        for (int i = 0; i < extra; ++i) {
            const unsigned char b = static_cast<unsigned char>(in.at(i));
            const bool ok = signed_ ? (b == (extendingByte ? 0xFF : 0x00))
                                    : (b == 0x00);
            if (!ok) {
                throw std::runtime_error("big-endian payload doesn't fit target width");
            }
        }
        return in.right(target);
    }
    const unsigned char fill = (signed_ && !in.isEmpty()
        && (static_cast<unsigned char>(in.at(0)) & 0x80)) ? 0xFF : 0x00;
    QByteArray out(target - in.size(), static_cast<char>(fill));
    out.append(in);
    return out;
}

void splitBe128(const QByteArray& be16, quint64& hi, quint64& lo)
{
    hi = 0; lo = 0;
    for (int i = 0; i < 8; ++i) {
        hi = (hi << 8) | static_cast<unsigned char>(be16.at(i));
        lo = (lo << 8) | static_cast<unsigned char>(be16.at(i + 8));
    }
}

QByteArray joinBe128(quint64 hi, quint64 lo)
{
    QByteArray out(16, '\0');
    for (int i = 7; i >= 0; --i) {
        out[i]     = static_cast<char>(hi & 0xFF); hi >>= 8;
        out[i + 8] = static_cast<char>(lo & 0xFF); lo >>= 8;
    }
    return out;
}

void splitBe256(const QByteArray& be32, quint64& hiHi, quint64& hiLo,
                quint64& loHi, quint64& loLo)
{
    quint64* cells[4] = {&hiHi, &hiLo, &loHi, &loLo};
    for (int s = 0; s < 4; ++s) {
        quint64 acc = 0;
        for (int i = 0; i < 8; ++i) {
            acc = (acc << 8) | static_cast<unsigned char>(be32.at(s * 8 + i));
        }
        *cells[s] = acc;
    }
}

QByteArray joinBe256(quint64 hiHi, quint64 hiLo, quint64 loHi, quint64 loLo)
{
    QByteArray out(32, '\0');
    quint64 vals[4] = {hiHi, hiLo, loHi, loLo};
    for (int s = 0; s < 4; ++s) {
        quint64 v = vals[s];
        for (int i = 7; i >= 0; --i) {
            out[s * 8 + i] = static_cast<char>(v & 0xFF);
            v >>= 8;
        }
    }
    return out;
}

} // namespace

stellar::SCVal Scv::toUint128(const QByteArray& bigEndian16)
{
    QByteArray be = padToBigEndian(bigEndian16, 16, false);
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_U128;
    splitBe128(be, v.u128.hi, v.u128.lo);
    return v;
}

stellar::SCVal Scv::toUint128(quint64 n)
{
    return toUint128Parts(quint64(0), n);
}

stellar::SCVal Scv::toUint128Parts(quint64 hi, quint64 lo)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_U128;
    v.u128.hi = hi;
    v.u128.lo = lo;
    return v;
}

QByteArray Scv::fromUint128(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_U128);
    return joinBe128(v.u128.hi, v.u128.lo);
}

stellar::SCVal Scv::toInt128(const QByteArray& bigEndian16)
{
    QByteArray be = padToBigEndian(bigEndian16, 16, true);
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_I128;
    quint64 hi, lo;
    splitBe128(be, hi, lo);
    v.i128.hi = static_cast<qint64>(hi);
    v.i128.lo = lo;
    return v;
}

stellar::SCVal Scv::toInt128(qint64 n)
{
    // Sign-extended: -100 is hi = -1, lo = 2^64 - 100, the bytes js encodes.
    return toInt128Parts(n < 0 ? qint64(-1) : qint64(0), static_cast<quint64>(n));
}

stellar::SCVal Scv::toInt128Parts(qint64 hi, quint64 lo)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_I128;
    v.i128.hi = hi;
    v.i128.lo = lo;
    return v;
}

QByteArray Scv::fromInt128(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_I128);
    return joinBe128(static_cast<quint64>(v.i128.hi), v.i128.lo);
}

stellar::SCVal Scv::toUint256(const QByteArray& bigEndian32)
{
    QByteArray be = padToBigEndian(bigEndian32, 32, false);
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_U256;
    splitBe256(be, v.u256.hiHi, v.u256.hiLo, v.u256.loHi, v.u256.loLo);
    return v;
}

QByteArray Scv::fromUint256(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_U256);
    return joinBe256(v.u256.hiHi, v.u256.hiLo, v.u256.loHi, v.u256.loLo);
}

stellar::SCVal Scv::toInt256(const QByteArray& bigEndian32)
{
    QByteArray be = padToBigEndian(bigEndian32, 32, true);
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_I256;
    quint64 hh, hl, lh, ll;
    splitBe256(be, hh, hl, lh, ll);
    v.i256.hiHi = static_cast<qint64>(hh);
    v.i256.hiLo = hl;
    v.i256.loHi = lh;
    v.i256.loLo = ll;
    return v;
}

QByteArray Scv::fromInt256(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_I256);
    return joinBe256(static_cast<quint64>(v.i256.hiHi),
                     v.i256.hiLo, v.i256.loHi, v.i256.loLo);
}

QString Scv::decimalText(const stellar::SCVal& v)
{
    QByteArray be;
    bool isSigned = false;
    switch (v.type) {
    case stellar::SCValType::SCV_U128: be = fromUint128(v); break;
    case stellar::SCValType::SCV_I128: be = fromInt128(v); isSigned = true; break;
    case stellar::SCValType::SCV_U256: be = fromUint256(v); break;
    case stellar::SCValType::SCV_I256: be = fromInt256(v); isSigned = true; break;
    default:
        throw std::runtime_error("decimalText takes a 128- or 256-bit integer");
    }
    unsigned char* bytes = reinterpret_cast<unsigned char*>(be.data());
    const int size = static_cast<int>(be.size());
    const bool negative = isSigned && (bytes[0] & 0x80);
    if (negative) {
        // Two's-complement magnitude: invert and add one. The minimum comes
        // back as itself, 2^127 or 2^255, which read unsigned is right.
        for (int i = 0; i < size; ++i) bytes[i] = static_cast<unsigned char>(~bytes[i]);
        for (int i = size - 1; i >= 0; --i) { if (++bytes[i] != 0) break; }
    }
    // Big-endian 32-bit words, divided by 10^9 until nothing is left; each
    // remainder is nine digits of the answer, least significant first.
    std::vector<quint32> words(static_cast<size_t>(size / 4), 0);
    for (int i = 0; i < size; ++i)
        words[static_cast<size_t>(i / 4)] = (words[static_cast<size_t>(i / 4)] << 8) | bytes[i];
    std::vector<quint32> chunks;
    for (;;) {
        bool zero = true;
        quint64 rem = 0;
        for (quint32& w : words) {
            const quint64 cur = (rem << 32) | w;
            w = static_cast<quint32>(cur / 1000000000u);
            rem = cur % 1000000000u;
            zero = zero && w == 0;
        }
        chunks.push_back(static_cast<quint32>(rem));
        if (zero) break;
    }
    QString out = negative ? QStringLiteral("-") : QString();
    out += QString::number(chunks.back());
    for (size_t i = chunks.size() - 1; i-- > 0;)
        out += QStringLiteral("%1").arg(chunks[i], 9, 10, QLatin1Char('0'));
    return out;
}

// ─── Vec / Map / comparator ──────────────────────────────────────────

namespace {

template <typename T>
int threeWay(const T& a, const T& b)
{
    return (b < a) - (a < b);
}

/** Raw bytes, lexicographically: a prefix sorts first (Rust's Ord on Vec<u8>). */
int compareBytes(const QByteArray& a, const QByteArray& b)
{
    const int common = static_cast<int>(std::min(a.size(), b.size()));
    const int c = common ? std::memcmp(a.constData(), b.constData(), static_cast<size_t>(common)) : 0;
    if (c != 0) return c < 0 ? -1 : 1;
    return threeWay(a.size(), b.size());
}

/** For the fixed-size unsigned encodings (addresses, errors, hashes) the XDR
 *  bytes are already in Ord order: the arm first, then the fields in order.
 *  Checked: a value with no encoding (an SCError of no known type) writes
 *  nothing, and unchecked, every such value compared equal to every other,
 *  so a map took two different ones for one duplicate key. */
template <typename T>
QByteArray encoded(const T& v)
{
    return xdr::encodeOrThrow(v, "the SCVal being compared");
}

/** An XDR optional (Rust's Option): absent before present. */
template <typename T, typename Compare>
int compareOptional(const T* a, const T* b, Compare compareValue)
{
    if (!a || !b) return threeWay(a != nullptr, b != nullptr);
    return compareValue(*a, *b);
}

int compareVec(const QList<stellar::SCVal>& a, const QList<stellar::SCVal>& b)
{
    const int common = static_cast<int>(std::min(a.size(), b.size()));
    for (int i = 0; i < common; ++i) {
        const int c = Scv::compare(a.at(i), b.at(i));
        if (c != 0) return c;
    }
    return threeWay(a.size(), b.size());
}

int compareEntries(const QList<stellar::SCMapEntry>& a, const QList<stellar::SCMapEntry>& b)
{
    const int common = static_cast<int>(std::min(a.size(), b.size()));
    for (int i = 0; i < common; ++i) {
        int c = Scv::compare(a.at(i).key, b.at(i).key);
        if (c == 0) c = Scv::compare(a.at(i).val, b.at(i).val);
        if (c != 0) return c;
    }
    return threeWay(a.size(), b.size());
}

/** Contract-instance storage: an optional map, held as an SCV_MAP when present. */
int compareStorage(const stellar::SCVal& a, const stellar::SCVal& b)
{
    const QList<stellar::SCMapEntry> none;
    return compareEntries(a.map ? *a.map : none, b.map ? *b.map : none);
}

int compareExecutable(const stellar::ContractExecutable& a, const stellar::ContractExecutable& b)
{
    const int c = threeWay(static_cast<qint32>(a.type), static_cast<qint32>(b.type));
    if (c != 0) return c;
    switch (a.type) {
    case stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM: {
        const int h = std::memcmp(a.wasmHash, b.wasmHash, sizeof a.wasmHash);
        return (h > 0) - (h < 0);
    }
    case stellar::ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF: {
        const int owner = compareBytes(encoded(a.externalRef.executableOwner),
                                       encoded(b.externalRef.executableOwner));
        return owner != 0 ? owner : compareBytes(a.externalRef.tag, b.externalRef.tag);
    }
    default:
        return 0;
    }
}

} // namespace

int Scv::compare(const stellar::SCVal& a, const stellar::SCVal& b)
{
    using T = stellar::SCValType;
    if (a.type != b.type)
        return threeWay(static_cast<qint32>(a.type), static_cast<qint32>(b.type));
    switch (a.type) {
    case T::SCV_VOID:
    case T::SCV_LEDGER_KEY_CONTRACT_INSTANCE:
        return 0;
    case T::SCV_BOOL:      return threeWay(a.b, b.b);
    case T::SCV_U32:       return threeWay(a.u32, b.u32);
    case T::SCV_I32:       return threeWay(a.i32, b.i32);
    case T::SCV_U64:       return threeWay(a.u64, b.u64);
    case T::SCV_I64:       return threeWay(a.i64, b.i64);
    case T::SCV_TIMEPOINT: return threeWay(a.timepoint, b.timepoint);
    case T::SCV_DURATION:  return threeWay(a.duration, b.duration);
    case T::SCV_LEDGER_KEY_NONCE: return threeWay(a.nonceKey.nonce, b.nonceKey.nonce);
    // Wide integers compare as tuples of their words, only the top one signed.
    case T::SCV_U128: {
        const int c = threeWay(a.u128.hi, b.u128.hi);
        return c != 0 ? c : threeWay(a.u128.lo, b.u128.lo);
    }
    case T::SCV_I128: {
        const int c = threeWay(a.i128.hi, b.i128.hi);
        return c != 0 ? c : threeWay(a.i128.lo, b.i128.lo);
    }
    case T::SCV_U256: {
        const quint64 x[4] = {a.u256.hiHi, a.u256.hiLo, a.u256.loHi, a.u256.loLo};
        const quint64 y[4] = {b.u256.hiHi, b.u256.hiLo, b.u256.loHi, b.u256.loLo};
        return std::lexicographical_compare(x, x + 4, y, y + 4) ? -1
             : std::lexicographical_compare(y, y + 4, x, x + 4) ? 1 : 0;
    }
    case T::SCV_I256: {
        const int c = threeWay(a.i256.hiHi, b.i256.hiHi);
        if (c != 0) return c;
        const quint64 x[3] = {a.i256.hiLo, a.i256.loHi, a.i256.loLo};
        const quint64 y[3] = {b.i256.hiLo, b.i256.loHi, b.i256.loLo};
        return std::lexicographical_compare(x, x + 3, y, y + 3) ? -1
             : std::lexicographical_compare(y, y + 3, x, x + 3) ? 1 : 0;
    }
    case T::SCV_BYTES:   return compareBytes(a.bytes, b.bytes);
    case T::SCV_STRING:  return compareBytes(a.str, b.str);
    case T::SCV_SYMBOL:  return compareBytes(a.symbol, b.symbol);
    case T::SCV_EXECUTABLE_TAG: return compareBytes(a.executableTag, b.executableTag);
    case T::SCV_VEC:     return compareOptional(a.vec.data(), b.vec.data(), compareVec);
    case T::SCV_MAP:     return compareOptional(a.map.data(), b.map.data(), compareEntries);
    case T::SCV_ADDRESS: return compareBytes(encoded(a.address), encoded(b.address));
    case T::SCV_ERROR:   return compareBytes(encoded(a.error), encoded(b.error));
    case T::SCV_CONTRACT_INSTANCE:
        return compareOptional(a.instance.data(), b.instance.data(),
                               [](const stellar::SCContractInstance& x, const stellar::SCContractInstance& y) {
            const int e = compareExecutable(x.executable, y.executable);
            return e != 0 ? e : compareOptional(x.storage.data(), y.storage.data(), compareStorage);
        });
    }
    throw std::runtime_error("SCVal compare: unknown type");
}

stellar::SCVal Scv::toVec(const QList<stellar::SCVal>& elements)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_VEC;
    v.vec = QSharedPointer<QList<stellar::SCVal>>::create(elements);
    return v;
}

QList<stellar::SCVal> Scv::fromVec(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_VEC);
    return v.vec ? *v.vec : QList<stellar::SCVal>();
}

stellar::SCVal Scv::toMap(const QList<stellar::SCMapEntry>& entries)
{
    // The host refuses an SCMap whose keys are not strictly ascending in its
    // own order, which compare() implements. compare() is 0 exactly when two
    // keys encode to the same XDR, so it also finds the duplicates.
    QList<stellar::SCMapEntry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const stellar::SCMapEntry& l, const stellar::SCMapEntry& r) {
                  return Scv::compare(l.key, r.key) < 0;
              });
    for (int i = 1; i < sorted.size(); ++i) {
        if (Scv::compare(sorted.at(i - 1).key, sorted.at(i).key) == 0) {
            throw std::runtime_error("SCMap contains duplicate keys");
        }
    }

    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_MAP;
    v.map = QSharedPointer<QList<stellar::SCMapEntry>>::create(sorted);
    return v;
}

stellar::SCVal Scv::toMap(const QList<stellar::SCVal>& keys,
                          const QList<stellar::SCVal>& values)
{
    if (keys.size() != values.size()) {
        throw std::runtime_error("SCMap: keys and values must have the same length");
    }
    QList<stellar::SCMapEntry> entries;
    entries.reserve(keys.size());
    for (int i = 0; i < keys.size(); ++i) {
        stellar::SCMapEntry e;
        e.key = keys.at(i);
        e.val = values.at(i);
        entries.append(e);
    }
    return toMap(entries);
}

QList<stellar::SCMapEntry> Scv::fromMap(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_MAP);
    return v.map ? *v.map : QList<stellar::SCMapEntry>();
}

// ─── Address / Error / NonceKey / ContractInstance (CAP-46) ──────────

stellar::SCVal Scv::toAddress(const stellar::SCAddress& xdr)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_ADDRESS;
    v.address = xdr;
    return v;
}

stellar::SCVal Scv::toAddress(const StellarAddress& addr)
{
    stellar::SCAddress xdr;
    const QByteArray& bytes = addr.getBytes();
    switch (addr.getType()) {
    case StellarAddress::Type::ACCOUNT:
        if (bytes.size() != 32) throw std::runtime_error("invalid ACCOUNT body");
        xdr.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        std::memcpy(xdr.accountId, bytes.constData(), 32);
        break;
    case StellarAddress::Type::MUXED_ACCOUNT:
        if (bytes.size() != 40) throw std::runtime_error("invalid MUXED body");
        xdr.type = stellar::SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT;
        std::memcpy(xdr.muxedEd25519, bytes.constData(), 32);
        {
            // Last 8 bytes are the muxed id, big-endian.
            quint64 id = 0;
            for (int i = 0; i < 8; ++i) {
                id = (id << 8) | static_cast<unsigned char>(bytes.at(32 + i));
            }
            xdr.muxedId = id;
        }
        break;
    case StellarAddress::Type::CONTRACT:
        if (bytes.size() != 32) throw std::runtime_error("invalid CONTRACT body");
        xdr.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        std::memcpy(xdr.contractId, bytes.constData(), 32);
        break;
    case StellarAddress::Type::CLAIMABLE_BALANCE:
        // 33-byte body = 1-byte type discriminant (V0=0) + 32-byte hash.
        if (bytes.size() != 33) throw std::runtime_error("invalid CB body");
        xdr.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE;
        std::memcpy(xdr.claimableHash, bytes.constData() + 1, 32);
        break;
    case StellarAddress::Type::LIQUIDITY_POOL:
        if (bytes.size() != 32) throw std::runtime_error("invalid LP body");
        xdr.type = stellar::SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL;
        std::memcpy(xdr.liquidityPoolId, bytes.constData(), 32);
        break;
    }
    return toAddress(xdr);
}

stellar::SCVal Scv::toAddress(const QString& strkey)
{
    return toAddress(StellarAddress::fromString(strkey));
}

stellar::SCAddress Scv::fromAddressXdr(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_ADDRESS);
    return v.address;
}

StellarAddress Scv::fromAddress(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_ADDRESS);
    const stellar::SCAddress& a = v.address;
    switch (a.type) {
    case stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT:
        return StellarAddress(StellarAddress::Type::ACCOUNT,
                              QByteArray(reinterpret_cast<const char*>(a.accountId), 32));
    case stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT:
        return StellarAddress(StellarAddress::Type::CONTRACT,
                              QByteArray(reinterpret_cast<const char*>(a.contractId), 32));
    case stellar::SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT: {
        QByteArray body(reinterpret_cast<const char*>(a.muxedEd25519), 32);
        quint64 id = a.muxedId;
        for (int i = 7; i >= 0; --i) {
            body.append(static_cast<char>((id >> (8 * i)) & 0xFF));
        }
        return StellarAddress(StellarAddress::Type::MUXED_ACCOUNT, body);
    }
    case stellar::SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE: {
        QByteArray body;
        body.append(static_cast<char>(0)); // V0 discriminant
        body.append(reinterpret_cast<const char*>(a.claimableHash), 32);
        return StellarAddress(StellarAddress::Type::CLAIMABLE_BALANCE, body);
    }
    case stellar::SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL:
        return StellarAddress(StellarAddress::Type::LIQUIDITY_POOL,
                              QByteArray(reinterpret_cast<const char*>(a.liquidityPoolId), 32));
    }
    throw std::runtime_error("unknown SCAddress type");
}

stellar::SCVal Scv::toError(stellar::SCErrorType type, quint32 contractCodeOrEnum)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_ERROR;
    v.error.type = type;
    // Only what the XDR can carry. A type above SCE_AUTH, or a code outside
    // SCErrorCode, built a value that was signed although js-stellar-sdk and
    // Core cannot decode it; it is refused here, where it is built.
    if (!stellar::isKnownSCErrorType(type))
        throw std::runtime_error("SCError: no such error type");
    if (type == stellar::SCErrorType::SCE_CONTRACT) {
        v.error.contractCode = contractCodeOrEnum;
    } else {
        if (contractCodeOrEnum > static_cast<quint32>(stellar::SCErrorCode::SCEC_UNEXPECTED_SIZE))
            throw std::runtime_error("SCError: no such error code");
        v.error.code = static_cast<stellar::SCErrorCode>(contractCodeOrEnum);
    }
    return v;
}

stellar::SCError Scv::fromError(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_ERROR);
    return v.error;
}

stellar::SCVal Scv::toLedgerKeyNonce(qint64 nonce)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_LEDGER_KEY_NONCE;
    v.nonceKey.nonce = nonce;
    return v;
}

qint64 Scv::fromLedgerKeyNonce(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_LEDGER_KEY_NONCE);
    return v.nonceKey.nonce;
}

stellar::SCVal Scv::toLedgerKeyContractInstance()
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE;
    return v;
}

stellar::SCVal Scv::toContractInstance(const stellar::ContractExecutable& executable,
                                       const stellar::SCVal& storage)
{
    stellar::SCVal v;
    v.type = stellar::SCValType::SCV_CONTRACT_INSTANCE;
    v.instance = QSharedPointer<stellar::SCContractInstance>::create();
    v.instance->executable = executable;
    // Treat default SCV_VOID storage as "absent" (matches Java SDK).
    if (storage.type != stellar::SCValType::SCV_VOID) {
        v.instance->storage = QSharedPointer<stellar::SCVal>::create(storage);
    }
    return v;
}

stellar::SCContractInstance Scv::fromContractInstance(const stellar::SCVal& v)
{
    requireType(v, stellar::SCValType::SCV_CONTRACT_INSTANCE);
    return v.instance ? *v.instance : stellar::SCContractInstance();
}
QSTELLAR_END_NS
