#ifndef LEDGERKEYENUMRANGETEST_H
#define LEDGERKEYENUMRANGETEST_H

#include <QObject>
#include <QtTest>
#include <QtEndian>
#include "../testcollector.h"

#include "../../xdr/sorobantxdata.h"
#include "../../src/scval/scv.h"

/**
 * The enums inside a Soroban ledger key are closed. A contract data key's
 * durability is TEMPORARY (0) or PERSISTENT (1), and a config setting key
 * names one of the ConfigSettingIDs stellar-xdr curr defines (0..20). Both
 * were read and written as any int32: a footprint with durability 3, from a
 * faulty RPC or bytes put together by hand, was re-encoded unchanged, passed
 * the byte-for-byte check and was signed. py-stellar-sdk ("3 is not a valid
 * ContractDataDurability"), js and core refuse it; a differential fuzz of
 * 59,551 mutations of real data found no other value this SDK took and py
 * did not.
 */
class LedgerKeyEnumRangeTest: public QObject
{
    Q_OBJECT

    static QByteArray encoded(const stellar::SorobanLedgerKey& key, QDataStream::Status* status = nullptr)
    {
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out << key;
        if (status)
            *status = out.status();
        return bytes;
    }

    static QDataStream::Status decodedStatus(QByteArray bytes, stellar::SorobanLedgerKey* key = nullptr)
    {
        stellar::SorobanLedgerKey read;
        QDataStream in(&bytes, QIODevice::ReadOnly);
        in >> read;
        if (key)
            *key = read;
        return in.status();
    }

    static stellar::SorobanLedgerKey contractData()
    {
        stellar::SorobanLedgerKey key;
        key.type = stellar::LedgerEntryType::CONTRACT_DATA;
        key.contractData.contract =
            Scv::toAddress(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC")).address;
        key.contractData.key = Scv::toLedgerKeyContractInstance();
        key.contractData.durability = stellar::ContractDataDurability::PERSISTENT;
        return key;
    }

private slots:
    void testContractDataDurabilityIsZeroOrOne()
    {
        QDataStream::Status status = QDataStream::Ok;
        QByteArray bytes = encoded(contractData(), &status);
        QCOMPARE(status, QDataStream::Ok);
        // type (4) + SCAddress contract (4 + 32) + SCV_LEDGER_KEY_CONTRACT_INSTANCE (4) + durability (4)
        QCOMPARE(bytes.size(), 48);
        QCOMPARE(bytes.right(4), QByteArray::fromHex("00000001"));
        stellar::SorobanLedgerKey read;
        QCOMPARE(decodedStatus(bytes, &read), QDataStream::Ok);
        QCOMPARE(read.contractData.durability, stellar::ContractDataDurability::PERSISTENT);

        bytes.replace(bytes.size() - 4, 4, QByteArray::fromHex("00000000"));
        QCOMPARE(decodedStatus(bytes, &read), QDataStream::Ok);
        QCOMPARE(read.contractData.durability, stellar::ContractDataDurability::TEMPORARY);
        for (const char* bad : {"00000002", "00000003", "7fffffff", "80000000", "ffffffff"}) {
            bytes.replace(bytes.size() - 4, 4, QByteArray::fromHex(bad));
            QVERIFY2(decodedStatus(bytes) != QDataStream::Ok, bad);
        }

        stellar::SorobanLedgerKey invalid = contractData();
        invalid.contractData.durability = static_cast<stellar::ContractDataDurability>(3);
        encoded(invalid, &status);
        QVERIFY(status != QDataStream::Ok);
    }

    void testConfigSettingIdIsOneStellarXdrDefines()
    {
        stellar::SorobanLedgerKey key;
        key.type = stellar::LedgerEntryType::CONFIG_SETTING;
        for (const qint32 id : {0, 10, stellar::MAX_CONFIG_SETTING_ID}) {
            key.configSettingID = id;
            QDataStream::Status status = QDataStream::Ok;
            const QByteArray bytes = encoded(key, &status);
            QCOMPARE(status, QDataStream::Ok);
            QCOMPARE(bytes.size(), 8);
            stellar::SorobanLedgerKey read;
            QCOMPARE(decodedStatus(bytes, &read), QDataStream::Ok);
            QCOMPARE(read.configSettingID, id);
        }
        QCOMPARE(stellar::MAX_CONFIG_SETTING_ID, 20);   // CONFIG_SETTING_FREEZE_BYPASS_TXS_DELTA
        for (const qint32 id : {21, 1000, -1}) {
            QByteArray bytes = QByteArray::fromHex("00000008");
            QByteArray value(4, '\0');
            qToBigEndian<qint32>(id, reinterpret_cast<uchar*>(value.data()));
            QVERIFY2(decodedStatus(bytes + value) != QDataStream::Ok, qPrintable(QString::number(id)));
            key.configSettingID = id;
            QDataStream::Status status = QDataStream::Ok;
            encoded(key, &status);
            QVERIFY2(status != QDataStream::Ok, qPrintable(QString::number(id)));
        }
    }
};

ADD_TEST(LedgerKeyEnumRangeTest)

#endif // LEDGERKEYENUMRANGETEST_H
