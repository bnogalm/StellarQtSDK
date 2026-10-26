#ifndef VOIDUNIONARMTEST_H
#define VOIDUNIONARMTEST_H

#include <QObject>
#include <QtTest>
#include <cstring>
#include <new>
#include "../testcollector.h"

#include "../../xdr/stellartransaction.h"

/**
 * A union arm declared `void` must put NOTHING on the wire.
 *
 * `stellar::Operation`'s union had a `MuxedAccount operationInflation` member
 * and the writer wrote it, but the canonical union in stellar-xdr says:
 *
 *     case INFLATION:
 *         void;
 *
 * So every inflation operation this SDK produced carried a whole extra account
 * where the format expects the next field — and read it out of a union member
 * that `InflationOperation::fillOperationBody` never constructs, which is
 * undefined behaviour sitting on top of a wire-format error.
 *
 * NOTHING IN THE SUITE NOTICED, and the reason is the one this codebase keeps
 * relearning: the bug was SYMMETRIC. The writer invented bytes and the reader
 * consumed exactly those bytes, so every round trip agreed with itself. It took
 * submitting an inflation operation to a real network to find it — Horizon
 * answered "Transaction Malformed: Horizon could not decode the transaction
 * envelope in this request".
 *
 * The defence against a symmetric bug is never a round trip. It is asserting
 * the EXACT ENCODED SIZE against bytes counted out by hand from RFC 4506, which
 * is what this file does.
 */
class VoidUnionArmTest: public QObject
{
    Q_OBJECT

    /**
     * The eight bytes an inflation operation occupies, written out by hand.
     *
     * An `Operation` is an optional source account followed by the union. An
     * absent optional is the four-byte FALSE (RFC 4506 §4.19); the union is its
     * four-byte big-endian discriminant followed by the selected arm, and a
     * `void` arm contributes zero bytes (§4.15). Four plus four plus nothing.
     */
    static QByteArray canonicalInflation()
    {
        QByteArray out;
        out.append(char(0x00));   // sourceAccount: absent
        out.append(char(0x00));
        out.append(char(0x00));
        out.append(char(0x00));
        out.append(char(0x00));   // type: INFLATION = 9
        out.append(char(0x00));
        out.append(char(0x00));
        out.append(char(0x09));
        return out;
    }

    static QByteArray be32(quint32 v)
    {
        QByteArray out(4, '\0');
        out[0] = char(v >> 24);
        out[1] = char(v >> 16);
        out[2] = char(v >> 8);
        out[3] = char(v);
        return out;
    }

    template <typename T>
    static QByteArray encoded(const T& value, QDataStream::Status* status = nullptr)
    {
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << value;
        if (status)
            *status = s.status();
        return out;
    }

    // Real envelopes, one per arm: js-stellar-sdk reads each and writes it back
    // byte for byte. The v0 is the v1 rewritten without the key discriminant.
    static QByteArray v1Envelope()
    {
        return QByteArray::fromBase64(
            "AAAAAgAAAAABmqcP7EidBlOEMq7a7KmtOwXiAculDUEFGoGJC1fYbQAAAGQASi+YAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAL"
            "AEovmAAAACoAAAAAAAAAAQtX2G0AAABAawnB9G97svXkhlFbAQ4bzXiBXZYecSQvITmwiM2It+RUgtTXU7bqb6fC4Q6hcRjk"
            "K1/BAmqYiednd1YOX44ODA==");
    }
    static QByteArray v0Envelope()
    {
        return QByteArray::fromBase64(
            "AAAAAAGapw/sSJ0GU4Qyrtrsqa07BeIBy6UNQQUagYkLV9htAAAAZABKL5gAAAABAAAAAAAAAAAAAAABAAAAAAAAAAsASi+Y"
            "AAAAKgAAAAAAAAABC1fYbQAAAEBrCcH0b3uy9eSGUVsBDhvNeIFdlh5xJC8hObCIzYi35FSC1NdTtupvp8LhDqFxGOQrX8EC"
            "apiJ52d3Vg5fjg4M");
    }
    static QByteArray feeBumpEnvelope()
    {
        return QByteArray::fromBase64(
            "AAAABQAAAAAs943PLiCo9GKOLmr/1zEoU9XBvVckTR3Ga5ZKv1l/LgAAAAAAAAGQAAAAAgAAAABPyPbdl+MLSsRe5fjJh9pD"
            "J2LOM4T2PZFvhEUhxS8r9gAAAGQAGlWyAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAC6HLJ0LLoaJqIoLHCBcvn3r2Pem"
            "7MrpcLNpQed0EAkJAAAAAAX14QAAAAAAAAAAAcUvK/YAAABA/RkamXAzwqZaSAXZQWkgxxEMwPbqzsMLox+u2UBQk1n2NOQC"
            "P0ODwT5nIHn/wtpmK8X1VbQNPRh9tw3JWrjxDgAAAAAAAAABv1l/LgAAAEA+CjyAFxe/LOmdx5TDnCHqNbd1TgP0OJTqatIe"
            "6K6YwxA2PFsFglfO3FDTk52w3jtlNiOaS4qLHwrwdH+tug8B");
    }

    static bool decodeInto(const QByteArray& bytes, stellar::TransactionEnvelope& envelope)
    {
        QDataStream in(bytes);
        in >> envelope;
        return in.status() == QDataStream::Ok && in.atEnd();
    }

    /** Reads all of `bytes` into `value`: true when nothing is left over and nothing failed. */
    template <typename T>
    static bool decodeAll(const QByteArray& bytes, T& value)
    {
        QDataStream in(bytes);
        in >> value;
        return in.status() == QDataStream::Ok && in.atEnd();
    }

    /** `bytes` with the four bytes `fromEnd` bytes before its end replaced by `value`. */
    static QByteArray patchedFromEnd(QByteArray bytes, int fromEnd, qint32 value)
    {
        bytes.replace(bytes.size() - fromEnd, 4, be32(quint32(value)));
        return bytes;
    }

    /** True when reading `bytes` into a fresh T marks the stream corrupt (and throws nothing). */
    template <typename T>
    static bool readIsCorrupt(const QByteArray& bytes)
    {
        T value;
        QDataStream in(bytes);
        in >> value;
        return in.status() == QDataStream::ReadCorruptData;
    }

    // The raw ed25519 keys of the accounts in the js-stellar-sdk 17.1.0 vectors below.
    static QByteArray otherKey() { return QByteArray::fromHex("fd1724385aa0c75b64fb78cd602fa1d991fdebf76b13c58ed702eac835e9f618"); }
    static QByteArray opSourceKey() { return QByteArray::fromHex("66be7e332c7a453332bd9d0a7f7db055f5c5ef1a06ada66d98b39fb6810c473a"); }
    static QByteArray poolId() { return QByteArray::fromHex("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7"); }
    static QByteArray keyBytes(const stellar::PublicKey& key)
    {
        return QByteArray(reinterpret_cast<const char*>(key.ed25519), sizeof key.ed25519);
    }

    /**
     * A real account, as testnet's getLedgerEntries answered for Friendbot's
     * key (its `xdr` field: the entry's type, then the AccountEntry). It
     * carries the V1, V2 and V3 extensions Core gives every account that has
     * sent a transaction since Protocol 19.
     */
    static QByteArray liveAccountEntryData()
    {
        return QByteArray::fromBase64(
            "AAAAAAAAAAAQfdFrLDgzSIIugR73qs8U0ZiKbwBUclTTPh5thlbgnAGenVf1H/DAAAAAogAAAV4AAAAAAAAAAAAAAAAAAAAA"
            "AQAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAADAAAAAAA2xZMAAAAAalUhpg==");
    }

    /*
     * Ledger entries written by js-stellar-sdk 17.1.0 (and read back byte for
     * byte by py stellar_sdk 14.1.0), each a whole LedgerEntry: last modified
     * 99, the data, and the entry's own extension.
     */
    /** An account with two signers, the second sponsored, V1 to V3, and the entry sponsored. */
    static QByteArray sponsoredAccountEntry()
    {
        return QByteArray::fromBase64(
            "AAAAYwAAAAAAAAAA6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAAB1vNFQAAACkAAAAqAAAAAwAAAAEAAAAA"
            "/RckOFqgx1tk+3jNYC+h2ZH96/drE8WO1wLqyDXp9hgAAAABAAAAC2V4YW1wbGUuY29tAAECAwQAAAACAAAAAP0XJDhaoMdb"
            "ZPt4zWAvodmR/ev3axPFjtcC6sg16fYYAAAAAQAAAAIzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMwAAAAIAAAAB"
            "AAAAAAAAAAUAAAAAAAAABwAAAAIAAAABAAAAAgAAAAIAAAAAAAAAAQAAAABmvn4zLHpFMzK9nQp/fbBV9cXvGgatpm2Ys5+2"
            "gQxHOgAAAAMAAAAAAAAR1wAAAABlU/EAAAAAAQAAAAEAAAAA/RckOFqgx1tk+3jNYC+h2ZH96/drE8WO1wLqyDXp9hgAAAAA");
    }
    /** A pool-share trustline with V1 and V2 (used by 4 pools). */
    static QByteArray poolShareTrustLineEntry()
    {
        return QByteArray::fromBase64(
            "AAAAYwAAAAEAAAAA/RckOFqgx1tk+3jNYC+h2ZH96/drE8WO1wLqyDXp9hgAAAAD3XsauDHCczEN2+xvl4cKqDwvvXjOIq3t"
            "N+y/TzOA+scAAAAAAAAB9H//////////AAAAAQAAAAEAAAAAAAAACwAAAAAAAAAMAAAAAgAAAAQAAAAAAAAAAA==");
    }
    /** A data entry "config" = "hello", sponsored. */
    static QByteArray sponsoredDataEntry()
    {
        return QByteArray::fromBase64(
            "AAAAYwAAAAMAAAAA6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAGY29uZmlnAAAAAAAFaGVsbG8AAAAAAAAA"
            "AAAAAQAAAAEAAAAAZr5+Myx6RTMyvZ0Kf32wVfXF7xoGraZtmLOftoEMRzoAAAAA");
    }
    /** A claimable balance with extension V1 (clawback enabled) and a NOT predicate. */
    static QByteArray claimableBalanceEntry()
    {
        return QByteArray::fromBase64(
            "AAAAYwAAAAQAAAAAREREREREREREREREREREREREREREREREREREREREREQAAAABAAAAAAAAAAD9FyQ4WqDHW2T7eM1gL6HZ"
            "kf3r92sTxY7XAurINen2GAAAAAMAAAABAAAABAAAAABrSdIAAAAAAVVTRAAAAAAA6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuS"
            "Qh7qaRRG0iwAAAAAAAADCQAAAAEAAAAAAAAAAQAAAAA=");
    }

private slots:

    /**
     * Writing one produces exactly those eight bytes.
     *
     * The SIZE is the assertion that matters. Before the fix this came out at
     * 8 plus a whole MuxedAccount, and no comparison of decoded VALUES could
     * have told the difference — both sides read back an inflation operation
     * and agreed about it.
     */
    void testInflationOperationIsEightBytes()
    {
        stellar::Operation operation;
        operation.sourceAccount.filled = 0;
        operation.type = stellar::OperationType::INFLATION;

        QByteArray out;
        QDataStream stream(&out, QIODevice::WriteOnly);
        stream << operation;

        QCOMPARE(out.size(), 8);
        QCOMPARE(out, canonicalInflation());
    }

    /**
     * And reading one consumes exactly those eight bytes and no more.
     *
     * A reader that took more than the writer produced would eat whatever came
     * next in the envelope — which, inside a transaction, is the following
     * operation. So `atEnd()` is not a tidiness check: it is the difference
     * between one wrong operation and every operation after it.
     */
    void testInflationOperationReadsBackFromEightBytes()
    {
        QByteArray wire = canonicalInflation();

        stellar::Operation operation;
        QDataStream stream(&wire, QIODevice::ReadOnly);
        stream >> operation;

        QCOMPARE(int(operation.type), int(stellar::OperationType::INFLATION));
        QCOMPARE(int(operation.sourceAccount.filled), 0);
        QCOMPARE(stream.status(), QDataStream::Ok);
        QVERIFY2(stream.atEnd(),
                 "the reader consumed more than the eight bytes an inflation "
                 "operation occupies, so it would eat the operation that "
                 "follows it");
    }

    /**
     * The reader's half of the same lesson. An arm the reader did not know was
     * read as nothing, and its payload was then read as whatever field came
     * next: a POOL_SHARE in a plain Asset, a memo type that does not exist, a
     * Soroban ledger key where only classic ones are expected. A muxed
     * account of a pre-auth or hash-x key type was read as one of its two
     * arms. Every closed union now marks the stream corrupt instead.
     */
    void testAnUnknownArmMarksTheStreamCorrupt()
    {
        auto corrupt = [](const QByteArray& prefix, quint32 discriminant, auto&& value) {
            QByteArray bytes = prefix;
            bytes.append(char(discriminant >> 24)).append(char(discriminant >> 16))
                 .append(char(discriminant >> 8)).append(char(discriminant));
            bytes.append(QByteArray(96, '\x11'));    // a payload the reader must not skip
            QDataStream in(bytes);
            in >> value;
            return in.status() != QDataStream::Ok;
        };
        const QByteArray none;
        QVERIFY(corrupt(none, 3, stellar::Asset()));             // POOL_SHARE: not an Asset arm
        QVERIFY(corrupt(none, 7, stellar::Asset()));
        QVERIFY(corrupt(none, 7, stellar::ChangeTrustAsset()));
        QVERIFY(corrupt(none, 7, stellar::TrustLineAsset()));
        QVERIFY(corrupt(none, 1, stellar::LiquidityPoolParameters()));
        QVERIFY(corrupt(none, 1, stellar::PublicKey()));
        QVERIFY(corrupt(none, 1, stellar::MuxedAccount()));        // PRE_AUTH_TX
        QVERIFY(corrupt(none, 0x101, stellar::MuxedAccount()));    // PRE_AUTH_TX | muxed bit
        QVERIFY(corrupt(none, 9, stellar::SignerKey()));
        QVERIFY(corrupt(none, 5, stellar::Memo()));
        QVERIFY(corrupt(none, 3, stellar::Preconditions()));
        QVERIFY(corrupt(none, 6, stellar::ClaimPredicate()));
        QVERIFY(corrupt(none, 1, stellar::Claimant()));
        QVERIFY(corrupt(none, 1, stellar::ClaimableBalanceID()));
        QVERIFY(corrupt(none, 6, stellar::LedgerKey()));          // CONTRACT_DATA
        QVERIFY(corrupt(none, 2, stellar::RevokeSponsorshipOp()));
        QVERIFY(corrupt(none, 9, stellar::TransactionEnvelope()));
        QVERIFY(corrupt(none, 4, stellar::HostFunction()));
        QVERIFY(corrupt(none, 2, stellar::ContractIDPreimage()));
        QVERIFY(corrupt(none, 3, stellar::SorobanAuthorizedFunction()));
        // Operation and AllowTrustOp put their discriminant after another field.
        QVERIFY(corrupt(QByteArray(4, '\0'), 99, stellar::Operation()));   // no source account
        QVERIFY(corrupt(QByteArray::fromHex("00000000") + QByteArray(32, '\x22'), 0,
                        stellar::AllowTrustOp()));                         // native: no AssetCode arm
    }

    /**
     * Two of them in a row still read back as two.
     *
     * This is the failure the size error really caused: a transaction carrying
     * an inflation operation and anything after it decoded as nonsense from the
     * inflation onwards, because every subsequent field was read from the wrong
     * offset.
     */
    void testTwoInflationsInARowDoNotOverrunEachOther()
    {
        QByteArray wire = canonicalInflation() + canonicalInflation();
        QCOMPARE(wire.size(), 16);

        stellar::Operation first, second;
        QDataStream stream(&wire, QIODevice::ReadOnly);
        stream >> first >> second;

        QCOMPARE(int(first.type), int(stellar::OperationType::INFLATION));
        QCOMPARE(int(second.type), int(stellar::OperationType::INFLATION));
        QCOMPARE(stream.status(), QDataStream::Ok);
        QVERIFY(stream.atEnd());
    }

    /**
     * And the writer's half: an arm it does not know is a failed write, not a
     * guess. A PublicKey of a type with no key still had its 32 key bytes
     * written, so a footprint whose issuer was type 1 (js and py refuse it)
     * was signed with leftover stack, live pointers, where the key would be.
     * A balance id or fee bump naming no arm went out as a bare discriminant.
     */
    void testAnArmTheWriterDoesNotKnowIsRefused()
    {
        QDataStream::Status status = QDataStream::Ok;
        stellar::PublicKey key;
        key.type = stellar::PublicKeyType(1);
        std::memset(key.ed25519, 0x5A, sizeof key.ed25519);
        const QByteArray written = encoded(key, &status);
        QCOMPARE(status, QDataStream::WriteFailed);
        QVERIFY2(!written.contains(QByteArray(32, '\x5A')), "key bytes written for a type that has none");

        // That footprint (TRUSTLINE key, issuer "00000001" and no key) is
        // refused when read, rather than read as 4 bytes and re-written as 36.
        const QByteArray footprint = QByteArray::fromBase64(
            "AAAAAAAAAAEAAAAAAAAAAAOhB7/zzhC+HXDdGOdLwJln5NYwm6UNXx3chmQSVTG4AAAAAQAAAAEAAAAAA6EHv/POEL4dcN0Y"
            "50vAmWfk1jCbpQ1fHdyGZBJVMbgAAAABVVNEAAAAAAEAABOIAAACWAAAArwAAAAAAAAQ4Q==");
        stellar::SorobanTransactionData data;
        QDataStream in(footprint);
        in >> data;
        QVERIFY(in.status() != QDataStream::Ok);

        stellar::ClaimableBalanceID id;
        id.type = stellar::ClaimableBalanceIDType(1);
        encoded(id, &status);
        QCOMPARE(status, QDataStream::WriteFailed);

        stellar::FeeBumpTransaction feeBump;
        feeBump.feeSource.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
        std::memset(feeBump.feeSource.ed25519, 0, sizeof feeBump.feeSource.ed25519);
        feeBump.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;   // a fee bump wraps v1 only
        encoded(feeBump, &status);
        QCOMPARE(status, QDataStream::WriteFailed);
    }

    /**
     * An asset's union starts as zeros. It was whatever the memory held, so a
     * code or issuer the caller left unset went on the wire (and into the
     * signed hash) as leftover stack. Built over bytes filled with 0xAB on
     * purpose, like a dirty stack frame.
     */
    void testAnAssetStartsAsZeros()
    {
        const QByteArray zeroIssuer = be32(0) + QByteArray(32, '\0');
        {
            alignas(stellar::Asset) unsigned char storage[sizeof(stellar::Asset)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* asset = new (storage) stellar::Asset;
            QCOMPARE(encoded(*asset), be32(0));                       // native
            asset->type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4;
            QCOMPARE(encoded(*asset), be32(1) + QByteArray(4, '\0') + zeroIssuer);
            asset->type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12;
            QCOMPARE(encoded(*asset), be32(2) + QByteArray(12, '\0') + zeroIssuer);
            asset->~Asset();
        }
        {
            alignas(stellar::TrustLineAsset) unsigned char storage[sizeof(stellar::TrustLineAsset)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* asset = new (storage) stellar::TrustLineAsset;
            asset->type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12;
            QCOMPARE(encoded(*asset), be32(2) + QByteArray(12, '\0') + zeroIssuer);
            asset->type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
            QCOMPARE(encoded(*asset), be32(3) + QByteArray(32, '\0'));
            asset->~TrustLineAsset();
        }
        {
            alignas(stellar::ChangeTrustAsset) unsigned char storage[sizeof(stellar::ChangeTrustAsset)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* asset = new (storage) stellar::ChangeTrustAsset;
            asset->type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4;
            QCOMPARE(encoded(*asset), be32(1) + QByteArray(4, '\0') + zeroIssuer);
            asset->~ChangeTrustAsset();
        }
        {
            // A copy of a native one copies no arm, so it has to start from zeros too.
            const stellar::ChangeTrustAsset native;
            alignas(stellar::ChangeTrustAsset) unsigned char storage[sizeof(stellar::ChangeTrustAsset)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* asset = new (storage) stellar::ChangeTrustAsset(native);
            asset->type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12;
            QCOMPARE(encoded(*asset), be32(2) + QByteArray(12, '\0') + zeroIssuer);
            asset->~ChangeTrustAsset();
        }
    }

    /**
     * A default operation, operation result or claim predicate writes zeros in
     * the arm its discriminant names. The constructors set only the
     * discriminant, so the destination of a default CREATE_ACCOUNT operation,
     * a result's code and an absolute time set by type alone were whatever the
     * memory held.
     */
    void testADefaultUnionWritesZerosInItsArm()
    {
        {
            alignas(stellar::Operation) unsigned char storage[sizeof(stellar::Operation)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* operation = new (storage) stellar::Operation;
            // no source account, CREATE_ACCOUNT, destination key type 0 and key, starting balance
            QCOMPARE(encoded(*operation), be32(0) + be32(0) + be32(0) + QByteArray(32, '\0') + QByteArray(8, '\0'));
            operation->~Operation();
        }
        {
            alignas(stellar::OperationResult) unsigned char storage[sizeof(stellar::OperationResult)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* result = new (storage) stellar::OperationResult;
            QCOMPARE(encoded(*result), be32(0) + be32(0) + be32(0));     // opINNER, CREATE_ACCOUNT, SUCCESS
            result->~OperationResult();
        }
        {
            alignas(stellar::ClaimPredicate) unsigned char storage[sizeof(stellar::ClaimPredicate)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* predicate = new (storage) stellar::ClaimPredicate;
            QCOMPARE(encoded(*predicate), be32(0));                       // unconditional
            predicate->type = stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME;
            QCOMPARE(encoded(*predicate), be32(4) + QByteArray(8, '\0'));
            predicate->~ClaimPredicate();
        }
    }

    /**
     * A single-arm union's discriminant names the arm its constructor builds.
     * It was left unset: a balance id with only its hash filled wrote four
     * stray bytes and no hash, and a fee bump put a stray inner type (and fee)
     * into the signed payload.
     */
    void testASingleArmUnionStartsOnThatArm()
    {
        {
            alignas(stellar::ClaimableBalanceID) unsigned char storage[sizeof(stellar::ClaimableBalanceID)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* id = new (storage) stellar::ClaimableBalanceID;
            QCOMPARE(encoded(*id), be32(0) + QByteArray(32, '\0'));
            id->~ClaimableBalanceID();
        }
        {
            alignas(stellar::FeeBumpTransaction) unsigned char storage[sizeof(stellar::FeeBumpTransaction)];
            std::memset(storage, 0xAB, sizeof storage);
            auto* feeBump = new (storage) stellar::FeeBumpTransaction;
            QVERIFY(feeBump->type == stellar::EnvelopeType::ENVELOPE_TYPE_TX);
            QCOMPARE(feeBump->fee, qint64(0));
            feeBump->~FeeBumpTransaction();
        }
    }

    /**
     * A default envelope names the fee-bump arm, so it holds one. It held
     * nothing: destroying one that was never read into (a vector of them
     * resized, say) ran the fee bump's destructor on raw memory and crashed.
     */
    void testADefaultEnvelopeHoldsTheArmItNames()
    {
        alignas(stellar::TransactionEnvelope) unsigned char storage[sizeof(stellar::TransactionEnvelope)];
        std::memset(storage, 0xAB, sizeof storage);
        auto* envelope = new (storage) stellar::TransactionEnvelope;
        QVERIFY(envelope->type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP);
        QVERIFY(envelope->feeBump.tx.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        QVERIFY(envelope->feeBump.signatures.value.isEmpty());
        envelope->~TransactionEnvelope();
    }

    /**
     * Any envelope can be assigned over any other, and over a default one. The
     * assignment destroyed only the v0 and v1 arms and never built the fee-bump
     * one, so a fee bump assigned over a v0, a v1 or a default envelope was
     * copied into memory that held another arm, or nothing, and crashed.
     */
    void testAnyEnvelopeCanBeAssignedOverAnyOther()
    {
        const QList<QByteArray> sources = { v0Envelope(), v1Envelope(), feeBumpEnvelope() };
        QList<QByteArray> targets = sources;
        targets.append(QByteArray());           // left as constructed
        for (const QByteArray& target : targets) {
            for (const QByteArray& source : sources) {
                alignas(stellar::TransactionEnvelope) unsigned char storage[sizeof(stellar::TransactionEnvelope)];
                std::memset(storage, 0xAB, sizeof storage);
                auto* envelope = new (storage) stellar::TransactionEnvelope;
                if (!target.isEmpty())
                    QVERIFY(decodeInto(target, *envelope));
                stellar::TransactionEnvelope from;
                QVERIFY(decodeInto(source, from));
                *envelope = from;
                QCOMPARE(encoded(*envelope), source);
                envelope->~TransactionEnvelope();
            }
        }
    }

    /**
     * e = e keeps e. The assignment destroyed its own arm, rebuilt it empty and
     * copied that onto itself: a signed envelope became one with no operations
     * and no signatures. FeeBumpTransaction, Operation, ClaimPredicate,
     * LedgerKey and RevokeSponsorshipOp assign the same way and lost their
     * contents the same way.
     */
    void testAssigningToItselfKeepsTheValue()
    {
        for (const QByteArray& bytes : { v0Envelope(), v1Envelope(), feeBumpEnvelope() }) {
            stellar::TransactionEnvelope envelope;
            QVERIFY(decodeInto(bytes, envelope));
            const stellar::TransactionEnvelope& same = envelope;
            envelope = same;
            QCOMPARE(encoded(envelope), bytes);
        }

        stellar::TransactionEnvelope envelope;
        QVERIFY(decodeInto(feeBumpEnvelope(), envelope));
        stellar::FeeBumpTransaction& feeBump = envelope.feeBump.tx;
        const QByteArray feeBumpBytes = encoded(feeBump);
        const stellar::FeeBumpTransaction& sameFeeBump = feeBump;
        feeBump = sameFeeBump;
        QCOMPARE(encoded(feeBump), feeBumpBytes);

        stellar::Operation operation;
        operation.sourceAccount.filled = 0;
        operation.fillSetOptionsOp().homeDomain.filler() = QStringLiteral("stellar.org");
        const QByteArray operationBytes = encoded(operation);
        const stellar::Operation& sameOperation = operation;
        operation = sameOperation;
        QCOMPARE(encoded(operation), operationBytes);

        stellar::ClaimPredicate predicate;
        stellar::ClaimPredicate before;
        before.setAbsBefore(1234);
        auto& both = predicate.fillAndPredicates();
        both.append(before);
        both.append(before);
        const QByteArray predicateBytes = encoded(predicate);
        const stellar::ClaimPredicate& samePredicate = predicate;
        predicate = samePredicate;
        QCOMPARE(encoded(predicate), predicateBytes);

        stellar::RevokeSponsorshipOp revoke;
        stellar::LedgerKey::Data& data = revoke.fillRevokeSponsorshipLedgerEntry().fillData();
        data.accountID.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        std::memset(data.accountID.ed25519, 0x44, sizeof data.accountID.ed25519);
        data.dataName = QStringLiteral("name");
        const QByteArray revokeBytes = encoded(revoke);
        const stellar::RevokeSponsorshipOp& sameRevoke = revoke;
        revoke = sameRevoke;
        QCOMPARE(encoded(revoke), revokeBytes);
        stellar::LedgerKey& key = revoke.ledgerKey;
        const stellar::LedgerKey& sameKey = key;
        key = sameKey;
        QCOMPARE(encoded(revoke), revokeBytes);
    }

    /**
     * Replacing an arm releases what the old one held, checked through the
     * signatures' shared data: a copy taken first must end up its only owner.
     * The fee-bump arm was never destroyed when another envelope was assigned
     * or read over it, and a fee bump's inner transaction was rebuilt over the
     * live one: each leaked the old contents (about 3 KB a time).
     */
    void testReplacingAnArmReleasesTheOldOne()
    {
        stellar::TransactionEnvelope v1;
        QVERIFY(decodeInto(v1Envelope(), v1));

        stellar::TransactionEnvelope envelope;
        QVERIFY(decodeInto(feeBumpEnvelope(), envelope));
        QVector<stellar::DecoratedSignature> held = envelope.feeBump.signatures.value;
        QVERIFY(!held.isDetached());
        envelope = v1;
        QVERIFY2(held.isDetached(), "assigning over a fee bump leaked it");

        QVERIFY(decodeInto(feeBumpEnvelope(), envelope));
        held = envelope.feeBump.signatures.value;
        QVERIFY(!held.isDetached());
        QVERIFY(decodeInto(v1Envelope(), envelope));
        QVERIFY2(held.isDetached(), "reading over a fee bump leaked it");

        stellar::FeeBumpTransaction feeBump;
        {
            stellar::TransactionEnvelope source;
            QVERIFY(decodeInto(feeBumpEnvelope(), source));
            feeBump = source.feeBump.tx;
        }
        held = feeBump.v1.signatures.value;
        QVERIFY(!held.isDetached());
        feeBump = stellar::FeeBumpTransaction();
        QVERIFY2(held.isDetached(), "assigning a fee bump leaked its inner transaction");
    }

    /**
     * An envelope that ends before the signatures it declares is a short read.
     * The array reader stopped quietly at the end of the input, so "two
     * signatures" with one (or none) present decoded as complete; js and py
     * refuse both.
     */
    void testAnEnvelopeShortOfItsSignaturesIsRefused()
    {
        const QByteArray whole = v1Envelope();       // ends with its one signature: count, hint, length, 64 bytes
        QByteArray oneOfTwo = whole;
        oneOfTwo.replace(whole.size() - 76, 4, be32(2));
        const QByteArray noneOfTwo = oneOfTwo.left(whole.size() - 72);
        for (const QByteArray& bytes : { oneOfTwo, noneOfTwo }) {
            stellar::TransactionEnvelope envelope;
            QDataStream in(bytes);
            in >> envelope;
            QVERIFY(in.status() != QDataStream::Ok);
        }
        stellar::TransactionEnvelope envelope;
        QVERIFY(decodeInto(whole, envelope));
        QCOMPARE(envelope.v1.signatures.value.size(), 1);
    }

    /**
     * An arm the writer does not know writes NOTHING, not even its
     * discriminant, and fails the write. Most of these went out as a bare
     * discriminant with the stream still Ok (a signer key, a muxed account of
     * a pre-auth type, a POOL_SHARE in a plain Asset, a Soroban ledger key, a
     * memo type 5...), and the rest wrote a prefix before failing.
     */
    void testAnArmTheWriterDoesNotKnowWritesNothing()
    {
        auto nothing = [](const auto& value) {
            QDataStream::Status status = QDataStream::Ok;
            const QByteArray written = encoded(value, &status);
            return status == QDataStream::WriteFailed && written.isEmpty();
        };
        stellar::PublicKey key;
        key.type = stellar::PublicKeyType(1);
        QVERIFY(nothing(key));
        stellar::SignerKey signer;
        signer.type = stellar::SignerKeyType(9);
        QVERIFY(nothing(signer));
        signer.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD;
        signer.ed25519SignedPayload.payloadLen = 65;            // opaque<64>
        QVERIFY(nothing(signer));
        stellar::MuxedAccount muxed;
        muxed.type = stellar::CryptoKeyType::KEY_TYPE_PRE_AUTH_TX;
        QVERIFY(nothing(muxed));
        muxed.type = stellar::CryptoKeyType(0x101);             // pre-auth with the muxed bit
        QVERIFY(nothing(muxed));
        stellar::Asset asset;
        asset.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        QVERIFY(nothing(asset));
        asset.type = stellar::AssetType(7);
        QVERIFY(nothing(asset));
        stellar::ChangeTrustAsset line;
        line.type = stellar::AssetType(7);
        QVERIFY(nothing(line));
        stellar::TrustLineAsset trustLineAsset;
        trustLineAsset.type = stellar::AssetType(7);
        QVERIFY(nothing(trustLineAsset));
        stellar::LiquidityPoolParameters params;
        params.type = stellar::LiquidityPoolType(1);
        QVERIFY(nothing(params));
        stellar::ClaimPredicate predicate;
        predicate.type = stellar::ClaimPredicateType(6);
        QVERIFY(nothing(predicate));
        stellar::Claimant claimant;
        claimant.type = stellar::ClaimantType(1);
        QVERIFY(nothing(claimant));
        stellar::ClaimableBalanceID id;
        id.type = stellar::ClaimableBalanceIDType(1);
        QVERIFY(nothing(id));
        stellar::LedgerKey ledgerKey;                           // its ACCOUNT arm is trivial
        ledgerKey.type = stellar::LedgerEntryType::CONTRACT_DATA;
        QVERIFY(nothing(ledgerKey));
        stellar::LedgerEntry entry;
        entry.type = stellar::LedgerEntryType::CONTRACT_DATA;
        QVERIFY(nothing(entry));
        stellar::Memo memo;
        memo.type = stellar::MemoType(5);
        QVERIFY(nothing(memo));
        stellar::Preconditions cond;                            // its time bounds are trivial
        cond.type = stellar::PreconditionType(3);
        QVERIFY(nothing(cond));
        stellar::RevokeSponsorshipOp revoke;                    // an ACCOUNT key: trivial
        revoke.type = stellar::RevokeSponsorshipType(2);
        QVERIFY(nothing(revoke));
        stellar::Operation operation;
        operation.sourceAccount.filled = 0;
        operation.type = stellar::OperationType(99);
        QVERIFY(nothing(operation));
        stellar::AllowTrustOp allow;
        allow.asset.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        QVERIFY(nothing(allow));
        stellar::OperationID opId;
        opId.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
        QVERIFY(nothing(opId));
        stellar::TransactionExt ext;
        ext.v = 2;
        QVERIFY(nothing(ext));
        const stellar::TransactionSignaturePayload payload(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        QVERIFY(nothing(payload));
        stellar::FeeBumpTransaction feeBump;
        feeBump.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;
        QVERIFY(nothing(feeBump));
        {
            stellar::TransactionEnvelope envelope;
            envelope.type = stellar::EnvelopeType(9);
            QVERIFY(nothing(envelope));
            envelope.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP;   // the arm it holds, for the destructor
        }
        stellar::ClaimAtom atom;
        atom.type = stellar::ClaimAtomType(3);
        QVERIFY(nothing(atom));
        stellar::ManageOfferSuccessResult success;
        success.effect = stellar::ManageOfferEffect(3);
        QVERIFY(nothing(success));
        stellar::AccountEntry account;
        account.ext.reserved = 2;
        QVERIFY(nothing(account));
        stellar::InnerTransactionResult inner;
        inner.code = stellar::TransactionResultCode::txFEE_BUMP_INNER_SUCCESS;   // not an inner code
        QVERIFY(nothing(inner));
    }

    /**
     * A signed payload's length is at most 64 and its padding is zeros. The
     * reader threw for a longer one (out of a stream reader, where every other
     * malformed input marks the stream) and never looked at the padding.
     */
    void testASignedPayloadSignerKeyIsCheckedNotThrown()
    {
        const QByteArray head = be32(3) + QByteArray(32, '\x01');
        const QByteArray clean = head + be32(1) + QByteArray("\x05\x00\x00\x00", 4);
        stellar::SignerKey key;
        QVERIFY(decodeAll(clean, key));
        QCOMPARE(encoded(key), clean);
        try {
            QVERIFY(readIsCorrupt<stellar::SignerKey>(head + be32(65) + QByteArray(68, '\0')));
            QVERIFY(readIsCorrupt<stellar::SignerKey>(head + be32(1) + QByteArray("\x05\x01\x00\x00", 4)));
        } catch (const std::exception& e) {
            QFAIL(e.what());
        }
    }

    /**
     * A second read into the same union releases the arm the first one built.
     * LedgerKey and ClaimPredicate built the new arm over the live one, and
     * TransactionSignaturePayload did too (and read a type with no arm as
     * nothing): each leaked what it held. Checked through shared data, as
     * above: a copy taken first must end up its only owner.
     */
    void testReadingIntoAUsedUnionReleasesTheOldArm()
    {
        stellar::LedgerKey key;
        key.fillData().dataName = QStringLiteral("first");
        QVector<char> heldName = key.data.dataName.value;
        QVERIFY(!heldName.isDetached());
        stellar::LedgerKey other;
        other.fillData().accountID.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        std::memset(other.data.accountID.ed25519, 0x44, sizeof other.data.accountID.ed25519);
        other.data.dataName = QStringLiteral("second");
        QVERIFY(decodeAll(encoded(other), key));
        QVERIFY2(heldName.isDetached(), "reading a data key over a data key leaked the first name");
        QCOMPARE(encoded(key), encoded(other));
        heldName = key.data.dataName.value;
        QVERIFY(decodeAll(be32(0) + be32(0) + QByteArray(32, '\x55'), key));   // an ACCOUNT key over it
        QVERIFY2(heldName.isDetached(), "reading an account key over a data key leaked the name");

        stellar::ClaimPredicate predicate;
        stellar::ClaimPredicate before;
        before.setAbsBefore(1);
        auto& both = predicate.fillAndPredicates();
        both.append(before);
        both.append(before);
        QVector<stellar::ClaimPredicate> heldOperands = predicate.andPredicates.value;
        QVERIFY(!heldOperands.isDetached());
        QVERIFY(decodeAll(be32(0), predicate));                 // unconditional over it
        QVERIFY2(heldOperands.isDetached(), "reading over an AND predicate leaked its operands");

        QByteArray payloadBytes;
        {
            stellar::TransactionEnvelope envelope;
            QVERIFY(decodeInto(v1Envelope(), envelope));
            payloadBytes = encoded(stellar::TransactionSignaturePayload(envelope.v1.tx, QByteArray(32, '\x01')));
        }
        stellar::TransactionSignaturePayload payload(stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        QVERIFY(decodeAll(payloadBytes, payload));              // shares its operations with nobody
        QVector<stellar::Operation> heldOperations = payload.tx.operations.value;
        QVERIFY(!heldOperations.isDetached());
        QVERIFY(decodeAll(payloadBytes, payload));
        QVERIFY2(heldOperations.isDetached(), "reading a payload over a payload leaked its transaction");
        QCOMPARE(encoded(payload), payloadBytes);
        // An envelope type with no arm in a payload is corrupt, not an empty payload.
        QDataStream unknown(QByteArray(32, '\x01') + be32(7) + QByteArray(64, '\x11'));
        unknown >> payload;
        QCOMPARE(unknown.status(), QDataStream::ReadCorruptData);
    }

    /**
     * A NOT predicate's operand is an XDR optional: 0 or 1. The count of
     * the array that holds it was read, and 2 threw out of the reader.
     */
    void testANotPredicatesPresentFlagIsZeroOrOne()
    {
        stellar::ClaimPredicate predicate;
        QVERIFY(decodeAll(be32(3) + be32(1) + be32(0), predicate));
        QCOMPARE(predicate.notPredicate.value.size(), 1);
        QVERIFY(decodeAll(be32(3) + be32(0), predicate));
        QCOMPARE(predicate.notPredicate.value.size(), 0);
        try {
            QVERIFY(readIsCorrupt<stellar::ClaimPredicate>(be32(3) + be32(2) + be32(0) + be32(0)));
        } catch (const std::exception& e) {
            QFAIL(e.what());
        }
    }

    /**
     * A real account reads in full. The reader stopped at V1's discriminant
     * and left the V2 and V3 extensions in the stream, so every account Core
     * had touched since Protocol 19 was read short.
     */
    void testARealAccountEntryReadsInFull()
    {
        const QByteArray bytes = liveAccountEntryData();
        QDataStream in(bytes);
        stellar::LedgerEntryType type = stellar::LedgerEntryType::TRUSTLINE;
        stellar::AccountEntry account;
        in >> type >> account;
        QCOMPARE(in.status(), QDataStream::Ok);
        QVERIFY2(in.atEnd(), "the account's extensions were left in the stream");
        QVERIFY(type == stellar::LedgerEntryType::ACCOUNT);
        QCOMPARE(account.balance, qint64(116703641458438336LL));
        QCOMPARE(account.seqNum, qint64(695784702302LL));
        QCOMPARE(account.ext.reserved, 1);
        QCOMPARE(account.v1.ext.reserved, 2);
        QCOMPARE(account.v1.v2.ext.reserved, 3);
        QCOMPARE(account.v1.v2.v3.seqLedger, quint32(3589523));
        QCOMPARE(account.v1.v2.v3.seqTime, quint64(1783964070));
        QCOMPARE(encoded(type) + encoded(account), bytes);
    }

    /**
     * The sponsors of an account's signers are optional accounts, one per
     * signer, and the entry's own sponsor is one too. They were read as bare
     * accounts: four bytes out of step from the first one on.
     */
    void testASponsoredAccountReadsAsJsWroteIt()
    {
        const QByteArray bytes = sponsoredAccountEntry();
        stellar::LedgerEntry entry;
        QVERIFY(decodeAll(bytes, entry));
        QCOMPARE(entry.lastModifiedLedgerSeq, quint32(99));
        QVERIFY(entry.type == stellar::LedgerEntryType::ACCOUNT);
        const stellar::AccountEntry& account = entry.account;
        QCOMPARE(account.signers.value.size(), 2);
        QCOMPARE(account.homeDomain.toString(), QStringLiteral("example.com"));
        QCOMPARE(account.v1.liabilities.buying, qint64(5));
        QCOMPARE(account.v1.liabilities.selling, qint64(7));
        const stellar::AccountEntryExtensionV2& v2 = account.v1.v2;
        QCOMPARE(v2.numSponsored, quint32(1));
        QCOMPARE(v2.numSponsoring, quint32(2));
        QCOMPARE(v2.signerSponsoringIDs.value.size(), 2);
        QCOMPARE(v2.signerSponsoringIDs.value.at(0).filled, 0);
        QCOMPARE(v2.signerSponsoringIDs.value.at(1).filled, 1);
        QCOMPARE(keyBytes(v2.signerSponsoringIDs.value.at(1).value), opSourceKey());
        QCOMPARE(v2.v3.seqLedger, quint32(4567));
        QCOMPARE(v2.v3.seqTime, quint64(1700000000));
        QCOMPARE(entry.ext.reserved, 1);
        QCOMPARE(entry.v1.sponsoringID.filled, 1);
        QCOMPARE(keyBytes(entry.v1.sponsoringID.value), otherKey());
        QCOMPARE(encoded(entry), bytes);
    }

    /**
     * Every classic ledger entry reads what js wrote and writes it back byte
     * for byte, extensions and all: a pool-share trustline (its asset was a
     * plain Asset, its V2 extension unread), a data entry (its ext was never
     * read or written), a claimable balance, an offer, a liquidity pool (no
     * model at all), and the entry's own extension (never read or written).
     */
    void testEveryClassicLedgerEntryReadsAsJsWroteIt()
    {
        const QList<QByteArray> vectors = {
            QByteArray::fromBase64(
                "AAAAYwAAAAAAAAAA6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAAAAAACgAAAAAAAAAFAAAAAAAAAAAAAAAA"
                "AAAAAAEAAAAAAAAAAAAAAAAAAAA="),                            // a plain account, no extension
            poolShareTrustLineEntry(),
            QByteArray::fromBase64(
                "AAAAYwAAAAEAAAAA/RckOFqgx1tk+3jNYC+h2ZH96/drE8WO1wLqyDXp9hgAAAABVVNEAAAAAADqSmxj4pxSCr71UHsTLsX5"
                "lUd2rr6+e5JCHuppFEbSLAAAAAAAAAH0AAAAAAAAA+gAAAABAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="), // a USD trustline, V1 only
            sponsoredDataEntry(),
            claimableBalanceEntry(),
            QByteArray::fromBase64(
                "AAAAYwAAAAIAAAAA6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAAAAAAKgAAAAAAAAABVVNEAAAAAADqSmxj"
                "4pxSCr71UHsTLsX5lUd2rr6+e5JCHuppFEbSLAAAAAAAAABkAAAAAwAAAAIAAAABAAAAAAAAAAA="),   // an offer
            QByteArray::fromBase64(
                "AAAAYwAAAAXdexq4McJzMQ3b7G+XhwqoPC+9eM4ire037L9PM4D6xwAAAAAAAAAAAAAAAVVTRAAAAAAA6kpsY+KcUgq+9VB7"
                "Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAeAAAAAAAAA+gAAAAAAAAH0AAAAAAAAAWGAAAAAAAAAAIAAAAA"),  // a pool
        };
        for (const QByteArray& bytes : vectors) {
            stellar::LedgerEntry entry;
            QVERIFY(decodeAll(bytes, entry));
            QCOMPARE(encoded(entry), bytes);
        }

        stellar::LedgerEntry entry;
        QVERIFY(decodeAll(poolShareTrustLineEntry(), entry));
        QVERIFY(entry.trustLine.asset.type == stellar::AssetType::ASSET_TYPE_POOL_SHARE);
        QCOMPARE(QByteArray(reinterpret_cast<const char*>(entry.trustLine.asset.liquidityPoolID), 32), poolId());
        QCOMPARE(entry.trustLine.v1.liabilities.buying, qint64(11));
        QCOMPARE(entry.trustLine.v1.v2.liquidityPoolUseCount, 4);
        QVERIFY(decodeAll(sponsoredDataEntry(), entry));
        QCOMPARE(entry.data.dataName.toString(), QStringLiteral("config"));
        QCOMPARE(entry.data.dataValue.value.size(), 5);
        QCOMPARE(keyBytes(entry.v1.sponsoringID.value), opSourceKey());
        QVERIFY(decodeAll(claimableBalanceEntry(), entry));
        QCOMPARE(entry.claimableBalance.amount, qint64(777));
        QCOMPARE(entry.claimableBalance.v1.flags, quint32(1));
        QVERIFY(entry.claimableBalance.claimants.value.at(0).v0.predicate.type
                == stellar::ClaimPredicateType::CLAIM_PREDICATE_NOT);
        QVERIFY(decodeAll(vectors.last(), entry));
        QCOMPARE(entry.liquidityPool.constantProduct.reserveB, qint64(2000));
        QCOMPARE(entry.liquidityPool.constantProduct.params.fee, 30);
    }

    /**
     * An extension discriminant with no arm is corrupt data, at every level;
     * so is a sponsor flag that is neither 0 nor 1, and a Soroban entry this
     * model does not hold. Each used to be read as "no extension" (or not
     * read at all) and the bytes after it as whatever came next.
     */
    void testAnExtensionArmThatDoesNotExistIsCorrupt()
    {
        const QByteArray account = liveAccountEntryData().mid(4);      // the AccountEntry alone
        {
            stellar::AccountEntry whole;
            QVERIFY(decodeAll(account, whole));
        }
        QVERIFY(readIsCorrupt<stellar::AccountEntry>(patchedFromEnd(account, 56, 2)));  // AccountEntry.ext
        QVERIFY(readIsCorrupt<stellar::AccountEntry>(patchedFromEnd(account, 36, 3)));  // V1.ext
        QVERIFY(readIsCorrupt<stellar::AccountEntry>(patchedFromEnd(account, 20, 2)));  // V2.ext
        QVERIFY(readIsCorrupt<stellar::AccountEntry>(patchedFromEnd(account, 16, 1)));  // V3.ext

        const QByteArray trustLine = poolShareTrustLineEntry();
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(trustLine, 36, 2)));  // TrustLineEntry.ext
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(trustLine, 16, 1)));  // V1.ext
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(trustLine, 8, 1)));   // V2.ext
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(trustLine, 4, 2)));   // LedgerEntry.ext

        const QByteArray balance = claimableBalanceEntry();
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(balance, 16, 2)));    // ClaimableBalanceEntry.ext
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(balance, 12, 1)));    // its V1.ext

        const QByteArray data = sponsoredDataEntry();
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(data, 52, 1)));       // DataEntry.ext
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(data, 44, 2)));       // sponsor present flag
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(patchedFromEnd(data, 4, 1)));        // LedgerEntryExtensionV1.ext

        QByteArray contractData = data;
        contractData.replace(4, 4, be32(6));                           // CONTRACT_DATA: no model here
        QVERIFY(readIsCorrupt<stellar::LedgerEntry>(contractData));
    }

    /**
     * The key of a pool-share trustline (js-stellar-sdk 17.1.0's
     * LedgerKey.trustline with a LiquidityPoolId) reads and writes back byte
     * for byte. Its asset was a plain Asset, which refused POOL_SHARE.
     */
    void testAPoolShareTrustlineKeyReadsAndWrites()
    {
        const QByteArray bytes = QByteArray::fromBase64(
            "AAAAAQAAAAD9FyQ4WqDHW2T7eM1gL6HZkf3r92sTxY7XAurINen2GAAAAAPdexq4McJzMQ3b7G+XhwqoPC+9eM4ire037L9P"
            "M4D6xw==");
        stellar::LedgerKey key;
        QVERIFY(decodeAll(bytes, key));
        QVERIFY(key.type == stellar::LedgerEntryType::TRUSTLINE);
        QCOMPARE(keyBytes(key.trustLine.accountID), otherKey());
        QVERIFY(key.trustLine.asset.type == stellar::AssetType::ASSET_TYPE_POOL_SHARE);
        QCOMPARE(QByteArray(reinterpret_cast<const char*>(key.trustLine.asset.liquidityPoolID), 32), poolId());
        QCOMPARE(encoded(key), bytes);
        const stellar::LedgerKey copy = key;
        QCOMPARE(encoded(copy), bytes);
    }

    /**
     * Core refuses a claimant whose predicate nests deeper than four levels
     * (validatePredicate). The reader had no limit: NOT inside NOT, eight
     * bytes a level, recursed as deep as the input went until the stack ran
     * out, and the writer put any depth on the wire for Core to refuse. Four
     * levels read and write; a fifth is corrupt data and a failed write, and
     * a hostile depth stops being read there.
     */
    void testAClaimPredicateNestedDeeperThanCoreAllowsIsRefused()
    {
        // `levels` predicates deep: NOT, present, ... around an UNCONDITIONAL.
        const auto nested = [](int levels) {
            QByteArray bytes;
            for (int i = 1; i < levels; ++i)
                bytes += be32(3) + be32(1);
            return bytes + be32(0);
        };
        const auto readsWhole = [](const QByteArray& bytes) {
            stellar::ClaimPredicate predicate;
            QDataStream in(bytes);
            in >> predicate;
            return in.status() == QDataStream::Ok && in.atEnd();
        };
        QVERIFY(readsWhole(nested(1)));
        QVERIFY(readsWhole(nested(4)));
        QVERIFY(!readsWhole(nested(5)));
        QVERIFY(!readsWhole(nested(100000)));

        // AND/OR count too: AND(OR(NOT(U), U), U) is four deep, one more is five.
        stellar::ClaimPredicate unconditional;
        stellar::ClaimPredicate inner;
        inner.fillNotPredicate().append(unconditional);
        stellar::ClaimPredicate orPredicate;
        orPredicate.fillOrPredicates().append(inner);
        orPredicate.orPredicates.append(unconditional);
        stellar::ClaimPredicate fourDeep;
        fourDeep.fillAndPredicates().append(orPredicate);
        fourDeep.andPredicates.append(unconditional);

        QDataStream::Status status = QDataStream::Ok;
        const QByteArray four = encoded(fourDeep, &status);
        QCOMPARE(status, QDataStream::Ok);
        QVERIFY(readsWhole(four));

        stellar::ClaimPredicate fiveDeep;
        fiveDeep.fillNotPredicate().append(fourDeep);
        QVERIFY(encoded(fiveDeep, &status).isEmpty());   // nothing written, not even the NOT
        QCOMPARE(status, QDataStream::WriteFailed);
        QVERIFY(!readsWhole(be32(3) + be32(1) + four));

        // As a claimant's predicate, inside what is signed.
        stellar::Claimant claimant;
        claimant.type = stellar::ClaimantType::CLAIMANT_TYPE_V0;
        claimant.v0.destination.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        std::memset(claimant.v0.destination.ed25519, 0x11, sizeof claimant.v0.destination.ed25519);
        claimant.v0.predicate = fiveDeep;
        encoded(claimant, &status);
        QCOMPARE(status, QDataStream::WriteFailed);
        claimant.v0.predicate = fourDeep;
        encoded(claimant, &status);
        QCOMPARE(status, QDataStream::Ok);
    }
};

ADD_TEST(VoidUnionArmTest)

#endif // VOIDUNIONARMTEST_H
