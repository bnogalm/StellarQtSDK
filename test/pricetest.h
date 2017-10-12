#ifndef PRICETEST_H
#define PRICETEST_H
#include <QObject>
#include <QtTest>
#include "testcollector.h"
#include "src/price.h"

class PriceTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testFromDouble() {
        QVERIFY(Price::fromString("0")->equals(new Price(0,1)));
        QVERIFY(Price::fromString("0.1")->equals( new Price(1,10)));
        QVERIFY(Price::fromString("0.01")->equals( new Price(1,100)));
        QVERIFY(Price::fromString("0.001")->equals( new Price(1,1000)));
        QVERIFY(Price::fromString("543.01793")->equals( new Price(54301793,100000)));
        QVERIFY(Price::fromString("319.69983")->equals( new Price(31969983,100000)));
        QVERIFY(Price::fromString("0.93")->equals( new Price(93,100)));
        QVERIFY(Price::fromString("0.5")->equals( new Price(1,2)));
        QVERIFY(Price::fromString("1.730")->equals( new Price(173,100)));
        QVERIFY(Price::fromString("0.85334384")->equals( new Price(5333399,6250000)));
        QVERIFY(Price::fromString("5.5")->equals( new Price(11,2)));
        QVERIFY(Price::fromString("2.72783")->equals( new Price(272783,100000)));
        QVERIFY(Price::fromString("638082.0")->equals( new Price(638082,1)));
        QVERIFY(Price::fromString("2.93850088")->equals( new Price(36731261,12500000)));
        QVERIFY(Price::fromString("58.04")->equals( new Price(1451,25)));
        QVERIFY(Price::fromString("41.265")->equals( new Price(8253,200)));
        QVERIFY(Price::fromString("5.1476")->equals( new Price(12869,2500)));
        QVERIFY(Price::fromString("95.14")->equals( new Price(4757,50)));
        QVERIFY(Price::fromString("0.74580")->equals( new Price(3729,5000)));
        QVERIFY(Price::fromString("4119.0")->equals( new Price(4119,1)));
        QVERIFY(Price::fromString("1073742464.5")->equals( new Price(1073742464,1)));
        QVERIFY(Price::fromString("1635962526.2")->equals( new Price(1635962526,1)));
        QVERIFY(Price::fromString("2147483647")->equals( new Price(2147483647,1)));
    }

};

ADD_TEST(PriceTest)
#endif // PRICETEST_H
