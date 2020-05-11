#ifndef STRKEYTEST_H
#define STRKEYTEST_H
#include <QObject>
#include "src/strkey.h"
#include "src/util.h"

#include <QtTest>
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

    void testDecodeInvalidVersionByte() {
        QString address = "GCZHXL5HXQX5ABDM26LHYRCQZ5OJFHLOPLZX47WEBP3V2PF5AVFK2A5D";
        try {
            StrKey::decodeCheck(StrKey::VersionByte::SEED, address.toLatin1());
            QFAIL("Should provocate an exception");
        } catch (std::runtime_error e) {
            QCOMPARE(QString(e.what()),QString("Version byte is invalid"));
        }
    }
    void testDecodeInvalidSeed() {
        QString seed = "SAA6NXOBOXP3RXGAXBW6PGFI5BPK4ODVAWITS4VDOMN5C2M4B66ZML";
        try {
            StrKey::decodeCheck(StrKey::VersionByte::SEED, seed.toLatin1());
            QFAIL("Should provocate an exception");
        } catch (std::runtime_error e) {

        }
    }


    void testDecodedVersionByte() {
        QCOMPARE( StrKey::VersionByte::MUXED_ACCOUNT,StrKey::decodeVersionByte("MBU2RRGLXH3E5CQHTD3ODLDF2BWDCYUSSBLLZ5GNW7JXHDIYKXZWGTOG"));
        QCOMPARE( StrKey::VersionByte::ACCOUNT_ID,StrKey::decodeVersionByte("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));
        QCOMPARE( StrKey::VersionByte::SEED,StrKey::decodeVersionByte("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE"));
        QCOMPARE( StrKey::VersionByte::PRE_AUTH_TX,StrKey::decodeVersionByte("TAQCSRX2RIDJNHFIFHWD63X7D7D6TRT5Y2S6E3TEMXTG5W3OECHZ2OG4"));
        QCOMPARE( StrKey::VersionByte::SHA256_HASH,StrKey::decodeVersionByte("XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD"));
    }


    void testDecodedVersionByteThrows() {
      try {
        StrKey::decodeVersionByte("INVALIDBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6INVALID");
        QFAIL("missing exception");
      } catch (std::runtime_error e) {
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
          } catch (std::runtime_error e) {
          }
      }

      void testCorruptedChecksum() {
          try {
              StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, QString("GA3D5KRYM6CB7OWQ6TWYRR3Z4T7GNZLKERYNZGGA5SOAOPIFY6YQHE55").toLatin1());
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Checksum invalid"));
          }
      }

      void testCorruptedPayload() {
          try {
              StrKey::decodeCheck(StrKey::VersionByte::ACCOUNT_ID, QString("GA3D5KRYM6CB7OWOOOORR3Z4T7GNZLKERYNZGGA5SOAOPIFY6YQHES5").toLatin1());
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
          }
      }


      void testMuxedDecodeErrors() {
          // non-canonical representation due to extra character
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOGA").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Encoded char array has leftover character."));
          }

          // non-canonical representation due to leftover bits set to 1 (some of the test strkeys are too short for a muxed account

          // 1 unused bit (length 69)
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOH").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));
          }


          // 4 unused bits (length 68)

          // 'B' is equivalent to 0b00001
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNB").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));
          }

          // 'C' is equivalent to 0b00010
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNC").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));
          }

          // 'E' is equivalent to 0b00100
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNE").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));
          }

          // 'I' is equivalent to 0b01000
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNI").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));

          }
          // '7' is equivalent to 0b11111
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKN7").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));

          }
          // '6' is equivalent to 0b11110
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKN6").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));

          }
          // '4' is equivalent to 0b11100
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKN4").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));

          }

          // 'Y' is equivalent to 0b11000
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNY").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Unused bits should be set to 0."));

          }
          // 'Q' is equivalent to 0b10000, so there should be no unused bits error
          // However, there will be a checksum error
          try {
              StrKey::decodeCheck(
                          StrKey::VersionByte::MUXED_ACCOUNT,
                          QString("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNQ").toLatin1()
                          );
              QFAIL("missing exception");
          } catch (std::runtime_error e) {
              QCOMPARE(QString(e.what()),QString("Checksum invalid"));
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
        QCOMPARE(StrKey::encodeStellarMuxedAccount(muxedAccount),address);

        QString muxedAddress = "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG";
        muxedAccount = StrKey::encodeToXDRMuxedAccount(muxedAddress);
        QVERIFY(stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519== muxedAccount.type);
        QVERIFY(memcmp(ed25519, muxedAccount.med25519.ed25519, sizeof(ed25519))==0);

        QCOMPARE(muxedAccount.med25519.id,9223372036854775808UL);
        QCOMPARE(StrKey::encodeStellarMuxedAccount(muxedAccount),muxedAddress);

        muxedAddress = "MAAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITLVL6";
        muxedAccount = StrKey::encodeToXDRMuxedAccount(muxedAddress);
        QVERIFY(stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519== muxedAccount.type);
        QVERIFY(memcmp(ed25519, muxedAccount.med25519.ed25519,sizeof(ed25519))==0);
        QVERIFY(muxedAccount.med25519.id==0UL);
        QCOMPARE(StrKey::encodeStellarMuxedAccount(muxedAccount),muxedAddress);
      }

};

ADD_TEST(StrKeyTest)
#endif // STRKEYTEST_H
