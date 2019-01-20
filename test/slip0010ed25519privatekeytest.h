#ifndef SLIP0010ED25519PRIVATEKEYTEST_H
#define SLIP0010ED25519PRIVATEKEYTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"
#include "slip10.h"
#include <QByteArray>

/**
 * Test cases from SLIP-0010 https://github.com/satoshilabs/slips/blob/master/slip-0010.md
 * Just relevant cases, Ed25519, private key
 */
class SLIP0010Ed25519PrivateKeyTest: public QObject
{
    Q_OBJECT

    QString case1Seed = "000102030405060708090a0b0c0d0e0f";

    QString case2Seed = "fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4"
                        "b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542";


    static QString deriveEd25519PrivateKey(QString seed, QVector<int> indexes = QVector<int>()) {
        return QString::fromLatin1(SLIP10::deriveEd25519PrivateKey(QByteArray::fromHex(seed.toLatin1()), indexes).toHex());
    }

private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void case1_m() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed),QString("2b4be7f19ee27bbf30c667b642d5f4aa69fd169872f8fc3059c08ebae2eb19e7"));
    }

    void case1_m_0h() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>() << 0),QString("68e0fe46dfb67e368c75379acec591dad19df3cde26e63b93a8e704f1dade7a3"));
    }

    void case1_m_0h_1h() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>() << 0<< 1),QString("b1d0bad404bf35da785a64ca1ac54b2617211d2777696fbffaf208f746ae84f2"));
    }

    void case1_m_0h_1h_2h() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>() << 0 << 1 << 2),QString("92a5b23c0b8a99e37d07df3fb9966917f5d06e02ddbd909c7e184371463e9fc9"));
    }

    void case1_m_0h_1h_2h_2h() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>() << 0 << 1 << 2 << 2),QString("30d1dc7e5fc04c31219ab25a27ae00b50f6fd66622f6e9c913253d6511d1e662"));
    }

    void case1_m_0h_1h_2h_1000000000h() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>() << 0 << 1 << 2 << 2 << 1000000000),QString("8f94d394a8e8fd6b1bc2f3f49f5c47e385281d5c17e65324b0f62483e37e8793"));
    }

    void case1_m_0h_already_hardened() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed,QVector<int>()<< 0),deriveEd25519PrivateKey(case1Seed, QVector<int>()<< static_cast<int>(0x80000000)));
    }

    void case1_m_0h_1h_already_hardened() {
        QCOMPARE(deriveEd25519PrivateKey(case1Seed, QVector<int>()<< 1),deriveEd25519PrivateKey(case1Seed, QVector<int>()<< static_cast<int>(0x80000001)));
    }

    void case2_m() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed),QString("171cb88b1b3c1db25add599712e36245d75bc65a1a5c9e18d76f9f2b1eab4012"));
    }

    void case2_m_0h() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed, QVector<int>()<< 0),QString("1559eb2bbec5790b0c65d8693e4d0875b1747f4970ae8b650486ed7470845635"));
    }

    void case2_m_0h_2147483647h() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed, QVector<int>()<< 0<< 2147483647),QString("ea4f5bfe8694d8bb74b7b59404632fd5968b774ed545e810de9c32a4fb4192f4"));
    }

    void case2_m_0h_2147483647h_1h() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed, QVector<int>()<< 0<< 2147483647<< 1),QString("3757c7577170179c7868353ada796c839135b3d30554bbb74a4b1e4a5a58505c"));
    }

    void case2_m_0h_2147483647h_1h_2147483646h() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed, QVector<int>()<< 0<< 2147483647<< 1<< 2147483646),QString("5837736c89570de861ebc173b1086da4f505d4adb387c6a1b1342d5e4ac9ec72"));
    }

    void case2_m_0h_2147483647h_1h_2147483646h_2h() {
        QCOMPARE(deriveEd25519PrivateKey(case2Seed, QVector<int>()<< 0<< 2147483647<< 1<< 2147483646<< 2),QString("551d333177df541ad876a60ea71f00447931c0a9da16f227c11ea080d7391b8d"));
    }

};

ADD_TEST(SLIP0010Ed25519PrivateKeyTest)

#endif // SLIP0010ED25519PRIVATEKEYTEST_H
