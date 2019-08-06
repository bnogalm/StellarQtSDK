#ifndef UTILTEST_H
#define UTILTEST_H
#include <QObject>

#include "src/util.h"
#include "src/keypair.h"
#include "src/transaction.h"

#include <QtTest>
#include "testcollector.h"


class UtilTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testBuildChallengeTx() {
        auto serverKeypair = KeyPair::random();
        auto clientKeypair = KeyPair::random();

        // infinite timebound
        auto tx = Util::buildChallengeTx(serverKeypair,clientKeypair->getAccountId(),"SDF",0);
        auto xdr = tx->toEnvelopeXdr();
        delete tx;

        QCOMPARE(xdr.tx.seqNum,0);
        QCOMPARE(xdr.tx.fee,static_cast<quint32>(100));
        QCOMPARE(xdr.tx.operations.value.size(),1);
        QCOMPARE(xdr.tx.timeBounds.value.minTime,static_cast<quint64>(0));
        QCOMPARE(xdr.tx.timeBounds.value.maxTime,static_cast<quint64>(0));
        auto op = xdr.tx.operations.value.at(0);

        QCOMPARE(op.type,stellar::OperationType::MANAGE_DATA);
        QCOMPARE(op.operationManageData.dataName.binary(),QByteArray("SDF auth"));
        //48 random bytes enconded in base64
        QCOMPARE(QByteArray::fromBase64(op.operationManageData.dataValue.value.binary()
                                        ,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::OmitTrailingEquals).size(),48);


        // 5 minutes timebound

        tx = Util::buildChallengeTx(serverKeypair,clientKeypair->getAccountId(),"SDF1",5*60);        
        xdr = tx->toEnvelopeXdr();
        delete tx;
        QCOMPARE(xdr.tx.seqNum,0);
        QCOMPARE(xdr.tx.fee,static_cast<quint32>(100));
        QCOMPARE(xdr.tx.operations.value.size(),1);
        QCOMPARE(xdr.tx.timeBounds.value.maxTime- xdr.tx.timeBounds.value.minTime,static_cast<quint64>(300));
        op = xdr.tx.operations.value.at(0);
        QCOMPARE(op.type,stellar::OperationType::MANAGE_DATA);
        QCOMPARE(op.operationManageData.dataName.binary(),QByteArray("SDF1 auth"));
        //48 random bytes enconded in base64
        QCOMPARE(QByteArray::fromBase64(op.operationManageData.dataValue.value.binary()
                                        ,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::OmitTrailingEquals).size(),48);

    }
};

ADD_TEST(UtilTest)
#endif // UTILTEST_H
