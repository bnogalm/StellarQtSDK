#ifndef MEMOTEST_H
#define MEMOTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/memo.h"
#include "../src/responses/accountresponse.h"
#include "../xdr/stellartransaction.h"
class MemoTest: public QObject
{
    Q_OBJECT

private slots:

    /**
     * A hash / return memo from Horizon is decoded strictly, as js and py
     * decode it: the lenient decoder skipped characters it did not know, so
     * a URL-safe '-' / '_' or a damaged value became other bytes.
     */
    void testHorizonMemoHashIsStrictBase64() {
        const QByteArray hash(32, '\xfb');
        const QByteArray good = hash.toBase64();
        Memo *ok = Memo::parse(QString("hash"), good);
        QVERIFY(dynamic_cast<MemoHash*>(ok));
        QCOMPARE(dynamic_cast<MemoHash*>(ok)->getBytes(), hash);
        delete ok;

        const QByteArray urlSafe = hash.toBase64(QByteArray::Base64UrlEncoding);
        QVERIFY(urlSafe != good);
        QVERIFY_EXCEPTION_THROWN(delete Memo::parse(QString("hash"), urlSafe), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(delete Memo::parse(QString("return"), QByteArray("!") + good), std::runtime_error);
    }

    /** An account data value that is not base64 decodes to nothing, not to other bytes. */
    void testAccountDataDecodesStrictly() {
        QVariantMap map;
        map.insert("good", QString("dGVzdA=="));
        map.insert("bad", QString("dGVz-dA=="));
        AccountResponseAttach::Data data(map);
        QCOMPARE(data.getDecoded("good"), QByteArray("test"));
        QVERIFY(data.getDecoded("bad").isEmpty());
    }

    void testMemoNone() {
        MemoNone* memo = Memo::none();
        QCOMPARE( stellar::MemoType::MEMO_NONE, memo->toXdr().type);
        stellar::Memo memoXdr = memo->toXdr();
        MemoNone * memoReconverted = dynamic_cast<MemoNone*>(Memo::fromXdr(memoXdr));
        QCOMPARE(stellar::MemoType::MEMO_NONE, memoReconverted->toXdr().type);
    }
    void testMemoTextSuccess() {
        MemoText* memo = Memo::text("test");
        QCOMPARE(stellar::MemoType::MEMO_TEXT, memo->toXdr().type);
        QCOMPARE(QString("test"), memo->getText());
        MemoText * memoReconverted=nullptr;
        {
            stellar::Memo memoXdr = memo->toXdr();
            memoReconverted = dynamic_cast<MemoText*>(Memo::fromXdr(memoXdr));
        }
        QCOMPARE(stellar::MemoType::MEMO_TEXT, memoReconverted->toXdr().type);
        QCOMPARE(QString("test"), memoReconverted->getText());
    }
    void testMemoTextUtf8() {
        MemoText* memo = Memo::text("三");
        QCOMPARE(stellar::MemoType::MEMO_TEXT, memo->toXdr().type);
        QCOMPARE(QString("三"), memo->getText());
    }
    void testMemoTextTooLong() {
        try {
            Memo::text("12345678901234567890123456789");
            QFAIL("Expected exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).contains("text must be <= 28 bytes."));
        }
    }
    void testMemoTextTooLongUtf8() {
        try {
            Memo::text("价值交易的开源协议!!");
            QFAIL("Expected exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).contains("text must be <= 28 bytes."));
        }
    }
    void testMemoId() {
        MemoId* memo = Memo::id(9223372036854775807L);
        QCOMPARE(9223372036854775807UL, memo->getId());
        QCOMPARE(stellar::MemoType::MEMO_ID, memo->toXdr().type);
        QCOMPARE(9223372036854775807UL, quint64(memo->toXdr().id));
        stellar::Memo memoXdr = memo->toXdr();
        MemoId * memoReconverted = dynamic_cast<MemoId*>(Memo::fromXdr(memoXdr));
        QCOMPARE(stellar::MemoType::MEMO_ID, memoReconverted->toXdr().type);
        QCOMPARE(9223372036854775807UL, memoReconverted->toXdr().id);
    }
    void testParseMemoId() {
        QString longId = "10048071741004807174";
        MemoId *memoId = dynamic_cast<MemoId*>(Memo::parse("id",longId));
        QCOMPARE(QString::number(memoId->getId()),longId);
    }

    /**
     * A memo id from text (Horizon's memo, or a form) is a uint64 in decimal
     * digits, as js Memo.id takes it ("Expects a uint64 as a string"; "01"
     * is read). Anything else was memo id 0, the id deposits are routed by.
     */
    void testParseMemoIdRefusesWhatIsNotAUint64() {
        QScopedPointer<Memo> max(Memo::parse(QString("id"), QByteArray("18446744073709551615")));
        QCOMPARE(dynamic_cast<MemoId*>(max.data())->getId(), Q_UINT64_C(18446744073709551615));
        QScopedPointer<Memo> zeros(Memo::parse(QString("id"), QString("007")));
        QCOMPARE(dynamic_cast<MemoId*>(zeros.data())->getId(), quint64(7));
        for (const char* text : {"abc", "", "-1", "+1", " 1", "1 ", "1.5", "1e3", "0x10",
                                 "18446744073709551616"}) {
            QVERIFY_EXCEPTION_THROWN(delete Memo::parse(QString("id"), QByteArray(text)), std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(delete Memo::parse(QString("id"), QString(text)), std::runtime_error);
        }
    }
    void testMemoHashSuccess() {
        MemoHash* memo = Memo::hash(QString("4142434445464748494a4b4c0000000000000000000000000000000000000000"));
        QCOMPARE(stellar::MemoType::MEMO_HASH, memo->toXdr().type);
        QString test = "ABCDEFGHIJKL";
        QCOMPARE(test, Util::paddedByteArrayToString(memo->getBytes()));
        QCOMPARE(QString("4142434445464748494a4b4c"), memo->getTrimmedHexValue());
    }
    void testMemoHashBytesSuccess() {
        QByteArray bytes = QByteArray(10,'A') + QByteArray(22, '\0');
        MemoHash* memo = Memo::hash(bytes);
        QCOMPARE(stellar::MemoType::MEMO_HASH, memo->toXdr().type);
        QCOMPARE(QString("AAAAAAAAAA"), Util::paddedByteArrayToString(memo->getBytes()));
        QCOMPARE(QString("4141414141414141414100000000000000000000000000000000000000000000"), memo->getHexValue());
        QCOMPARE(QString("41414141414141414141"), memo->getTrimmedHexValue());
        stellar::Memo memoXdr = memo->toXdr();
        MemoHash * memoReconverted = dynamic_cast<MemoHash*>(Memo::fromXdr(memoXdr));
        QCOMPARE(stellar::MemoType::MEMO_HASH, memoReconverted->toXdr().type);
        QCOMPARE(QString("41414141414141414141"), memoReconverted->getTrimmedHexValue());
    }
    void testMemoHashTooLong() {

        QByteArray longer(33,'\0');

        try {
            Memo::hash(longer);
            QFAIL("Expected exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).contains("exactly 32 bytes"));
        }
    }
    /** Shorter than 32 bytes was padded with zeros and signed: "" as the
     *  all-zero hash. js and py refuse it; so does this now. */
    void testMemoHashTooShortIsRefused() {
        for (const QString& hex : {QString(""), QString("ab"), QString(62, QLatin1Char('a'))}) {
            QVERIFY_EXCEPTION_THROWN(Memo::hash(hex), std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(Memo::returnHash(hex), std::runtime_error);
        }
        QVERIFY_EXCEPTION_THROWN(Memo::hash(QByteArray(31, 'a')), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Memo::hash(QByteArray()), std::runtime_error);
        QCOMPARE(Memo::hash(QByteArray(32, 'a'))->getBytes(), QByteArray(32, 'a'));
    }
    void testMemoHashInvalidHex() {
        try {
            Memo::hash(QString("test"));
            QFAIL("Expected exception");
        } catch (const std::runtime_error& exception) {
            Q_UNUSED(exception)
        }
    }
    void testMemoReturnHashSuccess() {
        MemoReturnHash* memo = Memo::returnHash(QString("4142434445464748494a4b4c0000000000000000000000000000000000000000"));
        QCOMPARE(stellar::MemoType::MEMO_RETURN, memo->toXdr().type);
        QCOMPARE(QString("4142434445464748494a4b4c"), memo->getTrimmedHexValue());
        stellar::Memo memoXdr = memo->toXdr();
        MemoReturnHash * memoReconverted = dynamic_cast<MemoReturnHash*>(Memo::fromXdr(memoXdr));
        QCOMPARE(stellar::MemoType::MEMO_RETURN, memoReconverted->toXdr().type);
        QCOMPARE(QString("4142434445464748494a4b4c"), memoReconverted->getTrimmedHexValue());
    }
    void testMemoSerializeDeserialize() {
        MemoNone* memoObjectNone = Memo::none();
        MemoText* memoObjectText = Memo::text("test");
        MemoId* memoObjectId = Memo::id(9223372036854775807L);
        MemoHash* memoObjectHash = Memo::hash(QString("4142434445464748494a4b4c0000000000000000000000000000000000000000"));
        MemoReturnHash* memoObjectReturnHash = Memo::returnHash(QString("4142434445464748494a4b4c0000000000000000000000000000000000000000"));

        QByteArray dataSerialized;
        QDataStream streamSerialize(&dataSerialized,QIODevice::WriteOnly);
        streamSerialize <<memoObjectNone->toXdr() <<memoObjectText->toXdr() << memoObjectId->toXdr() <<memoObjectHash->toXdr()<< memoObjectReturnHash->toXdr();

        QDataStream streamDeserialize(dataSerialized);
        stellar::Memo memoNone;
        stellar::Memo memoText;
        stellar::Memo memoId;
        stellar::Memo memoHash;
        stellar::Memo memoReturnHash;

        streamDeserialize >>memoNone >>memoText >>memoId >>memoHash >> memoReturnHash;

        QVERIFY(dynamic_cast<MemoNone*>(Memo::fromXdr(memoNone)));

        QVERIFY(dynamic_cast<MemoText*>(Memo::fromXdr(memoText)));
        QCOMPARE(memoObjectText->getText(), dynamic_cast<MemoText*>(Memo::fromXdr(memoText))->getText());

        QVERIFY(dynamic_cast<MemoId*>(Memo::fromXdr(memoId)));
        QCOMPARE(memoObjectId->getId(), dynamic_cast<MemoId*>(Memo::fromXdr(memoId))->getId());

        QVERIFY(dynamic_cast<MemoHash*>(Memo::fromXdr(memoHash)));
        QCOMPARE(memoObjectHash->getTrimmedHexValue(), dynamic_cast<MemoHash*>(Memo::fromXdr(memoHash))->getTrimmedHexValue());

        QVERIFY(dynamic_cast<MemoReturnHash*>(Memo::fromXdr(memoReturnHash)));
        QCOMPARE(memoObjectReturnHash->getTrimmedHexValue(), dynamic_cast<MemoReturnHash*>(Memo::fromXdr(memoReturnHash))->getTrimmedHexValue());
    }
};

ADD_TEST(MemoTest)
#endif // MEMOTEST_H


