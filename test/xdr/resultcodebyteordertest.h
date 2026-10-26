#ifndef RESULTCODEBYTEORDERTEST_H
#define RESULTCODEBYTEORDERTEST_H

#include <QObject>
#include <QtTest>
#include <cstring>
#include <new>
#include "../testcollector.h"

#include "../../xdr/stellartransaction.h"

/**
 * Result codes must be big-endian, like every other four-byte value in XDR.
 *
 * They were not. `XDR_SERIALIZER` is a raw memcpy — its own comment says "use
 * only for fixed array of chars, as it will not take care of byteorder" — and
 * it was applied to every result code enum and to the single-member structs
 * that wrap them. On a little-endian machine that writes the bytes backwards.
 *
 * Nothing noticed, for two reasons that are worth stating together because they
 * are the same trap this codebase has now hit four times.
 *
 * The first is that the bug is SYMMETRIC: the writer and the reader were wrong
 * in the same direction, so every round-trip test passed. The second is that
 * the two values anybody actually tested — txSUCCESS (0) and txFAILED (-1) —
 * are the only two whose byte patterns are palindromes. 0x00000000 and
 * 0xFFFFFFFF read the same in either order. Every other code, which is to say
 * every code that tells you WHY something failed, came back as a large
 * nonsense number: txBAD_SEQ decoded as -67108865.
 *
 * So the bytes below are written out by hand from RFC 4506, not produced by the
 * writer next door.
 */
class ResultCodeByteOrderTest: public QObject
{
    Q_OBJECT

    /** The four bytes XDR requires for a signed 32-bit value. */
    static QByteArray canonical(qint32 value)
    {
        QByteArray out(4, '\0');
        out[0] = char((value >> 24) & 0xFF);
        out[1] = char((value >> 16) & 0xFF);
        out[2] = char((value >> 8) & 0xFF);
        out[3] = char(value & 0xFF);
        return out;
    }

    template <typename T>
    static QByteArray encoded(const T& value)
    {
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << value;
        return out;
    }

    template <typename T>
    static T decoded(const QByteArray& bytes)
    {
        QDataStream s(bytes);
        T value{};
        s >> value;
        return value;
    }

    /** One claimed offer: somebody else's, id 777, native for native. */
    static QByteArray oneClaimedOffer()
    {
        QByteArray bytes = canonical(1);                // offersClaimed.len
        bytes += canonical(1);                          //   CLAIM_ATOM_TYPE_ORDER_BOOK
        bytes += canonical(0) + QByteArray(32, '\x22'); //   sellerID
        bytes += int64Bytes(777);                       //   offerID
        bytes += canonical(0) + int64Bytes(5);          //   assetSold native, amountSold
        bytes += canonical(0) + int64Bytes(6);          //   assetBought native, amountBought
        return bytes;
    }

    /** opINNER, MANAGE_SELL_OFFER, SUCCESS: the claimed offers given, then our created offer 12345. */
    static QByteArray sellOfferOperationResult(const QByteArray& offersClaimed)
    {
        QByteArray bytes;
        bytes += canonical(0) + canonical(3) + canonical(0);
        bytes += offersClaimed;
        bytes += canonical(0);                          // MANAGE_OFFER_CREATED
        bytes += canonical(0) + QByteArray(32, '\x33'); // offer.sellerID
        bytes += int64Bytes(12345);                     // offer.offerID
        bytes += canonical(0) + canonical(0);           // selling, buying native
        bytes += int64Bytes(10);                        // amount
        bytes += canonical(1) + canonical(2);           // price 1/2
        bytes += canonical(0) + canonical(0);           // flags, offer ext
        return bytes;
    }

    /** A txSUCCESS result around that operation result, both counts spelled by the caller. */
    static QByteArray offerResult(const QByteArray& resultsCount, const QByteArray& offersClaimed)
    {
        return int64Bytes(100) + canonical(0) + resultsCount
               + sellOfferOperationResult(offersClaimed) + canonical(0);
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testTransactionResultCodeIsBigEndian()
    {
        using stellar::TransactionResultCode;

        // -5 is FF FF FF FB. Written backwards it is FB FF FF FF, which reads
        // back as -67108865 — a number no result code ever has.
        QCOMPARE(encoded(TransactionResultCode::txBAD_SEQ), canonical(-5));
        QCOMPARE(encoded(TransactionResultCode::txINSUFFICIENT_FEE), canonical(-9));
        QCOMPARE(encoded(TransactionResultCode::txFEE_BUMP_INNER_FAILED), canonical(-13));
        QCOMPARE(encoded(TransactionResultCode::txFEE_BUMP_INNER_SUCCESS), canonical(1));

        QCOMPARE(decoded<TransactionResultCode>(canonical(-5)),
                 TransactionResultCode::txBAD_SEQ);
        QCOMPARE(decoded<TransactionResultCode>(canonical(-6)),
                 TransactionResultCode::txBAD_AUTH);
        QCOMPARE(decoded<TransactionResultCode>(canonical(-14)),
                 TransactionResultCode::txBAD_SPONSORSHIP);
    }

    void testThePalindromesThatHidTheBug()
    {
        using stellar::TransactionResultCode;
        // These two are the same in either byte order, which is exactly why a
        // suite that only ever saw them stayed green.
        QCOMPARE(encoded(TransactionResultCode::txSUCCESS), canonical(0));
        QCOMPARE(encoded(TransactionResultCode::txFAILED), canonical(-1));
    }

    void testOperationResultCodeIsBigEndian()
    {
        using stellar::OperationResultCode;
        QCOMPARE(encoded(OperationResultCode::opNO_ACCOUNT), canonical(-2));
        QCOMPARE(encoded(OperationResultCode::opEXCEEDED_WORK_LIMIT), canonical(-5));
        QCOMPARE(decoded<OperationResultCode>(canonical(-4)),
                 OperationResultCode::opTOO_MANY_SUBENTRIES);
    }

    void testEveryOperationResultCodeIsBigEndian()
    {
        // One per operation family, because they are twenty separate macro
        // invocations and a fix that missed one would be invisible again.
        QCOMPARE(encoded(stellar::PaymentResultCode::PAYMENT_NO_DESTINATION), canonical(-5));
        QCOMPARE(encoded(stellar::PathPaymentStrictReceiveResultCode
                             ::PATH_PAYMENT_STRICT_RECEIVE_UNDERFUNDED), canonical(-2));
        QCOMPARE(encoded(stellar::PathPaymentStrictSendResultCode
                             ::PATH_PAYMENT_STRICT_SEND_UNDERFUNDED), canonical(-2));
        QCOMPARE(encoded(stellar::ManageSellOfferResultCode
                             ::MANAGE_SELL_OFFER_UNDERFUNDED), canonical(-7));
        QCOMPARE(encoded(stellar::SetOptionsResultCode::SET_OPTIONS_BAD_SIGNER), canonical(-8));
        QCOMPARE(encoded(stellar::ChangeTrustResultCode::CHANGE_TRUST_LOW_RESERVE), canonical(-4));
        QCOMPARE(encoded(stellar::AllowTrustResultCode::ALLOW_TRUST_NO_TRUST_LINE), canonical(-2));
        QCOMPARE(encoded(stellar::AccountMergeResultCode::ACCOUNT_MERGE_NO_ACCOUNT), canonical(-2));
        QCOMPARE(encoded(stellar::InflationResultCode::INFLATION_NOT_TIME), canonical(-1));
        QCOMPARE(encoded(stellar::ManageDataResultCode::MANAGE_DATA_NAME_NOT_FOUND), canonical(-2));
        QCOMPARE(encoded(stellar::BumpSequenceResultCode::BUMP_SEQUENCE_BAD_SEQ), canonical(-1));
        QCOMPARE(encoded(stellar::ClawbackResultCode::CLAWBACK_NOT_CLAWBACK_ENABLED),
                 canonical(-2));
        QCOMPARE(encoded(stellar::SetTrustLineFlagsResultCode
                             ::SET_TRUST_LINE_FLAGS_CANT_REVOKE), canonical(-3));
        QCOMPARE(encoded(stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED), canonical(1));
    }

    void testTheResultStructsAreBigEndianToo()
    {
        // Each of these is a struct holding exactly one code, so the same raw
        // memcpy reversed them just as thoroughly.
        stellar::PaymentResult payment;
        payment.code = stellar::PaymentResultCode::PAYMENT_LINE_FULL;
        QCOMPARE(encoded(payment), canonical(-8));

        stellar::CreateAccountResult create;
        create.code = stellar::CreateAccountResultCode::CREATE_ACCOUNT_UNDERFUNDED;
        QCOMPARE(encoded(create), canonical(-2));

        stellar::SetOptionsResult options;
        options.code = stellar::SetOptionsResultCode::SET_OPTIONS_BAD_SIGNER;
        QCOMPARE(encoded(options), canonical(-8));

        const auto readBack = decoded<stellar::PaymentResult>(canonical(-5));
        QCOMPARE(readBack.code, stellar::PaymentResultCode::PAYMENT_NO_DESTINATION);
    }

    void testAFeeBumpResultCarriesItsInnerOperations()
    {
        // The inner reader used to say `in << obj.results` — an output operator
        // inside an input one. It compiled, wrote to a read-only buffer, set the
        // stream to WriteFailed, and handed back an inner transaction that
        // appeared to have done nothing at all. Neither side wrote `ext`
        // either, so every inner result was four bytes short of canonical.
        //
        // Assembled by hand:
        //   feeCharged 200, txFEE_BUMP_INNER_FAILED (-13)
        //   innerResultPair: 32-byte hash, then the inner result
        //     inner: feeCharged 0, txFAILED (-1), one op result
        //            (opINNER, PAYMENT, PAYMENT_UNDERFUNDED -2), ext v = 0
        //   ext v = 0
        QByteArray bytes;
        bytes += canonical(0) + canonical(200);      // outer feeCharged
        bytes += canonical(-13);                     // txFEE_BUMP_INNER_FAILED
        bytes += QByteArray(32, '\x11');             // inner transaction hash
        bytes += canonical(0) + canonical(0);        // inner feeCharged
        bytes += canonical(-1);                      // inner txFAILED
        bytes += canonical(1);                       // one operation result
        bytes += canonical(0);                       // opINNER
        bytes += canonical(1);                       // OperationType PAYMENT
        bytes += canonical(-2);                      // PAYMENT_UNDERFUNDED
        bytes += canonical(0);                       // inner ext.v
        bytes += canonical(0);                       // outer ext.v

        QDataStream in(bytes);
        stellar::TransactionResult result;
        in >> result;

        QCOMPARE(in.status(), QDataStream::Ok);
        QCOMPARE(result.code, stellar::TransactionResultCode::txFEE_BUMP_INNER_FAILED);
        QCOMPARE(result.innerResultPair.result.code,
                 stellar::TransactionResultCode::txFAILED);
        // The part that used to come back empty.
        QCOMPARE(result.innerResultPair.result.results.size(), 1);
        QCOMPARE(result.innerResultPair.result.results.at(0).paymentResult.code,
                 stellar::PaymentResultCode::PAYMENT_UNDERFUNDED);
        QVERIFY(in.atEnd());
    }

    void testAWholeResultFromHorizonReadsCorrectly()
    {
        // Horizon hands back result_xdr like this. Assembled by hand:
        //   feeCharged  = 100
        //   code        = txFAILED (-1)
        //   results.len = 1
        //     opINNER (0), type PAYMENT (1), PAYMENT_NO_DESTINATION (-5)
        //   ext.v       = 0
        QByteArray bytes;
        bytes += canonical(0) + canonical(100);      // int64 feeCharged, big-endian
        bytes += canonical(-1);                      // txFAILED
        bytes += canonical(1);                       // one operation result
        bytes += canonical(0);                       // opINNER
        bytes += canonical(1);                       // OperationType PAYMENT
        bytes += canonical(-5);                      // PAYMENT_NO_DESTINATION
        bytes += canonical(0);                       // ext.v

        QDataStream in(bytes);
        stellar::TransactionResult result;
        in >> result;

        QCOMPARE(result.feeCharged, qint64(100));
        QCOMPARE(result.code, stellar::TransactionResultCode::txFAILED);
        QCOMPARE(result.results.size(), 1);
        // The whole point: the reason, not a number nobody can look up.
        QCOMPARE(result.results.at(0).paymentResult.code,
                 stellar::PaymentResultCode::PAYMENT_NO_DESTINATION);
    }

    /**
     * The writer now puts the inner result's ext discriminant on the wire, so
     * it must start life as 0, its only arm. It had no initialiser, and a
     * default-constructed result encoded whatever the memory held there.
     *
     * Constructed over bytes filled with 0xAB on purpose: on a fresh stack
     * frame the old member could easily have read 0 by luck and passed.
     */
    void testAnInnerResultEncodesItsExtAsZero()
    {
        alignas(stellar::InnerTransactionResult)
            unsigned char storage[sizeof(stellar::InnerTransactionResult)];
        std::memset(storage, 0xAB, sizeof(storage));
        auto* inner = new (storage) stellar::InnerTransactionResult;
        inner->feeCharged = 100;
        inner->code = stellar::TransactionResultCode::txBAD_SEQ;   // no results arm

        QByteArray expected;
        expected += canonical(0) + canonical(100);   // int64 feeCharged
        expected += canonical(-5);                   // txBAD_SEQ
        expected += canonical(0);                    // ext.v
        QCOMPARE(encoded(*inner), expected);

        inner->~InnerTransactionResult();
    }

    static QByteArray int64Bytes(qint64 v)
    {
        return canonical(qint32(v >> 32)) + canonical(qint32(v & 0xffffffff));
    }

    /**
     * A manage-offer result that crossed another offer. ClaimAtom is a union,
     * and without its discriminant everything after it was misread: the
     * caller's own offer id came back as 0, or as another seller's. Assembled
     * by hand from Stellar-transaction.x.
     */
    void testAnOfferThatCrossedAnotherKeepsItsOwnId()
    {
        QByteArray bytes;
        bytes += int64Bytes(100);                       // feeCharged
        bytes += canonical(0);                          // txSUCCESS
        bytes += canonical(1);                          // one operation result
        bytes += canonical(0);                          // opINNER
        bytes += canonical(3);                          // MANAGE_SELL_OFFER
        bytes += canonical(0);                          // MANAGE_SELL_OFFER_SUCCESS
        bytes += canonical(1);                          // offersClaimed.len
        bytes += canonical(1);                          //   CLAIM_ATOM_TYPE_ORDER_BOOK
        bytes += canonical(0) + QByteArray(32, '\x22'); //   sellerID
        bytes += int64Bytes(777);                       //   offerID: somebody else's
        bytes += canonical(0) + int64Bytes(5);          //   assetSold native, amountSold
        bytes += canonical(0) + int64Bytes(6);          //   assetBought native, amountBought
        bytes += canonical(0);                          // MANAGE_OFFER_CREATED
        bytes += canonical(0) + QByteArray(32, '\x33'); // offer.sellerID
        bytes += int64Bytes(12345);                     // offer.offerID: ours
        bytes += canonical(0) + canonical(0);           // selling, buying native
        bytes += int64Bytes(10);                        // amount
        bytes += canonical(1) + canonical(2);           // price 1/2
        bytes += canonical(0);                          // flags
        bytes += canonical(0);                          // offer ext
        bytes += canonical(0);                          // result ext

        QDataStream in(bytes);
        stellar::TransactionResult result;
        in >> result;
        QCOMPARE(in.status(), QDataStream::Ok);
        QVERIFY(in.atEnd());
        const stellar::ManageOfferSuccessResult& success =
            result.results.at(0).manageSellOfferResult.success;
        QCOMPARE(success.offersClaimed.size(), 1);
        QVERIFY(success.offersClaimed.at(0).type == stellar::ClaimAtomType::CLAIM_ATOM_TYPE_ORDER_BOOK);
        QCOMPARE(success.offersClaimed.at(0).orderBook.offerID, qint64(777));
        QCOMPARE(success.offer.offerID, qint64(12345));
        QCOMPARE(encoded(result), bytes);
    }

    /** The three Soroban operation results had no arm: their payload stayed
     *  in the stream and was read as whatever came next. */
    void testASorobanOperationResultIsReadInFull()
    {
        QByteArray bytes;
        bytes += int64Bytes(100) + canonical(0) + canonical(1);
        bytes += canonical(0);                          // opINNER
        bytes += canonical(24);                         // INVOKE_HOST_FUNCTION
        bytes += canonical(0);                          // SUCCESS
        bytes += QByteArray(32, '\x44');                // the success hash
        bytes += canonical(0);                          // result ext
        QDataStream in(bytes);
        stellar::TransactionResult result;
        in >> result;
        QCOMPARE(in.status(), QDataStream::Ok);
        QVERIFY(in.atEnd());
        QVERIFY(result.results.at(0).invokeHostFunctionResult.code
                == stellar::InvokeHostFunctionResultCode::INVOKE_HOST_FUNCTION_SUCCESS);
        QCOMPARE(QByteArray(reinterpret_cast<const char*>(
                     result.results.at(0).invokeHostFunctionResult.success), 32),
                 QByteArray(32, '\x44'));
        QCOMPARE(encoded(result), bytes);

        // An operation type this does not know is a corrupt read, not a skip.
        QByteArray unknown = int64Bytes(100) + canonical(0) + canonical(1)
                             + canonical(0) + canonical(99) + canonical(0);
        QDataStream in2(unknown);
        stellar::TransactionResult r2;
        in2 >> r2;
        QVERIFY(in2.status() != QDataStream::Ok);
    }

    /**
     * A result array is an XDR array: a four-byte count that the rest of the
     * input must be able to hold. They were read with Qt's own QVector
     * framing, which reserved whatever count arrived (20 bytes asked for
     * gigabytes, then threw std::bad_alloc) and, since Qt 6.7, took
     * 0xFFFFFFFE as "a 64-bit count follows", which js and py refuse.
     */
    void testAResultArrayIsCountedTheXdrWay()
    {
        const QByteArray one = canonical(1);
        const QByteArray control = offerResult(one, oneClaimedOffer());
        {
            QDataStream in(control);
            stellar::TransactionResult result;
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
            QCOMPARE(result.results.at(0).manageSellOfferResult.success.offer.offerID, qint64(12345));
            QCOMPARE(encoded(result), control);         // four-byte counts on the way out too
        }

        const QByteArray qtCount = canonical(-2);       // FF FF FF FE: Qt's "64-bit count follows"
        const QByteArray huge = canonical(0x7fffffff);
        const QByteArray header = int64Bytes(100) + canonical(0);
        const QList<QByteArray> refused = {
            offerResult(qtCount + int64Bytes(1), canonical(0)),         // results
            header + qtCount + int64Bytes(qint64(1) << 40),
            header + huge + canonical(0),                               // 20 bytes in all
            offerResult(one, qtCount + int64Bytes(0)),                  // offersClaimed
            offerResult(one, huge),
            header + one + canonical(0) + canonical(13) + canonical(0) + huge + canonical(0),  // path payment offers
            header + one + canonical(0) + canonical(9) + canonical(0) + huge + canonical(0),   // inflation payouts
            int64Bytes(200) + canonical(1) + QByteArray(32, '\x33')    // a fee bump's inner results
                + int64Bytes(100) + canonical(0) + huge + canonical(0) + canonical(0),
        };
        for (const QByteArray& bytes : refused) {
            QDataStream in(bytes);
            stellar::TransactionResult result;
            try {
                in >> result;
            } catch (const std::exception& e) {
                QFAIL(e.what());
            }
            QVERIFY(in.status() != QDataStream::Ok);
        }
    }

    /**
     * Reading into a result that already held one replaces it. The reader
     * never cleared `results` or the inner pair, so a txBAD_SEQ read into an
     * object that held operation results still had them all.
     */
    void testAResultReadOverAnotherReplacesIt()
    {
        const QByteArray success = offerResult(canonical(1), oneClaimedOffer());
        const QByteArray badSeq = int64Bytes(100) + canonical(-5) + canonical(0);
        const QByteArray feeBump = int64Bytes(200) + canonical(1) + QByteArray(32, '\x33')
                                   + int64Bytes(0) + canonical(0) + canonical(1)
                                   + canonical(0) + canonical(1) + canonical(0)  // opINNER, PAYMENT, SUCCESS
                                   + canonical(0) + canonical(0);                // inner ext, ext
        stellar::TransactionResult result;
        for (const QByteArray& bytes : { success, badSeq, feeBump, badSeq }) {
            QDataStream in(bytes);
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
            QCOMPARE(result.results.size(), bytes == success ? 1 : 0);
            QCOMPARE(result.innerResultPair.result.results.size(), bytes == feeBump ? 1 : 0);
            QCOMPARE(encoded(result), bytes);
        }
    }

    /**
     * r = r keeps r, and reading into a used result releases what it held.
     * The assignment rebuilt its own arm empty before copying from it (a
     * created offer came back with id 0), and the reader built the new arm
     * over the live one, leaking it. The leak is checked through the claimed
     * offers' shared data: a copy taken first must end up its only owner.
     */
    void testAnOperationResultSurvivesAssignmentAndRereading()
    {
        const QByteArray sellOffer = sellOfferOperationResult(oneClaimedOffer());
        stellar::OperationResult result;
        {
            QDataStream in(sellOffer);
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
        }
        const stellar::OperationResult& same = result;
        result = same;
        QCOMPARE(result.manageSellOfferResult.success.offer.offerID, qint64(12345));
        QCOMPARE(encoded(result), sellOffer);

        QVector<stellar::ClaimAtom> held = result.manageSellOfferResult.success.offersClaimed;
        QVERIFY(!held.isDetached());
        const QByteArray createAccount = canonical(0) + canonical(0) + canonical(0);
        {
            QDataStream in(createAccount);
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
        }
        QVERIFY2(held.isDetached(), "reading over a manage-offer result leaked it");
        QCOMPARE(encoded(result), createAccount);
    }

    /**
     * An offer effect or an offer ext with no arm is a corrupt read. Effect 3
     * was read as "no offer follows" and ext 1 as nothing, and the bytes after
     * them were read as whatever came next.
     */
    void testAnUnknownOfferEffectOrExtIsCorrupt()
    {
        QByteArray effect = sellOfferOperationResult(canonical(0));
        effect.replace(16, 4, canonical(3));            // after opINNER, type, code, offersClaimed.len
        QByteArray ext = sellOfferOperationResult(canonical(0));
        ext.replace(ext.size() - 4, 4, canonical(1));
        for (const QByteArray& bytes : { effect, ext }) {
            QDataStream in(bytes);
            stellar::OperationResult result;
            in >> result;
            QVERIFY(in.status() != QDataStream::Ok);
        }
    }

    /**
     * The codes stellar-xdr curr has and this SDK did not: four transaction
     * codes, opTOO_MANY_SPONSORING, and the newer per-operation failures.
     * Each reads from its bytes and writes the same bytes back, by name.
     */
    void testTheNewerResultCodesAreKnown()
    {
        using stellar::TransactionResultCode;
        QCOMPARE(encoded(TransactionResultCode::txBAD_MIN_SEQ_AGE_OR_GAP), canonical(-15));
        QCOMPARE(encoded(TransactionResultCode::txMALFORMED), canonical(-16));
        QCOMPARE(encoded(TransactionResultCode::txSOROBAN_INVALID), canonical(-17));
        QCOMPARE(encoded(TransactionResultCode::txFROZEN_KEY_ACCESSED), canonical(-18));
        for (qint32 code : { -15, -16, -17, -18 }) {
            const QByteArray bytes = int64Bytes(100) + canonical(code) + canonical(0);
            QDataStream in(bytes);
            stellar::TransactionResult result;
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
            QCOMPARE(qint32(result.code), code);
            QCOMPARE(encoded(result), bytes);
        }

        QCOMPARE(encoded(stellar::OperationResultCode::opTOO_MANY_SPONSORING), canonical(-6));
        QCOMPARE(encoded(stellar::ChangeTrustResultCode::CHANGE_TRUST_TRUST_LINE_MISSING), canonical(-6));
        QCOMPARE(encoded(stellar::ChangeTrustResultCode::CHANGE_TRUST_CANNOT_DELETE), canonical(-7));
        QCOMPARE(encoded(stellar::ChangeTrustResultCode::CHANGE_TRUST_NOT_AUTH_MAINTAIN_LIABILITIES), canonical(-8));
        QCOMPARE(encoded(stellar::AllowTrustResultCode::ALLOW_TRUST_LOW_RESERVE), canonical(-6));
        QCOMPARE(encoded(stellar::ClaimClaimableBalanceResultCode::CLAIM_CLAIMABLE_BALANCE_TRUSTLINE_FROZEN), canonical(-6));
        QCOMPARE(encoded(stellar::RevokeSponsorshipResultCode::REVOKE_SPONSORSHIP_MALFORMED), canonical(-5));
        QCOMPARE(encoded(stellar::SetTrustLineFlagsResultCode::SET_TRUST_LINE_FLAGS_LOW_RESERVE), canonical(-5));
        QCOMPARE(encoded(stellar::LiquidityPoolDepositResultCode::LIQUIDITY_POOL_DEPOSIT_TRUSTLINE_FROZEN), canonical(-8));
        QCOMPARE(encoded(stellar::LiquidityPoolWithdrawResultCode::LIQUIDITY_POOL_WITHDRAW_TRUSTLINE_FROZEN), canonical(-6));
        QCOMPARE(encoded(stellar::PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_UNDER_DESTMIN), canonical(-12));

        // opTOO_MANY_SPONSORING as an operation result, and each new code in
        // its operation's result (all of them void arms).
        const QList<QPair<qint32, qint32>> operationCodes = {
            { 6, -6 }, { 6, -7 }, { 6, -8 },    // CHANGE_TRUST
            { 7, -6 },                           // ALLOW_TRUST
            { 15, -6 },                          // CLAIM_CLAIMABLE_BALANCE
            { 18, -5 },                          // REVOKE_SPONSORSHIP
            { 21, -5 },                          // SET_TRUST_LINE_FLAGS
            { 22, -8 },                          // LIQUIDITY_POOL_DEPOSIT
            { 23, -6 },                          // LIQUIDITY_POOL_WITHDRAW
            { 13, -12 },                         // PATH_PAYMENT_STRICT_SEND
        };
        QList<QByteArray> operationResults = { canonical(-6) };   // opTOO_MANY_SPONSORING
        for (const auto& typeAndCode : operationCodes)
            operationResults.append(canonical(0) + canonical(typeAndCode.first) + canonical(typeAndCode.second));
        for (const QByteArray& bytes : operationResults) {
            QDataStream in(bytes);
            stellar::OperationResult result;
            in >> result;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
            QCOMPARE(encoded(result), bytes);
        }
    }

    /**
     * A code outside its union is corrupt data, not a void arm. No result
     * union has a `default:`, so -19 for a transaction, -7 for an operation,
     * -10 for a payment or a fee-bump code inside a fee bump's inner result
     * has no encoding; each was read as "nothing follows", and the bytes
     * after it as the next field. A writer refuses them too.
     */
    void testAResultCodeWithNoArmIsCorrupt()
    {
        const QList<QByteArray> refused = {
            int64Bytes(100) + canonical(-19) + canonical(0),              // transaction
            int64Bytes(100) + canonical(2) + canonical(0),
            int64Bytes(100) + canonical(-1) + canonical(1) + canonical(-7) + canonical(0),     // operation
            int64Bytes(100) + canonical(-1) + canonical(1)
                + canonical(0) + canonical(1) + canonical(-10) + canonical(0),                 // payment
            int64Bytes(100) + canonical(-1) + canonical(1)
                + canonical(0) + canonical(6) + canonical(-9) + canonical(0),                  // change trust
            int64Bytes(100) + canonical(-1) + canonical(1)
                + canonical(0) + canonical(24) + canonical(-6) + canonical(0),                 // invoke host function
            int64Bytes(200) + canonical(1) + QByteArray(32, '\x33')                           // a fee bump whose inner
                + int64Bytes(0) + canonical(1) + canonical(0) + canonical(0),                  // result is a fee bump
        };
        for (const QByteArray& bytes : refused) {
            QDataStream in(bytes);
            stellar::TransactionResult result;
            in >> result;
            QCOMPARE(in.status(), QDataStream::ReadCorruptData);
        }

        stellar::PaymentResult payment;
        payment.code = stellar::PaymentResultCode(-10);
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << payment;
        QCOMPARE(s.status(), QDataStream::WriteFailed);
        QVERIFY(out.isEmpty());
    }

    /**
     * An operation result that cannot be encoded writes nothing at all. With
     * opINNER and a type that has no arm, the code and the type went out
     * first, eight bytes of a result nothing can read, and only then was the
     * write marked failed.
     */
    void testAnOperationResultWithNoArmWritesNothing()
    {
        const auto write = [](const stellar::OperationResult& result, QByteArray* bytes) {
            QDataStream s(bytes, QIODevice::WriteOnly);
            s << result;
            return s.status();
        };
        stellar::OperationResult result;
        for (qint32 type : { 27, -1 }) {
            result.code = stellar::OperationResultCode::opINNER;
            result.type = stellar::OperationType(type);
            QByteArray bytes;
            QCOMPARE(write(result, &bytes), QDataStream::WriteFailed);
            QVERIFY(bytes.isEmpty());
        }
        result.type = stellar::OperationType::CREATE_ACCOUNT;
        result.code = stellar::OperationResultCode(-7);
        QByteArray bytes;
        QCOMPARE(write(result, &bytes), QDataStream::WriteFailed);
        QVERIFY(bytes.isEmpty());

        // A code other than opINNER carries no type: whatever the field holds
        // is not written, and not checked.
        result.code = stellar::OperationResultCode::opBAD_AUTH;
        result.type = stellar::OperationType(27);
        bytes.clear();
        QCOMPARE(write(result, &bytes), QDataStream::Ok);
        QCOMPARE(bytes, canonical(-1));
        result.type = stellar::OperationType::CREATE_ACCOUNT;
    }

    /**
     * A create-claimable-balance result's code and its balance id are two
     * fields. It was a union: the code and the id's type shared storage, so
     * assigning an id turned UNDERFUNDED (-5) into SUCCESS (0). Found while
     * reading results in the live interop test.
     */
    void testAClaimableBalanceResultKeepsItsCode()
    {
        stellar::CreateClaimableBalanceResult result;
        result.code = stellar::CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_UNDERFUNDED;
        result.balanceID = stellar::ClaimableBalanceID();
        QCOMPARE(result.code, stellar::CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_UNDERFUNDED);
        QByteArray bytes;
        {
            QDataStream s(&bytes, QIODevice::WriteOnly);
            s << result;
        }
        QCOMPARE(bytes, canonical(-5));

        stellar::CreateClaimableBalanceResult success;
        success.code = stellar::CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_SUCCESS;
        memset(success.balanceID.v0, 0x11, sizeof(success.balanceID.v0));
        bytes.clear();
        {
            QDataStream s(&bytes, QIODevice::WriteOnly);
            s << success;
        }
        QCOMPARE(bytes.size(), 4 + 4 + 32);
        stellar::CreateClaimableBalanceResult back;
        back.code = stellar::CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_MALFORMED;
        QDataStream r(&bytes, QIODevice::ReadOnly);
        r >> back;
        QCOMPARE(r.status(), QDataStream::Ok);
        QCOMPARE(back.code, stellar::CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_SUCCESS);
        QCOMPARE(QByteArray(reinterpret_cast<const char*>(back.balanceID.v0), 32), QByteArray(32, '\x11'));
    }
};

ADD_TEST(ResultCodeByteOrderTest)
#endif // RESULTCODEBYTEORDERTEST_H
