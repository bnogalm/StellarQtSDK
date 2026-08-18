#ifndef REGRESSIONFIXESTEST_H
#define REGRESSIONFIXESTEST_H

#include <QObject>
#include <QtTest>
#include <stdexcept>
#include "../testcollector.h"

#include "../../src/pathpaymentstrictsendoperation.h"
#include "../../src/pathpaymentstrictreceiveoperation.h"
#include "../../src/claimant.h"
#include "../../src/predicate.h"
#include "../../src/managedataoperation.h"
#include "../../src/setoptionsoperation.h"
#include "../../src/inflationoperation.h"
#include "../../src/operation.h"
#include "../../src/accountconverter.h"
#include "../../src/signerkey.h"
#include "../../src/assettypenative.h"
#include "../../src/assettypecreditalphanum4.h"
#include "../../src/keypair.h"
#include "../../src/pathpaymentoperation.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/allowtrustoperation.h"
#include "../../src/responses/orderbookresponse.h"

/**
 * Regressions for the defects found while scripting the QStellar Lab guided
 * tour (see upgrade-plan/sdk-gaps-from-the-tour.md). None of them was covered:
 * the whole suite passed with every one of them present.
 */
class RegressionFixesTest: public QObject
{
    Q_OBJECT

    static const char* DEST() { return "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"; }
    static const char* ISSUER() { return "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"; }

    static Asset* credit(const char* code) {
        return new AssetTypeCreditAlphaNum4(code, QString(ISSUER()));
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** A1. setPath() deleted the Asset* held by the m_path cache without
     *  clearing it, so the next getPath() handed back dangling pointers and
     *  the destructor freed them again. getPath -> setPath -> getPath is
     *  exactly what a UI that paints the path before changing it does. */
    void testSetPathDoesNotDangleAfterGetPath()
    {
        QList<Asset*> path1;
        path1.append(credit("AAA"));
        QScopedPointer<PathPaymentStrictSendOperation> op(
            PathPaymentStrictSendOperation::create(
                new AssetTypeNative(), "10", QString(DEST()), new AssetTypeNative(), "9"));
        op->setPath(path1);
        for (Asset* a : path1) delete a;

        QCOMPARE(op->getPath().size(), 1);          // fills the lazy cache

        QList<Asset*> path2;
        path2.append(credit("BBB"));
        path2.append(credit("CCC"));
        op->setPath(path2);                          // deleted without clearing
        for (Asset* a : path2) delete a;

        QList<Asset*> back = op->getPath();          // before: dangling pointers
        QCOMPARE(back.size(), 2);
        QCOMPARE(back.at(0)->getType(), QString("credit_alphanum4"));
    }

    void testSetPathDoesNotDangleAfterGetPathStrictReceive()
    {
        QList<Asset*> path1;
        path1.append(credit("AAA"));
        QScopedPointer<PathPaymentStrictReceiveOperation> op(
            PathPaymentStrictReceiveOperation::create(
                new AssetTypeNative(), "10", QString(DEST()), new AssetTypeNative(), "9"));
        op->setPath(path1);
        for (Asset* a : path1) delete a;
        QCOMPARE(op->getPath().size(), 1);

        QList<Asset*> path2;
        path2.append(credit("BBB"));
        op->setPath(path2);
        for (Asset* a : path2) delete a;
        QCOMPARE(op->getPath().size(), 1);
    }

    /** A2. Copying a default Claimant dereferenced a null predicate, and
     *  assigning one copied the pointer (double-free) because there was no
     *  operator=. Putting it in a QList triggers both. */
    void testClaimantCopyAndAssignAreSafe()
    {
        Claimant empty;                       // m_predicate == nullptr
        Claimant copy(empty);                 // before: crash
        QCOMPARE(copy.getDestination(), QString());

        QList<Claimant> list;                 // QList copies and assigns inside
        list.append(empty);
        list.append(Claimant(QString(DEST()), new Predicate::Unconditional()));
        QCOMPARE(list.size(), 2);

        Claimant assigned;
        assigned = list.at(1);                // before: shallow copy -> double-free
        QCOMPARE(assigned.getDestination(), QString(DEST()));
        assigned = list.at(0);                // reassigning must free the old one
        QCOMPARE(assigned.getDestination(), QString());
    }

    /** A2b. The same rule of three hole, this time in the predicates. */
    void testPredicateAssignmentIsDeep()
    {
        Predicate::Not a(new Predicate::Unconditional());
        Predicate::Not b(new Predicate::AbsBefore(1600000000));
        a = b;                                 // before: shared pointer -> double-free
        QVERIFY(dynamic_cast<const Predicate::AbsBefore*>(&a.getInner()) != nullptr);

        QList<Predicate*> innerA;
        innerA.append(new Predicate::Unconditional());
        QList<Predicate*> innerB;
        innerB.append(new Predicate::RelBefore(60));
        Predicate::And x(innerA);
        Predicate::And y(innerB);
        x = y;
        QCOMPARE(x.getInner().size(), 1);
        QVERIFY(dynamic_cast<Predicate::RelBefore*>(x.getInner().at(0)) != nullptr);
    }

    /** A5. Array::append silently drops anything past 2, so a node with 3
     *  children was signed with only 2 of them and no warning at all. */
    void testPredicateRejectsMoreThanTwoOperands()
    {
        QList<Predicate*> three;
        three.append(new Predicate::Unconditional());
        three.append(new Predicate::AbsBefore(1600000000));
        three.append(new Predicate::RelBefore(60));
        Predicate::And node(three);
        bool threw = false;
        try { node.toXdr(); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** A8. The name was clipped in a loop and the value inside Array::set:
     *  what got signed differed from what was typed. Now it throws, like
     *  setHomeDomain. */
    void testManageDataRejectsOversizedNameAndValue()
    {
        bool threw = false;
        try { ManageDataOperation(QString(65, QChar('a')), QByteArray("x")); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        threw = false;
        try { ManageDataOperation(QString("ok"), QByteArray(65, 'x')); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        // The valid case right at the limit still works.
        ManageDataOperation ok(QString(64, QChar('a')), QByteArray(64, 'x'));
        QCOMPARE(ok.getName().size(), 64);
    }

    /** A9. weight & 0xFF turned 256 into 0, and weight 0 DELETES the signer:
     *  a range mistake could lock the user out of their own account. */
    void testSetSignerRejectsOutOfRangeWeight()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromAccountId(QString(DEST())));
        SignerKey key = SignerKey::ed25519PublicKey(kp.data());
        SetOptionsOperation op;
        bool threw = false;
        try { op.setSigner(key.toXdr(), 256); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        op.setSigner(key.toXdr(), 255);   // the valid limit still passes
    }

    /** A3. operator!= always returned true, breaking the equality contract
     *  of a Q_GADGET that lives inside QList in QML properties. */
    void testOrderBookRowInequalityIsConsistent()
    {
        OrderBookResponseAttach::Row a, b;
        QVERIFY(a == b);
        QVERIFY(!(a != b));               // before: always true
    }

    /** A6 + A7. create() was not static (so it did not compile like the rest
     *  of the operations) and fromXdr had no INFLATION case, so any envelope
     *  carrying one was undecodable. */
    void testInflationOperationBuildsAndDecodes()
    {
        QScopedPointer<InflationOperation> op(InflationOperation::create());
        QVERIFY(!op.isNull());

        AccountConverter conv;
        stellar::Operation xdr = op->toXdr(conv);
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::INFLATION));

        // A test that THROWS aborts its whole class and drags down the
        // verdicts of the rest, so it is caught and failed right here.
        Operation* decoded = nullptr;
        try { decoded = Operation::fromXdr(xdr); }
        catch (const std::exception& e) {
            QFAIL(qPrintable(QString("fromXdr cannot decode INFLATION: ") + e.what()));
        }
        QScopedPointer<Operation> back(decoded);
        QVERIFY(dynamic_cast<InflationOperation*>(back.data()) != nullptr);
    }

    // ─── Siblings of the same defect, found by sweeping the repo ──────

    /** The third path payment class (the deprecated one) had the SAME
     *  use-after-free as the two strict ones. */
    void testLegacyPathPaymentSetPathDoesNotDangle()
    {
        QList<Asset*> path1;
        path1.append(credit("AAA"));
        QScopedPointer<PathPaymentOperation> op(
            PathPaymentOperation::create(
                new AssetTypeNative(), "10", QString(DEST()), new AssetTypeNative(), "9"));
        op->setPath(path1);
        for (Asset* a : path1) delete a;
        QCOMPARE(op->getPath().size(), 1);

        QList<Asset*> path2;
        path2.append(credit("BBB"));
        op->setPath(path2);
        for (Asset* a : path2) delete a;
        QCOMPARE(op->getPath().size(), 1);
    }

    /** Balance is a Q_GADGET that owns two pointers and has a destructor,
     *  returned by value inside QList from AccountResponse::getBalances():
     *  without its own copy and assignment, copying it was a double-free. */
    void testAccountBalanceCopyAndAssignAreSafe()
    {
        QList<AccountResponseAttach::Balance> list;
        AccountResponseAttach::Balance a;
        list.append(a);
        list.append(a);                       // copy
        AccountResponseAttach::Balance b;
        b = list.at(0);                       // assignment
        QCOMPARE(list.size(), 2);
        Q_UNUSED(b)
    }

    /** The SetOptions constructor had the same & 0xFF mask as setSigner. */
    void testSetOptionsCtorRejectsOutOfRangeSignerWeight()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromAccountId(QString(DEST())));
        SignerKey key = SignerKey::ed25519PublicKey(kp.data());
        bool threw = false;
        try {
            SetOptionsOperation(nullptr, Integer(), Integer(), Integer(), Integer(),
                                Integer(), Integer(), QString(), key.toXdr(), 256);
        } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** fromPublicKey did a 32-byte memcpy without checking the size: with a
     *  shorter buffer it read out of bounds and the key was left half built. */
    void testKeyPairFromPublicKeyRejectsWrongSize()
    {
        bool threw = false;
        try { KeyPair::fromPublicKey(QByteArray(31, '\x01')); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** An asset code longer than 12 bytes was truncated by the memcpy and an
     *  authorization over a DIFFERENT asset got signed. */
    void testAllowTrustRejectsOversizedAssetCode()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromAccountId(QString(DEST())));
        bool threw = false;
        try {
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif
            AllowTrustOperation(kp.data(), QString("VERYLONGASSETCODE"), true, false);
#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
        } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** Codes were validated by counting QChars while the memcpy copies UTF-8
     *  bytes: 4 non-ASCII characters take over 4 bytes and got clipped. */
    void testCreditAssetRejectsMultibyteCodeThatOverflowsBytes()
    {
        bool threw = false;
        try {
            AssetTypeCreditAlphaNum4(QString::fromUtf8("EU\xC3\x91O"),   // 4 chars, 5 bytes
                                     QString(ISSUER()));
        } catch (...) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(RegressionFixesTest)
#endif // REGRESSIONFIXESTEST_H
