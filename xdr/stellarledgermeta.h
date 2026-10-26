#ifndef STELLARLEDGERMETA_H
#define STELLARLEDGERMETA_H

#include <QByteArray>
#include <QDataStream>
#include <QVector>
#include <QtGlobal>
#include <stdexcept>

#include "xdrhelper.h"
#include "stellartypes.h"
#include "stellarledgerentries.h"
#include "scval.h"
#include "sorobantxdata.h"

/**
 * TransactionMeta (Stellar-ledger.x, stellar-xdr curr): what applying a
 * transaction changed, and for a Soroban call the value it returned and the
 * events it emitted. It is what js-stellar-sdk decodes for
 * rpc.Server.getTransaction's `returnValue` (meta v3/v4 sorobanMeta.returnValue).
 *
 * Every arm a transaction's meta can hold is read, versions 0 to 4, ledger
 * entries of every type but CONFIG_SETTING (written only by network upgrades,
 * never by a transaction): one is refused as corrupt data rather than skipped,
 * since XDR cannot be skipped without knowing its shape. Readers mark the
 * stream ReadCorruptData on anything they do not know; use
 * stellar::decodeTransactionMeta(), which also requires the bytes to re-encode
 * exactly, so nothing is read as something else.
 */
namespace stellar
{
    using namespace xdr;

    // ─── Soroban ledger entries ────────────────────────────────────────

    /** ContractDataEntry: ext, then the contract data key's fields, then the value. */
    struct ContractDataEntry
    {
        Reserved ext;
        LedgerKeyContractData keyFields;   // contract, key, durability
        SCVal val;
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractDataEntry& e) {
        out << e.ext << e.keyFields << e.val;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractDataEntry& e) {
        in >> e.ext >> e.keyFields >> e.val;
        return in;
    }

    struct ContractCodeCostInputs
    {
        Reserved ext;
        quint32 nInstructions = 0, nFunctions = 0, nGlobals = 0, nTableEntries = 0, nTypes = 0,
                nDataSegments = 0, nElemSegments = 0, nImports = 0, nExports = 0, nDataSegmentBytes = 0;
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractCodeCostInputs& c) {
        out << c.ext << c.nInstructions << c.nFunctions << c.nGlobals << c.nTableEntries << c.nTypes
            << c.nDataSegments << c.nElemSegments << c.nImports << c.nExports << c.nDataSegmentBytes;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractCodeCostInputs& c) {
        in >> c.ext >> c.nInstructions >> c.nFunctions >> c.nGlobals >> c.nTableEntries >> c.nTypes
           >> c.nDataSegments >> c.nElemSegments >> c.nImports >> c.nExports >> c.nDataSegmentBytes;
        return in;
    }

    /** An XDR variable opaque (`opaque x<>`): length, bytes, zero padding. */
    inline QDataStream& writeXdrOpaque(QDataStream& out, const QByteArray& bytes) {
        out << quint32(bytes.size());
        if (!bytes.isEmpty())
            out.writeRawData(bytes.constData(), bytes.size());
        const int pad = (4 - (bytes.size() % 4)) % 4;
        if (pad) { const char zero[4] = {0, 0, 0, 0}; out.writeRawData(zero, pad); }
        return out;
    }
    inline QDataStream& readXdrOpaque(QDataStream& in, QByteArray& bytes) {
        bytes.clear();
        quint32 n = 0;
        in >> n;
        if (in.status() != QDataStream::Ok)
            return in;
        if (in.device() && quint64(n) > quint64(in.device()->bytesAvailable())) {
            in.setStatus(QDataStream::ReadPastEnd);
            return in;
        }
        bytes.resize(int(n));
        if (n && in.readRawData(bytes.data(), int(n)) != int(n)) {
            in.setStatus(QDataStream::ReadPastEnd);
            return in;
        }
        const int pad = (4 - int(n % 4)) % 4;
        if (pad) {
            char z[4] = {0, 0, 0, 0};
            if (in.readRawData(z, pad) != pad || z[0] || z[1] || z[2] || z[3])
                in.setStatus(QDataStream::ReadCorruptData);
        }
        return in;
    }

    struct ContractCodeEntry
    {
        qint32 extV = 0;                    // 0, or 1 with the cost inputs
        Reserved v1Ext;
        ContractCodeCostInputs costInputs;  // case 1
        Hash hash{};
        QByteArray code;
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractCodeEntry& e) {
        if (e.extV != 0 && e.extV != 1) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << e.extV;
        if (e.extV == 1)
            out << e.v1Ext << e.costInputs;
        out << e.hash;
        writeXdrOpaque(out, e.code);
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractCodeEntry& e) {
        in >> e.extV;
        if (e.extV == 1)
            in >> e.v1Ext >> e.costInputs;
        else if (e.extV != 0) { in.setStatus(QDataStream::ReadCorruptData); return in; }
        in >> e.hash;
        readXdrOpaque(in, e.code);
        return in;
    }

    struct TTLEntry
    {
        Hash keyHash{};                 // SHA-256 of the LedgerKey it keeps alive
        quint32 liveUntilLedgerSeq = 0;
    };
    inline QDataStream& operator<<(QDataStream& out, const TTLEntry& e) {
        out << e.keyHash << e.liveUntilLedgerSeq;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, TTLEntry& e) {
        in >> e.keyHash >> e.liveUntilLedgerSeq;
        return in;
    }

    /**
     * A LedgerEntry of any type a transaction's meta can carry: the six
     * classic ones (as stellar::LedgerEntry holds them) and contract data,
     * contract code and TTLs. CONFIG_SETTING is refused (see the file
     * comment). stellar::LedgerEntry itself stays classic-only, as 1.0 has it.
     */
    struct FullLedgerEntry
    {
        quint32 lastModifiedLedgerSeq = 0;
        LedgerEntryType type = LedgerEntryType::ACCOUNT;
        AccountEntry account;
        TrustLineEntry trustLine;
        OfferEntry offer;
        DataEntry data;
        ClaimableBalanceEntry claimableBalance;
        LiquidityPoolEntry liquidityPool;
        ContractDataEntry contractData;
        ContractCodeEntry contractCode;
        TTLEntry ttl;

        Reserved ext;               // 0 or 1
        LedgerEntryExtensionV1 v1;  // case 1: the sponsor
    };
    inline QDataStream& operator<<(QDataStream& out, const FullLedgerEntry& e) {
        if (e.ext.reserved != 0 && e.ext.reserved != 1) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << e.lastModifiedLedgerSeq << e.type;
        switch (e.type) {
        case LedgerEntryType::ACCOUNT:           out << e.account; break;
        case LedgerEntryType::TRUSTLINE:         out << e.trustLine; break;
        case LedgerEntryType::OFFER:             out << e.offer; break;
        case LedgerEntryType::DATA:              out << e.data; break;
        case LedgerEntryType::CLAIMABLE_BALANCE: out << e.claimableBalance; break;
        case LedgerEntryType::LIQUIDITY_POOL:    out << e.liquidityPool; break;
        case LedgerEntryType::CONTRACT_DATA:     out << e.contractData; break;
        case LedgerEntryType::CONTRACT_CODE:     out << e.contractCode; break;
        case LedgerEntryType::TTL:               out << e.ttl; break;
        default: out.setStatus(QDataStream::WriteFailed); return out;
        }
        out << e.ext.reserved;
        if (e.ext.reserved == 1)
            out << e.v1;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, FullLedgerEntry& e) {
        in >> e.lastModifiedLedgerSeq >> e.type;
        switch (e.type) {
        case LedgerEntryType::ACCOUNT:           in >> e.account; break;
        case LedgerEntryType::TRUSTLINE:         in >> e.trustLine; break;
        case LedgerEntryType::OFFER:             in >> e.offer; break;
        case LedgerEntryType::DATA:              in >> e.data; break;
        case LedgerEntryType::CLAIMABLE_BALANCE: in >> e.claimableBalance; break;
        case LedgerEntryType::LIQUIDITY_POOL:    in >> e.liquidityPool; break;
        case LedgerEntryType::CONTRACT_DATA:     in >> e.contractData; break;
        case LedgerEntryType::CONTRACT_CODE:     in >> e.contractCode; break;
        case LedgerEntryType::TTL:               in >> e.ttl; break;
        default: in.setStatus(QDataStream::ReadCorruptData); return in;
        }
        in >> e.ext.reserved;
        e.v1 = LedgerEntryExtensionV1();
        if (e.ext.reserved == 1)
            in >> e.v1;
        else if (e.ext.reserved != 0)
            in.setStatus(QDataStream::ReadCorruptData);
        return in;
    }

    // ─── Ledger entry changes ──────────────────────────────────────────

    enum class LedgerEntryChangeType : qint32
    {
        LEDGER_ENTRY_CREATED = 0,
        LEDGER_ENTRY_UPDATED = 1,
        LEDGER_ENTRY_REMOVED = 2,
        LEDGER_ENTRY_STATE = 3,
        LEDGER_ENTRY_RESTORED = 4   // an archived entry restored (Protocol 23)
    };

    /** One change: `entry` for every type but REMOVED, which carries `removed`. */
    struct LedgerEntryChange
    {
        LedgerEntryChangeType type = LedgerEntryChangeType::LEDGER_ENTRY_STATE;
        FullLedgerEntry entry;      // CREATED, UPDATED, STATE, RESTORED
        SorobanLedgerKey removed;   // REMOVED (every LedgerKey arm)
    };
    inline QDataStream& operator<<(QDataStream& out, const LedgerEntryChange& c) {
        const qint32 t = static_cast<qint32>(c.type);
        if (t < 0 || t > 4) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << t;
        if (c.type == LedgerEntryChangeType::LEDGER_ENTRY_REMOVED)
            out << c.removed;
        else
            out << c.entry;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, LedgerEntryChange& c) {
        qint32 t = -1;
        in >> t;
        if (t < 0 || t > 4) { in.setStatus(QDataStream::ReadCorruptData); return in; }
        c.type = static_cast<LedgerEntryChangeType>(t);
        if (c.type == LedgerEntryChangeType::LEDGER_ENTRY_REMOVED)
            in >> c.removed;
        else
            in >> c.entry;
        return in;
    }
    typedef QVector<LedgerEntryChange> LedgerEntryChanges;

    struct OperationMeta
    {
        LedgerEntryChanges changes;
    };
    inline QDataStream& operator<<(QDataStream& out, const OperationMeta& m) { return writeXdrList(out, m.changes); }
    inline QDataStream& operator>>(QDataStream& in, OperationMeta& m) { return readXdrList(in, m.changes); }

    // ─── Events ────────────────────────────────────────────────────────

    enum class ContractEventType : qint32 { SYSTEM = 0, CONTRACT = 1, DIAGNOSTIC = 2 };

    struct ContractEvent
    {
        Reserved ext;
        bool hasContractId = false;
        Hash contractId{};           // ContractID* contractID
        ContractEventType type = ContractEventType::CONTRACT;
        qint32 bodyV = 0;            // the body's only arm, 0
        QVector<SCVal> topics;
        SCVal data;
    };
    inline QDataStream& operator<<(QDataStream& out, const ContractEvent& e) {
        const qint32 t = static_cast<qint32>(e.type);
        if (t < 0 || t > 2 || e.bodyV != 0) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << e.ext;
        writeXdrBool(out, e.hasContractId);
        if (e.hasContractId)
            out << e.contractId;
        out << t << e.bodyV;
        writeXdrList(out, e.topics);
        out << e.data;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, ContractEvent& e) {
        in >> e.ext;
        readXdrBool(in, e.hasContractId);
        if (e.hasContractId)
            in >> e.contractId;
        qint32 t = -1;
        in >> t >> e.bodyV;
        if (t < 0 || t > 2 || e.bodyV != 0) { in.setStatus(QDataStream::ReadCorruptData); return in; }
        e.type = static_cast<ContractEventType>(t);
        readXdrList(in, e.topics);
        if (in.status() == QDataStream::Ok)
            in >> e.data;
        return in;
    }

    struct DiagnosticEvent
    {
        bool inSuccessfulContractCall = false;
        ContractEvent event;
    };
    inline QDataStream& operator<<(QDataStream& out, const DiagnosticEvent& e) {
        writeXdrBool(out, e.inSuccessfulContractCall);
        out << e.event;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, DiagnosticEvent& e) {
        readXdrBool(in, e.inSuccessfulContractCall);
        in >> e.event;
        return in;
    }

    enum class TransactionEventStage : qint32
    {
        TRANSACTION_EVENT_STAGE_BEFORE_ALL_TXS = 0,
        TRANSACTION_EVENT_STAGE_AFTER_TX = 1,
        TRANSACTION_EVENT_STAGE_AFTER_ALL_TXS = 2
    };

    /** A transaction-level event (today the fee charged or refunded). */
    struct TransactionEvent
    {
        TransactionEventStage stage = TransactionEventStage::TRANSACTION_EVENT_STAGE_BEFORE_ALL_TXS;
        ContractEvent event;
    };
    inline QDataStream& operator<<(QDataStream& out, const TransactionEvent& e) {
        const qint32 s = static_cast<qint32>(e.stage);
        if (s < 0 || s > 2) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << s << e.event;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, TransactionEvent& e) {
        qint32 s = -1;
        in >> s;
        if (s < 0 || s > 2) { in.setStatus(QDataStream::ReadCorruptData); return in; }
        e.stage = static_cast<TransactionEventStage>(s);
        in >> e.event;
        return in;
    }

    // ─── Soroban meta ──────────────────────────────────────────────────

    /** SorobanTransactionMetaExt: 0, or 1 with the resource fees charged. */
    struct SorobanTransactionMetaExt
    {
        qint32 v = 0;
        Reserved v1Ext;
        qint64 totalNonRefundableResourceFeeCharged = 0;
        qint64 totalRefundableResourceFeeCharged = 0;
        qint64 rentFeeCharged = 0;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionMetaExt& e) {
        if (e.v != 0 && e.v != 1) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << e.v;
        if (e.v == 1)
            out << e.v1Ext << e.totalNonRefundableResourceFeeCharged
                << e.totalRefundableResourceFeeCharged << e.rentFeeCharged;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionMetaExt& e) {
        in >> e.v;
        if (e.v == 1)
            in >> e.v1Ext >> e.totalNonRefundableResourceFeeCharged
               >> e.totalRefundableResourceFeeCharged >> e.rentFeeCharged;
        else if (e.v != 0)
            in.setStatus(QDataStream::ReadCorruptData);
        return in;
    }

    /** Meta v3's Soroban part: the events, the return value, the diagnostics. */
    struct SorobanTransactionMeta
    {
        SorobanTransactionMetaExt ext;
        QVector<ContractEvent> events;
        SCVal returnValue;
        QVector<DiagnosticEvent> diagnosticEvents;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionMeta& m) {
        out << m.ext;
        writeXdrList(out, m.events);
        out << m.returnValue;
        writeXdrList(out, m.diagnosticEvents);
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionMeta& m) {
        in >> m.ext;
        readXdrList(in, m.events);
        if (in.status() == QDataStream::Ok)
            in >> m.returnValue;
        readXdrList(in, m.diagnosticEvents);
        return in;
    }

    /** Meta v4's Soroban part: the return value, when the call has one. */
    struct SorobanTransactionMetaV2
    {
        SorobanTransactionMetaExt ext;
        bool hasReturnValue = false;
        SCVal returnValue;
    };
    inline QDataStream& operator<<(QDataStream& out, const SorobanTransactionMetaV2& m) {
        out << m.ext;
        writeXdrBool(out, m.hasReturnValue);
        if (m.hasReturnValue)
            out << m.returnValue;
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, SorobanTransactionMetaV2& m) {
        in >> m.ext;
        readXdrBool(in, m.hasReturnValue);
        if (m.hasReturnValue)
            in >> m.returnValue;
        return in;
    }

    /** Meta v4's per-operation part: its changes and the events it emitted. */
    struct OperationMetaV2
    {
        Reserved ext;
        LedgerEntryChanges changes;
        QVector<ContractEvent> events;
    };
    inline QDataStream& operator<<(QDataStream& out, const OperationMetaV2& m) {
        out << m.ext;
        writeXdrList(out, m.changes);
        writeXdrList(out, m.events);
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, OperationMetaV2& m) {
        in >> m.ext;
        readXdrList(in, m.changes);
        readXdrList(in, m.events);
        return in;
    }

    // ─── TransactionMeta ───────────────────────────────────────────────

    struct TransactionMetaV1
    {
        LedgerEntryChanges txChanges;
        QVector<OperationMeta> operations;
    };
    struct TransactionMetaV2
    {
        LedgerEntryChanges txChangesBefore;
        QVector<OperationMeta> operations;
        LedgerEntryChanges txChangesAfter;
    };
    struct TransactionMetaV3
    {
        Reserved ext;
        LedgerEntryChanges txChangesBefore;
        QVector<OperationMeta> operations;
        LedgerEntryChanges txChangesAfter;
        bool hasSorobanMeta = false;
        SorobanTransactionMeta sorobanMeta;
    };
    struct TransactionMetaV4
    {
        Reserved ext;
        LedgerEntryChanges txChangesBefore;
        QVector<OperationMetaV2> operations;
        LedgerEntryChanges txChangesAfter;
        bool hasSorobanMeta = false;
        SorobanTransactionMetaV2 sorobanMeta;
        QVector<TransactionEvent> events;
        QVector<DiagnosticEvent> diagnosticEvents;
    };

    /** The meta of one applied transaction; `v` names the arm that is filled. */
    struct TransactionMeta
    {
        qint32 v = 0;
        QVector<OperationMeta> operations;  // v0
        TransactionMetaV1 v1;
        TransactionMetaV2 v2;
        TransactionMetaV3 v3;
        TransactionMetaV4 v4;
    };
    inline QDataStream& operator<<(QDataStream& out, const TransactionMeta& m) {
        if (m.v < 0 || m.v > 4) { out.setStatus(QDataStream::WriteFailed); return out; }
        out << m.v;
        switch (m.v) {
        case 0: writeXdrList(out, m.operations); break;
        case 1: writeXdrList(out, m.v1.txChanges); writeXdrList(out, m.v1.operations); break;
        case 2:
            writeXdrList(out, m.v2.txChangesBefore);
            writeXdrList(out, m.v2.operations);
            writeXdrList(out, m.v2.txChangesAfter);
            break;
        case 3:
            out << m.v3.ext;
            writeXdrList(out, m.v3.txChangesBefore);
            writeXdrList(out, m.v3.operations);
            writeXdrList(out, m.v3.txChangesAfter);
            writeXdrBool(out, m.v3.hasSorobanMeta);
            if (m.v3.hasSorobanMeta)
                out << m.v3.sorobanMeta;
            break;
        case 4:
            out << m.v4.ext;
            writeXdrList(out, m.v4.txChangesBefore);
            writeXdrList(out, m.v4.operations);
            writeXdrList(out, m.v4.txChangesAfter);
            writeXdrBool(out, m.v4.hasSorobanMeta);
            if (m.v4.hasSorobanMeta)
                out << m.v4.sorobanMeta;
            writeXdrList(out, m.v4.events);
            writeXdrList(out, m.v4.diagnosticEvents);
            break;
        }
        return out;
    }
    inline QDataStream& operator>>(QDataStream& in, TransactionMeta& m) {
        in >> m.v;
        switch (m.v) {
        case 0: readXdrList(in, m.operations); break;
        case 1: readXdrList(in, m.v1.txChanges); readXdrList(in, m.v1.operations); break;
        case 2:
            readXdrList(in, m.v2.txChangesBefore);
            readXdrList(in, m.v2.operations);
            readXdrList(in, m.v2.txChangesAfter);
            break;
        case 3:
            in >> m.v3.ext;
            readXdrList(in, m.v3.txChangesBefore);
            readXdrList(in, m.v3.operations);
            readXdrList(in, m.v3.txChangesAfter);
            readXdrBool(in, m.v3.hasSorobanMeta);
            if (in.status() == QDataStream::Ok && m.v3.hasSorobanMeta)
                in >> m.v3.sorobanMeta;
            break;
        case 4:
            in >> m.v4.ext;
            readXdrList(in, m.v4.txChangesBefore);
            readXdrList(in, m.v4.operations);
            readXdrList(in, m.v4.txChangesAfter);
            readXdrBool(in, m.v4.hasSorobanMeta);
            if (in.status() == QDataStream::Ok && m.v4.hasSorobanMeta)
                in >> m.v4.sorobanMeta;
            readXdrList(in, m.v4.events);
            readXdrList(in, m.v4.diagnosticEvents);
            break;
        default:
            in.setStatus(QDataStream::ReadCorruptData);
            break;
        }
        return in;
    }

    /**
     * Decodes a TransactionMeta from its XDR bytes, exactly: the bytes must
     * be one whole meta that this SDK writes back byte for byte (an arm it
     * does not know, a CONFIG_SETTING entry, trailing bytes, non-zero
     * padding, a bool that is not 0 or 1 all throw std::runtime_error naming
     * what was wrong), so nothing is read as something else.
     */
    inline TransactionMeta decodeTransactionMeta(const QByteArray& bytes)
    {
        TransactionMeta meta;
        QByteArray raw = bytes;
        QDataStream in(&raw, QIODevice::ReadOnly);
        try {
            in >> meta;
        } catch (const std::exception& e) {
            // The SCVal / SCAddress readers throw on what they do not know.
            throw std::runtime_error(std::string("TransactionMeta: ") + e.what());
        }
        if (in.status() != QDataStream::Ok)
            throw std::runtime_error("TransactionMeta: not a TransactionMeta this SDK can read "
                                     "(truncated, or an arm it does not know)");
        if (!in.atEnd())
            throw std::runtime_error("TransactionMeta: bytes after the end of the meta");
        QByteArray again;
        try {
            again = encodeOrThrow(meta, "TransactionMeta");
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("TransactionMeta: ") + e.what());
        }
        if (again != bytes)
            throw std::runtime_error("TransactionMeta: the bytes do not re-encode exactly "
                                     "(non-canonical XDR)");
        return meta;
    }

    /**
     * The value the Soroban host function returned on chain, as js-stellar-sdk
     * reads it (meta v3 sorobanMeta.returnValue, v4 sorobanMeta.returnValue
     * when present). False for meta v0-v2, for a classic transaction, and for
     * a v4 meta whose Soroban part carries no value (extend/restore footprint,
     * a failed call).
     */
    inline bool returnValueOf(const TransactionMeta& meta, SCVal* value)
    {
        if (meta.v == 3 && meta.v3.hasSorobanMeta) {
            if (value) *value = meta.v3.sorobanMeta.returnValue;
            return true;
        }
        if (meta.v == 4 && meta.v4.hasSorobanMeta && meta.v4.sorobanMeta.hasReturnValue) {
            if (value) *value = meta.v4.sorobanMeta.returnValue;
            return true;
        }
        return false;
    }
}

#endif // STELLARLEDGERMETA_H
