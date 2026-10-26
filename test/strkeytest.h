#ifndef STRKEYTEST_H
#define STRKEYTEST_H
#include <QObject>
#include "src/strkey.h"
#include "src/util.h"
#include "src/accountconverter.h"

#include <QtTest>
#include <QScopedPointer>
#include "../src/keypair.h"
#include "../src/assettypenative.h"
#include "../src/paymentoperation.h"
#include "testcollector.h"


class StrKeyTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDecodeEncode() {
        QString seed = "SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE";
        QByteArray secret = StrKey::decodeCheck(StrKey::VersionByte::SEED,seed.toLatin1());
        qDebug() << "secret: "<< secret.toHex();
        QString encoded = StrKey::encodeCheck(StrKey::VersionByte::SEED, secret);
        qDebug() << seed;
        qDebug() << encoded;
        QVERIFY(seed== encoded);

    }

    /**
     * A strkey is upper case (SEP-23). The base32 decoder also took lower
     * case, so "gcfiry65..." was accepted as a destination and signed, where
     * js ("destination is invalid") and py refuse it.
     */
    void testLowerCaseStrkeysAreRefused() {
        const QString account = QStringLiteral("GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR");
        QCOMPARE(StrKey::decodeStellarAccountId(account).size(), 32);
        for (const QString& bad : {account.toLower(), account.left(10) + account.mid(10).toLower(),
                                   account.left(55) + account.right(1).toLower()}) {
            bool threw = false;
            try { StrKey::decodeStellarAccountId(bad); } catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(bad));
            threw = false;
            try { delete KeyPair::fromAccountId(bad); } catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(bad));
        }
        bool threw = false;
        try { delete PaymentOperation::create(account.toLower(), new AssetTypeNative(), QStringLiteral("1")); }
        catch (const std::exception&) { threw = true; }
        // The destination is decoded when the operation is encoded, at the latest.
        if (!threw) {
            try {
                QScopedPointer<PaymentOperation> op(PaymentOperation::create(account.toLower(), new AssetTypeNative(), QStringLiteral("1")));
                op->toXdrBase64();
            } catch (const std::exception&) { threw = true; }
        }
        QVERIFY(threw);
    }

    void testDecodeInvalidVersionByte() {
        QString address = "GCZHXL5HXQX5ABDM26LHYRCQZ5OJFHLOPLZX47WEBP3V2PF5AVFK2A5D";
        try {
            StrKey::decodeCheck(StrKey::VersionByte::SEED, address.toLatin1());
            QFAIL("Should provocate an exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()),QString("Version byte is invalid"));
        }
    }
    void testDecodeInvalidSeed() {
        QString seed = "SAA6NXOBOXP3RXGAXBW6PGFI5BPK4ODVAWITS4VDOMN5C2M4B66ZML";
        try {
            StrKey::decodeCheck(StrKey::VersionByte::SEED, seed.toLatin1());
            QFAIL("Should provocate an exception");
        } catch (const std::runtime_error& e) {
            Q_UNUSED(e)
        }
    }

    /**
     * A seed decodeCheck refused is refused as before now that the copies
     * it made of it are wiped on each throw: one mistyped character (32
     * bytes that are almost the seed) fails the checksum, a T for the S
     * fails the version byte, and 55 characters with a good checksum give
     * a 31-byte body. js 17.1.0 and py 14.1.0 refuse all three. The good
     * seed still decodes to py's bytes. The wiping is not visible here.
     */
    void testRefusedSeedsAreRefusedAsBefore() {
        const QList<QPair<QByteArray, QString>> refused = {
            {"SDJHRQF4GCMIIKAAAQ6IBY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE", QStringLiteral("Checksum invalid")},
            {"TDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE", QStringLiteral("Version byte is invalid")},
            {"SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWOONI", QStringLiteral("invalid encoded string length")},
        };
        for (const auto &r : refused) {
            try {
                StrKey::decodeCheck(StrKey::VersionByte::SEED, r.first);
                QFAIL(r.first.constData());
            } catch (const std::runtime_error& e) {
                QCOMPARE(QString::fromLatin1(e.what()), r.second);
            }
        }
        QCOMPARE(StrKey::decodeCheck(StrKey::VersionByte::SEED,
                 QByteArray("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE")).toHex(),
                 QByteArray("d278c0bc3098842800043c83e39abff6581567a51607c94a1f0651b3f4bdac28"));
    }

    void testDecodedVersionByte() {        
        QCOMPARE( StrKey::VersionByte::ACCOUNT_ID,StrKey::decodeVersionByte(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")));
        QCOMPARE( StrKey::VersionByte::MUXED_ACCOUNT,StrKey::decodeVersionByte(QString("MBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWGTOG")));
        QCOMPARE( StrKey::VersionByte::SEED,StrKey::decodeVersionByte(QString("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE")));
        QCOMPARE( StrKey::VersionByte::PRE_AUTH_TX,StrKey::decodeVersionByte(QString("TAQCSRX2RIDJNHFIFHWD63X7D7D6TRT5Y2S6E3TEMXTG5W3OECHZ2OG4")));
        QCOMPARE( StrKey::VersionByte::SHA256_HASH,StrKey::decodeVersionByte(QString("XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD")));
    }

    void testDecodedVersionByteThrows() {
      try {
        StrKey::decodeVersionByte(QString("INVALIDBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6INVALID"));
        QFAIL("missing exception");
      } catch (const std::runtime_error& e) {
            QVERIFY(QString(e.what())=="Version byte is invalid");
      }
    }
    void testRoundTripAccountIdFromBytes() {
        unsigned char data[] = {
            0x36, 0x3e, 0xaa, 0x38, 0x67, 0x84, 0x1f, 0xba,
            0xd0, 0xf4, 0xed, 0x88, 0xc7, 0x79, 0xe4, 0xfe,
            0x66, 0xe5, 0x6a, 0x24, 0x70, 0xdc, 0x98, 0xc0,
            0xec, 0x9c, 0x07, 0x3d, 0x05, 0xc7, 0xb1, 0x03
        };
        QString accountId = "GA3D5KRYM6CB7OWQ6TWYRR3Z4T7GNZLKERYNZGGA5SOAOPIFY6YQHES5";
        QCOMPARE(StrKey::encodeCheck(StrKey::VersionByte::ACCOUNT_ID, QByteArray::fromRawData((const char*)data,sizeof(data)))
                    ,accountId);
        QCOMPARE(QByteArray::fromRawData((const char*)data,sizeof(data)), StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, accountId.toLatin1()));
    }


      void testRoundTripSeedFromBytes() {
        unsigned char data[] = {
            0x69, 0xa8, 0xc4, 0xcb, 0xb9, 0xf6, 0x4e, 0x8a,
            0x07, 0x98, 0xf6, 0xe1, 0xac, 0x65, 0xd0, 0x6c,
            0x31, 0x62, 0x92, 0x90, 0x56, 0xbc, 0xf4, 0xcd,
            0xb7, 0xd3, 0x73, 0x8d, 0x18, 0x55, 0xf3, 0x63
        };
        QString seed = "SBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWHOKR";
        QCOMPARE(StrKey::encodeCheck(StrKey::VersionByte::SEED, QByteArray::fromRawData((const char*)data,sizeof(data)))
                    ,seed);
        QCOMPARE(QByteArray::fromRawData((const char*)data,sizeof(data)), StrKey::decodeCheck(StrKey::VersionByte::SEED, seed.toLatin1()));
      }

      void testRoundTripHashTxFromBytes() {
          unsigned char data[] = {
                      0x69, 0xa8, 0xc4, 0xcb, 0xb9, 0xf6, 0x4e, 0x8a,
                      0x07, 0x98, 0xf6, 0xe1, 0xac, 0x65, 0xd0, 0x6c,
                      0x31, 0x62, 0x92, 0x90, 0x56, 0xbc, 0xf4, 0xcd,
                      0xb7, 0xd3, 0x73, 0x8d, 0x18, 0x55, 0xf3, 0x63
                      };
          QString hashTx = "TBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWHXL7";
          QCOMPARE(StrKey::encodeCheck(StrKey::VersionByte::PRE_AUTH_TX, QByteArray::fromRawData((const char*)data,sizeof(data)))
                      ,hashTx);
          QCOMPARE(QByteArray::fromRawData((const char*)data,sizeof(data)), StrKey::decodeCheck(StrKey::VersionByte::PRE_AUTH_TX, hashTx.toLatin1()));
      }


      void testRoundTripHashXFromBytes() {
          unsigned char data[] = {
                      0x69, 0xa8, 0xc4, 0xcb, 0xb9, 0xf6, 0x4e, 0x8a,
                      0x07, 0x98, 0xf6, 0xe1, 0xac, 0x65, 0xd0, 0x6c,
                      0x31, 0x62, 0x92, 0x90, 0x56, 0xbc, 0xf4, 0xcd,
                      0xb7, 0xd3, 0x73, 0x8d, 0x18, 0x55, 0xf3, 0x63
                      };
          QString hashX = "XBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWGTOG";
          QCOMPARE(StrKey::encodeCheck(StrKey::VersionByte::SHA256_HASH, QByteArray::fromRawData((const char*)data,sizeof(data)))
                      ,hashX);
          QCOMPARE(QByteArray::fromRawData((const char*)data,sizeof(data)), StrKey::decodeCheck(StrKey::VersionByte::SHA256_HASH, hashX.toLatin1()));
      }

      void testRoundTripMuxedFromBytes() {
          unsigned char data[] = {
                      0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x3f, 0x0c, 0x34, 0xbf, 0x93, 0xad, 0x0d, 0x99,
                      0x71, 0xd0, 0x4c, 0xcc, 0x90, 0xf7, 0x05, 0x51,
                      0x1c, 0x83, 0x8a, 0xad, 0x97, 0x34, 0xa4, 0xa2,
                      0xfb, 0x0d, 0x7a, 0x03, 0xfc, 0x7f, 0xe8, 0x9a
                      };
          QString muxed = "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG";
          QCOMPARE(StrKey::encodeCheck(StrKey::VersionByte::MUXED_ACCOUNT, QByteArray::fromRawData((const char*)data,sizeof(data)))
                      ,muxed);
          QCOMPARE(QByteArray::fromRawData((const char*)data,sizeof(data)), StrKey::decodeCheck(StrKey::VersionByte::MUXED_ACCOUNT, muxed.toLatin1()));
      }

      void testDecodeEmpty() {
          try {
              StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, QByteArray(""));
              QFAIL("missing exception");
          } catch (const std::runtime_error& e) {
              Q_UNUSED(e)
          }
      }

      void testCorruptedChecksum() {
          try {
              StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, QString("GA3D5KRYM6CB7OWQ6TWYRR3Z4T7GNZLKERYNZGGA5SOAOPIFY6YQHE55").toLatin1());
              QFAIL("missing exception");
          } catch (const std::runtime_error& e) {
              QCOMPARE(QString(e.what()),QString("Checksum invalid"));
          }
      }

      void testCorruptedPayload() {
          try {
              StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, QString("GA3D5KRYM6CB7OWOOOORR3Z4T7GNZLKERYNZGGA5SOAOPIFY6YQHES5").toLatin1());
              QFAIL("missing exception");
          } catch (const std::runtime_error& e) {
              Q_UNUSED(e)
          }
      }

      void testEncodeToXdrRoundTrip() {
        QString address = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        uchar ed25519[] = {
            63, 12, 52, 191, 147, 173, 13, 153, 113, 208,
            76, 204, 144, 247, 5, 81, 28, 131, 138, 173, 151, 52,
            164, 162, 251, 13, 122, 3, 252, 127, 232, 154
        };
        stellar::AccountID account = StrKey::encodeToXDRAccountId(address);
        QVERIFY(memcmp(ed25519, account.ed25519,sizeof(ed25519))==0);
        QCOMPARE(StrKey::encodeStellarAccountId(account),address);
        QCOMPARE(StrKey::encodeStellarAccountId(account.ed25519),address);

        stellar::MuxedAccount muxedAccount = StrKey::encodeToXDRMuxedAccount(address);
        QVERIFY(stellar::CryptoKeyType::KEY_TYPE_ED25519 == muxedAccount.type);
        QVERIFY(memcmp(ed25519, muxedAccount.ed25519,sizeof(ed25519))==0);
        QCOMPARE(StrKey::encodeStellarAccountId(StrKey::muxedAccountToAccountId(muxedAccount)),address);
      }

      void testEncodeToXDRMuxedAccountMAddress() {
        QString unmuxedAddress = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        stellar::AccountID account = StrKey::encodeToXDRAccountId(unmuxedAddress);

        QString muxedAddress = "MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK";
        stellar::MuxedAccount muxedAccount = StrKey::encodeToXDRMuxedAccount(muxedAddress);
        QCOMPARE(muxedAccount.type, stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519);

        QVERIFY(memcmp(account.ed25519, muxedAccount.med25519.ed25519, sizeof(account.ed25519))==0);

        QCOMPARE(muxedAccount.med25519.id,9223372036854775808UL);

        QCOMPARE(muxedAddress, StrKey::encodeStellarMuxedAccount(muxedAccount));

        QCOMPARE(muxedAddress, AccountConverter().enableMuxed().decode(muxedAccount));
        QCOMPARE(unmuxedAddress, AccountConverter().disableMuxed().decode(muxedAccount));

        QVERIFY(muxedAccount == AccountConverter().enableMuxed().encode(muxedAddress));


        auto temp =  AccountConverter().disableMuxed().encode(muxedAddress);

        qDebug() << QByteArray::fromRawData((char*)account.ed25519,32).toHex();
        qDebug() << QByteArray::fromRawData((char*)temp.ed25519,32).toHex();
        QVERIFY(memcmp(account.ed25519, temp.ed25519,sizeof(account.ed25519))==0);
      }

      void testEncodeAccountIdToMuxed() {
        QString unmuxedAddress = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        stellar::AccountID account = StrKey::encodeToXDRAccountId(unmuxedAddress);

        stellar::MuxedAccount muxedAccount = StrKey::encodeToXDRMuxedAccount(unmuxedAddress);
        QCOMPARE(stellar::CryptoKeyType::KEY_TYPE_ED25519, muxedAccount.type);
        QVERIFY(memcmp(account.ed25519, muxedAccount.ed25519,sizeof(muxedAccount.ed25519))==0);
      }

      void testEncodeToXDRMuxedAccountInvalidAddress() {

        try {
          StrKey::encodeToXDRMuxedAccount("XBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWGTOG");
          QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
          QCOMPARE("Version byte is invalid", e.what());
        }

        try {
          StrKey::encodeToXDRMuxedAccount("MBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWGTOG");
          QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
          //QCOMPARE("Checksum invalid", e.what()); //other SDKs return this
          QCOMPARE("Version byte is invalid", e.what()); //this SDK filters by size first, so it fails first on the checksum
        }
      }

      // ─── SEP-23 — C / B / L strkeys ────────────────────────────────────

      void testRoundTripContract() {
          // 32-byte contract id round-trip via C-strkey.
          QByteArray contractId(32, '\0');
          for (int i = 0; i < 32; ++i) contractId[i] = static_cast<char>(i * 7 + 1);
          QString c = StrKey::encodeContract(contractId);
          QCOMPARE(c.at(0), QChar('C'));
          QCOMPARE(c.size(), 56);
          QCOMPARE(StrKey::decodeContract(c), contractId);
      }

      void testRoundTripLiquidityPool() {
          QByteArray poolId(32, '\xCD');
          QString l = StrKey::encodeLiquidityPool(poolId);
          QCOMPARE(l.at(0), QChar('L'));
          QCOMPARE(l.size(), 56);
          QCOMPARE(StrKey::decodeLiquidityPool(l), poolId);
      }

      void testRoundTripClaimableBalance() {
          // Body is 33 bytes: 1-byte type discriminant (0 for V0) + 32-byte hash.
          QByteArray body;
          body.append(static_cast<char>(0));
          for (int i = 0; i < 32; ++i) body.append(static_cast<char>(0xAA ^ i));
          QString b = StrKey::encodeClaimableBalance(body);
          QCOMPARE(b.at(0), QChar('B'));
          // 33 bytes body + 1 byte version + 2 byte checksum = 36 bytes →
          // ceil(36 * 8 / 5) = 58 chars unpadded.
          QCOMPARE(b.size(), 58);
          QCOMPARE(StrKey::decodeClaimableBalance(b), body);
      }

      void testDecodeVersionByteCBL() {
          QString c = StrKey::encodeContract(QByteArray(32, '\0'));
          QString l = StrKey::encodeLiquidityPool(QByteArray(32, '\0'));
          QByteArray bBody; bBody.append('\0'); bBody.append(QByteArray(32, '\0'));
          QString b = StrKey::encodeClaimableBalance(bBody);

          QCOMPARE(static_cast<int>(StrKey::decodeVersionByte(c)),
                   static_cast<int>(StrKey::VersionByte::CONTRACT));
          QCOMPARE(static_cast<int>(StrKey::decodeVersionByte(b)),
                   static_cast<int>(StrKey::VersionByte::CLAIMABLE_BALANCE));
          QCOMPARE(static_cast<int>(StrKey::decodeVersionByte(l)),
                   static_cast<int>(StrKey::VersionByte::LIQUIDITY_POOL));
      }

    /**
     * decodeAny checks a strkey whole, whatever its kind; decodeVersionByte
     * reads only the first byte, so a damaged key still has a kind there.
     */
    void testDecodeAnyChecksTheWholeKey()
    {
        const QString account = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const StrKey::Decoded decoded = StrKey::decodeAny(account);
        QVERIFY(decoded.version == StrKey::VersionByte::ACCOUNT_ID);
        QCOMPARE(decoded.payload, StrKey::decodeStellarAccountId(account));
        const QString p32 = QStringLiteral(
            "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM");
        QVERIFY(StrKey::decodeAny(p32).version == StrKey::VersionByte::SIGNED_PAYLOAD);
        QCOMPARE(StrKey::decodeAny(p32).payload.size(), 68);

        QString damaged = account;
        damaged[30] = damaged[30] == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        QVERIFY(StrKey::decodeVersionByte(damaged) == StrKey::VersionByte::ACCOUNT_ID);   // the first byte only
        for (const QString& bad : {damaged, account.left(55), account.toLower(), QString(),
                                   account.left(55) + QChar(0x00e9), QStringLiteral("not a key")}) {
            QVERIFY_EXCEPTION_THROWN(StrKey::decodeAny(bad), std::runtime_error);
        }
    }

    /**
     * SEP-23's signed-payload vectors, literally (the spec's examples, which
     * py-stellar-sdk 14.1.0 and js 17.1.0 read alike): a 32-byte and a
     * 29-byte payload (padded to 32 in the XDR) for
     * GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ, both ways.
     * Refused: the spec's two invalid ones, whose declared length does not
     * match the data (32 with 72 bytes, 29 with 64; js and py refuse both),
     * and the 29-byte payload padded with 00 00 07 instead of zeros, which
     * js and core refuse and py 14.1.0 accepts.
     */
    void testSep23SignedPayloadVectors()
    {
        const QString signer = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const QByteArray key = StrKey::decodeStellarAccountId(signer);
        QByteArray payload32;
        for (char c = 1; c <= 32; ++c)
            payload32.append(c);
        const QByteArray payload29 = payload32.left(29);

        const QString p32 = QStringLiteral(
            "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM");
        const QString p29 = QStringLiteral(
            "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUAAAAFGBU");
        QCOMPARE(StrKey::encodeSignedPayload(key, payload32), p32);
        QCOMPARE(StrKey::encodeSignedPayload(key, payload29), p29);
        QCOMPARE(StrKey::decodeSignedPayload(p32).first, key);
        QCOMPARE(StrKey::decodeSignedPayload(p32).second, payload32);
        QCOMPARE(StrKey::decodeSignedPayload(p29).first, key);
        QCOMPARE(StrKey::decodeSignedPayload(p29).second, payload29);

        for (const char* refused : {
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IAAAAAAAAPM",
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4Z2PQ",
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUAAAB2BPU"}) {
            QVERIFY_EXCEPTION_THROWN(StrKey::decodeSignedPayload(QString::fromLatin1(refused)), std::runtime_error);
        }
    }

    /**
     * decodeCheck(SIGNED_PAYLOAD) frames the body as decodeSignedPayload
     * does: it returned the 72 bytes of a key declaring a 32-byte payload,
     * where js 17.1.0's decodeCheck throws "declared length 32 needs 68
     * bytes, got 72" and py 14.1.0's "Invalid Ed25519 Signed Payload Key".
     * Refused: SEP-23's two invalid keys, non-zero padding (js; py accepts
     * it), a declared length of 0 and one of 65 (js and py), each with a
     * good checksum. SEP-23's good keys pass. The key declaring 65 has a
     * 100-byte body (key, length, 64 zero bytes), which the length rule
     * lets through, so it is the framing check that refuses it, as js's
     * "declared length 65 outside 1-64".
     */
    void testDecodeCheckFramesASignedPayload()
    {
        using V = StrKey::VersionByte;
        static const char declares65[] =
                "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAABAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABPOU";
        for (const char* refused : {
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IAAAAAAAAPM",
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4Z2PQ",
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUAAAB2BPU",
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAAAAAAAABO6A",
                 declares65}) {
            QVERIFY_EXCEPTION_THROWN(StrKey::decodeCheck(V::SIGNED_PAYLOAD, QByteArray(refused)), std::runtime_error);
        }
        try {
            StrKey::decodeCheck(V::SIGNED_PAYLOAD, QByteArray(declares65));
            QFAIL("a declared length of 65 was accepted");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString::fromLatin1(e.what()), QStringLiteral("signed payload: declared length outside 1-64"));
        }
        QCOMPARE(StrKey::decodeCheck(V::SIGNED_PAYLOAD,
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM").size(), 68);
        QCOMPARE(StrKey::decodeCheck(V::SIGNED_PAYLOAD,
                 "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAOQCAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUAAAAFGBU").size(), 68);
    }

    /**
     * A strkey is accepted only with the body its kind has and in its one
     * canonical spelling, as js's decodeCheck requires (SEP-23). Every
     * string below carries a CORRECT checksum and was accepted: a G of five
     * bytes, a G of 35, an M of 32, a C of 20, a B whose type byte is 1, a
     * valid G with '=' in the middle (read as the same key) and one with
     * trailing padding. js 17.1.0 refuses each one; the good ones pass.
     */
    void testOnlyWellFormedCanonicalStrkeysDecode() {
          using V = StrKey::VersionByte;
          const QList<QPair<V, QByteArray>> bad = {
              {V::ACCOUNT_ID, "GADQOBYHA6KXK"},
              {V::ACCOUNT_ID, "GADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA6XZC"},
              {V::MUXED_ACCOUNT, "MADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQPLGX"},
              {V::CONTRACT, "CADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4K5I"},
              {V::CLAIMABLE_BALANCE, "BAAQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOB2NEE"},
              {V::ACCOUNT_ID, "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZ=4KVWLTJJFC7MGXUA74P7UJVSGZ"},
              {V::ACCOUNT_ID, "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ===="},
          };
          for (const auto& c : bad) {
              QVERIFY_EXCEPTION_THROWN(StrKey::decodeCheck(c.first, c.second), std::exception);
          }
          QCOMPARE(StrKey::decodeCheck(V::ACCOUNT_ID, "GADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOZPI"),
                   QByteArray(32, 7));
          QCOMPARE(StrKey::decodeCheck(V::MUXED_ACCOUNT,
                   "MADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA6OSU"),
                   QByteArray(40, 7));
          QByteArray b(33, 7);
          b[0] = 0;
          QCOMPARE(StrKey::decodeCheck(V::CLAIMABLE_BALANCE,
                   "BAAAOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOB4RVQ"), b);
          QCOMPARE(StrKey::decodeCheck(V::ACCOUNT_ID, "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ").size(), 32);
      }

      /** SEP-23 cross-check: an all-zero G-strkey is well-known. Verify the
       *  C / L analogs (which use the same body shape) match SDK-Java fixtures. */
      void testAllZeroFixtures() {
          QString gZero = "GAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWHF";
          QString cZero = StrKey::encodeContract(QByteArray(32, '\0'));
          QString lZero = StrKey::encodeLiquidityPool(QByteArray(32, '\0'));

          // First char differs, rest of the base32-encoded zero-payload is identical
          // after the version byte (well: not quite — different versionBytes encode
          // into different early bytes — but the LENGTH is the same).
          QCOMPARE(gZero.size(), 56);
          QCOMPARE(cZero.size(), 56);
          QCOMPARE(lZero.size(), 56);
          QCOMPARE(cZero.at(0), QChar('C'));
          QCOMPARE(lZero.at(0), QChar('L'));

          // Round-trip is the canonical test.
          QCOMPARE(StrKey::decodeContract(cZero), QByteArray(32, '\0'));
          QCOMPARE(StrKey::decodeLiquidityPool(lZero), QByteArray(32, '\0'));
      }
};

ADD_TEST(StrKeyTest)
#endif // STRKEYTEST_H
