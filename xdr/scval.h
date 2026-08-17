#ifndef SCVAL_H
#define SCVAL_H

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QSharedPointer>
#include <stdexcept>

#include "xdrhelper.h"
#include "stellartypes.h"
#include "stellarledgerentries.h"

namespace stellar
{
    using namespace xdr;

    /**
     * CAP-46 — SCValType enumerates all the value kinds Soroban exposes.
     * Values match the canonical Stellar XDR ordering exactly.
     */
    enum class SCValType : qint32
    {
        SCV_BOOL = 0,
        SCV_VOID = 1,
        SCV_ERROR = 2,
        SCV_U32 = 3,
        SCV_I32 = 4,
        SCV_U64 = 5,
        SCV_I64 = 6,
        SCV_TIMEPOINT = 7,
        SCV_DURATION = 8,
        SCV_U128 = 9,
        SCV_I128 = 10,
        SCV_U256 = 11,
        SCV_I256 = 12,
        SCV_BYTES = 13,
        SCV_STRING = 14,
        SCV_SYMBOL = 15,
        SCV_VEC = 16,
        SCV_MAP = 17,
        SCV_ADDRESS = 18,
        SCV_CONTRACT_INSTANCE = 19,
        SCV_LEDGER_KEY_CONTRACT_INSTANCE = 20,
        SCV_LEDGER_KEY_NONCE = 21
    };

    /**
     * Fixed-size wide-integer payloads (CAP-46). Stored as Qt-native words
     * (qint64/quint64) split high/low for trivial copyability. Conversion
     * to/from a canonical big-endian QByteArray happens in Scv::toUint128
     * et al.
     */
    struct UInt128Parts { quint64 hi = 0; quint64 lo = 0; };
    struct Int128Parts  { qint64  hi = 0; quint64 lo = 0; };
    struct UInt256Parts { quint64 hiHi = 0; quint64 hiLo = 0; quint64 loHi = 0; quint64 loLo = 0; };
    struct Int256Parts  { qint64  hiHi = 0; quint64 hiLo = 0; quint64 loHi = 0; quint64 loLo = 0; };

    /**
     * SCAddress (CAP-46). The first two variants (ACCOUNT, CONTRACT) are
     * available since Protocol 20. The remaining three (MUXED_ACCOUNT,
     * CLAIMABLE_BALANCE, LIQUIDITY_POOL) target Protocol 23+ but are wired
     * here so the round-trip with StellarAddress can cover all five.
     */
    enum class SCAddressType : qint32
    {
        SC_ADDRESS_TYPE_ACCOUNT = 0,
        SC_ADDRESS_TYPE_CONTRACT = 1,
        SC_ADDRESS_TYPE_MUXED_ACCOUNT = 2,
        SC_ADDRESS_TYPE_CLAIMABLE_BALANCE = 3,
        SC_ADDRESS_TYPE_LIQUIDITY_POOL = 4
    };

    struct SCAddress
    {
        SCAddressType type = SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        uint256 accountId{};      // ACCOUNT — raw ed25519 pubkey
        uint256 contractId{};     // CONTRACT — 32-byte contract hash
        uint256 muxedEd25519{};   // MUXED_ACCOUNT — ed25519 base
        quint64 muxedId = 0;      // MUXED_ACCOUNT — muxed id
        uint256 claimableHash{};  // CLAIMABLE_BALANCE — V0 hash
        uint256 liquidityPoolId{};// LIQUIDITY_POOL — pool id
    };
    inline QDataStream& operator<<(QDataStream& out, const SCAddress& a) {
        out << a.type;
        switch (a.type) {
        case SCAddressType::SC_ADDRESS_TYPE_ACCOUNT:
            // PublicKey discriminant + ed25519 bytes
            out << qint32(0); out << a.accountId; break;
        case SCAddressType::SC_ADDRESS_TYPE_CONTRACT:
            out << a.contractId; break;
        case SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT:
            out << a.muxedEd25519 << a.muxedId; break;
        case SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE:
            out << qint32(0); out << a.claimableHash; break; // V0 discriminant + hash
        case SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL:
            out << a.liquidityPoolId; break;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SCAddress& a) {
        in >> a.type;
        qint32 disc = 0;
        switch (a.type) {
        case SCAddressType::SC_ADDRESS_TYPE_ACCOUNT:
            in >> disc; in >> a.accountId; break;
        case SCAddressType::SC_ADDRESS_TYPE_CONTRACT:
            in >> a.contractId; break;
        case SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT:
            in >> a.muxedEd25519 >> a.muxedId; break;
        case SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE:
            in >> disc; in >> a.claimableHash; break;
        case SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL:
            in >> a.liquidityPoolId; break;
        }
        return in;
    }

    /** SCError (CAP-46). Two-flavour discriminated union: SCE_CONTRACT carries
     *  a user-supplied uint32 code; the other categories carry an SCErrorCode
     *  enum value. */
    enum class SCErrorType : qint32
    {
        SCE_CONTRACT = 0,
        SCE_WASM_VM = 1,
        SCE_CONTEXT = 2,
        SCE_STORAGE = 3,
        SCE_OBJECT = 4,
        SCE_CRYPTO = 5,
        SCE_EVENTS = 6,
        SCE_BUDGET = 7,
        SCE_VALUE = 8,
        SCE_AUTH = 9
    };
    enum class SCErrorCode : qint32
    {
        SCEC_ARITH_DOMAIN = 0,
        SCEC_INDEX_BOUNDS = 1,
        SCEC_INVALID_INPUT = 2,
        SCEC_MISSING_VALUE = 3,
        SCEC_EXISTING_VALUE = 4,
        SCEC_EXCEEDED_LIMIT = 5,
        SCEC_INVALID_ACTION = 6,
        SCEC_INTERNAL_ERROR = 7,
        SCEC_UNEXPECTED_TYPE = 8,
        SCEC_UNEXPECTED_SIZE = 9
    };

    struct SCError
    {
        SCErrorType type = SCErrorType::SCE_CONTRACT;
        quint32 contractCode = 0;                // for SCE_CONTRACT
        SCErrorCode code = SCErrorCode::SCEC_ARITH_DOMAIN; // for the rest
    };
    inline QDataStream& operator<<(QDataStream& out, const SCError& e) {
        out << e.type;
        if (e.type == SCErrorType::SCE_CONTRACT) out << e.contractCode;
        else out << e.code;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SCError& e) {
        in >> e.type;
        if (e.type == SCErrorType::SCE_CONTRACT) in >> e.contractCode;
        else in >> e.code;
        return in;
    }

    /** SCNonceKey (CAP-46). */
    struct SCNonceKey { qint64 nonce = 0; };
    inline QDataStream& operator<<(QDataStream& out, const SCNonceKey& k) {
        out << k.nonce; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SCNonceKey& k) {
        in >> k.nonce; return in;
    }

    /**
     * ContractExecutableExternalRef (CAP-85, Protocol 28) — the payload of the
     * externally managed executable: the contract that owns the shared code
     * reference, plus the tag naming which reference of that owner to use.
     * Pointing several contracts at one reference is what makes a fleet
     * upgrade atomic.
     */
    struct ContractExecutableExternalRef
    {
        SCAddress  executableOwner;
        QByteArray tag;   // SCString — UTF-8 bytes, same wire form as SCV_STRING
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractExecutableExternalRef& r) {
        out << r.executableOwner;
        out << static_cast<qint32>(r.tag.size());
        if (!r.tag.isEmpty()) out.writeRawData(r.tag.constData(), r.tag.size());
        const quint32 pad = (4 - (r.tag.size() % 4)) % 4;
        if (pad) { char zero[4] = {0,0,0,0}; out.writeRawData(zero, static_cast<int>(pad)); }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractExecutableExternalRef& r) {
        in >> r.executableOwner;
        qint32 n; in >> n;
        if (n < 0) throw std::runtime_error("ContractExecutableExternalRef: negative tag length");
        r.tag.resize(n);
        if (n > 0) in.readRawData(r.tag.data(), n);
        const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
        if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
        return in;
    }

    /** ContractExecutable (CAP-46; EXTERNAL_REF added by CAP-85 / Protocol 28). */
    enum class ContractExecutableType : qint32
    {
        CONTRACT_EXECUTABLE_WASM = 0,
        CONTRACT_EXECUTABLE_STELLAR_ASSET = 1,
        CONTRACT_EXECUTABLE_EXTERNAL_REF = 2   // CAP-85, Protocol 28
    };
    struct ContractExecutable
    {
        ContractExecutableType type = ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        uint256 wasmHash{};                          // only for WASM variant
        ContractExecutableExternalRef externalRef;   // only for EXTERNAL_REF variant
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractExecutable& e) {
        out << e.type;
        switch (e.type) {
        case ContractExecutableType::CONTRACT_EXECUTABLE_WASM:
            out << e.wasmHash; break;
        case ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET:
            break;  // void arm
        case ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF:
            out << e.externalRef; break;
        default:
            throw std::runtime_error("ContractExecutable: unknown executable type");
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractExecutable& e) {
        in >> e.type;
        switch (e.type) {
        case ContractExecutableType::CONTRACT_EXECUTABLE_WASM:
            in >> e.wasmHash; break;
        case ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET:
            break;  // void arm
        case ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF:
            in >> e.externalRef; break;
        default:
            // Never skip an unknown arm silently: its payload would stay in the
            // stream and desync every field decoded after it.
            throw std::runtime_error("ContractExecutable: unknown executable type");
        }
        return in;
    }

    struct SCVal;        // fwd: SCVal contains containers of itself
    struct SCMapEntry;

    /** SCContractInstance (CAP-46): contract executable + optional storage map
     *  (storage held as Optional<SCVal> of type SCV_MAP). */
    struct SCContractInstance
    {
        ContractExecutable executable;
        // The storage map is held as a shared SCVal (typically SCV_MAP or
        // null). Using SCVal sidesteps the recursive type problem the same
        // way SCVal::map does.
        QSharedPointer<SCVal> storage; // null = absent
    };

    /**
     * SCVal — Soroban host value. This release ships the primitive variants
     * (Bool, Void, U32/I32, U64/I64, TimePoint, Duration, Bytes, String,
     * Symbol), 128/256-bit integers, and Vec / Map. The remaining variants
     * (errors, address, contract-instance, ledger-key-*) are scheduled for
     * later iterations of 0.9.0 — operator<< / operator>> throws
     * `unimplemented` for them until then.
     *
     * Vec / Map are stored behind `QSharedPointer` to break the recursive
     * type instantiation: `SCVal` contains `QList<SCVal>` (vec) and
     * `QList<SCMapEntry>` where `SCMapEntry` holds two `SCVal`s. With value
     * semantics for the containers, the type would be incomplete at its own
     * member declaration; shared-pointer indirection sidesteps that and
     * gives us cheap copy semantics (containers are reference-counted).
     */
    struct SCVal
    {
        SCValType type = SCValType::SCV_VOID;

        // Primitive scalar payloads.
        bool b = false;
        quint32 u32 = 0;
        qint32 i32 = 0;
        quint64 u64 = 0;
        qint64 i64 = 0;
        quint64 timepoint = 0;
        quint64 duration = 0;

        // 128 / 256 bit integers.
        UInt128Parts u128;
        Int128Parts  i128;
        UInt256Parts u256;
        Int256Parts  i256;

        // Variable-length payloads. For SYMBOL, max len is 32 bytes.
        QByteArray bytes;   // SCV_BYTES
        QByteArray str;     // SCV_STRING (UTF-8 bytes)
        QByteArray symbol;  // SCV_SYMBOL (ASCII bytes, ≤ 32)

        // Recursive containers (CAP-46). Lazily allocated; null when unused.
        QSharedPointer<QList<SCVal>>      vec;
        QSharedPointer<QList<SCMapEntry>> map;

        // CAP-46 — additional variants landed in Iter 4.
        SCAddress  address;             // SCV_ADDRESS
        SCError    error;               // SCV_ERROR
        SCNonceKey nonceKey;            // SCV_LEDGER_KEY_NONCE
        QSharedPointer<SCContractInstance> instance; // SCV_CONTRACT_INSTANCE
        // SCV_LEDGER_KEY_CONTRACT_INSTANCE has no payload.

        SCVal() = default;
    };

    struct SCMapEntry
    {
        SCVal key;
        SCVal val;
    };

    inline QDataStream& operator<<(QDataStream& out, const SCVal& v)
    {
        out << v.type;
        switch (v.type) {
        case SCValType::SCV_VOID: break;
        case SCValType::SCV_BOOL:
            out << static_cast<qint32>(v.b ? 1 : 0); break;
        case SCValType::SCV_U32:
            out << v.u32; break;
        case SCValType::SCV_I32:
            out << v.i32; break;
        case SCValType::SCV_U64:
            out << v.u64; break;
        case SCValType::SCV_I64:
            out << v.i64; break;
        case SCValType::SCV_TIMEPOINT:
            out << v.timepoint; break;
        case SCValType::SCV_DURATION:
            out << v.duration; break;
        case SCValType::SCV_BYTES: {
            out << static_cast<qint32>(v.bytes.size());
            if (!v.bytes.isEmpty()) out.writeRawData(v.bytes.constData(), v.bytes.size());
            const quint32 pad = (4 - (v.bytes.size() % 4)) % 4;
            if (pad) { char zero[4] = {0,0,0,0}; out.writeRawData(zero, pad); }
            break;
        }
        case SCValType::SCV_STRING: {
            out << static_cast<qint32>(v.str.size());
            if (!v.str.isEmpty()) out.writeRawData(v.str.constData(), v.str.size());
            const quint32 pad = (4 - (v.str.size() % 4)) % 4;
            if (pad) { char zero[4] = {0,0,0,0}; out.writeRawData(zero, pad); }
            break;
        }
        case SCValType::SCV_SYMBOL: {
            if (v.symbol.size() > 32) throw std::runtime_error("SCSymbol exceeds 32 bytes");
            out << static_cast<qint32>(v.symbol.size());
            if (!v.symbol.isEmpty()) out.writeRawData(v.symbol.constData(), v.symbol.size());
            const quint32 pad = (4 - (v.symbol.size() % 4)) % 4;
            if (pad) { char zero[4] = {0,0,0,0}; out.writeRawData(zero, pad); }
            break;
        }
        case SCValType::SCV_U128:
            out << v.u128.hi << v.u128.lo; break;
        case SCValType::SCV_I128:
            out << v.i128.hi << v.i128.lo; break;
        case SCValType::SCV_U256:
            out << v.u256.hiHi << v.u256.hiLo << v.u256.loHi << v.u256.loLo; break;
        case SCValType::SCV_I256:
            out << v.i256.hiHi << v.i256.hiLo << v.i256.loHi << v.i256.loLo; break;
        case SCValType::SCV_VEC: {
            const QList<SCVal>* xs = v.vec.data();
            const qint32 n = xs ? static_cast<qint32>(xs->size()) : 0;
            out << n;
            if (xs) for (const SCVal& e : *xs) out << e;
            break;
        }
        case SCValType::SCV_MAP: {
            const QList<SCMapEntry>* xs = v.map.data();
            const qint32 n = xs ? static_cast<qint32>(xs->size()) : 0;
            out << n;
            if (xs) for (const SCMapEntry& e : *xs) { out << e.key; out << e.val; }
            break;
        }
        case SCValType::SCV_ADDRESS:
            out << v.address; break;
        case SCValType::SCV_ERROR:
            out << v.error; break;
        case SCValType::SCV_LEDGER_KEY_NONCE:
            out << v.nonceKey; break;
        case SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE:
            break;  // no payload
        case SCValType::SCV_CONTRACT_INSTANCE: {
            if (!v.instance) throw std::runtime_error("SCV_CONTRACT_INSTANCE storage is null");
            out << v.instance->executable;
            // Optional<SCVal> storage: 4-byte present flag + (if present) the SCVal.
            const bool present = !v.instance->storage.isNull();
            out << qint32(present ? 1 : 0);
            if (present) out << *v.instance->storage;
            break;
        }
        default:
            throw std::runtime_error("SCVal serializer: variant not implemented in this iteration");
        }
        return out;
    }

    inline QDataStream& operator>>(QDataStream& in, SCVal& v)
    {
        in >> v.type;
        switch (v.type) {
        case SCValType::SCV_VOID: break;
        case SCValType::SCV_BOOL: {
            qint32 b; in >> b; v.b = (b != 0); break;
        }
        case SCValType::SCV_U32:
            in >> v.u32; break;
        case SCValType::SCV_I32:
            in >> v.i32; break;
        case SCValType::SCV_U64:
            in >> v.u64; break;
        case SCValType::SCV_I64:
            in >> v.i64; break;
        case SCValType::SCV_TIMEPOINT:
            in >> v.timepoint; break;
        case SCValType::SCV_DURATION:
            in >> v.duration; break;
        case SCValType::SCV_BYTES: {
            qint32 n; in >> n;
            if (n < 0) throw std::runtime_error("SCBytes negative length");
            v.bytes.resize(n);
            if (n > 0) in.readRawData(v.bytes.data(), n);
            const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
            if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
            break;
        }
        case SCValType::SCV_STRING: {
            qint32 n; in >> n;
            if (n < 0) throw std::runtime_error("SCString negative length");
            v.str.resize(n);
            if (n > 0) in.readRawData(v.str.data(), n);
            const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
            if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
            break;
        }
        case SCValType::SCV_SYMBOL: {
            qint32 n; in >> n;
            if (n < 0 || n > 32) throw std::runtime_error("SCSymbol invalid length");
            v.symbol.resize(n);
            if (n > 0) in.readRawData(v.symbol.data(), n);
            const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
            if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
            break;
        }
        case SCValType::SCV_U128:
            in >> v.u128.hi >> v.u128.lo; break;
        case SCValType::SCV_I128:
            in >> v.i128.hi >> v.i128.lo; break;
        case SCValType::SCV_U256:
            in >> v.u256.hiHi >> v.u256.hiLo >> v.u256.loHi >> v.u256.loLo; break;
        case SCValType::SCV_I256:
            in >> v.i256.hiHi >> v.i256.hiLo >> v.i256.loHi >> v.i256.loLo; break;
        case SCValType::SCV_VEC: {
            qint32 n; in >> n;
            if (n < 0) throw std::runtime_error("SCVec negative length");
            v.vec = QSharedPointer<QList<SCVal>>::create();
            v.vec->reserve(n);
            for (qint32 i = 0; i < n; ++i) { SCVal e; in >> e; v.vec->append(e); }
            break;
        }
        case SCValType::SCV_MAP: {
            qint32 n; in >> n;
            if (n < 0) throw std::runtime_error("SCMap negative length");
            v.map = QSharedPointer<QList<SCMapEntry>>::create();
            v.map->reserve(n);
            for (qint32 i = 0; i < n; ++i) {
                SCMapEntry e;
                in >> e.key;
                in >> e.val;
                v.map->append(e);
            }
            break;
        }
        case SCValType::SCV_ADDRESS:
            in >> v.address; break;
        case SCValType::SCV_ERROR:
            in >> v.error; break;
        case SCValType::SCV_LEDGER_KEY_NONCE:
            in >> v.nonceKey; break;
        case SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE:
            break;
        case SCValType::SCV_CONTRACT_INSTANCE: {
            v.instance = QSharedPointer<SCContractInstance>::create();
            in >> v.instance->executable;
            qint32 present = 0; in >> present;
            if (present) {
                v.instance->storage = QSharedPointer<SCVal>::create();
                in >> *v.instance->storage;
            }
            break;
        }
        default:
            throw std::runtime_error("SCVal deserializer: variant not implemented in this iteration");
        }
        return in;
    }

} // namespace stellar

#endif // SCVAL_H
