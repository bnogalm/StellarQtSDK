#ifndef SOROBANTXDATA_H
#define SOROBANTXDATA_H

#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QtGlobal>

#include "xdrhelper.h"
#include "stellartypes.h"
#include "stellarledgerentries.h"
#include "scval.h"

namespace stellar
{
    using namespace xdr;

    /**
     * SorobanLedgerKey — discriminated union for the LedgerKey variants used
     * inside a Soroban footprint (CAP-46). Kept separate from the legacy
     * `stellar::LedgerKey` to avoid widening that union (which has tight
     * placement-new plumbing) and to keep the include graph one-directional:
     *
     *   stellartypes.h  ←  stellarledgerentries.h  ←  scval.h  ←  sorobantxdata.h
     *
     * The on-wire format matches the canonical XDR LedgerKey for the
     * Soroban discriminators (CONTRACT_DATA / CONTRACT_CODE / TTL). Builder
     * usage focuses on these three; the other discriminators are not
     * commonly part of a Soroban footprint.
     */

    /** LedgerKey::contractData inner struct. */
    struct LedgerKeyContractData
    {
        SCAddress contract;
        SCVal key;
        ContractDataDurability durability = ContractDataDurability::TEMPORARY;
    };
    inline QDataStream& operator<<(QDataStream& out, const LedgerKeyContractData& v) {
        out << v.contract << v.key << v.durability;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerKeyContractData& v) {
        in >> v.contract >> v.key >> v.durability;
        return in;
    }

    /** LedgerKey::contractCode inner struct (carries the wasm hash). */
    struct LedgerKeyContractCode { uint256 hash{}; };
    inline QDataStream& operator<<(QDataStream& out, const LedgerKeyContractCode& v) {
        out << v.hash; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerKeyContractCode& v) {
        in >> v.hash; return in;
    }

    /** LedgerKey::ttl inner struct. */
    struct LedgerKeyTtl { uint256 keyHash{}; };
    inline QDataStream& operator<<(QDataStream& out, const LedgerKeyTtl& v) {
        out << v.keyHash; return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerKeyTtl& v) {
        in >> v.keyHash; return in;
    }

    struct SorobanLedgerKey
    {
        LedgerEntryType type = LedgerEntryType::CONTRACT_DATA;
        // Only one of these is meaningful at a time, governed by `type`.
        // They are POD (no destructors), so we don't need placement-new
        // plumbing.
        LedgerKeyContractData contractData;
        LedgerKeyContractCode contractCode;
        LedgerKeyTtl          ttl;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanLedgerKey& k) {
        out << k.type;
        switch (k.type) {
        case LedgerEntryType::CONTRACT_DATA: out << k.contractData; break;
        case LedgerEntryType::CONTRACT_CODE: out << k.contractCode; break;
        case LedgerEntryType::TTL:           out << k.ttl; break;
        default:
            throw std::runtime_error("SorobanLedgerKey: unsupported LedgerEntryType");
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanLedgerKey& k) {
        in >> k.type;
        switch (k.type) {
        case LedgerEntryType::CONTRACT_DATA: in >> k.contractData; break;
        case LedgerEntryType::CONTRACT_CODE: in >> k.contractCode; break;
        case LedgerEntryType::TTL:           in >> k.ttl; break;
        default:
            throw std::runtime_error("SorobanLedgerKey: unsupported LedgerEntryType on read");
        }
        return in;
    }

    /** CAP-46 — footprint: read-only and read-write key sets. */
    struct LedgerFootprint
    {
        QList<SorobanLedgerKey> readOnly;
        QList<SorobanLedgerKey> readWrite;
    };
    inline QDataStream& operator<<(QDataStream& out, const LedgerFootprint& f) {
        out << static_cast<qint32>(f.readOnly.size());
        for (const SorobanLedgerKey& k : f.readOnly) out << k;
        out << static_cast<qint32>(f.readWrite.size());
        for (const SorobanLedgerKey& k : f.readWrite) out << k;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerFootprint& f) {
        qint32 n; in >> n;
        if (n < 0) throw std::runtime_error("LedgerFootprint readOnly len");
        f.readOnly.clear(); f.readOnly.reserve(n);
        for (qint32 i = 0; i < n; ++i) { SorobanLedgerKey k; in >> k; f.readOnly.append(k); }
        in >> n;
        if (n < 0) throw std::runtime_error("LedgerFootprint readWrite len");
        f.readWrite.clear(); f.readWrite.reserve(n);
        for (qint32 i = 0; i < n; ++i) { SorobanLedgerKey k; in >> k; f.readWrite.append(k); }
        return in;
    }

    /** CAP-46 — Soroban resource declarations (instructions + IO budget). */
    struct SorobanResources
    {
        LedgerFootprint footprint;
        quint32 instructions = 0;
        quint32 diskReadBytes = 0;
        quint32 writeBytes = 0;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanResources& r) {
        out << r.footprint << r.instructions << r.diskReadBytes << r.writeBytes;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanResources& r) {
        in >> r.footprint >> r.instructions >> r.diskReadBytes >> r.writeBytes;
        return in;
    }

    /** CAP-46 — full Soroban transaction data (footprint + resources + fee). */
    struct SorobanTransactionData
    {
        SorobanResources resources;
        qint64 resourceFee = 0;
        Reserved ext;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionData& d) {
        out << d.ext << d.resources << d.resourceFee;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionData& d) {
        in >> d.ext >> d.resources >> d.resourceFee;
        return in;
    }

} // namespace stellar

#endif // SOROBANTXDATA_H
