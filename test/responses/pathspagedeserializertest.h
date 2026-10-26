#ifndef PATHSPAGEDESERIALIZERTEST_H
#define PATHSPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/page.h"
#include "../../src/responses/pathresponse.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
class PathsPageDeserializerTest: public QObject
{
    Q_OBJECT

public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<PathResponse> pathsPage;
        pathsPage.loadFromJson(json);

        QVERIFY(pathsPage.getNextPage()==nullptr);

        QCOMPARE(pathsPage.get(0).getDestinationAmount(), QString("20.0000000"));
        QVERIFY(pathsPage.get(0).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));
        QCOMPARE(pathsPage.get(0).getPath().size(), 0);
        QCOMPARE(pathsPage.get(0).getSourceAmount(), QString("30.0000000"));
        QVERIFY(pathsPage.get(0).getSourceAsset().equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));

        QCOMPARE(pathsPage.get(1).getDestinationAmount(), QString("50.0000000"));
        QVERIFY(pathsPage.get(1).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GBFMFKDUFYYITWRQXL4775CVUV3A3WGGXNJUAP4KTXNEQ2HG7JRBITGH")))));
        QCOMPARE(pathsPage.get(1).getPath().size(), 1);
        QVERIFY(pathsPage.get(1).getPath().at(0)->equals( Asset::createNonNativeAsset("GBP", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));
        QCOMPARE(pathsPage.get(1).getSourceAmount(), QString("60.0000000"));
        QVERIFY(pathsPage.get(1).getSourceAsset().equals( Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GBRAOXQDNQZRDIOK64HZI4YRDTBFWNUYH3OIHQLY4VEK5AIGMQHCLGXI")))));

        QCOMPARE(pathsPage.get(2).getDestinationAmount(), QString("200.0000000"));
        QVERIFY(pathsPage.get(2).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GBRCOBK7C7UE72PB5JCPQU3ZI45ZCEM7HKQ3KYV3YD3XB7EBOPBEDN2G")))));
        QCOMPARE(pathsPage.get(2).getPath().size(), 2);
        QVERIFY(pathsPage.get(2).getPath().at(0)->equals( Asset::createNonNativeAsset("GBP", KeyPair::fromAccountId(QString("GAX7B3ZT3EOZW5POAMV4NGPPKCYUOYW2QQDIAF23JAXF72NMGRYPYOPM")))));
        QVERIFY(pathsPage.get(2).getPath().at(1)->equals( Asset::createNonNativeAsset("PLN", KeyPair::fromAccountId(QString("GACWIA2XGDFWWN3WKPX63JTK4S2J5NDPNOIVYMZY6RVTS7LWF2VHZLV3")))));
        QCOMPARE(pathsPage.get(2).getSourceAmount(), QString("300.0000000"));
        QVERIFY(pathsPage.get(2).getSourceAsset().equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GC7J5IHS3GABSX7AZLRINXWLHFTL3WWXLU4QX2UGSDEAIAQW2Q72U3KH")))));
    }
private:
    const QByteArray json = "{\n"
            "    \"_embedded\": {\n"
            "        \"records\": [\n"
            "            {\n"
            "                \"destination_amount\": \"20.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [],\n"
            "                \"source_amount\": \"30.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            },\n"
            "            {\n"
            "                \"destination_amount\": \"50.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GBFMFKDUFYYITWRQXL4775CVUV3A3WGGXNJUAP4KTXNEQ2HG7JRBITGH\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [\n"
            "                    {\n"
            "                        \"asset_code\": \"GBP\",\n"
            "                        \"asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    }\n"
            "                ],\n"
            "                \"source_amount\": \"60.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GBRAOXQDNQZRDIOK64HZI4YRDTBFWNUYH3OIHQLY4VEK5AIGMQHCLGXI\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            },\n"
            "            {\n"
            "                \"destination_amount\": \"200.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GBRCOBK7C7UE72PB5JCPQU3ZI45ZCEM7HKQ3KYV3YD3XB7EBOPBEDN2G\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [\n"
            "                    {\n"
            "                        \"asset_code\": \"GBP\",\n"
            "                        \"asset_issuer\": \"GAX7B3ZT3EOZW5POAMV4NGPPKCYUOYW2QQDIAF23JAXF72NMGRYPYOPM\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    },\n"
            "                    {\n"
            "                        \"asset_code\": \"PLN\",\n"
            "                        \"asset_issuer\": \"GACWIA2XGDFWWN3WKPX63JTK4S2J5NDPNOIVYMZY6RVTS7LWF2VHZLV3\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    }\n"
            "                ],\n"
            "                \"source_amount\": \"300.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GC7J5IHS3GABSX7AZLRINXWLHFTL3WWXLU4QX2UGSDEAIAQW2Q72U3KH\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            }        ]\n"
            "    },\n"
            "    \"_links\": {\n"
            "        \"self\": {\n"
            "            \"href\": \"/paths\"\n"
            "        }\n"
            "    }\n"
            "}";


    // ---- Exact reading (1.0.4) -------------------------------------------
private:
    /** `json` with the member at `path` ("a.b[1].c") set to `value`, or removed when undefined. */
    static QJsonValue edited(const QJsonValue& json, QStringList path, const QJsonValue& value)
    {
        const QString step = path.takeFirst();
        const int bracket = step.indexOf(QLatin1Char('['));
        const QString key = bracket < 0 ? step : step.left(bracket);
        QJsonObject object = json.toObject();
        if (bracket >= 0) {
            const int index = step.mid(bracket + 1, step.size() - bracket - 2).toInt();
            QJsonArray array = object.value(key).toArray();
            array[index] = path.isEmpty() ? value : edited(array.at(index), path, value);
            object[key] = array;
        } else if (path.isEmpty()) {
            if (value.isUndefined())
                object.remove(key);
            else
                object[key] = value;
        } else {
            object[key] = edited(object.value(key), path, value);
        }
        return object;
    }

    /** What a T loading `json` with `member` set to `value` throws; empty if it loads. */
    template<class T>
    static QString refusalOf(const QByteArray& json, const QString& member, const QJsonValue& value)
    {
        const QJsonValue changed = edited(QJsonDocument::fromJson(json).object(),
                                          member.split(QLatin1Char('.')), value);
        T response;
        try {
            response.loadFromJson(QJsonDocument(changed.toObject()).toJson());
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }

private slots:

    /**
     * A path's amounts and assets, exactly: an amount given as a number was
     * left empty, and a hop or an end whose code does not fit its type came
     * out as another asset, which a path payment built from it would sign.
     */
    void testPathIsReadExactly()
    {
        typedef Page<PathResponse> Paths;
        const QByteArray real = realPubnetStrictReceivePaths();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Paths>(real, r + "destination_amount", 100).startsWith(r + "destination_amount: "));
        QVERIFY(refusalOf<Paths>(real, r + "source_amount", QString("438.84665740")).startsWith(r + "source_amount: "));
        QVERIFY(refusalOf<Paths>(real, r + "destination_asset_code", QString("USDCX")).startsWith(r + "destination_asset_code: "));
        QVERIFY(refusalOf<Paths>(real, r + "source_asset_type", QJsonValue(QJsonValue::Undefined)).startsWith(r + "source_asset_type: "));
        QVERIFY(refusalOf<Paths>(real, r + "path", QJsonArray{QJsonObject{{"asset_type", "credit_alphanum4"},
                                                                          {"asset_code", "yUSDC"},
                                                                          {"asset_issuer", "GDGTVWSM4MGS4T7Z6W4RPWOCHE2I6RDFCIFZGS3DOA63LWQTRNZNTTFF"}}})
                    .startsWith(r + "path[0].asset_code: "));
        QVERIFY(refusalOf<Paths>(real, r + "path", QJsonArray{QJsonObject{{"asset_type", "native"}}, 5})
                    .startsWith(r + "path[1]: "));
        QVERIFY(refusalOf<Paths>(real, r + "path", QJsonValue(QJsonValue::Undefined)).startsWith(r + "path: "));
        // A hop that is an asset loads.
        QVERIFY(refusalOf<Paths>(real, r + "path", QJsonArray{QJsonObject{{"asset_type", "credit_alphanum12"},
                                                                          {"asset_code", "yUSDC"},
                                                                          {"asset_issuer", "GDGTVWSM4MGS4T7Z6W4RPWOCHE2I6RDFCIFZGS3DOA63LWQTRNZNTTFF"}}})
                    .isEmpty());
    }

    /** Real pubnet paths (Horizon sends them without links) load with their exact amounts. */
    void testRealHorizonPaths()
    {
        Page<PathResponse> receive;
        receive.loadFromJson(realPubnetStrictReceivePaths());
        QCOMPARE(receive.size(), 1);
        QCOMPARE(receive.get(0).getSourceAmount(), QString("438.8466574"));
        QCOMPARE(receive.get(0).getDestinationAmount(), QString("100.0000000"));
        QCOMPARE(receive.get(0).getDestinationAsset().getType(), QString("credit_alphanum4"));
        Page<PathResponse> send;
        send.loadFromJson(realPubnetStrictSendPaths());
        QCOMPARE(send.get(0).getDestinationAmount(), QString("227.8684409"));
    }
private:
    /** Real pubnet strict-receive paths (Horizon sends no links). */
    static QByteArray realPubnetStrictReceivePaths()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"source_asset_type\":\"native\",\"source_amount\":\"438.8466574\",\"destination_asset_type\":\"credit_alphanum4\",\"destination_asset_code\":\"USDC\",\"destination_asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\"destination_amount\":\"100.0000000\",\"path\":[]}]}}");
    }

    /** Real pubnet strict-send paths (Horizon sends no links). */
    static QByteArray realPubnetStrictSendPaths()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"source_asset_type\":\"native\",\"source_amount\":\"1000.0000000\",\"destination_asset_type\":\"credit_alphanum4\",\"destination_asset_code\":\"USDC\",\"destination_asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\"destination_amount\":\"227.8684409\",\"path\":[]}]}}");
    }

private slots:
};





ADD_TEST(PathsPageDeserializerTest)
#endif // PATHSPAGEDESERIALIZERTEST_H
