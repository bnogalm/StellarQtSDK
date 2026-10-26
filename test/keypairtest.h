#ifndef KEYPAIRTEST_H
#define KEYPAIRTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "../src/messageauth.h"
#include <memory>
#include <limits>
#include "testcollector.h"
static const char SEED[] = "1123740522f11bfef6b3671f51e159ccf589ccf8965262dd5f97d1721d383dd4";


class KeyPairTest: public QObject
{
    Q_OBJECT
public:


private slots:

    /**
     * SEP-53 from a keypair, with text, bytes and a C string: the spec's
     * vectors, which py-stellar-sdk 14.1.0 signs alike (text as UTF-8).
     */
    void testSep53MessagesFromAKeyPair()
    {
        std::unique_ptr<KeyPair> key(KeyPair::fromSecretSeed(QStringLiteral("SAKICEVQLYWGSOJS4WW7HZJWAHZVEEBS527LHK5V4MLJALYKICQCJXMW")));
        QCOMPARE(key->getAccountId(), QStringLiteral("GBXFXNDLV4LSWA4VB7YIL5GBD7BVNR22SGBTDKMO2SBZZHDXSKZYCP7L"));
        const QByteArray hello = QByteArray::fromBase64(
            "fO5dbYhXUhBMhe6kId/cuVq/AfEnHRHEvsP8vXh03M1uLpi5e46yO2Q8rEBzu3feXQewcQE5GArp88u6ePK6BA==");
        QCOMPARE(key->signMessage("Hello, World!"), hello);
        QCOMPARE(key->signMessage(QStringLiteral("Hello, World!")), hello);
        QCOMPARE(key->signMessage(QByteArray("Hello, World!")), hello);
        QCOMPARE(MessageAuth::signMessage("Hello, World!", key.get()), hello);
        const QString japanese = QString::fromUtf8("\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf"
                                                   "\xe3\x80\x81\xe4\xb8\x96\xe7\x95\x8c\xef\xbc\x81");
        QCOMPARE(key->signMessage(japanese), QByteArray::fromBase64(
            "CDU265Xs8y3OWbB/56H9jPgUss5G9A0qFuTqH2zs2YDgTm+++dIfmAEceFqB7bhfN3am59lCtDXrCtwH2k1GBA=="));
        const QByteArray binary = QByteArray::fromBase64("2zZDP1sa1BVBfLP7TeeMk3sUbaxAkUhBhDiNdrksaFo=");
        QCOMPARE(key->signMessage(binary), QByteArray::fromBase64(
            "VA1+7hefNwv2NKScH6n+Sljj15kLAge+M2wE7fzFOf+L0MMbssA1mwfJZRyyrhBORQRle10X1Dxpx+UOI4EbDQ=="));

        std::unique_ptr<KeyPair> publicOnly(KeyPair::fromAccountId(key->getAccountId()));
        QVERIFY(publicOnly->verifyMessage("Hello, World!", hello));
        QVERIFY(publicOnly->verifyMessage(QStringLiteral("Hello, World!"), hello));
        QVERIFY(!publicOnly->verifyMessage("Hello, World?", hello));
        QVERIFY(MessageAuth::verifyMessage(key->getAccountId(), QStringLiteral("Hello, World!"), hello));
        QCOMPARE(QByteArray(MessageAuth::PREFIX), QByteArray("Stellar Signed Message:\n"));
    }

    /**
     * SEP-53 over ascii, UTF-8 (2-, 3- and 4-byte characters), binary with
     * NUL bytes, an empty message and 1000 bytes (i % 256): the signatures
     * js-stellar-sdk 17.1.0's Keypair.signMessage and py-stellar-sdk
     * 14.1.0's sign_message both give for the key of raw seed 0x01..0x20.
     * Each verifies, as bytes and (for text) as the QString; not under
     * another key, nor over another message.
     */
    void testSep53MessagesSignAsJsAndPy()
    {
        std::unique_ptr<KeyPair> key(KeyPair::fromSecretSeed(QStringLiteral("SAAQEAYEAUDAOCAJBIFQYDIOB4IBCEQTCQKRMFYYDENBWHA5DYPSBF5K")));
        const QString id = key->getAccountId();
        QCOMPARE(id, QStringLiteral("GB43KVROR7TFJ6KAPCYRF2FJROTZAH4FHLTJLPWX4DRZCC5NASLGITR6"));
        QByteArray longMessage;
        for (int i = 0; i < 1000; ++i)
            longMessage.append(char(i % 256));
        const struct { QByteArray message; QByteArray signature; } vectors[] = {
            {QByteArray(), QByteArray::fromBase64(
                 "IahmwhVvKdHGGgeiLxywqfTyuZcLgdV7Jq7W+falkR3yCtEKd5UaybY9c4mcq9SmkLE66IhUnKeRkiAdknyABg==")},
            {QByteArray("Stellar Signed Message test 0123456789 ~!@#$%^&*()"), QByteArray::fromBase64(
                 "vQdUjBg/gFk7hBRXWPJshV+WBpZnj5G10SPSHkPXJI/PLq3H423+Gfk8KadUhOSS+sm2GBaJW5PETLiWcAe/CA==")},
            {QByteArray::fromBase64("T2zDoSDDsWFuZMO6IOKCrCDwn5qA"), QByteArray::fromBase64(
                 "qeYaJEuOxJZK8rQBtuPW0VPiC1WEwyfBPhNm5uWiz/a6/Wq6p3cak47+XMQ0ez1a+EzppbUaG1BvjG8N42PfCw==")},
            {QByteArray::fromBase64("AAECAP/+Cg2AAA=="), QByteArray::fromBase64(
                 "Lez0uexhKA+gC51nxWnHIBXTbQSTykRLjnEfCFvFRLvdTGZEYtQ9AwIxkT8GNJLYuP+v4aVlPG0AQ5Ka3/8LAg==")},
            {longMessage, QByteArray::fromBase64(
                 "jsfELwY4F2516Z0sbHmm9SR8Tgkc8gzzkvzoCe4ksU9AtL08g+/2S/zeMMvXQK5apePGCqLyEQXfEb4cNC+pDw==")},
        };
        QCOMPARE(vectors[3].message.size(), 10);
        std::unique_ptr<KeyPair> other(KeyPair::fromSecretSeed(QStringLiteral("SAKICEVQLYWGSOJS4WW7HZJWAHZVEEBS527LHK5V4MLJALYKICQCJXMW")));
        for (const auto& v : vectors) {
            QCOMPARE(key->signMessage(v.message), v.signature);
            QCOMPARE(MessageAuth::signMessage(v.message, key.get()), v.signature);
            QVERIFY(MessageAuth::verifyMessage(id, v.message, v.signature));
            QVERIFY(key->verifyMessage(v.message, v.signature));
            QVERIFY(!other->verifyMessage(v.message, v.signature));
            QVERIFY(!key->verifyMessage(QByteArray(v.message).append('\0'), v.signature));
        }
        // Text is signed as its UTF-8 bytes, as py signs a str and js a
        // string: "Ola nandu", accented, the euro sign and U+1F680.
        const QString text = QString::fromUtf8(vectors[2].message);
        QCOMPARE(text.size(), 14);
        QCOMPARE(key->signMessage(text), vectors[2].signature);
        QVERIFY(MessageAuth::verifyMessage(id, text, vectors[2].signature));
        QCOMPARE(key->signMessage(QString()), vectors[0].signature);
        QCOMPARE(key->signMessage(""), vectors[0].signature);
        QCOMPARE(key->signMessage("Stellar Signed Message test 0123456789 ~!@#$%^&*()"), vectors[1].signature);
    }

    /**
     * A key by SLIP-10 path: m/44'/148'/n' is SEP-5's account n, as
     * fromBip39Seed(seed, n) derives it; another path derives its own key;
     * an index that is not hardened, or not a number below 2^31, is refused.
     */
    void testAKeyByDerivationPath()
    {
        const QByteArray seed = Util::mnemonicToBIP39Seed(QStringLiteral(
            "illness spike retreat truth genius clock brain pass fit cave bargain toe"));
        std::unique_ptr<KeyPair> byNumber(KeyPair::fromBip39Seed(seed, 3));
        std::unique_ptr<KeyPair> byPath(KeyPair::fromBip39Seed(seed, QStringLiteral("m/44'/148'/3'")));
        std::unique_ptr<KeyPair> byH(KeyPair::fromBip39Seed(seed, QStringLiteral("m/44H/148H/3H")));
        QCOMPARE(byPath->getAccountId(), byNumber->getAccountId());
        QCOMPARE(byH->getAccountId(), byNumber->getAccountId());
        // SEP-5's vector for this phrase, account 0.
        std::unique_ptr<KeyPair> first(KeyPair::fromBip39Seed(seed, QStringLiteral("m/44'/148'/0'")));
        QCOMPARE(first->getAccountId(), QStringLiteral("GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6"));
        std::unique_ptr<KeyPair> other(KeyPair::fromBip39Seed(seed, QStringLiteral("m/44'/148'/0'/1'")));
        QVERIFY(other->getAccountId() != first->getAccountId());
        for (const char* bad : {"44'/148'/0'", "m/44'/148'/0", "m/44'/-1'/0'", "m/44'/2147483648'",
                                "m/44'/ 1'", "m/44'//0'", "M/44'/148'/0'"}) {
            QVERIFY_EXCEPTION_THROWN(delete KeyPair::fromBip39Seed(seed, QString::fromLatin1(bad)), std::runtime_error);
        }
    }

    /**
     * A strkey given to the bytes overload says which overload reads it
     * (it said "you seems using an user encoded string as bytearray seed"),
     * and no message repeats the seed. The keypair keeps its own copy of
     * the bytes, so a caller may wipe them.
     */
    void testSecretSeedMessagesAndCopies()
    {
        const QString seed = QStringLiteral("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE");
        auto message = [](auto call) {
            try {
                call();
            } catch (const std::exception& e) {
                return QString::fromUtf8(e.what());
            }
            return QString();
        };
        const QString strkeyAsBytes = message([&]() { delete KeyPair::fromSecretSeed(seed.toLatin1()); });
        QVERIFY2(strkeyAsBytes.contains(QStringLiteral("fromSecretSeed(QString)")), qPrintable(strkeyAsBytes));
        QVERIFY(!strkeyAsBytes.contains(seed));
        QVERIFY(message([&]() { delete KeyPair::fromSecretSeed(QByteArray(31, 'x')); }).contains(QStringLiteral("32 bytes")));
        QString damaged = seed;
        damaged[10] = QLatin1Char('A');
        const QString bad = message([&]() { delete KeyPair::fromSecretSeed(damaged); });
        QVERIFY(!bad.isEmpty());
        QVERIFY(!bad.contains(damaged));

        QByteArray raw = StrKey::decodeStellarSecretSeed(seed.toLatin1());
        std::unique_ptr<KeyPair> key(KeyPair::fromSecretSeed(raw));
        raw.fill('\0');
        QCOMPARE(key->getSecretSeed(), seed);
    }

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testSign(){
        QString expectedSig = "587d4b472eeef7d07aafcd0b049640b0bb3f39784118c2e2b73a04fa2f64c9c538b4b2d0f5335e968a480021fdc23e98c0ddf424cb15d8131df8cb6c4bb58309";

        KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(QString(SEED)));
        QByteArray data = "hello world";
        QByteArray sig = keypair->sign(data);
        //qDebug() <<"generated:"<< sig.toHex() ;
        //qDebug() <<"expected :"<< expectedSig;
        QVERIFY(Util::hexToBytes(expectedSig)==sig);

    }
    void testSignAndVerify(){

        KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(QString(SEED)));
        QByteArray data = "hello world";
        QByteArray sig = keypair->sign(data);
        QVERIFY(keypair->verify(data,sig));
    }


    void testVerifyFalse() {
      QString badSig = "687d4b472eeef7d07aafcd0b049640b0bb3f39784118c2e2b73a04fa2f64c9c538b4b2d0f5335e968a480021fdc23e98c0ddf424cb15d8131df8cb6c4bb58309";
      QByteArray corrupt;
      corrupt.append('\0');
      QByteArray data = "hello world";
      KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(SEED));
      QVERIFY(!keypair->verify(data , Util::hexToBytes(badSig)));
      QVERIFY(!keypair->verify(data , corrupt));
  }

    /** A valid signature with bytes appended is no signature: it verified,
     *  the tail ignored. Exactly 64 bytes, as js and py. */
    void testVerifyNeedsExactly64Bytes() {
      QScopedPointer<KeyPair> keypair(KeyPair::fromSecretSeed(Util::hexToBytes(SEED)));
      const QByteArray data = "hello world";
      const QByteArray sig = keypair->sign(data);
      QCOMPARE(sig.size(), 64);
      QVERIFY(keypair->verify(data, sig));
      QVERIFY(!keypair->verify(data, sig + QByteArray(1, '\0')));
      QVERIFY(!keypair->verify(data, sig + sig));
      QVERIFY(!keypair->verify(data, sig.left(63)));
  }

    /**
     * A signature whose S is not below the group order L is refused, as
     * libsodium (stellar-core), js 17.1.0 and py 14.1.0 refuse it: only
     * S < 2^253 was checked, so (R, S + L) verified wherever (R, S) did.
     * The vector is py's signature of "m0" with the seed 0, 1, ..., 31 and
     * that signature with L added to S (js and py: false / BadSignatureError).
     */
    void testVerifyRefusesANonCanonicalS() {
      QScopedPointer<KeyPair> keypair(KeyPair::fromSecretSeed(
          QStringLiteral("SAAACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6NKI")));
      QCOMPARE(keypair->getAccountId(), QStringLiteral("GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q"));
      const QByteArray data = "m0";
      const QByteArray sig = QByteArray::fromHex(
          "f000bfa72001890dec1b44fa06687defaa5f87da689ab0b6fcb0384b066b29e7"
          "bf6150b599f4e6b81e1e0bbc87687eb0e38a3560f52e05292c567369171f9807");
      const QByteArray malleated = QByteArray::fromHex(
          "f000bfa72001890dec1b44fa06687defaa5f87da689ab0b6fcb0384b066b29e7"
          "ac354612b457f910f5ba025f66625dc5e38a3560f52e05292c567369171f9817");
      QCOMPARE(keypair->sign(data), sig);
      QVERIFY(keypair->verify(data, sig));
      QVERIFY(!keypair->verify(data, malleated));
      QScopedPointer<KeyPair> publicOnly(KeyPair::fromAccountId(keypair->getAccountId()));
      QVERIFY(publicOnly->verify(data, sig));
      QVERIFY(!publicOnly->verify(data, malleated));
  }

    /**
     * A public key of small order (order 1, 2, 4 or 8, in any of its
     * encodings: the sign bit set, y = p or p + 1) verifies nothing, as
     * libsodium (stellar-core), js 17.1.0 and py 14.1.0 refuse it: [h]A is
     * one of 8 points, so a signature with S = 1 and R = B + T, T the right
     * one of them, verified for those keys without any secret key. R is not
     * of small order itself. The 14 forgeries below are all of libsodium's
     * blocklist, each with and without the sign bit; js and py answer false
     * (BadSignatureError) for each, and so did PyNaCl's crypto_sign_open.
     */
    void testVerifyRefusesASmallOrderPublicKey() {
      struct Forgery { const char *publicKey; const char *r; const char *message; };
      static const Forgery forgeries[] = {
          { "0000000000000000000000000000000000000000000000000000000000000000",
            "9bad33f580df7ecc49df5342bac8145d5bedc40f573d1b067f3c4ce449689a15", "m1" },
          { "0000000000000000000000000000000000000000000000000000000000000080",
            "9bad33f580df7ecc49df5342bac8145d5bedc40f573d1b067f3c4ce449689a15", "m1" },
          { "0100000000000000000000000000000000000000000000000000000000000000",
            "5866666666666666666666666666666666666666666666666666666666666666", "m0" },
          { "0100000000000000000000000000000000000000000000000000000000000080",
            "5866666666666666666666666666666666666666666666666666666666666666", "m0" },
          { "26e8958fc2b227b045c3f489f2ef98f0d5dfac05d3c63339b13802886d53fc05",
            "5866666666666666666666666666666666666666666666666666666666666666", "m3" },
          { "26e8958fc2b227b045c3f489f2ef98f0d5dfac05d3c63339b13802886d53fc85",
            "5252cc0a7f208133b620acbd4537eba2a4123bf0a8c2e4f980c3b31bb69765ea", "m0" },
          { "c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac037a",
            "5866666666666666666666666666666666666666666666666666666666666666", "m1" },
          { "c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac03fa",
            "55ae61520ca466adcc4ae4a32dc1633a5d749c64a5b50f136fc3469f27e487e6", "m1" },
          { "ecffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
            "9599999999999999999999999999999999999999999999999999999999999999", "m0" },
          { "ecffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "9599999999999999999999999999999999999999999999999999999999999999", "m0" },
          { "edffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
            "9bad33f580df7ecc49df5342bac8145d5bedc40f573d1b067f3c4ce449689a15", "m0" },
          { "edffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "9599999999999999999999999999999999999999999999999999999999999999", "m0" },
          { "eeffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
            "5866666666666666666666666666666666666666666666666666666666666666", "m0" },
          { "eeffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "5866666666666666666666666666666666666666666666666666666666666666", "m0" },
      };
      const QByteArray s = QByteArray::fromHex(
          "0100000000000000000000000000000000000000000000000000000000000000");
      for (const Forgery &f : forgeries) {
          QScopedPointer<KeyPair> key(KeyPair::fromPublicKey(QByteArray::fromHex(f.publicKey)));
          QVERIFY2(!key->verify(QByteArray(f.message), QByteArray::fromHex(f.r) + s), f.publicKey);
      }
      // The identity: R = B verified with any message, through SEP-53 too.
      QScopedPointer<KeyPair> identity(KeyPair::fromPublicKey(QByteArray::fromHex(forgeries[2].publicKey)));
      QCOMPARE(identity->getAccountId(), QStringLiteral("GAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHV4"));
      const QByteArray forged = QByteArray::fromHex(forgeries[2].r) + s;
      QVERIFY(!identity->verify(QByteArray("hello world"), forged));
      QVERIFY(!MessageAuth::verifyMessage(identity->getAccountId(), QStringLiteral("hello world"), forged));
  }

    /**
     * A signature whose R is of small order is refused, as libsodium
     * (stellar-core) and py 14.1.0 refuse it (BadSignatureError); js 17.1.0
     * answers true here, and core wins. The key is a = SHA-512("seed") mod L,
     * R the identity (01 00 ... 00) and S = h * a mod L, with
     * h = SHA-512(R || A || "stellar signed message") mod L: [S]B - [h]A is
     * the identity, so it verified, though the network refuses it.
     */
    void testVerifyRefusesASmallOrderR() {
      QScopedPointer<KeyPair> key(KeyPair::fromPublicKey(QByteArray::fromHex(
          "4f32305e96ff2d4090c4945c48e9250c2db5a1ca292c589da7d4d73d1b58cd9b")));
      QCOMPARE(key->getAccountId(), QStringLiteral("GBHTEMC6S37S2QEQYSKFYSHJEUGC3NNBZIUSYWE5U7KNOPI3LDGZXDDP"));
      const QByteArray sig = QByteArray::fromHex(
          "0100000000000000000000000000000000000000000000000000000000000000"
          "55e016b524054314b082caf7bd08ef9c1863ec205d8d4543aff15fe53b69830a");
      QVERIFY(!key->verify(QByteArray("stellar signed message"), sig));
  }

    /**
     * RFC 8032 section 7.1, tests 1 to 3: ordinary signatures still verify
     * after the small-order and canonical checks (js, py and PyNaCl: true),
     * and fail with another message.
     */
    void testVerifyRfc8032Vectors() {
      struct Vector { const char *publicKey; const char *message; const char *signature; };
      static const Vector vectors[] = {
          { "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a", "",
            "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b" },
          { "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c", "72",
            "92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00" },
          { "fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025", "af82",
            "6291d657deec24024827e69c3abe01a30ce548a284743a445e3680d7db5ac3ac18ff9b538d16f290ae67f760984dc6594a7c15e9716ed28dc027beceea1ec40a" },
      };
      for (const Vector &v : vectors) {
          QScopedPointer<KeyPair> key(KeyPair::fromPublicKey(QByteArray::fromHex(v.publicKey)));
          const QByteArray message = QByteArray::fromHex(v.message);
          const QByteArray sig = QByteArray::fromHex(v.signature);
          QVERIFY2(key->verify(message, sig), v.publicKey);
          QVERIFY2(!key->verify(message + "x", sig), v.publicKey);
      }
  }

    void testFromSecretSeed() {
        QHash<QString, QString> keypairs;
        keypairs.insert("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE", "GCZHXL5HXQX5ABDM26LHYRCQZ5OJFHLOPLZX47WEBP3V2PF5AVFK2A5D");
        keypairs.insert("SDTQN6XUC3D2Z6TIG3XLUTJIMHOSON2FMSKCTM2OHKKH2UX56RQ7R5Y4", "GDEAOZWTVHQZGGJY6KG4NAGJQ6DXATXAJO3AMW7C4IXLKMPWWB4FDNFZ");
        keypairs.insert("SDIREFASXYQVEI6RWCQW7F37E6YNXECQJ4SPIOFMMMJRU5CMDQVW32L5", "GD2EVR7DGDLNKWEG366FIKXO2KCUAIE3HBUQP4RNY7LEZR5LDKBYHMM6");
        keypairs.insert("SDAPE6RHEJ7745VQEKCI2LMYKZB3H6H366I33A42DG7XKV57673XLCC2", "GDLXVH2BTLCLZM53GF7ELZFF4BW4MHH2WXEA4Z5Z3O6DPNZNR44A56UJ");
        keypairs.insert("SDYZ5IYOML3LTWJ6WIAC2YWORKVO7GJRTPPGGNJQERH72I6ZCQHDAJZN", "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");

        for (QString seed : keypairs.keys()) {

            QString accountId = keypairs.value(seed);
            KeyPair *keypair = KeyPair::fromSecretSeed(seed);
            //qDebug() << accountId <<" ==? "<< keypair->getAccountId();
            QVERIFY(accountId== keypair->getAccountId());
            //qDebug() << seed <<" ==? "<< keypair->getSecretSeed();
            QVERIFY(seed== keypair->getSecretSeed());
        }
    }
    void testRandomMix() {
         QByteArray rand;
         for(int i=0;i<32;i++){
             rand.append(i);
         }
         KeyPair* issuer = KeyPair::random(rand);
         QVERIFY(issuer);
         rand = rand.remove(0,1);
         try{
            KeyPair* issuer = KeyPair::random(rand);
            Q_UNUSED(issuer)
            QFAIL("missing exception");
         }
         catch(const std::runtime_error& e)
         {
            Q_UNUSED(e)
         }
    }


    void testCanSign() {
      KeyPair* keypair;
      keypair = KeyPair::fromSecretSeed(QString("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE"));
      QVERIFY(keypair->canSign());
      keypair = KeyPair::fromAccountId(QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
      QVERIFY(!keypair->canSign());
    }

    void testSignWithoutSecret() {
        KeyPair* keypair = KeyPair::fromAccountId(QString("GDEAOZWTVHQZGGJY6KG4NAGJQ6DXATXAJO3AMW7C4IXLKMPWWB4FDNFZ"));
        QString data = "hello world";
        try {
            keypair->sign(data.toUtf8());
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString("KeyPair does not contain secret key. Use KeyPair::fromSecretSeed method to create a new KeyPair with a secret key."), QString(e.what()));
        }
    }

    /**
     * fromBip39Seed derived from any bytes at all: an empty seed (what a
     * failed hex decode hands over) gave a key anybody can derive from the
     * same nothing. BIP-32/SLIP-10 seeds are 16 to 64 bytes; BIP-39's are 64.
     */
    void testBip39SeedOutsideBip32BoundsIsRefused() {
        QVERIFY_EXCEPTION_THROWN(KeyPair::fromBip39Seed(QByteArray(), 0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(KeyPair::fromBip39Seed(QByteArray::fromHex("zz"), 0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(KeyPair::fromBip39Seed(QByteArray(15, '\x01'), 0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(KeyPair::fromBip39Seed(QByteArray(65, '\x01'), 0), std::runtime_error);
        KeyPair* shortest = KeyPair::fromBip39Seed(QByteArray(16, '\x01'), 0);
        KeyPair* longest = KeyPair::fromBip39Seed(QByteArray(64, '\x01'), 0);
        QVERIFY(shortest->canSign());
        QVERIFY(longest->canSign());
        delete shortest;
        delete longest;
    }

    /**
     * fromMnemonic took a negative account number and derived another
     * account with it (-2^31 gave account 0's key, -1 account 2^31 - 1's);
     * it is refused. py 14.1.0 derives a third, non-hardened key for it.
     * The last account, 2^31 - 1, is py's.
     */
    void testFromMnemonicRefusesANegativeAccount() {
        const QString words = QStringLiteral("illness spike retreat truth genius clock brain pass fit cave bargain toe");
        for (int bad : {-1, std::numeric_limits<int>::min()}) {
            QVERIFY_EXCEPTION_THROWN(delete KeyPair::fromMnemonic(words, QString(), bad), std::runtime_error);
        }
        std::unique_ptr<KeyPair> last(KeyPair::fromMnemonic(words, QString(), std::numeric_limits<int>::max()));
        QCOMPARE(last->getAccountId(), QStringLiteral("GDFBFRFJ32FAS747QDP3IQBL4JQHJNSVPUYDD2VL74GY7XIPFPX4CNS4"));
    }

    /**
     * The paths that now wipe their copies (decodeCheck's re-encoding of a
     * seed, the SLIP-10 key in fromBip39Seed and SLIP-10's intermediates,
     * mnemonicProblem's entropy, the PBKDF2 password) still give what they
     * gave: py 14.1.0's accounts 0 and 1 of SEP-5's phrase, a seed read back
     * from its strkey, and a phrase checked right after one refused early.
     * The wiping itself is not visible from a test.
     */
    void testWipedCopiesLeaveTheResultsAlone() {
        const QString words = QStringLiteral("illness spike retreat truth genius clock brain pass fit cave bargain toe");
        std::unique_ptr<KeyPair> first(KeyPair::fromMnemonic(words));
        QCOMPARE(first->getSecretSeed(), QStringLiteral("SBGWSG6BTNCKCOB3DIFBGCVMUPQFYPA2G4O34RMTB343OYPXU5DJDVMN"));
        std::unique_ptr<KeyPair> second(KeyPair::fromBip39Seed(Util::mnemonicToBIP39Seed(words), 1));
        QCOMPARE(second->getSecretSeed(), QStringLiteral("SCEPFFWGAG5P2VX5DHIYK3XEMZYLTYWIPWYEKXFHSK25RVMIUNJ7CTIS"));
        std::unique_ptr<KeyPair> again(KeyPair::fromSecretSeed(first->getSecretSeed()));
        QCOMPARE(again->getAccountId(), QStringLiteral("GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6"));

        QString unknown = words;
        unknown.replace(QStringLiteral("bargain"), QStringLiteral("zzzzq"));
        QCOMPARE(Util::mnemonicProblem(unknown), QStringLiteral("word 11 is not in the wordlist"));
        QCOMPARE(Util::mnemonicProblem(words), QString());
    }

};

ADD_TEST(KeyPairTest)

#endif // KEYPAIRTEST_H
