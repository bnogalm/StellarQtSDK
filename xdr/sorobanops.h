#ifndef SOROBANOPS_H
#define SOROBANOPS_H

#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QSharedPointer>
#include <QtGlobal>
#include <stdexcept>

#include "xdrhelper.h"
#include "stellartypes.h"
#include "stellarledgerentries.h"
#include "scval.h"
#include "sorobantxdata.h"

namespace stellar
{
    using namespace xdr;

    // CAP-46-8 — Soroban host function invocation operands.

    /** Function name + args for SAC / WASM contract invocation. */
    struct InvokeContractArgs
    {
        SCAddress contractAddress;
        QByteArray functionName;  // SCSymbol (≤32 bytes)
        QList<SCVal> args;
    };
    inline QDataStream& operator<<(QDataStream& out, const InvokeContractArgs& v) {
        out << v.contractAddress;
        if (v.functionName.size() > 32) throw std::runtime_error("functionName > 32 bytes");
        out << static_cast<qint32>(v.functionName.size());
        if (!v.functionName.isEmpty()) out.writeRawData(v.functionName.constData(), v.functionName.size());
        const quint32 pad = (4 - (v.functionName.size() % 4)) % 4;
        if (pad) { char z[4] = {0,0,0,0}; out.writeRawData(z, pad); }
        out << static_cast<qint32>(v.args.size());
        for (const SCVal& a : v.args) out << a;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, InvokeContractArgs& v) {
        in >> v.contractAddress;
        qint32 n; in >> n;
        if (n < 0 || n > 32) throw std::runtime_error("functionName invalid length");
        v.functionName.resize(n);
        if (n > 0) in.readRawData(v.functionName.data(), n);
        const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
        if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
        in >> n;
        if (n < 0) throw std::runtime_error("invokeContract args negative");
        v.args.clear(); v.args.reserve(n);
        for (qint32 i = 0; i < n; ++i) { SCVal a; in >> a; v.args.append(a); }
        return in;
    }

    enum class ContractIDPreimageType : qint32
    {
        CONTRACT_ID_PREIMAGE_FROM_ADDRESS = 0,
        CONTRACT_ID_PREIMAGE_FROM_ASSET = 1
    };

    struct ContractIDPreimage
    {
        ContractIDPreimageType type = ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS;
        SCAddress fromAddressAddr;   // FROM_ADDRESS
        uint256 fromAddressSalt{};   // FROM_ADDRESS
        Asset fromAssetAsset;        // FROM_ASSET
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractIDPreimage& v) {
        out << v.type;
        switch (v.type) {
        case ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS:
            out << v.fromAddressAddr << v.fromAddressSalt; break;
        case ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ASSET:
            out << v.fromAssetAsset; break;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractIDPreimage& v) {
        in >> v.type;
        switch (v.type) {
        case ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS:
            in >> v.fromAddressAddr >> v.fromAddressSalt; break;
        case ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ASSET:
            in >> v.fromAssetAsset; break;
        }
        return in;
    }

    struct CreateContractArgs
    {
        ContractIDPreimage contractIDPreimage;
        ContractExecutable executable;
    };
    inline QDataStream& operator<<(QDataStream& out, const CreateContractArgs& v) {
        out << v.contractIDPreimage << v.executable; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, CreateContractArgs& v) {
        in >> v.contractIDPreimage >> v.executable; return in;
    }

    struct CreateContractArgsV2
    {
        ContractIDPreimage contractIDPreimage;
        ContractExecutable executable;
        QList<SCVal> constructorArgs;
    };
    inline QDataStream& operator<<(QDataStream& out, const CreateContractArgsV2& v) {
        out << v.contractIDPreimage << v.executable;
        out << static_cast<qint32>(v.constructorArgs.size());
        for (const SCVal& a : v.constructorArgs) out << a;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, CreateContractArgsV2& v) {
        in >> v.contractIDPreimage >> v.executable;
        qint32 n; in >> n;
        if (n < 0) throw std::runtime_error("constructorArgs negative");
        v.constructorArgs.clear(); v.constructorArgs.reserve(n);
        for (qint32 i = 0; i < n; ++i) { SCVal a; in >> a; v.constructorArgs.append(a); }
        return in;
    }

    enum class HostFunctionType : qint32
    {
        HOST_FUNCTION_TYPE_INVOKE_CONTRACT = 0,
        HOST_FUNCTION_TYPE_CREATE_CONTRACT = 1,
        HOST_FUNCTION_TYPE_UPLOAD_CONTRACT_WASM = 2,
        HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2 = 3
    };

    struct HostFunction
    {
        HostFunctionType type = HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT;
        InvokeContractArgs invokeContract;
        CreateContractArgs createContract;
        QByteArray uploadWasm;                  // opaque<> bytes
        CreateContractArgsV2 createContractV2;
    };
    inline QDataStream& operator<<(QDataStream& out, const HostFunction& v) {
        out << v.type;
        switch (v.type) {
        case HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT:
            out << v.invokeContract; break;
        case HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT:
            out << v.createContract; break;
        case HostFunctionType::HOST_FUNCTION_TYPE_UPLOAD_CONTRACT_WASM: {
            out << static_cast<qint32>(v.uploadWasm.size());
            if (!v.uploadWasm.isEmpty())
                out.writeRawData(v.uploadWasm.constData(), v.uploadWasm.size());
            const quint32 pad = (4 - (v.uploadWasm.size() % 4)) % 4;
            if (pad) { char z[4] = {0,0,0,0}; out.writeRawData(z, pad); }
            break;
        }
        case HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2:
            out << v.createContractV2; break;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, HostFunction& v) {
        in >> v.type;
        switch (v.type) {
        case HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT:
            in >> v.invokeContract; break;
        case HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT:
            in >> v.createContract; break;
        case HostFunctionType::HOST_FUNCTION_TYPE_UPLOAD_CONTRACT_WASM: {
            qint32 n; in >> n;
            if (n < 0) throw std::runtime_error("uploadWasm negative");
            v.uploadWasm.resize(n);
            if (n > 0) in.readRawData(v.uploadWasm.data(), n);
            const quint32 pad = (4 - (static_cast<quint32>(n) % 4)) % 4;
            if (pad) { char z[4]; in.readRawData(z, static_cast<int>(pad)); }
            break;
        }
        case HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2:
            in >> v.createContractV2; break;
        }
        return in;
    }

    // CAP-46-8 — Soroban authorization. Minimal coverage in 0.10.0:
    // SOURCE_ACCOUNT credentials (caller==invoker), and ADDRESS for
    // pre-built signed entries from external code.

    enum class SorobanCredentialsType : qint32
    {
        SOROBAN_CREDENTIALS_SOURCE_ACCOUNT = 0,
        SOROBAN_CREDENTIALS_ADDRESS = 1,
        // CAP-71 (Protocol 27). ADDRESS_V2 carries the same payload as ADDRESS
        // but its signature is made over ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_
        // WITH_ADDRESS, which binds the signer's address into the preimage and
        // so prevents replay between accounts that share a private key.
        SOROBAN_CREDENTIALS_ADDRESS_V2 = 2,
        SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES = 3
    };

    struct SorobanAddressCredentials
    {
        SCAddress address;
        qint64 nonce = 0;
        quint32 signatureExpirationLedger = 0;
        SCVal signature;  // user-defined SCVal; typically SCV_VEC of SCV_MAP
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanAddressCredentials& v) {
        out << v.address << v.nonce << v.signatureExpirationLedger << v.signature;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanAddressCredentials& v) {
        in >> v.address >> v.nonce >> v.signatureExpirationLedger >> v.signature;
        return in;
    }

    /**
     * SorobanDelegateSignature (CAP-71, Protocol 27). Self-recursive: a
     * delegate may itself be backed by further delegates. `nestedDelegates` is
     * held by QSharedPointer to break the self-containment, exactly as SCVal
     * does for its vec/map; a null pointer means the empty list.
     */
    struct SorobanDelegateSignature;
    inline QDataStream& operator<<(QDataStream& out, const SorobanDelegateSignature& d);
    inline QDataStream& operator>>(QDataStream& in, SorobanDelegateSignature& d);

    /** Upper bound on a delegate array. CAP-71 sets no explicit cap, but an
     *  unbounded 4-byte count read from a hostile stream is an OOM primitive:
     *  0x7FFFFFFF entries of a ~400-byte struct asks for hundreds of GB. Same
     *  reasoning as the `xdr array length out of bounds` guard in xdrhelper.h. */
    static const qint32 MAX_SOROBAN_DELEGATES = 4096;

    /** Delegates nest recursively, so decoding must be depth-limited: ~44 bytes
     *  of wire buys a nesting level that costs ~400 bytes of stack, i.e. ~110 KB
     *  of hostile XDR overflows a 1 MB stack — a crash no catch() can intercept. */
    static const int MAX_SOROBAN_DELEGATE_DEPTH = 32;

    inline int& sorobanDelegateDepth() { static thread_local int depth = 0; return depth; }
    struct SorobanDelegateDepthGuard
    {
        SorobanDelegateDepthGuard() {
            if (++sorobanDelegateDepth() > MAX_SOROBAN_DELEGATE_DEPTH) {
                --sorobanDelegateDepth();   // no destructor runs when a ctor throws
                throw std::runtime_error("SorobanDelegateSignature: delegate nesting too deep");
            }
        }
        ~SorobanDelegateDepthGuard() { --sorobanDelegateDepth(); }
    };

    /** CAP-71-01 requires every delegate array — top level and every nested one
     *  — to be sorted by ascending `address` with no duplicates; Core rejects
     *  the whole invocation before entering the contract otherwise. XDR order
     *  is the lexicographic order of the encoded bytes. */
    inline QByteArray encodedSCAddress(const SCAddress& a) {
        QByteArray bytes;
        QDataStream s(&bytes, QIODevice::WriteOnly);
        s << a;
        return bytes;
    }
    inline void checkDelegatesSortedUnique(const QList<SorobanDelegateSignature>& xs);

    struct SorobanDelegateSignature
    {
        SCAddress address;
        SCVal signature;
        QSharedPointer<QList<SorobanDelegateSignature>> nestedDelegates;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanDelegateSignature& d) {
        out << d.address << d.signature;
        const qint32 n = d.nestedDelegates ? static_cast<qint32>(d.nestedDelegates->size()) : 0;
        out << n;
        if (d.nestedDelegates) {
            checkDelegatesSortedUnique(*d.nestedDelegates);
            for (const SorobanDelegateSignature& child : *d.nestedDelegates) out << child;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanDelegateSignature& d) {
        SorobanDelegateDepthGuard depthGuard;
        in >> d.address >> d.signature;
        qint32 n = 0; in >> n;
        if (n < 0 || n > MAX_SOROBAN_DELEGATES)
            throw std::runtime_error("SorobanDelegateSignature: delegate count out of bounds");
        d.nestedDelegates.reset();   // decoding must replace, never merge
        if (n > 0) {
            d.nestedDelegates = QSharedPointer<QList<SorobanDelegateSignature>>::create();
            for (qint32 i = 0; i < n; ++i) {
                // QDataStream never throws on exhaustion: it flags ReadPastEnd
                // and hands back default values, so without this the loop would
                // append n empty delegates for a truncated stream.
                if (in.status() != QDataStream::Ok)
                    throw std::runtime_error("SorobanDelegateSignature: truncated delegate array");
                SorobanDelegateSignature child;
                in >> child;
                d.nestedDelegates->append(child);
            }
            checkDelegatesSortedUnique(*d.nestedDelegates);
        }
        return in;
    }

    inline void checkDelegatesSortedUnique(const QList<SorobanDelegateSignature>& xs) {
        for (int i = 1; i < xs.size(); ++i) {
            const QByteArray prev = encodedSCAddress(xs.at(i - 1).address);
            const QByteArray cur  = encodedSCAddress(xs.at(i).address);
            if (cur == prev)
                throw std::runtime_error("SorobanDelegateSignature: duplicate delegate address");
            if (cur < prev)
                throw std::runtime_error("SorobanDelegateSignature: delegates must be sorted by ascending address");
        }
    }

    struct SorobanAddressCredentialsWithDelegates
    {
        SorobanAddressCredentials addressCredentials;
        QList<SorobanDelegateSignature> delegates;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanAddressCredentialsWithDelegates& v) {
        out << v.addressCredentials;
        checkDelegatesSortedUnique(v.delegates);
        out << static_cast<qint32>(v.delegates.size());
        for (const SorobanDelegateSignature& d : v.delegates) out << d;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanAddressCredentialsWithDelegates& v) {
        in >> v.addressCredentials;
        qint32 n = 0; in >> n;
        if (n < 0 || n > MAX_SOROBAN_DELEGATES)
            throw std::runtime_error("SorobanAddressCredentialsWithDelegates: delegate count out of bounds");
        v.delegates.clear();
        for (qint32 i = 0; i < n; ++i) {
            if (in.status() != QDataStream::Ok)
                throw std::runtime_error("SorobanAddressCredentialsWithDelegates: truncated delegate array");
            SorobanDelegateSignature d;
            in >> d;
            v.delegates.append(d);
        }
        checkDelegatesSortedUnique(v.delegates);
        return in;
    }

    struct SorobanCredentials
    {
        SorobanCredentialsType type = SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        SorobanAddressCredentials address;                        // ADDRESS and ADDRESS_V2
        SorobanAddressCredentialsWithDelegates addressWithDelegates;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanCredentials& v) {
        out << v.type;
        switch (v.type) {
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT:
            break;  // void arm
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS:
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2:
            out << v.address; break;
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES:
            out << v.addressWithDelegates; break;
        default:
            throw std::runtime_error("SorobanCredentials: unknown credentials type");
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanCredentials& v) {
        in >> v.type;
        switch (v.type) {
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT:
            break;  // void arm
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS:
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2:
            in >> v.address; break;
        case SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES:
            in >> v.addressWithDelegates; break;
        default:
            // Reading only the discriminant would leave the payload in the
            // stream and desync every field after it.
            throw std::runtime_error("SorobanCredentials: unknown credentials type");
        }
        return in;
    }

    /**
     * SorobanAuthorizedInvocation tree: function + nested subInvocations.
     * Held by QSharedPointer to break the recursive self-containment.
     */
    enum class SorobanAuthorizedFunctionType : qint32
    {
        SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN = 0,
        SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_HOST_FN = 1,
        SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_V2_HOST_FN = 2
    };

    struct SorobanAuthorizedInvocation;

    struct SorobanAuthorizedFunction
    {
        SorobanAuthorizedFunctionType type =
            SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        InvokeContractArgs contractFn;
        CreateContractArgs createContractHostFn;
        CreateContractArgsV2 createContractV2HostFn;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanAuthorizedFunction& v) {
        out << v.type;
        switch (v.type) {
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN:
            out << v.contractFn; break;
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_HOST_FN:
            out << v.createContractHostFn; break;
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_V2_HOST_FN:
            out << v.createContractV2HostFn; break;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanAuthorizedFunction& v) {
        in >> v.type;
        switch (v.type) {
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN:
            in >> v.contractFn; break;
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_HOST_FN:
            in >> v.createContractHostFn; break;
        case SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_V2_HOST_FN:
            in >> v.createContractV2HostFn; break;
        }
        return in;
    }

    struct SorobanAuthorizedInvocation
    {
        SorobanAuthorizedFunction function;
        QSharedPointer<QList<SorobanAuthorizedInvocation>> subInvocations;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanAuthorizedInvocation& v) {
        out << v.function;
        const QList<SorobanAuthorizedInvocation>* xs = v.subInvocations.data();
        const qint32 n = xs ? static_cast<qint32>(xs->size()) : 0;
        out << n;
        if (xs) for (const auto& s : *xs) out << s;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanAuthorizedInvocation& v) {
        in >> v.function;
        qint32 n; in >> n;
        if (n < 0) throw std::runtime_error("subInvocations negative");
        v.subInvocations = QSharedPointer<QList<SorobanAuthorizedInvocation>>::create();
        v.subInvocations->reserve(n);
        for (qint32 i = 0; i < n; ++i) {
            SorobanAuthorizedInvocation s; in >> s; v.subInvocations->append(s);
        }
        return in;
    }

    struct SorobanAuthorizationEntry
    {
        SorobanCredentials credentials;
        SorobanAuthorizedInvocation rootInvocation;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanAuthorizationEntry& v) {
        out << v.credentials << v.rootInvocation; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanAuthorizationEntry& v) {
        in >> v.credentials >> v.rootInvocation; return in;
    }

    // CAP-46-8 — Soroban operations.

    struct InvokeHostFunctionOp
    {
        HostFunction hostFunction;
        QList<SorobanAuthorizationEntry> auth;
    };
    inline QDataStream& operator<<(QDataStream& out, const InvokeHostFunctionOp& v) {
        out << v.hostFunction;
        out << static_cast<qint32>(v.auth.size());
        for (const auto& e : v.auth) out << e;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, InvokeHostFunctionOp& v) {
        in >> v.hostFunction;
        qint32 n; in >> n;
        if (n < 0) throw std::runtime_error("auth array negative");
        v.auth.clear(); v.auth.reserve(n);
        for (qint32 i = 0; i < n; ++i) {
            SorobanAuthorizationEntry e; in >> e; v.auth.append(e);
        }
        return in;
    }

    struct ExtendFootprintTTLOp
    {
        Reserved ext;
        quint32 extendTo = 0;
    };
    inline QDataStream& operator<<(QDataStream& out, const ExtendFootprintTTLOp& v) {
        out << v.ext << v.extendTo; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ExtendFootprintTTLOp& v) {
        in >> v.ext >> v.extendTo; return in;
    }

    struct RestoreFootprintOp
    {
        Reserved ext;
    };
    inline QDataStream& operator<<(QDataStream& out, const RestoreFootprintOp& v) {
        out << v.ext; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, RestoreFootprintOp& v) {
        in >> v.ext; return in;
    }

} // namespace stellar

#endif // SOROBANOPS_H
