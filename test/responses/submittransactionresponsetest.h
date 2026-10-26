#ifndef SUBMITTRANSACTIONRESPONSETEST_H
#define SUBMITTRANSACTIONRESPONSETEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/submittransactionresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

class SubmitTransactionResponseTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserializeTransactionFailureResponse() {
        QString json = "{\n"
                       "  \"type\": \"https://stellar.org/horizon-errors/transaction_failed\",\n"
                       "  \"title\": \"Transaction Failed\",\n"
                       "  \"status\": 400,\n"
                       "  \"detail\": \"The transaction failed when submitted to the stellar network. The `extras.result_codes` field on this response contains further details.  Descriptions of each code can be found at: https://www.stellar.org/developers/learn/concepts/list-of-operations.html\",\n"
                       "  \"instance\": \"horizon-testnet-001.prd.stellar001.internal.stellar-ops.com/4elYz2fHhC-528285\",\n"
                       "  \"extras\": {\n"
                       "    \"envelope_xdr\": \"AAAAAKpmDL6Z4hvZmkTBkYpHftan4ogzTaO4XTB7joLgQnYYAAAAZAAAAAAABeoyAAAAAAAAAAEAAAAAAAAAAQAAAAAAAAABAAAAAD3sEVVGZGi/NoC3ta/8f/YZKMzyi9ZJpOi0H47x7IqYAAAAAAAAAAAF9eEAAAAAAAAAAAA=\",\n"
                       "    \"result_codes\": {\n"
                       "      \"transaction\": \"tx_no_source_account\"\n"
                       "    },\n"
                       "    \"result_xdr\": \"AAAAAAAAAAD////4AAAAAA==\"\n"
                       "  }\n"
                       "}";

        SubmitTransactionResponse *submitTransactionResponse = new SubmitTransactionResponse(nullptr,nullptr);
        submitTransactionResponse->loadFromJson(json.toLatin1());

        QVERIFY(submitTransactionResponse->isSuccess()== false);
        qDebug() << submitTransactionResponse->getEnvelopeXdr();
        qDebug() <<QString("AAAAAKpmDL6Z4hvZmkTBkYpHftan4ogzTaO4XTB7joLgQnYYAAAAZAAAAAAABeoyAAAAAAAAAAEAAAAAAAAAAQAAAAAAAAABAAAAAD3sEVVGZGi/NoC3ta/8f/YZKMzyi9ZJpOi0H47x7IqYAAAAAAAAAAAF9eEAAAAAAAAAAAA=");
        QVERIFY(submitTransactionResponse->getEnvelopeXdr()== QString("AAAAAKpmDL6Z4hvZmkTBkYpHftan4ogzTaO4XTB7joLgQnYYAAAAZAAAAAAABeoyAAAAAAAAAAEAAAAAAAAAAQAAAAAAAAABAAAAAD3sEVVGZGi/NoC3ta/8f/YZKMzyi9ZJpOi0H47x7IqYAAAAAAAAAAAF9eEAAAAAAAAAAAA="));
        qDebug() << submitTransactionResponse->getResultXdr();
        qDebug() << QString("AAAAAAAAAAD////4AAAAAA==");
        QVERIFY(submitTransactionResponse->getResultXdr() == QString("AAAAAAAAAAD////4AAAAAA=="));
        QVERIFY(submitTransactionResponse->getExtras().getResultCodes().getTransactionResultCode() == QString("tx_no_source_account"));

    }


    void testDeserializeOperationFailureResponse() {
        QString json = "{\n"
                       "  \"type\": \"https://stellar.org/horizon-errors/transaction_failed\",\n"
                       "  \"title\": \"Transaction Failed\",\n"
                       "  \"status\": 400,\n"
                       "  \"detail\": \"The transaction failed when submitted to the stellar network. The `extras.result_codes` field on this response contains further details.  Descriptions of each code can be found at: https://www.stellar.org/developers/learn/concepts/list-of-operations.html\",\n"
                       "  \"instance\": \"horizon-testnet-001.prd.stellar001.internal.stellar-ops.com/4elYz2fHhC-528366\",\n"
                       "  \"extras\": {\n"
                       "    \"envelope_xdr\": \"AAAAAF2O0axA67+p2jMunG6G188kDSHIvqQ13d9l29YCSA/uAAAAZAAvvc0AAAABAAAAAAAAAAEAAAAAAAAAAQAAAAAAAAABAAAAAD3sEVVGZGi/NoC3ta/8f/YZKMzyi9ZJpOi0H47x7IqYAAAAAAAAAAAF9eEAAAAAAAAAAAECSA/uAAAAQFuZVAjftHa+JZes1VxSk8naOfjjAz9V86mY1AZf8Ik6PtTsBpDsCfG57EYsq4jWyZcT+vhXyWsw5evF1ELqMw4=\",\n"
                       "    \"result_codes\": {\n"
                       "      \"transaction\": \"tx_failed\",\n"
                       "      \"operations\": [\n"
                       "        \"op_no_destination\"\n"
                       "      ]\n"
                       "    },\n"
                       "    \"result_xdr\": \"AAAAAAAAAGT/////AAAAAQAAAAAAAAAB////+wAAAAA=\"\n"
                       "  }\n"
                       "}";

        SubmitTransactionResponse* submitTransactionResponse = new SubmitTransactionResponse(nullptr,nullptr);
        submitTransactionResponse->loadFromJson(json.toLatin1());
        QVERIFY(submitTransactionResponse->isSuccess()== false);
        QVERIFY(submitTransactionResponse->getEnvelopeXdr()==QString("AAAAAF2O0axA67+p2jMunG6G188kDSHIvqQ13d9l29YCSA/uAAAAZAAvvc0AAAABAAAAAAAAAAEAAAAAAAAAAQAAAAAAAAABAAAAAD3sEVVGZGi/NoC3ta/8f/YZKMzyi9ZJpOi0H47x7IqYAAAAAAAAAAAF9eEAAAAAAAAAAAECSA/uAAAAQFuZVAjftHa+JZes1VxSk8naOfjjAz9V86mY1AZf8Ik6PtTsBpDsCfG57EYsq4jWyZcT+vhXyWsw5evF1ELqMw4="));
        QVERIFY(submitTransactionResponse->getResultXdr()== QString("AAAAAAAAAGT/////AAAAAQAAAAAAAAAB////+wAAAAA="));
        QVERIFY(submitTransactionResponse->getExtras().getResultCodes().getTransactionResultCode()==QString("tx_failed"));
        QVERIFY(submitTransactionResponse->getExtras().getResultCodes().getOperationsResultCodes()[0]==QString("op_no_destination"));
    }


    void testDeserializeWithDiagnosticEventsXdr() {
        QString json = "{\n"
                       "  \"type\": \"https://stellar.org/horizon-errors/transaction_failed\",\n"
                       "  \"title\": \"Transaction Failed\",\n"
                       "  \"status\": 400,\n"
                       "  \"extras\": {\n"
                       "    \"envelope_xdr\": \"AAAAAA==\",\n"
                       "    \"result_codes\": {\n"
                       "      \"transaction\": \"tx_failed\",\n"
                       "      \"operations\": [ \"op_invoke_host_function_resource_limit_exceeded\" ]\n"
                       "    },\n"
                       "    \"result_xdr\": \"AAAAAAAAAGT/////AAAAAA==\",\n"
                       "    \"diagnostic_events_xdr\": [\n"
                       "      \"AAAAAQAAAAA=\",\n"
                       "      \"AAAAAgAAAAA=\"\n"
                       "    ]\n"
                       "  }\n"
                       "}";

        SubmitTransactionResponse* submitTransactionResponse = new SubmitTransactionResponse(nullptr,nullptr);
        submitTransactionResponse->loadFromJson(json.toLatin1());

        QVERIFY(submitTransactionResponse->isSuccess()== false);
        QCOMPARE(submitTransactionResponse->getExtras().getResultCodes().getTransactionResultCode(), QString("tx_failed"));
        QStringList events = submitTransactionResponse->getExtras().getDiagnosticEventsXdr();
        QCOMPARE(events.size(), 2);
        QCOMPARE(events.at(0), QString("AAAAAQAAAAA="));
        QCOMPARE(events.at(1), QString("AAAAAgAAAAA="));
    }


    void testDeserializeSuccessResponse() {
        QString json = "{\n"
                       "  \"_links\": {\n"
                       "    \"transaction\": {\n"
                       "      \"href\": \"https://horizon-testnet.stellar.org/transactions/ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\"\n"
                       "    }\n"
                       "  },\n"
                       "  \"hash\": \"ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\",\n"
                       "  \"ledger\": 3128812,\n"
                       "  \"envelope_xdr\": \"AAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAZAAT3TUAAAAwAAAAAAAAAAAAAAABAAAAAAAAAAMAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAAAAAAG/dhGXAAAAQLuStfImg0OeeGAQmvLkJSZ1MPSkCzCYNbGqX5oYNuuOqZ5SmWhEsC7uOD9ha4V7KengiwNlc0oMNqBVo22S7gk=\",\n"
                       "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAAAAAAAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAAAAPEAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAA==\",\n"
                       "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAvoHwAAAACAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAAAAPEAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAAAAAAEAL6B8AAAAAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAABZ9zvNAABPdNQAAADAAAAAEAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAA==\"\n"
                       "}";

        SubmitTransactionResponse *submitTransactionResponse = new SubmitTransactionResponse(nullptr,nullptr);
        submitTransactionResponse->loadFromJson(json.toLatin1());

        QVERIFY(submitTransactionResponse->isSuccess()== true);
        QVERIFY(submitTransactionResponse->getHash()== "ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940");
        QVERIFY(submitTransactionResponse->getLedger()==3128812);
        QString expected("AAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAZAAT3TUAAAAwAAAAAAAAAAAAAAABAAAAAAAAAAMAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAAAAAAG/dhGXAAAAQLuStfImg0OeeGAQmvLkJSZ1MPSkCzCYNbGqX5oYNuuOqZ5SmWhEsC7uOD9ha4V7KengiwNlc0oMNqBVo22S7gk=");
        QVERIFY(submitTransactionResponse->getEnvelopeXdr()== expected);
        QVERIFY(submitTransactionResponse->getResultXdr()== "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAAAAAAAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAAAAPEAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAA==");
        qDebug()<< "RES "<<submitTransactionResponse->getOfferIdFromResult(0);
        QVERIFY(submitTransactionResponse->getOfferIdFromResult(0)== 241);
    }


    void testDeserializeNoOfferID() {
        QByteArray json = "{\n"
                          "  \"_links\": {\n"
                          "    \"transaction\": {\n"
                          "      \"href\": \"https://horizon-testnet.stellar.org/transactions/ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\"\n"
                          "    }\n"
                          "  },\n"
                          "  \"hash\": \"ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\",\n"
                          "  \"ledger\": 3128812,\n"
                          "  \"envelope_xdr\": \"AAAAAP0uDCJGzWofMAUkG/F3YCPctgLkeP3VTr3P1mHKznAHAAAAZAA5klgAAABLAAAAAAAAAAAAAAABAAAAAQAAAAD9LgwiRs1qHzAFJBvxd2Aj3LYC5Hj91U69z9Zhys5wBwAAAAMAAAABRVVSAAAAAAD9LgwiRs1qHzAFJBvxd2Aj3LYC5Hj91U69z9Zhys5wBwAAAAFVU0QAAAAAAOw4Vbv3zJ23hiG1bjU7M/AOuVNHKnREERoitIr1Zj/ZAAAAAAOThwAAAAACAAAAAQAAAAAAAAAAAAAAAAAAAAHKznAHAAAAQEe1jyNCeK3TckPuuKeWIICf6nvz2zBZ8mbbUamWLnOFMMqvQPTllOe9DIdloNxaixgle9zi2F+yyOhLzpNhkAg=\",\n"
                          "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAEAAAAA7DhVu/fMnbeGIbVuNTsz8A65U0cqdEQRGiK0ivVmP9kAAAAAAAASJQAAAAFVU0QAAAAAAOw4Vbv3zJ23hiG1bjU7M/AOuVNHKnREERoitIr1Zj/ZAAAAAAcnDgAAAAABRVVSAAAAAAD9LgwiRs1qHzAFJBvxd2Aj3LYC5Hj91U69z9Zhys5wBwAAAAADk4cAAAAAAgAAAAA=\",\n"
                          "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAvoHwAAAACAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAAAAPEAAAABSU5SAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAAAFVU0QAAAAAADSMMRmQGDH6EJzkgi/7PoKhphMHyNGQgDp2tlS/dhGXAAAAAAX14QAAAAAKAAAAAQAAAAAAAAAAAAAAAAAAAAEAL6B8AAAAAAAAAAA0jDEZkBgx+hCc5IIv+z6CoaYTB8jRkIA6drZUv3YRlwAAABZ9zvNAABPdNQAAADAAAAAEAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAA==\"\n"
                          "}";

        SubmitTransactionResponse submitTransactionResponse(nullptr,nullptr);
        submitTransactionResponse.loadFromJson(json);
        QCOMPARE(submitTransactionResponse.isSuccess(), true);
        QVERIFY(submitTransactionResponse.getOfferIdFromResult(0)==0);
    }


    /**
     * getOfferIdFromResult answers 0 for any result it cannot read, and never
     * throws. A 20-byte result_xdr counting 0x7fffffff operations made it
     * reserve gigabytes and throw std::bad_alloc into the app; a count in Qt's
     * 64-bit framing (FFFFFFFE, then the count) was read and its offer id
     * handed back, though js and py refuse those bytes.
     */
    void testAnUnreadableResultHasNoOfferIdAndDoesNotThrow() {
        auto offerId = [](const char* resultXdr) -> qint64 {
            SubmitTransactionResponse response(nullptr, nullptr);
            response.loadFromJson(QByteArray("{\"hash\": \"ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\", \"envelope_xdr\": \"AAAAAA==\", \"ledger\": 5, \"result_xdr\": \"") + resultXdr + "\"}");
            try {
                return response.getOfferIdFromResult(0);
            } catch (...) {
                return -1;
            }
        };
        // One created offer, 777001: read as it should be.
        QCOMPARE(offerId("AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAAAAAAAAAAAABERERERERERERERERERERERERERERERERERERERERERAAAAAAAL2ykAAAAAAAAAAVVTRAAAAAAAIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIAAAAAAA9CQAAAAAEAAAACAAAAAAAAAAAAAAAA"),
                 qint64(777001));
        const char* unreadable[] = {
            // the same, results counted FFFFFFFE + int64 1
            "AAAAAAAAAGQAAAAA/////gAAAAAAAAABAAAAAAAAAAMAAAAAAAAAAAAAAAAAAAAAEREREREREREREREREREREREREREREREREREREREREREAAAAAAAvbKQAAAAAAAAABVVNEAAAAAAAiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIgAAAAAAD0JAAAAAAQAAAAIAAAAAAAAAAAAAAAA=",
            "AAAAAAAAAGQAAAAAf////wAAAAA=",                      // results 0x7fffffff, nothing after
            "AAAAAAAAAGQAAAAA/////gAAAQAAAAAAAAAAAA==",          // results FFFFFFFE + int64 2^40
            // offersClaimed 0x7fffffff, then FFFFFFFE + int64 0
            "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAH////8AAAAAAAAAABERERERERERERERERERERERERERERERERERERERERERAAAAAAAL2yoAAAAAAAAAAVVTRAAAAAAAIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIAAAAAAA9CQAAAAAEAAAACAAAAAAAAAAAAAAAA",
            "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAP////4AAAAAAAAAAAAAAAAAAAAAEREREREREREREREREREREREREREREREREREREREREREAAAAAAAvbKwAAAAAAAAABVVNEAAAAAAAiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIgAAAAAAD0JAAAAAAQAAAAIAAAAAAAAAAAAAAAA=",
            // a fee bump's inner results: 0x7fffffff, then FFFFFFFE + int64 1
            "AAAAAAAAAMgAAAABMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMAAAAAAAAAZAAAAAB/////AAAAAAAAAAA=",
            "AAAAAAAAAMgAAAABMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMAAAAAAAAAZAAAAAD////+AAAAAAAAAAEAAAAAAAAAAwAAAAAAAAAAAAAAAAAAAAAREREREREREREREREREREREREREREREREREREREREREQAAAAAAC9ssAAAAAAAAAAFVU0QAAAAAACIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiAAAAAAAPQkAAAAABAAAAAgAAAAAAAAAAAAAAAAAAAAA=",
            "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAANAAAAAH////8AAAAA",  // path payment offers 0x7fffffff
            "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAJAAAAAH////8AAAAA",  // inflation payouts 0x7fffffff
        };
        for (const char* resultXdr : unreadable)
            QCOMPARE(offerId(resultXdr), qint64(0));
    }


    /**
     * A submission Horizon took is the transaction as applied. Without its
     * ledger it read as not successful, without result_xdr the offer ids
     * read 0, and without its hash or envelope getHash() and
     * getEnvelopeXdr() read empty; a ledger that is not a number read as 0,
     * a failure. A problem document (a refused submission) is still read as
     * one.
     */
    void testASuccessIsReadWhole()
    {
        const QByteArray success = QJsonDocument(QJsonObject{
            {"hash", "ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940"},
            {"ledger", 3128812},
            {"envelope_xdr", "AAAAAA=="},
            {"result_xdr", "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAAAAAAA"}}).toJson();
        for (const char* key : { "hash", "ledger", "envelope_xdr", "result_xdr" }) {
            QJsonObject missing = QJsonDocument::fromJson(success).object();
            missing.remove(QLatin1String(key));
            SubmitTransactionResponse response(nullptr, nullptr);
            QVERIFY_EXCEPTION_THROWN(response.loadFromJson(QJsonDocument(missing).toJson()), std::runtime_error);
        }
        QJsonObject wrong = QJsonDocument::fromJson(success).object();
        wrong["ledger"] = QString("3128812th");
        SubmitTransactionResponse notANumber(nullptr, nullptr);
        QVERIFY_EXCEPTION_THROWN(notANumber.loadFromJson(QJsonDocument(wrong).toJson()), std::runtime_error);

        SubmitTransactionResponse applied(nullptr, nullptr);
        applied.loadFromJson(success);
        QVERIFY(applied.isSuccess());
        QCOMPARE(applied.getLedger(), Q_INT64_C(3128812));
        QVERIFY(!applied.isTimeout());

        const QByteArray refused = "{\"type\":\"https://stellar.org/horizon-errors/transaction_failed\","
                                   "\"title\":\"Transaction Failed\",\"status\":400,"
                                   "\"extras\":{\"envelope_xdr\":\"AAAAAA==\",\"result_codes\":{\"transaction\":\"tx_bad_seq\"},"
                                   "\"result_xdr\":\"AAAAAAAAAGT////7AAAAAA==\"}}";
        SubmitTransactionResponse failure(nullptr, nullptr);
        failure.loadFromJson(refused);
        QVERIFY(!failure.isSuccess());
        QCOMPARE(failure.getExtras().getResultCodes().getTransactionResultCode(), QString("tx_bad_seq"));
    }

    /**
     * result_xdr is decoded as base64 exactly: the lenient decoder skipped
     * characters that are not base64, so a garbled result still produced an
     * offer id.
     */
    void testOfferIdNeedsAResultThatIsBase64()
    {
        auto offerId = [](const QByteArray& resultXdr) -> qint64 {
            SubmitTransactionResponse response(nullptr, nullptr);
            response.loadFromJson(QByteArray("{\"hash\": \"ee14b93fcd31d4cfe835b941a0a8744e23a6677097db1fafe0552d8657bed940\", \"envelope_xdr\": \"AAAAAA==\", \"ledger\": 5, \"result_xdr\": \"") + resultXdr + "\"}");
            return response.getOfferIdFromResult(0);
        };
        const QByteArray result = "AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAADAAAAAAAAAAAAAAAAAAAAABERERERERERERERERERERERERERERERERERERERERERAAAAAAAL2ykAAAAAAAAAAVVTRAAAAAAAIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIAAAAAAA9CQAAAAAEAAAACAAAAAAAAAAAAAAAA";
        QCOMPARE(offerId(result), qint64(777001));
        QByteArray garbled = result;
        garbled.insert(20, "!*");
        QCOMPARE(offerId(garbled), qint64(0));
    }
};

ADD_TEST(SubmitTransactionResponseTest)
#endif // SUBMITTRANSACTIONRESPONSETEST_H
