#include "scv.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "../stellaraddress.h"

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
    return toString(s.toUtf8());
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

// ─── Vec / Map / comparator ──────────────────────────────────────────

namespace {

QByteArray serializeScVal(const stellar::SCVal& v)
{
    QByteArray bytes;
    QDataStream s(&bytes, QIODevice::WriteOnly);
    s << v;
    return bytes;
}

} // namespace

int Scv::compare(const stellar::SCVal& a, const stellar::SCVal& b)
{
    const QByteArray ab = serializeScVal(a);
    const QByteArray bb = serializeScVal(b);
    const int c = std::memcmp(ab.constData(), bb.constData(),
                              std::min(ab.size(), bb.size()));
    if (c != 0) return c;
    return ab.size() - bb.size();
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
    // Canonicalize: sort entries by key in XDR-byte ascending order. This
    // matches the ordering Soroban core uses to compare maps, so two
    // semantically-equal SCMaps serialize byte-identical.
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
    if (type == stellar::SCErrorType::SCE_CONTRACT) {
        v.error.contractCode = contractCodeOrEnum;
    } else {
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
