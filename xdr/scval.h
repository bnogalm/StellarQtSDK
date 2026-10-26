#ifndef SCVAL_H
#define SCVAL_H

#include <QtGlobal>
#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QSharedPointer>
#include <stdexcept>
#include <string>

#include "xdrhelper.h"
#include "stellartypes.h"
#include "stellarledgerentries.h"

namespace stellar
{
    using namespace xdr;

    /**
     * A count or length read from the wire, checked against what is left of
     * the input BEFORE anything is allocated for it. Every element takes at
     * least `minBytesEach` bytes, so a count the rest of the input cannot hold
     * is refused: 12 bytes claiming 3,000,000 vector elements used to commit
     * 3 GB before the read ran dry, and 2^31-1 ended in std::bad_alloc. The
     * SDK decodes from a QByteArray, so bytesAvailable() is exact.
     */
    inline void checkWireCount(QDataStream& in, qint32 n, qint64 minBytesEach, const char* what)
    {
        if (n < 0)
            throw std::runtime_error(std::string(what) + ": negative length");
        if (in.device() && qint64(n) * minBytesEach > in.device()->bytesAvailable())
            throw std::runtime_error(std::string(what) + ": length exceeds the remaining input");
    }

    /** QDataStream never throws when it runs dry: it flags ReadPastEnd and
     *  hands back zeros. An element loop stops at the first error instead of
     *  appending default values for the rest of the claimed count. */
    inline void checkWireElement(QDataStream& in, const char* what)
    {
        if (in.status() != QDataStream::Ok)
            throw std::runtime_error(std::string(what) + ": truncated or malformed element");
    }

    /** The 0-3 bytes that pad an opaque or string of `n` bytes to a 4-byte
     *  boundary are zeros: core (xdrpp), the Rust stellar-xdr and js refuse
     *  anything else (py-stellar-sdk 14.1.0 takes it and writes zeros back).
     *  They used to be read and thrown away here; anything else now marks
     *  the stream corrupt and throws, as xdr::Array and readXdrOpaque do. */
    inline void readXdrPadding(QDataStream& in, qint32 n, const char* what)
    {
        const int pad = int((4 - (static_cast<quint32>(n) % 4)) % 4);
        if (!pad)
            return;
        char z[4] = {0, 0, 0, 0};
        in.readRawData(z, pad);
        for (int i = 0; i < pad; ++i) {
            if (z[i] != 0) {
                in.setStatus(QDataStream::ReadCorruptData);
                throw std::runtime_error(std::string(what) + ": non-zero padding");
            }
        }
    }

    /** An XDR bool or optional-present flag is 0 or 1 (see xdr::isXdrBool).
     *  Anything else used to read as true and was written back as 1; it now
     *  marks the stream corrupt and, like every other malformed value here,
     *  throws. */
    inline void checkXdrFlag(QDataStream& in, qint32 flag, const char* what)
    {
        if (!isXdrBool(flag)) {
            in.setStatus(QDataStream::ReadCorruptData);
            throw std::runtime_error(std::string(what) + ": a bool or present flag is 0 or 1");
        }
    }

    /**
     * SCVal nests (vec, map, contract-instance storage), so decoding is
     * depth-limited: a level costs a few bytes of wire and a few KB of stack,
     * and ~400 nested vectors (5 KB of hostile XDR) overflowed the stack — a
     * crash no catch() can intercept. The Soroban host refuses values nested
     * deeper than 100 (DEFAULT_HOST_DEPTH_LIMIT), so every value it produces
     * or accepts is well inside the limit. Depth 1 is a value with no
     * containers around it.
     */
    static const int MAX_SCVAL_DEPTH = 128;

    inline int& scValDepth() { static thread_local int depth = 0; return depth; }
    struct SCValDepthGuard
    {
        SCValDepthGuard() {
            if (++scValDepth() > MAX_SCVAL_DEPTH) {
                --scValDepth();   // no destructor runs when a ctor throws
                throw std::runtime_error("SCVal: nesting too deep");
            }
        }
        ~SCValDepthGuard() { --scValDepth(); }
    };

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
        SCV_LEDGER_KEY_NONCE = 21,
        SCV_EXECUTABLE_TAG = 22   // CAP-85, Protocol 28 — carries an SCString
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
            // Canonical: struct MuxedEd25519Account { uint64 id; uint256 ed25519; }
            // — id FIRST. The SEP-23 strkey payload is the other way round
            // (ed25519 then id), which is what this used to emit; the classic
            // MuxedAccount in stellartransaction.h already gets it right.
            out << a.muxedId << a.muxedEd25519; break;
        case SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE:
            out << qint32(0); out << a.claimableHash; break; // V0 discriminant + hash
        case SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL:
            out << a.liquidityPoolId; break;
        default:
            throw std::runtime_error("SCAddress: unknown address type");
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SCAddress& a) {
        in >> a.type;
        qint32 disc = 0;
        switch (a.type) {
        case SCAddressType::SC_ADDRESS_TYPE_ACCOUNT:
            in >> disc;
            // PublicKey has one arm, ed25519 (0). Any other was read as an
            // ed25519 key and written back as 0: a different address.
            if (disc != 0) throw std::runtime_error("SCAddress: unknown public key type");
            in >> a.accountId; break;
        case SCAddressType::SC_ADDRESS_TYPE_CONTRACT:
            in >> a.contractId; break;
        case SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT:
            in >> a.muxedId >> a.muxedEd25519; break;   // id first — see the writer
        case SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE:
            in >> disc;
            // ClaimableBalanceID has one arm, V0 (0); same reason as above.
            if (disc != 0) throw std::runtime_error("SCAddress: unknown claimable balance id type");
            in >> a.claimableHash; break;
        case SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL:
            in >> a.liquidityPoolId; break;
        default:
            // Same reason as ContractExecutable: an unknown arm's payload
            // would stay in the stream and desync everything after it.
            throw std::runtime_error("SCAddress: unknown address type");
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

    /** stellar-xdr's union has an arm for SCE_CONTRACT (any uint32) and one for
     *  each of the nine other types (an SCErrorCode), and no default. */
    inline bool isKnownSCErrorType(SCErrorType t)
    {
        const qint32 raw = static_cast<qint32>(t);
        return raw >= static_cast<qint32>(SCErrorType::SCE_CONTRACT)
            && raw <= static_cast<qint32>(SCErrorType::SCE_AUTH);
    }
    inline bool isKnownSCErrorCode(SCErrorCode c)
    {
        const qint32 raw = static_cast<qint32>(c);
        return raw >= static_cast<qint32>(SCErrorCode::SCEC_ARITH_DOMAIN)
            && raw <= static_cast<qint32>(SCErrorCode::SCEC_UNEXPECTED_SIZE);
    }
    /** An SCError the XDR can carry: see isKnownSCErrorType. */
    inline bool isEncodableSCError(const SCError& e)
    {
        if (!isKnownSCErrorType(e.type))
            return false;
        return e.type == SCErrorType::SCE_CONTRACT || isKnownSCErrorCode(e.code);
    }

    /**
     * A type above SCE_AUTH, or a code outside SCErrorCode, used to go out with
     * the stream Ok and come back in the same way: bytes that survived the
     * byte-exact re-encode check and were signed, and that neither
     * js-stellar-sdk nor Core can decode. Nothing is written for them now, and
     * reading one marks the stream corrupt (the SCVal reader then throws).
     */
    inline QDataStream& operator<<(QDataStream& out, const SCError& e) {
        if (!isEncodableSCError(e)) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << e.type;
        if (e.type == SCErrorType::SCE_CONTRACT) out << e.contractCode;
        else out << e.code;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SCError& e) {
        in >> e.type;
        if (!isKnownSCErrorType(e.type)) {
            // No arm: whatever follows is not this value's.
            in.setStatus(QDataStream::ReadCorruptData);
            return in;
        }
        if (e.type == SCErrorType::SCE_CONTRACT) {
            in >> e.contractCode;
        } else {
            in >> e.code;
            if (!isKnownSCErrorCode(e.code))
                in.setStatus(QDataStream::ReadCorruptData);
        }
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
        qint32 n = 0; in >> n;
        checkWireCount(in, n, 1, "ContractExecutableExternalRef tag");
        r.tag.resize(n);
        if (n > 0) in.readRawData(r.tag.data(), n);
        readXdrPadding(in, n, "ContractExecutableExternalRef tag");
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
        QByteArray executableTag;   // SCV_EXECUTABLE_TAG (CAP-85) — SCString bytes
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
        // An error the XDR cannot carry: not even its SCV_ERROR discriminant.
        if (v.type == SCValType::SCV_ERROR && !isEncodableSCError(v.error)) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
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
        // SCV_VEC / SCV_MAP are XDR OPTIONALS, not plain arrays: the canonical
        // union declares `SCVec *vec;` / `SCMap *map;` ("Vec and Map are
        // recursive so need to live behind an option, due to xdrpp
        // limitations"). RFC 4506 §4.19 encodes `T *x` as a 4-byte boolean
        // followed by the value when present. Omitting that flag made every
        // container the SDK wrote 4 bytes shorter than Core's, and every
        // container Core produced misparse — the reader took the present flag
        // for the element count. A null pointer is the absent case; an
        // allocated empty list is present-with-zero-elements, and the two are
        // distinct on the wire.
        case SCValType::SCV_VEC: {
            const bool present = !v.vec.isNull();
            out << qint32(present ? 1 : 0);
            if (present) {
                out << static_cast<qint32>(v.vec->size());
                for (const SCVal& e : *v.vec) out << e;
            }
            break;
        }
        case SCValType::SCV_MAP: {
            const bool present = !v.map.isNull();
            out << qint32(present ? 1 : 0);
            if (present) {
                out << static_cast<qint32>(v.map->size());
                for (const SCMapEntry& e : *v.map) { out << e.key; out << e.val; }
            }
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
        case SCValType::SCV_EXECUTABLE_TAG: {
            // SCString, same wire form as SCV_STRING.
            out << static_cast<qint32>(v.executableTag.size());
            if (!v.executableTag.isEmpty())
                out.writeRawData(v.executableTag.constData(), v.executableTag.size());
            const quint32 pad = (4 - (v.executableTag.size() % 4)) % 4;
            if (pad) { char zero[4] = {0,0,0,0}; out.writeRawData(zero, static_cast<int>(pad)); }
            break;
        }
        case SCValType::SCV_CONTRACT_INSTANCE: {
            if (!v.instance) throw std::runtime_error("SCV_CONTRACT_INSTANCE storage is null");
            out << v.instance->executable;
            // Canonical XDR is `SCMap* storage`, i.e. a 4-byte present flag
            // followed by a BARE length-prefixed SCMapEntry array — NOT a full
            // SCVal. Writing the SCVal would prepend its SCV_MAP discriminant
            // and put four extra bytes on the wire, which is what previously
            // made every instance disagree with Core. `storage` is modelled as
            // an SCVal purely so the recursive type works; only the map body
            // is serialised.
            const bool present = !v.instance->storage.isNull();
            out << qint32(present ? 1 : 0);
            if (present) {
                const SCVal& st = *v.instance->storage;
                if (st.type != SCValType::SCV_MAP)
                    throw std::runtime_error("SCContractInstance storage must be an SCV_MAP");
                // The SCMap inside a present storage is NOT itself optional, so
                // a null list is simply zero entries.
                out << static_cast<qint32>(st.map ? st.map->size() : 0);
                if (st.map) for (const SCMapEntry& e : *st.map) { out << e.key; out << e.val; }
            }
            break;
        }
        default:
            throw std::runtime_error("SCVal serializer: variant not implemented in this iteration");
        }
        return out;
    }

    inline QDataStream& operator>>(QDataStream& in, SCVal& v)
    {
        SCValDepthGuard depthGuard;
        in >> v.type;
        switch (v.type) {
        case SCValType::SCV_VOID: break;
        case SCValType::SCV_BOOL: {
            qint32 flag = 0; in >> flag;
            checkXdrFlag(in, flag, "SCV_BOOL");
            v.b = (flag == 1); break;
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
            qint32 n = 0; in >> n;
            checkWireCount(in, n, 1, "SCBytes");
            v.bytes.resize(n);
            if (n > 0) in.readRawData(v.bytes.data(), n);
            readXdrPadding(in, n, "SCBytes");
            break;
        }
        case SCValType::SCV_STRING: {
            qint32 n = 0; in >> n;
            checkWireCount(in, n, 1, "SCString");
            v.str.resize(n);
            if (n > 0) in.readRawData(v.str.data(), n);
            readXdrPadding(in, n, "SCString");
            break;
        }
        case SCValType::SCV_SYMBOL: {
            qint32 n = 0; in >> n;
            if (n < 0 || n > 32) throw std::runtime_error("SCSymbol invalid length");
            checkWireCount(in, n, 1, "SCSymbol");
            v.symbol.resize(n);
            if (n > 0) in.readRawData(v.symbol.data(), n);
            readXdrPadding(in, n, "SCSymbol");
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
            qint32 present = 0; in >> present;   // XDR optional (see the writer)
            checkXdrFlag(in, present, "SCVec");
            if (!present) { v.vec.reset(); break; }
            qint32 n = 0; in >> n;
            checkWireCount(in, n, 4, "SCVec");   // each element starts with its 4-byte type
            // Not reserved: an SCVal is ~120x its smallest encoding, and every
            // level of a nested value could claim the rest of the input again.
            v.vec = QSharedPointer<QList<SCVal>>::create();
            for (qint32 i = 0; i < n; ++i) {
                checkWireElement(in, "SCVec");
                SCVal e; in >> e; v.vec->append(e);
            }
            break;
        }
        case SCValType::SCV_MAP: {
            qint32 present = 0; in >> present;   // XDR optional (see the writer)
            checkXdrFlag(in, present, "SCMap");
            if (!present) { v.map.reset(); break; }
            qint32 n = 0; in >> n;
            checkWireCount(in, n, 8, "SCMap");   // a key and a value, 4 bytes each at least
            v.map = QSharedPointer<QList<SCMapEntry>>::create();   // not reserved, as SCVec
            for (qint32 i = 0; i < n; ++i) {
                checkWireElement(in, "SCMap");
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
            in >> v.error;
            if (in.status() == QDataStream::ReadCorruptData)
                throw std::runtime_error("SCError: no such error type, or no such code for it");
            break;
        case SCValType::SCV_LEDGER_KEY_NONCE:
            in >> v.nonceKey; break;
        case SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE:
            break;
        case SCValType::SCV_EXECUTABLE_TAG: {
            qint32 n = 0; in >> n;
            checkWireCount(in, n, 1, "SCV_EXECUTABLE_TAG");
            v.executableTag.resize(n);
            if (n > 0) in.readRawData(v.executableTag.data(), n);
            readXdrPadding(in, n, "SCV_EXECUTABLE_TAG");
            break;
        }
        case SCValType::SCV_CONTRACT_INSTANCE: {
            v.instance = QSharedPointer<SCContractInstance>::create();
            in >> v.instance->executable;
            qint32 present = 0; in >> present;
            checkXdrFlag(in, present, "SCContractInstance storage");
            if (present) {
                // Bare SCMap on the wire (see the writer); rebuild it as the
                // SCV_MAP the rest of the SDK expects to find here.
                qint32 n = 0; in >> n;
                checkWireCount(in, n, 8, "SCContractInstance storage");
                v.instance->storage = QSharedPointer<SCVal>::create();
                v.instance->storage->type = SCValType::SCV_MAP;
                v.instance->storage->map = QSharedPointer<QList<SCMapEntry>>::create();   // not reserved, as SCVec
                for (qint32 i = 0; i < n; ++i) {
                    checkWireElement(in, "SCContractInstance storage");
                    SCMapEntry e;
                    in >> e.key;
                    in >> e.val;
                    v.instance->storage->map->append(e);
                }
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
