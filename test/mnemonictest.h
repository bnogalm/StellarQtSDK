#ifndef MNEMONICTEST_H
#define MNEMONICTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/util.h"
#include "../src/keypair.h"
#include <QScopedPointer>

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

    /** The official BIP-39 vectors, all derived with the passphrase "TREZOR". */
    void testSeedVectorsWithPassphrase()
    {
        QCOMPARE(Util::mnemonicToBIP39Seed(
                     "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
                     "abandon abandon about", "TREZOR"),
                 QByteArray::fromHex("c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e534955"
                                     "31f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04"));
        QCOMPARE(Util::mnemonicToBIP39Seed(
                     "legal winner thank year wave sausage worth useful legal winner thank yellow",
                     "TREZOR"),
                 QByteArray::fromHex("2e8905819b8723fe2c1d161860e5ee1830318dbf49a83bd451cfb8440c28bd6f"
                                     "a457fe1296106559a3c80937a1c1069be3a3a5bd381ee6260e8d9739fce1f607"));
        QCOMPARE(Util::mnemonicToBIP39Seed("zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong", "TREZOR"),
                 QByteArray::fromHex("ac27495480225222079d7be181583751e86f571027b0497b5b5d11218e0a8a13"
                                     "332572917f0f8e5a589620c6f15b11c61dee327651a14c34e18231052e48c069"));
    }

    /**
     * BIP-39 salts PBKDF2 with "mnemonic" + the passphrase in NFKD. Only the
     * words were normalised, so a passphrase typed precomposed (what most
     * keyboards produce) gave a different wallet from the same passphrase
     * decomposed, and neither was the wallet other BIP-39 software derives.
     * Expected seed: py stellar_sdk 14.1.0 StellarMnemonic.to_bip39_seed and
     * hashlib.pbkdf2_hmac over unicodedata.normalize('NFKD', ...), which
     * agree; the passphrase as typed gave 8a2d3630411eef1f...
     */
    void testPassphraseIsNormalisedToNfkd()
    {
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QByteArray expected = QByteArray::fromHex(
            "a8145bfa29142b52940d43d97091e59c6de667974b7d9c487953b84e0539ebd9"
            "585be22f5b30845a213c4f029d1d604dee37effc8c8cb2d52a935fbce6b8ac45");

        // "Password" typed with a precomposed a- and o-diaeresis (U+00E4,
        // U+00F6), then U+334D (squared katakana "metre") and U+FB01 (the
        // "fi" ligature), two compatibility characters, and U+212B, the
        // angstrom sign.
        const QString typed = QStringLiteral("P") + QChar(0x00E4) + QStringLiteral("ssw")
                              + QChar(0x00F6) + QStringLiteral("rd ") + QChar(0x334D)
                              + QStringLiteral(" ") + QChar(0xFB01) + QStringLiteral(" ")
                              + QChar(0x212B);
        // The same passphrase in NFKD, written out: a/o + combining diaeresis,
        // the katakana spelled out, "fi", and A + combining ring.
        const QString nfkd = QStringLiteral("Pa") + QChar(0x0308) + QStringLiteral("sswo")
                             + QChar(0x0308) + QStringLiteral("rd ") + QChar(0x30E1)
                             + QChar(0x30FC) + QChar(0x30C8) + QChar(0x30EB)
                             + QStringLiteral(" fi A") + QChar(0x030A);
        QVERIFY(typed != nfkd);
        QCOMPARE(Util::mnemonicToBIP39Seed(words, typed), expected);
        QCOMPARE(Util::mnemonicToBIP39Seed(words, nfkd), expected);
        QCOMPARE(Util::mnemonicToBIP39Seed(words, typed.normalized(QString::NormalizationForm_C)),
                 expected);

        // Not text at all: an unpaired surrogate used to be signed over as
        // U+FFFD, a passphrase nobody typed.
        QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39Seed(words, QStringLiteral("pass") + QChar(0xD800)),
                                 std::runtime_error);
    }

    /**
     * The sentence is NFKD-normalised as a whole, as BIP-39 defines it, so a
     * no-break or ideographic space separates words as it does for every
     * other implementation. Normalised word by word, it stayed part of the
     * word and a valid phrase was refused.
     */
    void testSentenceIsNormalisedToNfkd()
    {
        const QByteArray expected = QByteArray::fromHex(
            "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e534955"
            "31f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04");
        QString sentence = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                          "abandon abandon abandon abandon abandon about");
        sentence[7] = QChar(0x00A0);     // no-break space
        sentence[15] = QChar(0x3000);    // ideographic space
        QCOMPARE(Util::mnemonicToBIP39Seed(sentence, QStringLiteral("TREZOR")), expected);
        QVERIFY(Util::validateMnemonic(sentence));
        QVERIFY(!Util::validateMnemonic(sentence + QChar(0xDC00)));
    }

    /**
     * 1.0.0-1.0.3 salted PBKDF2 with "mnemonic" + the passphrase as typed.
     * Normalising it to NFKD moved every wallet created with a passphrase
     * whose NFKD form differs (a precomposed accent, a compatibility
     * character) to other accounts, and nothing could reach the old ones: the
     * funds were out of reach after the upgrade. The legacy derivation is
     * kept, explicitly named, for recovering them. Expected: python hashlib
     * pbkdf2_hmac('sha512', words, ('mnemonic' + passphrase).encode('utf-8'),
     * 2048) with no normalisation, which is what the 1.0.3 code (27b1d1f)
     * computes, and py stellar_sdk 14.1.0 for the accounts.
     */
    void testLegacyPassphraseDerivationRecoversOldWallets()
    {
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        // The passphrase of testPassphraseIsNormalisedToNfkd, typed and in NFKD.
        const QString typed = QStringLiteral("P") + QChar(0x00E4) + QStringLiteral("ssw")
                              + QChar(0x00F6) + QStringLiteral("rd ") + QChar(0x334D)
                              + QStringLiteral(" ") + QChar(0xFB01) + QStringLiteral(" ")
                              + QChar(0x212B);
        const QString nfkd = QStringLiteral("Pa") + QChar(0x0308) + QStringLiteral("sswo")
                             + QChar(0x0308) + QStringLiteral("rd ") + QChar(0x30E1)
                             + QChar(0x30FC) + QChar(0x30C8) + QChar(0x30EB)
                             + QStringLiteral(" fi A") + QChar(0x030A);
        const QByteArray legacy = QByteArray::fromHex(
            "8a2d3630411eef1fd1e6de429f04cb50cf0bc505bfa66275dfe5a0ed09f75117"
            "fcb7d35c1e7a7f499b9d5af834885894bfa87f749350df4c7fc84ff9ddf67ea8");
        const QByteArray bip39 = QByteArray::fromHex(
            "a8145bfa29142b52940d43d97091e59c6de667974b7d9c487953b84e0539ebd9"
            "585be22f5b30845a213c4f029d1d604dee37effc8c8cb2d52a935fbce6b8ac45");

        // Non-ASCII, not in NFKD: the two derivations differ, and the legacy
        // one is 1.0.3's.
        QVERIFY(Util::legacyPassphraseDerivesDifferently(typed));
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(words, typed), legacy);
        QCOMPARE(Util::mnemonicToBIP39Seed(words, typed), bip39);
        QScopedPointer<KeyPair> old(KeyPair::fromBip39Seed(legacy, 0));
        QCOMPARE(old->getAccountId(), QStringLiteral("GB5OYYOSOUO52LZI6TOMCJIZ726BNNZZFI3RKISAUZWFZ25MQFZLHKYZ"));
        QCOMPARE(old->getSecretSeed(), QStringLiteral("SDKSJVB6YMUIOTKAHBF7EVX4ZC5ZMVWRLNAHO4V4NLZX3ZUXV6QNDBGW"));
        old.reset(KeyPair::fromBip39Seed(legacy, 1));
        QCOMPARE(old->getAccountId(), QStringLiteral("GA4VE6QLOCX4BMGCH6JRNVNNK2BVNWTM7OGKOTITTCVYGHGOLUWEALL4"));

        // Non-ASCII already in NFKD, and ASCII: the same seed both ways.
        QVERIFY(!Util::legacyPassphraseDerivesDifferently(nfkd));
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(words, nfkd), bip39);
        const QByteArray trezor = QByteArray::fromHex(
            "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e534955"
            "31f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04");
        QVERIFY(!Util::legacyPassphraseDerivesDifferently(QStringLiteral("TREZOR")));
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(words, QStringLiteral("TREZOR")), trezor);
        QCOMPARE(Util::mnemonicToBIP39Seed(words, QStringLiteral("TREZOR")), trezor);
        QVERIFY(!Util::legacyPassphraseDerivesDifferently(QString()));
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(words), Util::mnemonicToBIP39Seed(words));

        // The words are read and refused as mnemonicToBIP39Seed reads them,
        // and a passphrase that is not text is refused, not derived from.
        QString spaced = words;
        spaced[7] = QChar(0x00A0);
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(spaced, typed), legacy);
        QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39SeedLegacyPassphrase(
                                     QStringLiteral("abandon abandon about"), typed),
                                 std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39SeedLegacyPassphrase(
                                     words.left(words.size() - 5) + QStringLiteral("zzzzz"), typed),
                                 std::runtime_error);
        const QString unpaired = QStringLiteral("pass") + QChar(0xD800);
        QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39SeedLegacyPassphrase(words, unpaired),
                                 std::runtime_error);
        QVERIFY(!Util::legacyPassphraseDerivesDifferently(unpaired));
    }
};

ADD_TEST(MnemonicTest)
#endif // MNEMONICTEST_H
