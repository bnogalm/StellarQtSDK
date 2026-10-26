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
     * The on-wire format is the canonical XDR LedgerKey, every arm of it. A
     * footprint names accounts and trustlines too — every Stellar Asset
     * Contract transfer does — and a reader that knew only CONTRACT_DATA /
     * CONTRACT_CODE / TTL refused those transactions and every simulation of
     * them.
     */

    /** LedgerKey::contractData inner struct. */
    struct LedgerKeyContractData
    {
        SCAddress contract;
        SCVal key;
        ContractDataDurability durability = ContractDataDurability::TEMPORARY;
    };
    /**
     * TEMPORARY (0) or PERSISTENT (1), and nothing else: the durability was
     * read and written as any int32, so a footprint with durability 3 (from a
     * faulty RPC, or bytes put together by hand) was re-encoded unchanged,
     * passed the byte-for-byte check and was signed. core, py and js refuse
     * it.
     */
    inline bool isContractDataDurability(qint32 v) { return v == 0 || v == 1; }
    inline QDataStream& operator<<(QDataStream& out, const LedgerKeyContractData& v) {
        if (!isContractDataDurability(static_cast<qint32>(v.durability))) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << v.contract << v.key << static_cast<qint32>(v.durability);
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerKeyContractData& v) {
        qint32 durability = 0;
        in >> v.contract >> v.key >> durability;
        if (in.status() == QDataStream::Ok && !isContractDataDurability(durability))
            in.setStatus(QDataStream::ReadCorruptData);
        v.durability = static_cast<ContractDataDurability>(durability);
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

    /**
     * The last ConfigSettingID in stellar-xdr curr
     * (CONFIG_SETTING_FREEZE_BYPASS_TXS_DELTA). A CONFIG_SETTING key names one
     * of 0..this; any other int32 was read and re-encoded as it was.
     */
    constexpr qint32 MAX_CONFIG_SETTING_ID = 20;

    struct SorobanLedgerKey
    {
        LedgerEntryType type = LedgerEntryType::CONTRACT_DATA;
        // Only one of these is meaningful at a time, governed by `type`, and
        // only that one is read or written.
        AccountID             accountID{};        // ACCOUNT, TRUSTLINE, OFFER (seller), DATA
        TrustLineAsset        trustLineAsset;     // TRUSTLINE
        qint64                offerID = 0;        // OFFER
        string64              dataName;           // DATA
        ClaimableBalanceID    claimableBalanceID; // CLAIMABLE_BALANCE
        PoolID                liquidityPoolID{};  // LIQUIDITY_POOL
        LedgerKeyContractData contractData;       // CONTRACT_DATA
        LedgerKeyContractCode contractCode;       // CONTRACT_CODE
        qint32                configSettingID = 0;// CONFIG_SETTING, 0..MAX_CONFIG_SETTING_ID
        LedgerKeyTtl          ttl;                // TTL
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanLedgerKey& k) {
        out << k.type;
        switch (k.type) {
        case LedgerEntryType::ACCOUNT:           out << k.accountID; break;
        case LedgerEntryType::TRUSTLINE:         out << k.accountID << k.trustLineAsset; break;
        case LedgerEntryType::OFFER:             out << k.accountID << k.offerID; break;
        case LedgerEntryType::DATA:              out << k.accountID << k.dataName; break;
        case LedgerEntryType::CLAIMABLE_BALANCE: out << k.claimableBalanceID; break;
        case LedgerEntryType::LIQUIDITY_POOL:    out << k.liquidityPoolID; break;
        case LedgerEntryType::CONTRACT_DATA:     out << k.contractData; break;
        case LedgerEntryType::CONTRACT_CODE:     out << k.contractCode; break;
        case LedgerEntryType::CONFIG_SETTING:
            if (k.configSettingID < 0 || k.configSettingID > MAX_CONFIG_SETTING_ID) {
                out.setStatus(QDataStream::WriteFailed);
                return out;
            }
            out << k.configSettingID;
            break;
        case LedgerEntryType::TTL:               out << k.ttl; break;
        default:
            throw std::runtime_error("SorobanLedgerKey: unsupported LedgerEntryType");
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanLedgerKey& k) {
        in >> k.type;
        switch (k.type) {
        case LedgerEntryType::ACCOUNT:           in >> k.accountID; break;
        case LedgerEntryType::TRUSTLINE:         in >> k.accountID >> k.trustLineAsset; break;
        case LedgerEntryType::OFFER:             in >> k.accountID >> k.offerID; break;
        case LedgerEntryType::DATA:              in >> k.accountID >> k.dataName; break;
        case LedgerEntryType::CLAIMABLE_BALANCE: in >> k.claimableBalanceID; break;
        case LedgerEntryType::LIQUIDITY_POOL:    in >> k.liquidityPoolID; break;
        case LedgerEntryType::CONTRACT_DATA:     in >> k.contractData; break;
        case LedgerEntryType::CONTRACT_CODE:     in >> k.contractCode; break;
        case LedgerEntryType::CONFIG_SETTING:
            in >> k.configSettingID;
            if (in.status() == QDataStream::Ok
                    && (k.configSettingID < 0 || k.configSettingID > MAX_CONFIG_SETTING_ID))
                in.setStatus(QDataStream::ReadCorruptData);
            break;
        case LedgerEntryType::TTL:               in >> k.ttl; break;
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
    /** A length no larger than what is left to read (each key is at least 4 bytes). */
    inline bool footprintLengthFits(QDataStream& in, qint32 n) {
        return n >= 0 && (!in.device() || n <= in.device()->bytesAvailable() / 4);
    }
    /** Not reserved up front: a SorobanLedgerKey is hundreds of bytes in
     *  memory and as little as 8 on the wire, so reserve(count) let a short
     *  hostile footprint commit ~240 times its size (Qt 6, whose QList holds
     *  the keys themselves) before it was refused.
     *  The lists grow with what was actually read, as SCVec does. */
    inline QDataStream& operator>>(QDataStream& in, LedgerFootprint& f) {
        qint32 n; in >> n;
        if (!footprintLengthFits(in, n)) throw std::runtime_error("LedgerFootprint readOnly len");
        f.readOnly.clear();
        for (qint32 i = 0; i < n; ++i) {
            checkWireElement(in, "LedgerFootprint readOnly");
            SorobanLedgerKey k; in >> k; f.readOnly.append(k);
        }
        in >> n;
        if (!footprintLengthFits(in, n)) throw std::runtime_error("LedgerFootprint readWrite len");
        f.readWrite.clear();
        for (qint32 i = 0; i < n; ++i) {
            checkWireElement(in, "LedgerFootprint readWrite");
            SorobanLedgerKey k; in >> k; f.readWrite.append(k);
        }
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

    /**
     * The extension of SorobanTransactionData:
     *
     *   union switch (int v) {
     *   case 0: void;
     *   case 1: SorobanResourcesExtV0 resourceExt;   // uint32 archivedSorobanEntries<>
     *   }
     *
     * v = 1 is what RPC hands back since Protocol 23 when archived entries
     * must be restored automatically. It used to be read as a 4-byte
     * "reserved" int, and the archived-entry list was then misread as the
     * footprint.
     */
    struct SorobanTransactionDataExt
    {
        qint32 v = 0;
        QVector<quint32> archivedSorobanEntries;    // v == 1 only
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionDataExt& e) {
        out << e.v;
        if (e.v == 1) {
            out << static_cast<quint32>(e.archivedSorobanEntries.size());
            for (quint32 index : e.archivedSorobanEntries) out << index;
        } else if (e.v != 0) {
            out.setStatus(QDataStream::WriteFailed);
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionDataExt& e) {
        in >> e.v;
        e.archivedSorobanEntries.clear();
        if (e.v == 1) {
            quint32 n = 0;
            in >> n;
            if (in.device() && n > quint64(in.device()->bytesAvailable()) / 4) {
                in.setStatus(QDataStream::ReadCorruptData);
                return in;
            }
            e.archivedSorobanEntries.reserve(static_cast<int>(n));
            for (quint32 i = 0; i < n && in.status() == QDataStream::Ok; ++i) {
                quint32 index = 0;
                in >> index;
                e.archivedSorobanEntries.append(index);
            }
        } else if (e.v != 0) {
            in.setStatus(QDataStream::ReadCorruptData);   // an arm that does not exist
        }
        return in;
    }

    /** CAP-46 — full Soroban transaction data (footprint + resources + fee). */
    struct SorobanTransactionData
    {
        SorobanResources resources;
        qint64 resourceFee = 0;
        SorobanTransactionDataExt ext;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionData& d) {
        out << d.ext << d.resources << d.resourceFee;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionData& d) {
        in >> d.ext;
        if (in.status() != QDataStream::Ok)
            return in;                          // an unknown ext: the rest is not read as anything
        in >> d.resources >> d.resourceFee;
        return in;
    }

} // namespace stellar

#endif // SOROBANTXDATA_H
