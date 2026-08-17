#ifndef MNEMONICTEST_H
#define MNEMONICTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/util.h"

/**
 * BIP-39 mnemonic generation / validation — asserted against the canonical
 * test vectors (also reproduced with the py `mnemonic` library).
 */
class MnemonicTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testEntropyToMnemonicVectors()
    {
        QCOMPARE(Util::entropyToMnemonic(QByteArray::fromHex("00000000000000000000000000000000")),
                 QString("abandon abandon abandon abandon abandon abandon abandon abandon "
                         "abandon abandon abandon about"));
        QCOMPARE(Util::entropyToMnemonic(QByteArray::fromHex("7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f")),
                 QString("legal winner thank year wave sausage worth useful legal winner thank yellow"));
        QCOMPARE(Util::entropyToMnemonic(QByteArray::fromHex("0102030405060708090a0b0c0d0e0f10")),
                 QString("absurd avoid scissors anxiety gather lottery category door army half long camera"));
        QCOMPARE(Util::entropyToMnemonic(QByteArray::fromHex(
                     "0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20")),
                 QString("absurd avoid scissors anxiety gather lottery category door army half long cage "
                         "bachelor another expect people blade school educate curtain scrub monitor lady beyond"));
    }

    void testValidateMnemonic()
    {
        QVERIFY(Util::validateMnemonic(
            "absurd avoid scissors anxiety gather lottery category door army half long camera"));
        // Tampered last word → checksum mismatch.
        QVERIFY(!Util::validateMnemonic(
            "absurd avoid scissors anxiety gather lottery category door army half long abandon"));
        // Unknown word.
        QVERIFY(!Util::validateMnemonic(
            "absurd avoid scissors anxiety gather lottery category door army half long notaword"));
        // Wrong word count.
        QVERIFY(!Util::validateMnemonic("abandon abandon about"));
    }

    void testGenerateMnemonicRoundTrip()
    {
        const QString m12 = Util::generateMnemonic(128);
        QCOMPARE(m12.split(' ').size(), 12);
        QVERIFY(Util::validateMnemonic(m12));

        const QString m24 = Util::generateMnemonic(256);
        QCOMPARE(m24.split(' ').size(), 24);
        QVERIFY(Util::validateMnemonic(m24));

        // Two generations differ (random entropy).
        QVERIFY(Util::generateMnemonic(128) != Util::generateMnemonic(128));
    }
};

ADD_TEST(MnemonicTest)
#endif // MNEMONICTEST_H
