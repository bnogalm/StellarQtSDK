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
        } catch (FormatException e) {

        }
    }
    void testDecodeInvalidSeed() {
        QString seed = "SAA6NXOBOXP3RXGAXBW6PGFI5BPK4ODVAWITS4VDOMN5C2M4B66ZML";
        try {
            StrKey::decodeCheck(StrKey::VersionByte::SEED, seed.toLatin1());
            QFAIL("Should provocate an exception");
        } catch (FormatException e) {}
    }

};

ADD_TEST(StrKeyTest)
#endif // STRKEYTEST_H
