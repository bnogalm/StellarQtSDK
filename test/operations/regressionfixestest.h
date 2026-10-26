#ifndef REGRESSIONFIXESTEST_H
#define REGRESSIONFIXESTEST_H

#include <QObject>
#include <QtTest>
#include <stdexcept>
// The header declares the deprecated class, so the include itself warns.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include "../../src/responses/operations/allowtrustoperationresponse.h"
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif
#include "../../src/responses/operations/setoptionsoperationresponse.h"
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
#include "../../src/createclaimablebalanceoperation.h"
#include "../../src/assettypecreditalphanum4.h"
#include "../../src/keypair.h"
#include "../../src/pathpaymentoperation.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/allowtrustoperation.h"
#include "../../src/responses/orderbookresponse.h"
#include "../../src/paymentoperation.h"
#include "../../src/responses/page.h"
#include "../../src/responses/claimablebalanceresponse.h"
#include "../../src/responses/operations/createclaimablebalanceoperationresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedPointer>
#include <cstring>
#include <limits>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

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

    static QString maxSeconds() { return QString::number(std::numeric_limits<qint64>::max()); }

    /** A predicate as a line of text: and(abs:0,unconditional), not(rel:60). */
    static QString shape(const Predicate& predicate)
    {
        Predicate* p = const_cast<Predicate*>(&predicate);
        if (dynamic_cast<Predicate::Unconditional*>(p))
            return QStringLiteral("unconditional");
        if (auto* n = dynamic_cast<Predicate::Not*>(p))
            return QStringLiteral("not(") + shape(n->getInner()) + QLatin1Char(')');
        QString kind;
        QList<Predicate*> operands;
        if (auto* a = dynamic_cast<Predicate::And*>(p)) {
            kind = QStringLiteral("and");
            operands = a->getInner();
        } else if (auto* o = dynamic_cast<Predicate::Or*>(p)) {
            kind = QStringLiteral("or");
            operands = o->getInner();
        }
        if (!kind.isEmpty()) {
            QStringList parts;
            for (Predicate* operand : operands)
                parts << shape(*operand);
            return kind + QLatin1Char('(') + parts.join(QLatin1Char(',')) + QLatin1Char(')');
        }
        if (auto* t = dynamic_cast<Predicate::AbsBefore*>(p))
            return QStringLiteral("abs:") + QString::number(t->getTimestampSeconds());
        if (auto* r = dynamic_cast<Predicate::RelBefore*>(p))
            return QStringLiteral("rel:") + QString::number(r->getSecondsSinceClose());
        return QStringLiteral("?");
    }

    /** A claimant's XDR bytes, as a CreateClaimableBalance operation signs them. */
    static QByteArray claimantXdr(const Claimant& claimant)
    {
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << claimant.toXdr();
        return bytes;
    }

    /** A claimant of DEST() whose predicate is or(first, second). */
    static QList<Claimant> orClaimants(Predicate* first, Predicate* second)
    {
        return QList<Claimant>() << Claimant(QString(DEST()),
                                             new Predicate::Or(QList<Predicate*>() << first << second));
    }

    /** A predicate type Predicate::create does not know, so copying it throws. */
    struct ForeignPredicate : public Predicate {
        bool equals(Predicate* o) const override { return o == this; }
        int hashCode() const override { return 1; }
        stellar::ClaimPredicate toXdr() const override { return stellar::ClaimPredicate(); }
    };

    /** The predicate `json` holds, read by Predicate::create(QVariant), as shape() writes it. */
    static QString shapeOf(const QJsonValue& json)
    {
        QScopedPointer<Predicate> read(Predicate::create(json.toVariant()));
        return shape(*read);
    }

    static QJsonValue json(const char* text)
    {
        return QJsonDocument::fromJson(QByteArray(text)).object();
    }

    /**
     * What Predicate::fromJson(`value`, "p") refuses it with, when
     * Predicate::create(QVariant) refuses it too; otherwise which one read it.
     */
    static QString refusalOf(const QJsonValue& value)
    {
        QString why;
        try {
            delete Predicate::fromJson(value, QStringLiteral("p"));
            return QStringLiteral("fromJson read it");
        } catch (const std::runtime_error& e) {
            why = QString::fromUtf8(e.what());
        }
        try {
            delete Predicate::create(value.toVariant());
            return QStringLiteral("create(QVariant) read it: ") + why;
        } catch (const std::runtime_error&) {
        }
        return why;
    }

    /** The same for ClaimantConverter::fromJson(`text`, "claimants") and convertData. */
    static QString claimantsRefusalOf(const char* text)
    {
        const QJsonValue value = QJsonDocument::fromJson(QByteArray(text)).array();
        QString why;
        try {
            ClaimantConverter::fromJson(value, QStringLiteral("claimants"));
            return QStringLiteral("fromJson read it");
        } catch (const std::runtime_error& e) {
            why = QString::fromUtf8(e.what());
        }
        try {
            ClaimantConverter::convertData(value.toArray().toVariantList());
            return QStringLiteral("convertData read it: ") + why;
        } catch (const std::runtime_error&) {
        }
        return why;
    }

    /** The claimants of record `record` of a page of claimable balances. */
    static QJsonArray claimantsOf(const QByteArray& page, int record)
    {
        return QJsonDocument::fromJson(page).object().value(QStringLiteral("_embedded")).toObject()
            .value(QStringLiteral("records")).toArray().at(record).toObject()
            .value(QStringLiteral("claimants")).toArray();
    }

    /** Each claimant's predicate of `claimants`, read by Predicate::create(QVariant). */
    static QStringList shapesOf(const QJsonArray& claimants)
    {
        QStringList shapes;
        for (const QJsonValue& claimant : claimants)
            shapes << shapeOf(claimant.toObject().value(QStringLiteral("predicate")));
        return shapes;
    }

    /** Real testnet claimable balances (links dropped from the records),
     *  made for this test: times past the year 9999 up to the last an int64
     *  holds, and predicates nested the 4 levels Core allows. */
    static QByteArray realTestnetClaimableBalances()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000a587ccfcb3e08c87da746a8f5f7816ca8997edb694098feb31c2190c22b32ba5\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000a587ccfcb3e08c87da746a8f5f7816ca8997edb694098feb31c2190c22b32ba5/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000a587ccfcb3e08c87da746a8f5f7816ca8997edb694098feb31c2190c22b32ba5/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"00000000a587ccfcb3e08c87da746a8f5f7816ca8997edb694098feb31c2190c22b32ba5\",\"a"
            "sset\":\"native\",\"amount\":\"2.5000000\",\"sponsor\":\"GDNZVVY2LIMUQ3BAFU7CNXIPKPCEPLQVWCC3ZJK32YX3SZDR4X4VHIBD\",\"la"
            "st_modified_ledger\":4927095,\"last_modified_time\":\"2026-09-29T05:17:42Z\",\"claimants\":[{\"destination\":\"GCQFHLK"
            "QCPU5PJKWI3U4HEAW4NGKQNRJ3UVCIWBCIBOM5G4JF4EOTSA5\",\"predicate\":{\"and\":[{\"or\":[{\"not\":{\"abs_before\":\"1970-01-"
            "01T00:00:00Z\",\"abs_before_epoch\":\"0\"}},{\"abs_before\":\"2026-09-29T05:17:42Z\",\"abs_before_epoch\":\"1790659062\"}"
            "]},{\"unconditional\":true}]}},{\"destination\":\"GAPJX3CDN6ZD5IC5W35IZQB5VJDXJ2X3RYHO7ZH7OQBIILVWYS6ZNPQW\",\"pred"
            "icate\":{\"or\":[{\"and\":[{\"not\":{\"abs_before\":\"2026-09-29T06:17:42Z\",\"abs_before_epoch\":\"1790662662\"}},{\"abs_be"
            "fore\":\"2026-09-21T14:13:20Z\",\"abs_before_epoch\":\"1790000000\"}]},{\"not\":{\"unconditional\":true}}]}},{\"destinat"
            "ion\":\"GD4E7KUTIZ4Z76GE2DBJP23CXYYEUSX67FWFTKAZWSCZTFHO2VZ4QB6B\",\"predicate\":{\"abs_before\":\"2026-09-29T05:17:"
            "43Z\",\"abs_before_epoch\":\"1790659063\"}},{\"destination\":\"GA64UBAZ7T6V7UZP7SCIVBFFRLVQBUDXC2JFN5WMSBL7NFFQPFGWF"
            "FER\",\"predicate\":{\"not\":{\"not\":{\"not\":{\"abs_before\":\"2026-09-30T05:17:42Z\",\"abs_before_epoch\":\"1790745462\"}}"
            "}}}],\"flags\":{\"clawback_enabled\":false},\"paging_token\":\"4927095-00000000a587ccfcb3e08c87da746a8f5f7816ca8997"
            "edb694098feb31c2190c22b32ba5\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff775268480ec3088e84\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff775268480ec3088e84/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff775268480ec3088e84/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff775268480ec3088e"
            "84\",\"asset\":\"native\",\"amount\":\"1.2345678\",\"sponsor\":\"GDNZVVY2LIMUQ3BAFU7CNXIPKPCEPLQVWCC3ZJK32YX3SZDR4X4VHIB"
            "D\",\"last_modified_ledger\":4927095,\"last_modified_time\":\"2026-09-29T05:17:42Z\",\"claimants\":[{\"destination\":\"G"
            "CQFHLKQCPU5PJKWI3U4HEAW4NGKQNRJ3UVCIWBCIBOM5G4JF4EOTSA5\",\"predicate\":{\"abs_before\":\"+292277026596-12-04T15:3"
            "0:07Z\",\"abs_before_epoch\":\"9223372036854775807\"}},{\"destination\":\"GAPJX3CDN6ZD5IC5W35IZQB5VJDXJ2X3RYHO7ZH7OQ"
            "BIILVWYS6ZNPQW\",\"predicate\":{\"abs_before\":\"+292277026596-12-04T15:30:07Z\",\"abs_before_epoch\":\"92233720368547"
            "75807\"}},{\"destination\":\"GD4E7KUTIZ4Z76GE2DBJP23CXYYEUSX67FWFTKAZWSCZTFHO2VZ4QB6B\",\"predicate\":{\"abs_before\""
            ":\"+10000-01-01T00:00:00Z\",\"abs_before_epoch\":\"253402300800\"}},{\"destination\":\"GA64UBAZ7T6V7UZP7SCIVBFFRLVQBU"
            "DXC2JFN5WMSBL7NFFQPFGWFFER\",\"predicate\":{\"abs_before\":\"9999-12-31T23:59:59Z\",\"abs_before_epoch\":\"25340230079"
            "9\"}},{\"destination\":\"GCO6VSU7AGJBQS2DBPKVRBR5BTESVATKGUCFYGPRMRSGY3PUL22ICW2N\",\"predicate\":{\"not\":{\"abs_befo"
            "re\":\"+146138514283-06-19T07:45:04Z\",\"abs_before_epoch\":\"4611686018427387904\"}}}],\"flags\":{\"clawback_enabled\""
            ":false},\"paging_token\":\"4927095-00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff775268480ec3088e84\"}]}"
            ",\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances?cursor=&limit=10&order=asc"
            "&sponsor=GDNZVVY2LIMUQ3BAFU7CNXIPKPCEPLQVWCC3ZJK32YX3SZDR4X4VHIBD\"},\"next\":{\"href\":\"https://horizon-testnet."
            "stellar.org/claimable_balances?cursor=4927095-00000000f9af0b5353a90f1b88fec09134beea1e468a53519d1aff77526848"
            "0ec3088e84&limit=10&order=asc&sponsor=GDNZVVY2LIMUQ3BAFU7CNXIPKPCEPLQVWCC3ZJK32YX3SZDR4X4VHIBD\"},\"prev\":{\"hr"
            "ef\":\"https://horizon-testnet.stellar.org/claimable_balances?cursor=4927095-00000000a587ccfcb3e08c87da746a8f5"
            "f7816ca8997edb694098feb31c2190c22b32ba5&limit=10&order=desc&sponsor=GDNZVVY2LIMUQ3BAFU7CNXIPKPCEPLQVWCC3ZJK3"
            "2YX3SZDR4X4VHIBD\"}}}");
    }

    /** The claimants of the two create_claimable_balance operations that
     *  made them, as Horizon shows the operations: with the rel_before
     *  that Core turned into an abs_before in the balances. */
    static QByteArray realTestnetOperationClaimants()
    {
        return QByteArray(
            "[{\"destination\":\"GCQFHLKQCPU5PJKWI3U4HEAW4NGKQNRJ3UVCIWBCIBOM5G4JF4EOTSA5\",\"predicate\":{\"abs_before\":\"+29227"
            "7026596-12-04T15:30:07Z\",\"abs_before_epoch\":\"9223372036854775807\"}},{\"destination\":\"GAPJX3CDN6ZD5IC5W35IZQB5"
            "VJDXJ2X3RYHO7ZH7OQBIILVWYS6ZNPQW\",\"predicate\":{\"rel_before\":\"9223372036854775807\"}},{\"destination\":\"GD4E7KUT"
            "IZ4Z76GE2DBJP23CXYYEUSX67FWFTKAZWSCZTFHO2VZ4QB6B\",\"predicate\":{\"abs_before\":\"+10000-01-01T00:00:00Z\",\"abs_be"
            "fore_epoch\":\"253402300800\"}},{\"destination\":\"GA64UBAZ7T6V7UZP7SCIVBFFRLVQBUDXC2JFN5WMSBL7NFFQPFGWFFER\",\"pred"
            "icate\":{\"abs_before\":\"9999-12-31T23:59:59Z\",\"abs_before_epoch\":\"253402300799\"}},{\"destination\":\"GCO6VSU7AGJB"
            "QS2DBPKVRBR5BTESVATKGUCFYGPRMRSGY3PUL22ICW2N\",\"predicate\":{\"not\":{\"abs_before\":\"+146138514283-06-19T07:45:04"
            "Z\",\"abs_before_epoch\":\"4611686018427387904\"}}},{\"destination\":\"GCQFHLKQCPU5PJKWI3U4HEAW4NGKQNRJ3UVCIWBCIBOM5"
            "G4JF4EOTSA5\",\"predicate\":{\"and\":[{\"or\":[{\"not\":{\"abs_before\":\"1970-01-01T00:00:00Z\",\"abs_before_epoch\":\"0\"}}"
            ",{\"rel_before\":\"0\"}]},{\"unconditional\":true}]}},{\"destination\":\"GAPJX3CDN6ZD5IC5W35IZQB5VJDXJ2X3RYHO7ZH7OQBI"
            "ILVWYS6ZNPQW\",\"predicate\":{\"or\":[{\"and\":[{\"not\":{\"rel_before\":\"3600\"}},{\"abs_before\":\"2026-09-21T14:13:20Z\","
            "\"abs_before_epoch\":\"1790000000\"}]},{\"not\":{\"unconditional\":true}}]}},{\"destination\":\"GD4E7KUTIZ4Z76GE2DBJP23"
            "CXYYEUSX67FWFTKAZWSCZTFHO2VZ4QB6B\",\"predicate\":{\"rel_before\":\"1\"}},{\"destination\":\"GA64UBAZ7T6V7UZP7SCIVBFFR"
            "LVQBUDXC2JFN5WMSBL7NFFQPFGWFFER\",\"predicate\":{\"not\":{\"not\":{\"not\":{\"rel_before\":\"86400\"}}}}}]");
    }

    /** Two real pubnet claimable balances (links dropped from the records). */
    static QByteArray realPubnetClaimableBalances()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"id\":\"0000000051f69c0b34a7403185392cb9c283279f58d634069ed776c5070260435191bd52\",\"a"
            "sset\":\"OVRL:GBZH36ATUXJZKFRMQTAAW42MWNM34SOA4N6E7DQ62V3G5NVITC3QOVRL\",\"amount\":\"0.0010000\",\"sponsor\":\"GCKIK5"
            "UOKKGXCDCWGY3BAKJ2T5H6BXOXHI4SMMEUSUE6NWLJP3F2KQUU\",\"last_modified_ledger\":64674588,\"last_modified_time\":\"20"
            "26-09-29T05:12:01Z\",\"claimants\":[{\"destination\":\"GDKNDOQL23432Z7HMTFELXPFXAAUDWDJZXOTR73TSWY4XUGN2YMQHVM3\",\""
            "predicate\":{\"abs_before\":\"2026-09-29T13:12:01Z\",\"abs_before_epoch\":\"1790687521\"}},{\"destination\":\"GCKIK5UOKK"
            "GXCDCWGY3BAKJ2T5H6BXOXHI4SMMEUSUE6NWLJP3F2KQUU\",\"predicate\":{\"not\":{\"abs_before\":\"2026-09-29T13:12:01Z\",\"abs"
            "_before_epoch\":\"1790687521\"}}}],\"flags\":{\"clawback_enabled\":false},\"paging_token\":\"64674588-0000000051f69c0b"
            "34a7403185392cb9c283279f58d634069ed776c5070260435191bd52\"},{\"id\":\"000000006c9e6e21e8cad720b1ef573f88f4ea0a85"
            "9115c9d6bf6eb14fcbb21843cb6b65\",\"asset\":\"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\"amo"
            "unt\":\"1.1000000\",\"sponsor\":\"GCC5IXH2SBLKBFMQZBUKKOU4ZOZRYDBHT2FOSTQM6WOJZVJ4JSXIBS42\",\"last_modified_ledger\""
            ":64674486,\"last_modified_time\":\"2026-09-29T05:03:31Z\",\"claimants\":[{\"destination\":\"GBUJRK66RRSX6WXMO2XNEXXBD"
            "IFLFRMNWH34XB32GH6ORGOZP37QZ6AP\",\"predicate\":{\"unconditional\":true}}],\"flags\":{\"clawback_enabled\":false},\"pa"
            "ging_token\":\"64674486-000000006c9e6e21e8cad720b1ef573f88f4ea0a859115c9d6bf6eb14fcbb21843cb6b65\"}]},\"_links\":"
            "{\"self\":{\"href\":\"https://horizon.stellar.org/claimable_balances?cursor=&limit=200&order=desc\"},\"next\":{\"href"
            "\":\"https://horizon.stellar.org/claimable_balances?cursor=64674396-00000000e460beb89c780ebdf05ff011dcab5fccdc"
            "4c2127519429a6a86b8cd78862de09&limit=200&order=desc\"},\"prev\":{\"href\":\"https://horizon.stellar.org/claimable_"
            "balances?cursor=64674588-0000000051f69c0b34a7403185392cb9c283279f58d634069ed776c5070260435191bd52&limit=200&"
            "order=asc\"}}}");
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

    /** A revoke sponsorship of a type with no arm decoded as no operation at
     *  all: fromXdr returned nullptr (and set the source account on it when
     *  there was one). */
    void testAnUnknownRevokeSponsorshipTypeIsRefused()
    {
        stellar::Operation xdr;
        xdr.sourceAccount.filled = 0;
        xdr.fillRevokeSponsorshipOp().type = stellar::RevokeSponsorshipType(5);   // its ACCOUNT key is trivial
        Operation* op = nullptr;
        bool threw = false;
        try { op = Operation::fromXdr(xdr); } catch (const std::runtime_error&) { threw = true; }
        delete op;
        QVERIFY(threw);
    }

    /** Operation::toXdrBase64 turned any failure into an empty string, and a
     *  write that failed half way (here at a payment asset that names
     *  POOL_SHARE, no arm of a plain Asset) into the base64 of the half. */
    void testOperationToXdrBase64RefusesWhatItCannotEncode()
    {
        stellar::PaymentOp op;
        op.destination.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
        std::memset(op.destination.ed25519, 0x11, sizeof op.destination.ed25519);
        op.asset.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        op.amount = 1;
        QScopedPointer<PaymentOperation> payment(PaymentOperation::build(AccountConverter(), op));
        bool threw = false;
        try { payment->toXdrBase64(); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        op.asset.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        QScopedPointer<PaymentOperation> native(PaymentOperation::build(AccountConverter(), op));
        QVERIFY(!native->toXdrBase64().isEmpty());
    }

    /**
     * Predicate::create(QVariant) read abs_before with QDateTime, which reads
     * no year past 9999: Horizon's "+292277026596-12-04T15:30:07Z" (the last
     * second an int64 holds, what a rel_before of 2^63-1 becomes) and
     * "+10000-01-01T00:00:00Z" came out as an invalid time, and so as another
     * time. Real Horizon JSON, testnet and pubnet, reads exactly, the same as
     * the responses read it, and signs what it read.
     */
    void testPredicatesFromRealHorizonJsonAreExact()
    {
        const QString max = maxSeconds();
        const QStringList nested = QStringList()
            << QStringLiteral("and(or(not(abs:0),abs:1790659062),unconditional)")
            << QStringLiteral("or(and(not(abs:1790662662),abs:1790000000),not(unconditional))")
            << QStringLiteral("abs:1790659063")
            << QStringLiteral("not(not(not(abs:1790745462)))");
        const QStringList far = QStringList()
            << QStringLiteral("abs:") + max << QStringLiteral("abs:") + max
            << QStringLiteral("abs:253402300800") << QStringLiteral("abs:253402300799")
            << QStringLiteral("not(abs:4611686018427387904)");
        QCOMPARE(shapesOf(claimantsOf(realTestnetClaimableBalances(), 0)), nested);
        QCOMPARE(shapesOf(claimantsOf(realTestnetClaimableBalances(), 1)), far);

        // The operations that made them, with their rel_before.
        QCOMPARE(shapesOf(QJsonDocument::fromJson(realTestnetOperationClaimants()).array()),
                 QStringList() << QStringLiteral("abs:") + max << QStringLiteral("rel:") + max
                               << QStringLiteral("abs:253402300800") << QStringLiteral("abs:253402300799")
                               << QStringLiteral("not(abs:4611686018427387904)")
                               << QStringLiteral("and(or(not(abs:0),rel:0),unconditional)")
                               << QStringLiteral("or(and(not(rel:3600),abs:1790000000),not(unconditional))")
                               << QStringLiteral("rel:1") << QStringLiteral("not(not(not(rel:86400)))"));

        QCOMPARE(shapesOf(claimantsOf(realPubnetClaimableBalances(), 0)),
                 QStringList() << QStringLiteral("abs:1790687521") << QStringLiteral("not(abs:1790687521)"));
        QCOMPARE(shapesOf(claimantsOf(realPubnetClaimableBalances(), 1)),
                 QStringList() << QStringLiteral("unconditional"));

        // The responses read the same predicates.
        Page<ClaimableBalanceResponse> page;
        page.loadFromJson(realTestnetClaimableBalances());
        QCOMPARE(page.size(), 2);
        for (int r = 0; r < page.size(); ++r) {
            QStringList read;
            for (const Claimant& claimant : page.get(r).getClaimants())
                read << shape(claimant.getPredicate());
            QCOMPARE(read, r == 0 ? nested : far);
        }

        // And what is read is what gets signed.
        for (const QJsonValue& claimant : claimantsOf(realTestnetClaimableBalances(), 1)) {
            QScopedPointer<Predicate> read(Predicate::create(claimant.toObject().value(QStringLiteral("predicate")).toVariant()));
            stellar::ClaimPredicate xdr = read->toXdr();
            QScopedPointer<Predicate> back(Predicate::fromXdr(xdr));
            QCOMPARE(shape(*back), shape(*read));
        }
    }

    /**
     * Predicate::create(QVariant) guessed where it could not read: a time
     * that is none became an invalid time (another time), one with no zone
     * the computer's local time, a rel_before that is no number 0
     * ("claimable within 0 seconds"), {"unconditional": false} was
     * unconditional (and so was a time with "unconditional" beside it), an
     * and/or of any length made a node Core refuses, abs_before_epoch was
     * never looked at, and nothing bounded the nesting. It reads Horizon's
     * form exactly now, and both doors refuse the rest naming the member.
     */
    void testPredicateCreateRefusesWhatItCannotReadExactly()
    {
        struct Case { const char* json; const char* field; };
        const Case cases[] = {
            { "{\"abs_before\": \"yesterday\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2020-09-28T17:57:04\"}", "p.abs_before: " },          // was local time
            { "{\"abs_before\": \"2020-09-28T17:57:04+02:00\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2020-09-28T17:57:04.000Z\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2020-09-28 17:57:04Z\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2021-02-29T00:00:00Z\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2020-09-28T24:00:00Z\"}", "p.abs_before: " },
            { "{\"abs_before\": \"10000-01-01T00:00:00Z\"}", "p.abs_before: " },        // Horizon writes a '+'
            { "{\"abs_before\": \"+9999-12-31T23:59:59Z\"}", "p.abs_before: " },        // and only past 9999
            { "{\"abs_before\": \"+010000-01-01T00:00:00Z\"}", "p.abs_before: " },
            { "{\"abs_before\": \"1969-12-31T23:59:59Z\"}", "p.abs_before: " },         // Core: negative
            { "{\"abs_before\": \"+292277026596-12-04T15:30:08Z\"}", "p.abs_before: " }, // past 2^63-1
            { "{\"abs_before\": \"+1000000000000-01-01T00:00:00Z\"}", "p.abs_before: " },
            { "{\"abs_before\": 1790687521}", "p.abs_before: " },
            { "{\"abs_before\": \"2026-09-29T13:12:01Z\", \"abs_before_epoch\": \"1790687522\"}", "p.abs_before: " },
            { "{\"abs_before\": \"+10000-01-01T00:00:00Z\", \"abs_before_epoch\": \"253402300801\"}", "p.abs_before: " },
            { "{\"abs_before\": \"2026-09-29T13:12:01Z\", \"abs_before_epoch\": \"1790687521.0\"}", "p.abs_before_epoch: " },
            { "{\"abs_before\": \"2026-09-29T13:12:01Z\", \"abs_before_epoch\": \"-1\"}", "p.abs_before_epoch: " },
            { "{\"abs_before_epoch\": \"1790687521\"}", "p.abs_before: " },
            { "{\"rel_before\": \"12s\"}", "p.rel_before: " },                          // was 0
            { "{\"rel_before\": \"\"}", "p.rel_before: " },                             // was 0
            { "{\"rel_before\": 1.5}", "p.rel_before: " },                            // was 1
            { "{\"rel_before\": \"-1\"}", "p.rel_before: " },                           // Core: negative
            { "{\"rel_before\": \"9223372036854775808\"}", "p.rel_before: " },          // was 0
            { "{\"unconditional\": false}", "p.unconditional: " },                    // was unconditional
            { "{\"unconditional\": \"true\"}", "p.unconditional: " },
            { "{\"unconditional\": true, \"abs_before\": \"2026-09-29T13:12:01Z\"}", "p.abs_before: " },
            { "{\"not\": {\"unconditional\": true}, \"rel_before\": \"60\"}", "p.rel_before: " },
            { "{\"and\": [{\"unconditional\": true}]}", "p.and: " },
            { "{\"and\": [{\"unconditional\": true}, {\"unconditional\": true}, {\"unconditional\": true}]}", "p.and: " },
            { "{\"or\": []}", "p.or: " },
            { "{\"or\": {\"unconditional\": true}}", "p.or: " },
            { "{\"and\": [{\"unconditional\": true}, {\"sometimes\": true}]}", "p.and[1]: " },
            { "{\"not\": [{\"unconditional\": true}]}", "p.not: " },
            { "{\"not\": null}", "p.not: " },
            { "{\"sometimes\": true}", "p: " },
            { "{}", "p: " },
            // Deeper than the 4 levels Core allows (validatePredicate): was
            // read and signed, and Core refused it.
            { "{\"not\": {\"not\": {\"not\": {\"not\": {\"unconditional\": true}}}}}", "p.not.not.not.not: " },
            { "{\"and\": [{\"or\": [{\"not\": {\"not\": {\"unconditional\": true}}}, {\"unconditional\": true}]}, {\"unconditional\": true}]}",
              "p.and[0].or[0].not.not: " },
        };
        for (const Case& c : cases) {
            const QString why = refusalOf(json(c.json));
            QVERIFY2(why.startsWith(QLatin1String(c.field)),
                     qPrintable(QString::fromLatin1(c.json) + QStringLiteral(" -> ") + why));
        }
        QVERIFY(refusalOf(QJsonValue(QStringLiteral("unconditional"))).startsWith(QLatin1String("p: ")));
        QVERIFY(refusalOf(QJsonValue()).startsWith(QLatin1String("p: ")));

        // What Horizon writes still reads, at the edges.
        const QString max = maxSeconds();
        QCOMPARE(shapeOf(json("{\"not\": {\"not\": {\"not\": {\"unconditional\": true}}}}")),
                 QStringLiteral("not(not(not(unconditional)))"));
        QCOMPARE(shapeOf(json("{\"abs_before\": \"1970-01-01T00:00:00Z\"}")), QStringLiteral("abs:0"));
        QCOMPARE(shapeOf(json("{\"abs_before\": \"2024-02-29T00:00:00Z\"}")), QStringLiteral("abs:1709164800"));
        QCOMPARE(shapeOf(json("{\"abs_before\": \"+292277026596-12-04T15:30:07Z\"}")), QStringLiteral("abs:") + max);
        QCOMPARE(shapeOf(json("{\"abs_before\": \"+10000-01-01T00:00:00Z\", \"abs_before_epoch\": 253402300800}")),
                 QStringLiteral("abs:253402300800"));
        QCOMPARE(shapeOf(json("{\"rel_before\": \"0\"}")), QStringLiteral("rel:0"));
        QCOMPARE(shapeOf(json("{\"rel_before\": \"9223372036854775807\"}")), QStringLiteral("rel:") + max);
        // Given from QML, a whole number is a number.
        QScopedPointer<Predicate> fromQml(Predicate::create(QVariantMap{ { QStringLiteral("rel_before"), 3600 } }));
        QCOMPARE(shape(*fromQml), QStringLiteral("rel:3600"));
    }

    /**
     * The claimant list converter read each item with toMap() and
     * toString(): an item that was no object, a destination that was no
     * account id (or a muxed or contract one, which no claimant can be) and
     * any other key went through, and the predicate was read leniently.
     */
    void testClaimantConverterIsExact()
    {
        const QJsonArray real = QJsonDocument::fromJson(realTestnetOperationClaimants()).array();
        const QList<Claimant> claimants = ClaimantConverter::convertData(real.toVariantList());
        QCOMPARE(claimants.size(), 9);
        QCOMPARE(claimants.at(0).getDestination(), QString("GCQFHLKQCPU5PJKWI3U4HEAW4NGKQNRJ3UVCIWBCIBOM5G4JF4EOTSA5"));
        QCOMPARE(shape(claimants.at(0).getPredicate()), QStringLiteral("abs:") + maxSeconds()); // was another time
        QCOMPARE(shape(claimants.at(1).getPredicate()), QStringLiteral("rel:") + maxSeconds());
        QCOMPARE(ClaimantConverter::fromJson(claimantsOf(realPubnetClaimableBalances(), 0)).size(), 2);

        struct Case { const char* json; const char* field; };
        const Case cases[] = {
            { "[{\"destination\": \"GNOTANACCOUNT\", \"predicate\": {\"unconditional\": true}}]", "claimants[0].destination: " },
            { "[{\"destination\": \"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK\", \"predicate\": {\"unconditional\": true}}]",
              "claimants[0].destination: " },
            { "[{\"destination\": \"CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC\", \"predicate\": {\"unconditional\": true}}]",
              "claimants[0].destination: " },
            { "[{\"destination\": 5, \"predicate\": {\"unconditional\": true}}]", "claimants[0].destination: " },
            { "[{\"predicate\": {\"unconditional\": true}}]", "claimants[0].destination: " },
            { "[{\"destination\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"}]", "claimants[0].predicate: " },
            { "[{\"destination\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\", \"predicate\": {\"unconditional\": true}, \"flags\": 1}]",
              "claimants[0].flags: " },
            { "[{\"destination\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\", \"predicate\": {\"rel_before\": \"12s\"}}]",
              "claimants[0].predicate.rel_before: " },
            { "[{\"destination\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\", \"predicate\": {\"unconditional\": true}}, "
              "\"GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7\"]", "claimants[1]: " },
        };
        for (const Case& c : cases) {
            const QString why = claimantsRefusalOf(c.json);
            QVERIFY2(why.startsWith(QLatin1String(c.field)),
                     qPrintable(QString::fromLatin1(c.json) + QStringLiteral(" -> ") + why));
        }
        bool threw = false;
        try { ClaimantConverter::fromJson(QJsonObject(), QStringLiteral("claimants")); }
        catch (const std::runtime_error& e) { threw = QString::fromUtf8(e.what()).startsWith(QLatin1String("claimants: ")); }
        QVERIFY(threw);
    }

    /**
     * And/Or equals() asked only that each operand be among the other
     * node's: or(a, a) equalled or(a, b), so Claimant == took two different
     * claimants for one and setPredicate kept the old predicate. Then it
     * matched the operands in any order, so and(b, a) equalled and(a, b),
     * although the XDR array is ordered and the two encode (and sign)
     * differently; py-stellar-sdk compares left with left and right with
     * right. hashCode() hashed the operands' addresses, so equal nodes hashed
     * apart, and then summed their hashes, so a node and its swap always
     * hashed alike. A default Not (no operand) crashed equals() and
     * hashCode(), and toXdr() sent an and/or of fewer than two operands,
     * which Core refuses.
     */
    void testPredicateEqualityIsExact()
    {
        Predicate::Or twice(QList<Predicate*>() << new Predicate::Unconditional() << new Predicate::Unconditional());
        Predicate::Or other(QList<Predicate*>() << new Predicate::Unconditional() << new Predicate::AbsBefore(60));
        Predicate::Or same(QList<Predicate*>() << new Predicate::Unconditional() << new Predicate::AbsBefore(60));
        Predicate::Or swapped(QList<Predicate*>() << new Predicate::AbsBefore(60) << new Predicate::Unconditional());
        QVERIFY(!twice.equals(&other));                   // was equal
        QVERIFY(!other.equals(&twice));
        QVERIFY(other.equals(&same));
        QVERIFY(same.equals(&other));
        QCOMPARE(other.hashCode(), same.hashCode());      // hashed apart
        QVERIFY(!other.equals(&swapped));                 // was equal
        QVERIFY(!swapped.equals(&other));
        // Not a promise for every pair of operands, but true of these: the
        // sum hashed every node and its swap alike.
        QVERIFY(other.hashCode() != swapped.hashCode());
        Predicate::And andTwice(QList<Predicate*>() << new Predicate::RelBefore(5) << new Predicate::RelBefore(5));
        Predicate::And andOther(QList<Predicate*>() << new Predicate::RelBefore(5) << new Predicate::RelBefore(6));
        Predicate::And andSame(QList<Predicate*>() << new Predicate::RelBefore(5) << new Predicate::RelBefore(6));
        QVERIFY(!andTwice.equals(&andOther));             // was equal
        QVERIFY(andOther.equals(&andSame));
        QCOMPARE(andOther.hashCode(), andSame.hashCode());
        Predicate::And andSwapped(QList<Predicate*>() << new Predicate::RelBefore(6) << new Predicate::RelBefore(5));
        QVERIFY(!andOther.equals(&andSwapped));           // was equal
        QVERIFY(!andSwapped.equals(&andOther));
        QVERIFY(andOther.hashCode() != andSwapped.hashCode());

        // The order counts at every depth, not only at the top.
        Predicate::Not deep(new Predicate::Or(QList<Predicate*>() << new Predicate::Unconditional()
                                                                  << new Predicate::Not(new Predicate::And(andOther))));
        Predicate::Not deepSame(new Predicate::Or(QList<Predicate*>() << new Predicate::Unconditional()
                                                                      << new Predicate::Not(new Predicate::And(andSame))));
        Predicate::Not deepSwapped(new Predicate::Or(QList<Predicate*>() << new Predicate::Unconditional()
                                                                         << new Predicate::Not(new Predicate::And(andSwapped))));
        QVERIFY(deep.equals(&deepSame));
        QCOMPARE(deep.hashCode(), deepSame.hashCode());
        QVERIFY(!deep.equals(&deepSwapped));              // was equal
        QVERIFY(!deepSwapped.equals(&deep));

        // A missing operand equals only a missing one, in the same place.
        Predicate::And holeFirst(QList<Predicate*>() << nullptr << new Predicate::RelBefore(5));
        Predicate::And holeFirstToo(QList<Predicate*>() << nullptr << new Predicate::RelBefore(5));
        Predicate::And holeLast(QList<Predicate*>() << new Predicate::RelBefore(5) << nullptr);
        QVERIFY(holeFirst.equals(&holeFirstToo));
        QCOMPARE(holeFirst.hashCode(), holeFirstToo.hashCode());
        QVERIFY(!holeFirst.equals(&holeLast));            // was equal
        QVERIFY(!holeLast.equals(&holeFirst));
        QVERIFY(!holeFirst.equals(&andTwice));
        QVERIFY(!andTwice.equals(&holeFirst));

        Claimant claimant(QString(DEST()), new Predicate::Or(twice));
        const Claimant changed(QString(DEST()), new Predicate::Or(other));
        QVERIFY(!(claimant == changed));                  // was equal
        QVERIFY(claimant != changed);
        claimant.setPredicate(other);                     // was kept or(unconditional, unconditional)
        QCOMPARE(shape(claimant.getPredicate()), QStringLiteral("or(unconditional,abs:60)"));
        QVERIFY(claimant == Claimant(QString(DEST()), new Predicate::Or(same)));
        const Claimant reordered(QString(DEST()), new Predicate::Or(swapped));
        QVERIFY(!(claimant == reordered));                // was equal
        QVERIFY(claimant != reordered);

        Predicate::Not empty, alsoEmpty;
        Predicate::Not notUnconditional(new Predicate::Unconditional());
        QVERIFY(empty.equals(&alsoEmpty));                // crashed
        QVERIFY(!empty.equals(&notUnconditional));
        QVERIFY(!notUnconditional.equals(&empty));
        QCOMPARE(empty.hashCode(), alsoEmpty.hashCode());
        QVERIFY_EXCEPTION_THROWN(empty.toXdr(), std::runtime_error);

        Predicate::Or one(QList<Predicate*>() << new Predicate::Unconditional());
        QVERIFY_EXCEPTION_THROWN(one.toXdr(), std::runtime_error);  // went out
        Predicate::And none;
        QVERIFY_EXCEPTION_THROWN(none.toXdr(), std::runtime_error);  // went out
    }

    /**
     * AllowTrustOperationResponse never freed the key pairs and the asset
     * its getters made, and SetOptionsOperationResponse never freed its
     * signer's key pair (and kept a freed one when the member was set again).
     */
    void testResponseGetterCachesAreFreed()
    {
#if defined(_MSC_VER) && defined(_DEBUG)
#  pragma warning(push)
#  pragma warning(disable: 4996)
        _CrtMemState before, after, difference;
        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 50; ++i) {
            AllowTrustOperationResponse response;
            response.setTrustor(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
            response.setTrustee(QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
            response.setAssetType(QStringLiteral("credit_alphanum4"));
            response.setAssetCode(QStringLiteral("USD"));
            response.setAssetIssuer(QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
            response.getTrustor();
            response.getTrustee();
            response.getAsset();
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        const qint64 left = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(left < 50, qPrintable(QStringLiteral("%1 blocks left on the heap").arg(left)));

        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 50; ++i) {
            SetOptionsOperationResponse response;
            response.setSignerKey(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
            response.getSigner();
            response.setSignerKey(QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
            response.getSigner();
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        const qint64 leftSigner = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(leftSigner < 50, qPrintable(QStringLiteral("%1 blocks left on the heap").arg(leftSigner)));
#  pragma warning(pop)
#else
        QSKIP("counts blocks on the MSVC debug heap");
#endif
    }

    /**
     * A Claimant made with the default constructor has no predicate, and
     * CreateClaimableBalanceOperation (and getPredicate()) dereferenced null:
     * a crash. js and py take no predicate as unconditional; so does this, and
     * the claimant's XDR is js-stellar-sdk 17.1.0's for new Claimant(G...).
     */
    void testAClaimantWithNoPredicateIsUnconditional()
    {
        Claimant claimant;
        claimant.setDestination(QString(DEST()));
        QCOMPARE(claimant.getPredicate().toXdr().type, stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL);
        QScopedPointer<CreateClaimableBalanceOperation> op(CreateClaimableBalanceOperation::create(
            QStringLiteral("1"), new AssetTypeNative(), QList<Claimant>{claimant}));
        QScopedPointer<CreateClaimableBalanceOperation> explicitOne(CreateClaimableBalanceOperation::create(
            QStringLiteral("1"), new AssetTypeNative(),
            QList<Claimant>{Claimant(QString(DEST()), new Predicate::Unconditional())}));
        QCOMPARE(op->toXdrBase64(), explicitOne->toXdrBase64());
    }

    /** setPredicate replaced the predicate without freeing the old one. */
    void testClaimantSetPredicateFreesTheOldOne()
    {
#if defined(_MSC_VER) && defined(_DEBUG)
        Predicate::AbsBefore a(60), b(120);
        Claimant claimant(QString(DEST()), new Predicate::AbsBefore(1));
        _CrtMemState before, after, difference;
        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 200; ++i)
            claimant.setPredicate(i % 2 ? a : b);         // 200 copies; each one leaked
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        // A size_t difference: fewer blocks wrap around, back to negative here.
        const qint64 blocks = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(blocks < 100, qPrintable(QStringLiteral("%1 more heap blocks").arg(blocks)));
        QCOMPARE(shape(claimant.getPredicate()), QStringLiteral("abs:60"));
#else
        QSKIP("counts heap blocks with the MSVC debug heap");
#endif
    }

    /**
     * Claimant::setPredicate keeps what it holds when the new predicate
     * equals it, and And/Or equality ignored the operands' order: on a
     * claimant holding and(a, b), setPredicate(and(b, a)) kept and(a, b), so
     * toXdr() put a first and the operation signed was not the one the caller
     * set (other bytes, another transaction hash).
     */
    void testClaimantSetPredicateKeepsOperandOrder()
    {
        const qint64 a = 1700000000, b = 60;          // abs_before a, rel_before b
        Predicate::And ab(QList<Predicate*>() << new Predicate::AbsBefore(a) << new Predicate::RelBefore(b));
        Predicate::And ba(QList<Predicate*>() << new Predicate::RelBefore(b) << new Predicate::AbsBefore(a));
        Claimant claimant(QString(DEST()), new Predicate::And(ab));
        const QByteArray abBytes = claimantXdr(claimant);
        const QByteArray baBytes = claimantXdr(Claimant(QString(DEST()), new Predicate::And(ba)));
        QVERIFY(abBytes != baBytes);

        claimant.setPredicate(ba);                     // kept and(a, b)
        QCOMPARE(shape(claimant.getPredicate()), QStringLiteral("and(rel:60,abs:1700000000)"));
        const stellar::Claimant xdr = claimant.toXdr();
        QVERIFY(xdr.v0.predicate.type == stellar::ClaimPredicateType::CLAIM_PREDICATE_AND);
        QCOMPARE(int(xdr.v0.predicate.andPredicates.value.size()), 2);
        QVERIFY(xdr.v0.predicate.andPredicates.value.at(0).type
                == stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME);
        QCOMPARE(xdr.v0.predicate.andPredicates.value.at(0).relBefore, b);
        QVERIFY(xdr.v0.predicate.andPredicates.value.at(1).type
                == stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME);
        QCOMPARE(xdr.v0.predicate.andPredicates.value.at(1).absBefore, a);
        QCOMPARE(claimantXdr(claimant), baBytes);      // were abBytes
        // The Claimant XDR py-stellar-sdk 14.1.0 and js-stellar-sdk 17.1.0
        // write for DEST() and and(rel_before 60, abs_before 1700000000), and
        // for and(abs_before 1700000000, rel_before 60).
        QCOMPARE(claimantXdr(claimant).toBase64(),
                 QByteArray("AAAAAAAAAAA9Tt/Wc9qlDVCiwx+gZxhBBTrPPggv2lc+QlJgo93rkAAAAAEAAAACAAAABQAAAAAAAAA8"
                            "AAAABAAAAABlU/EA"));
        QCOMPARE(abBytes.toBase64(),
                 QByteArray("AAAAAAAAAAA9Tt/Wc9qlDVCiwx+gZxhBBTrPPggv2lc+QlJgo93rkAAAAAEAAAACAAAABAAAAABlU/EA"
                            "AAAABQAAAAAAAAA8"));

        claimant.setPredicate(ab);                     // and back, a first again
        QCOMPARE(claimantXdr(claimant), abBytes);

        // The same for or(a, b), one level down: not(or(a, b)) -> not(or(b, a)).
        Predicate::Not notOrAb(new Predicate::Or(QList<Predicate*>() << new Predicate::AbsBefore(a)
                                                                     << new Predicate::RelBefore(b)));
        Predicate::Not notOrBa(new Predicate::Or(QList<Predicate*>() << new Predicate::RelBefore(b)
                                                                     << new Predicate::AbsBefore(a)));
        claimant.setPredicate(notOrAb);
        QCOMPARE(shape(claimant.getPredicate()), QStringLiteral("not(or(abs:1700000000,rel:60))"));
        claimant.setPredicate(notOrBa);                // kept not(or(a, b))
        QCOMPARE(shape(claimant.getPredicate()), QStringLiteral("not(or(rel:60,abs:1700000000))"));
        QCOMPARE(claimantXdr(claimant), claimantXdr(Claimant(QString(DEST()), new Predicate::Not(notOrBa))));
        QCOMPARE(claimantXdr(claimant).toBase64(),       // the same SDKs' bytes for not(or(b, a))
                 QByteArray("AAAAAAAAAAA9Tt/Wc9qlDVCiwx+gZxhBBTrPPggv2lc+QlJgo93rkAAAAAMAAAABAAAAAgAAAAIAAAAF"
                            "AAAAAAAAADwAAAAEAAAAAGVT8QA="));
    }

    /**
     * Or and And copied only the operands that were there: a copy of
     * and(null, b) was and(b), its operand moved to the front, so it no
     * longer equalled what it copied, and a Claimant (which copies its
     * predicate) no longer equalled its own copy.
     */
    void testPredicateCopiesKeepEachOperandInPlace()
    {
        Predicate::And holeFirst(QList<Predicate*>() << nullptr << new Predicate::RelBefore(5));
        Predicate::Or holeLast(QList<Predicate*>() << new Predicate::AbsBefore(6) << nullptr);

        Predicate::And andCopy(holeFirst);             // was and(rel:5)
        QCOMPARE(int(andCopy.getInner().size()), 2);
        QVERIFY(andCopy.getInner().at(0) == nullptr);
        QVERIFY(andCopy.equals(&holeFirst));
        QVERIFY(holeFirst.equals(&andCopy));
        QCOMPARE(andCopy.hashCode(), holeFirst.hashCode());
        Predicate::Or orCopy(holeLast);                // was or(abs:6)
        QVERIFY(orCopy.equals(&holeLast));
        QVERIFY(orCopy.getInner().at(1) == nullptr);

        Predicate::And andAssigned(QList<Predicate*>() << new Predicate::Unconditional()
                                                       << new Predicate::Unconditional());
        andAssigned = holeFirst;                       // was and(rel:5)
        QVERIFY(andAssigned.equals(&holeFirst));
        Predicate::Or orAssigned;
        orAssigned = holeLast;                         // was or(abs:6)
        QVERIFY(orAssigned.equals(&holeLast));

        // A copy with a hole is refused on the wire, as the original is.
        QVERIFY_EXCEPTION_THROWN(holeFirst.toXdr(), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(andCopy.toXdr(), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(orCopy.toXdr(), std::runtime_error);

        const Claimant claimant(QString(DEST()),
                                new Predicate::And(QList<Predicate*>() << nullptr << new Predicate::RelBefore(5)));
        const Claimant copy(claimant);
        QVERIFY(claimant == copy);                     // was not equal
        QVERIFY(!(claimant != copy));
        QList<Claimant> list;
        list << claimant;
        QVERIFY(list.at(0) == claimant);               // was not equal
    }

    /**
     * And/Or assignment deleted the operands the node held before copying
     * the new ones: when a copy threw (a predicate type Predicate::create
     * does not know), the node was left half assigned, holding neither its
     * own operands nor the other's.
     */
    void testPredicateAssignmentThatThrowsKeepsTheNode()
    {
        Predicate::And andForeign(QList<Predicate*>() << new Predicate::Unconditional() << new ForeignPredicate());
        Predicate::And andHeld(QList<Predicate*>() << new Predicate::RelBefore(5) << new Predicate::AbsBefore(6));
        QVERIFY_EXCEPTION_THROWN(andHeld = andForeign, std::runtime_error);
        QCOMPARE(shape(andHeld), QStringLiteral("and(rel:5,abs:6)"));   // was and(unconditional)
        Predicate::Or orForeign(QList<Predicate*>() << new Predicate::Unconditional() << new ForeignPredicate());
        Predicate::Or orHeld(QList<Predicate*>() << new Predicate::AbsBefore(6) << new Predicate::RelBefore(5));
        QVERIFY_EXCEPTION_THROWN(orHeld = orForeign, std::runtime_error);
        QCOMPARE(shape(orHeld), QStringLiteral("or(abs:6,rel:5)"));     // was or(unconditional)
    }

    /** An And/Or copy that threw leaked the operands it had copied so far. */
    void testPredicateCopyThatThrowsFreesWhatItCopied()
    {
#if defined(_MSC_VER) && defined(_DEBUG)
        Predicate::And andForeign(QList<Predicate*>() << new Predicate::Unconditional() << new ForeignPredicate());
        Predicate::Or orForeign(QList<Predicate*>() << new Predicate::Unconditional() << new ForeignPredicate());
        _CrtMemState before, after, difference;
        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 100; ++i) {                // 200 copies; each one leaked a block
            QVERIFY_EXCEPTION_THROWN(Predicate::And{andForeign}, std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(Predicate::Or{orForeign}, std::runtime_error);
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        // A size_t difference: fewer blocks wrap around, back to negative here.
        const qint64 blocks = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(blocks < 100, qPrintable(QStringLiteral("%1 more heap blocks").arg(blocks)));
#else
        QSKIP("counts heap blocks with the MSVC debug heap");
#endif
    }

    /**
     * Claimant and Not assignment deleted the predicate they held before
     * copying the other's: when the copy threw (a predicate type
     * Predicate::create does not know, alone or nested in and/or), the member
     * was left pointing at the freed predicate, read after that, and deleted
     * again by the destructor. The claimant had taken the other's destination
     * too. A throw leaves both as they were.
     */
    void testClaimantAndNotAssignmentThatThrowsKeepsWhatTheyHeld()
    {
        const Claimant foreign(QString(ISSUER()), new ForeignPredicate());
        const Claimant nested(QString(ISSUER()), new Predicate::Not(new Predicate::And(
                                  QList<Predicate*>() << new Predicate::Unconditional() << new ForeignPredicate())));
        for (const Claimant* other : { &foreign, &nested }) {
            Claimant held(QString(DEST()), new Predicate::AbsBefore(6));
            QVERIFY_EXCEPTION_THROWN(held = *other, std::runtime_error);
            QCOMPARE(held.getDestination(), QString(DEST()));             // was the other's
            QCOMPARE(shape(held.getPredicate()), QStringLiteral("abs:6"));   // was freed
            QCOMPARE(claimantXdr(held), claimantXdr(Claimant(QString(DEST()), new Predicate::AbsBefore(6))));
        }                                                                // and freed again here

        Predicate::Not notForeign(new ForeignPredicate());
        Predicate::Not notNested(new Predicate::Or(QList<Predicate*>() << new Predicate::RelBefore(5)
                                                                       << new ForeignPredicate()));
        for (const Predicate::Not* other : { &notForeign, &notNested }) {
            Predicate::Not held(new Predicate::AbsBefore(6));
            QVERIFY_EXCEPTION_THROWN(held = *other, std::runtime_error);
            QCOMPARE(shape(held), QStringLiteral("not(abs:6)"));            // was freed
            Predicate::Not copy;
            copy = held;                                                 // what it held is whole
            QVERIFY(copy.equals(&held));
        }                                                                // and freed again here
    }

    /**
     * An XDR and/or whose operand is refused ("Corrupted predicate": an
     * and/or of fewer than 2, which a claimant in a decoded envelope can
     * carry) leaked the operands converted before it.
     */
    void testPredicateFromXdrThatThrowsFreesWhatItConverted()
    {
        stellar::ClaimPredicate lonely;                  // and(unconditional)
        lonely.fillAndPredicates().append(Predicate::Unconditional().toXdr());
        stellar::ClaimPredicate outer;                   // or(abs_before 6, and(unconditional))
        stellar::Array<stellar::ClaimPredicate, 2>& operands = outer.fillOrPredicates();
        operands.append(Predicate::AbsBefore(6).toXdr());
        operands.append(lonely);
        stellar::Claimant claimant = Claimant(QString(DEST()), new Predicate::Unconditional()).toXdr();
        claimant.v0.predicate = outer;
        try {
            delete Predicate::fromXdr(outer);
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("Corrupted predicate"));
        }
        QVERIFY_EXCEPTION_THROWN(Claimant::fromXdr(claimant), std::runtime_error);
#if defined(_MSC_VER) && defined(_DEBUG)
        _CrtMemState before, after, difference;
        _CrtMemCheckpoint(&before);
        for (int i = 0; i < 100; ++i) {                // 200 conversions; each one leaked a block
            QVERIFY_EXCEPTION_THROWN(delete Predicate::fromXdr(outer), std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(Claimant::fromXdr(claimant), std::runtime_error);
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        // A size_t difference: fewer blocks wrap around, back to negative here.
        const qint64 blocks = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(blocks < 100, qPrintable(QStringLiteral("%1 more heap blocks").arg(blocks)));
#else
        QSKIP("counts heap blocks with the MSVC debug heap");
#endif
    }

    /**
     * ClaimableBalanceResponse::setClaimants and
     * CreateClaimableBalanceOperationResponse::setClaimants return early when
     * the new list == the old one, and QList == asks Claimant ==: while
     * And/Or equality ignored the order, a list whose predicate came in
     * reordered was dropped, claimantsChanged was not emitted and
     * getClaimants() kept the old order.
     */
    void testSetClaimantsSeesReorderedOperands()
    {
        const qint64 a = 1700000000;
        const QList<Claimant> first = orClaimants(new Predicate::AbsBefore(a), new Predicate::Unconditional());
        const QList<Claimant> firstToo = orClaimants(new Predicate::AbsBefore(a), new Predicate::Unconditional());
        const QList<Claimant> swapped = orClaimants(new Predicate::Unconditional(), new Predicate::AbsBefore(a));
        const QString swappedShape = QStringLiteral("or(unconditional,abs:1700000000)");

        ClaimableBalanceResponse balance;
        balance.setClaimants(first);
        QSignalSpy balanceChanged(&balance, &ClaimableBalanceResponse::claimantsChanged);
        balance.setClaimants(firstToo);                // the same claimants: nothing changes
        QCOMPARE(balanceChanged.count(), 0);
        balance.setClaimants(swapped);                 // was dropped
        QCOMPARE(balanceChanged.count(), 1);
        QCOMPARE(balance.getClaimants().size(), 1);
        QCOMPARE(shape(balance.getClaimants().at(0).getPredicate()), swappedShape);
        QCOMPARE(claimantXdr(balance.getClaimants().at(0)), claimantXdr(swapped.at(0)));

        CreateClaimableBalanceOperationResponse operation;
        operation.setClaimants(first);
        QSignalSpy operationChanged(&operation, &CreateClaimableBalanceOperationResponse::claimantsChanged);
        operation.setClaimants(firstToo);
        QCOMPARE(operationChanged.count(), 0);
        operation.setClaimants(swapped);               // was dropped
        QCOMPARE(operationChanged.count(), 1);
        QCOMPARE(operation.getClaimants().size(), 1);
        QCOMPARE(shape(operation.getClaimants().at(0).getPredicate()), swappedShape);
        QCOMPARE(claimantXdr(operation.getClaimants().at(0)), claimantXdr(swapped.at(0)));
    }
};

ADD_TEST(RegressionFixesTest)
#endif // REGRESSIONFIXESTEST_H
