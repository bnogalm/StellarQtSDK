#ifndef RESPONSEHANDLERTEST_H
#define RESPONSEHANDLERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/server.h"
#include "../../src/network.h"
#include "../../src/keypair.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/responses/problem.h"
#include "../../src/exception/sdkexception.h"
#include "../../src/exception/networkexception.h"
#include "../../src/exception/toomanyrequestsexception.h"
#include "../../src/exception/badrequestexception.h"
#include "../../src/exception/badresponseexception.h"
#include "../../src/exception/requesttimeoutexception.h"
#include "../../src/exception/unknownresponseexception.h"

#include "../fakeserver.h"

/**
 * Port (extended) of org.stellar.sdk.requests.ResponseHandlerTest from java-stellar-sdk.
 *
 * Adapted to the Qt async model: the SDK no longer throws on HTTP errors from
 * QNetworkReply paths — it emits Response::error() and exposes the typed
 * exception via Response::lastException().
 *
 * Closes problems-report.md §4.1 (preprocessResponse enum vs HTTP status) and
 * §4.2 (processResponse swallows runtime_error without emitting error()).
 */
class ResponseHandlerTest: public QObject
{
    Q_OBJECT

    static constexpr quint16 PORT = 8083;
    FakeServer* m_fakeServer = nullptr;
    Server* m_server = nullptr;

    void setUp()
    {
        m_fakeServer = new FakeServer(PORT);
        m_server = new Server(QString("http://localhost:%1").arg(PORT));
        Network::useTestNetwork();
    }

    void tearDown()
    {
        delete m_server;
        m_server = nullptr;
        delete m_fakeServer;
        m_fakeServer = nullptr;
    }

    static QString sampleAccountId()
    {
        // Real well-formed G-strkey with valid checksum (reused from
        // operationtest.h: KeyPair::fromSecretSeed("SC4CGETADV...")).
        // Not actually on testnet — we only need it parseable.
        return QStringLiteral("GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF");
    }

    /**
     * Trigger an account request and wait for either ready() or error().
     * Returns the AccountResponse (ownership stays with the test; caller deletes).
     */
    AccountResponse* requestAccount()
    {
        KeyPair* kp = KeyPair::fromAccountId(sampleAccountId());
        AccountResponse* r = m_server->accounts().account(kp);
        delete kp;

        bool ready = false;
        bool errored = false;
        connect(r, &Response::ready,  [&]{ ready = true; });
        connect(r, &Response::error,  [&]{ errored = true; });
        WAIT_FOR(!ready && !errored);
        Q_UNUSED(ready)
        Q_UNUSED(errored)
        return r;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** 429 with Retry-After header → TooManyRequestsException(retryAfter=10). */
    void testTooManyRequests()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral("");
        reply.responseCode = QStringLiteral("429 Too Many Requests");
        reply.contentType = QStringLiteral("text/plain");
        reply.extraHeaders = QStringLiteral("Retry-After: 10\r\n");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r != nullptr);
        QVERIFY(r->lastException() != nullptr);
        auto* tmre = dynamic_cast<qstellar::exception::TooManyRequestsException*>(r->lastException());
        QVERIFY2(tmre != nullptr, "expected TooManyRequestsException");
        QCOMPARE(tmre->getRetryAfter(), 10);
        delete r;
        tearDown();
    }

    /** 429 without Retry-After → TooManyRequestsException(retryAfter=-1). */
    void testTooManyRequestsNoHeader()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral("");
        reply.responseCode = QStringLiteral("429 Too Many Requests");
        reply.contentType = QStringLiteral("text/plain");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* tmre = dynamic_cast<qstellar::exception::TooManyRequestsException*>(r->lastException());
        QVERIFY2(tmre != nullptr, "expected TooManyRequestsException");
        QCOMPARE(tmre->getRetryAfter(), -1); // -1 sentinel for "no header"
        delete r;
        tearDown();
    }

    /** 400 with application/problem+json → BadRequestException + parsed Problem. */
    void testBadRequest400WithProblem()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral(
            "{"
            " \"type\": \"https://stellar.org/horizon-errors/transaction_failed\","
            " \"title\": \"Transaction Failed\","
            " \"status\": 400,"
            " \"detail\": \"The transaction failed when submitted to the stellar network.\","
            " \"extras\": {"
            "   \"hash\": \"abc123\","
            "   \"envelope_xdr\": \"AAAAAA==\","
            "   \"result_xdr\": \"AAAAAA==\","
            "   \"result_codes\": {"
            "     \"transaction\": \"tx_failed\","
            "     \"operations\": [\"op_underfunded\"]"
            "   }"
            " }"
            "}");
        reply.responseCode = QStringLiteral("400 Bad Request");
        reply.contentType = QStringLiteral("application/problem+json");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* bre = dynamic_cast<qstellar::exception::BadRequestException*>(r->lastException());
        QVERIFY2(bre != nullptr, "expected BadRequestException");
        QCOMPARE(bre->getCode(), 400);
        const Problem* p = bre->getProblem();
        QVERIFY2(p != nullptr, "expected Problem parsed from problem+json body");
        QCOMPARE(p->getStatus(), 400);
        QCOMPARE(p->getTitle(), QStringLiteral("Transaction Failed"));
        QCOMPARE(p->getType(), QStringLiteral("https://stellar.org/horizon-errors/transaction_failed"));
        QCOMPARE(p->getExtras().getHash(), QStringLiteral("abc123"));
        QCOMPARE(p->getExtras().getResultCodes().getTransaction(), QStringLiteral("tx_failed"));
        QCOMPARE(p->getExtras().getResultCodes().getOperations(),
                 QStringList() << QStringLiteral("op_underfunded"));
        delete r;
        tearDown();
    }

    /** 504 → RequestTimeoutException. */
    void test504RequestTimeout()
    {
        setUp();
        FakeServer::Reply reply;
        reply.responseCode = QStringLiteral("504 Gateway Timeout");
        reply.contentType = QStringLiteral("text/plain");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* ex = dynamic_cast<qstellar::exception::RequestTimeoutException*>(r->lastException());
        QVERIFY2(ex != nullptr, "expected RequestTimeoutException");
        QCOMPARE(ex->getCode(), 504);
        delete r;
        tearDown();
    }

    /** 500 → BadResponseException. */
    void test500BadResponse()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral("server boom");
        reply.responseCode = QStringLiteral("500 Internal Server Error");
        reply.contentType = QStringLiteral("text/plain");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* ex = dynamic_cast<qstellar::exception::BadResponseException*>(r->lastException());
        QVERIFY2(ex != nullptr, "expected BadResponseException");
        QCOMPARE(ex->getCode(), 500);
        delete r;
        tearDown();
    }

    /**
     * Status 418 — the Qt SDK routes every 4xx (except 429) to
     * BadRequestException. A conscious divergence from the Java SDK
     * (which sends only 400/404 there and the rest to UnknownResponse).
     * Pragmatic: any 4xx is a client error from the SDK's POV.
     */
    void testUnknownResponseStatus()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral("teapot");
        reply.responseCode = QStringLiteral("418 I'm a teapot");
        reply.contentType = QStringLiteral("text/plain");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* ex = dynamic_cast<qstellar::exception::BadRequestException*>(r->lastException());
        QVERIFY2(ex != nullptr, "expected BadRequestException for 4xx (incl. 418)");
        QCOMPARE(ex->getCode(), 418);
        delete r;
        tearDown();
    }

    /**
     * 200 with malformed JSON → emits error() and attaches a NetworkException.
     *
     * Before the §4.2 fix this test would hang (no ready(), no error()).
     * What we verify for §4.2: (a) error() fires, (b) lastException is
     * informative. The concrete subclass is secondary (depends on what
     * the JSON path throws).
     */
    void testMalformedJsonEmitsError()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral("<<<not-json-at-all>>>");
        reply.responseCode = QStringLiteral("200 OK");
        reply.contentType = QStringLiteral("application/hal+json");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);

        AccountResponse* r = requestAccount();
        // (a) error() must have fired for lastException to be set
        QVERIFY2(r->lastException() != nullptr, "expected an exception attached after error()");
        // (b) must be in the NetworkException hierarchy (not bare SdkException)
        auto* netEx = dynamic_cast<qstellar::exception::NetworkException*>(r->lastException());
        QVERIFY2(netEx != nullptr, qPrintable(QStringLiteral("expected NetworkException, got: %1")
                                              .arg(QString::fromLatin1(r->lastException()->what()))));
        // (c) message should reference JSON parsing for caller diagnostics
        QVERIFY2(QString::fromLatin1(r->lastException()->what()).contains(
                     QStringLiteral("JSON"), Qt::CaseInsensitive),
                 qPrintable(QStringLiteral("expected message containing 'JSON'; got: %1")
                            .arg(QString::fromLatin1(r->lastException()->what()))));
        delete r;
        tearDown();
    }

    /** Round-trip Problem JSON → object → fields. */
    void testProblemDeserialization()
    {
        QString json = QStringLiteral(
            "{"
            " \"type\": \"https://stellar.org/horizon-errors/before_history\","
            " \"title\": \"Data Requested Is Before Recorded History\","
            " \"status\": 410,"
            " \"detail\": \"This Horizon instance is configured to only track a portion of the Stellar network's latest history.\""
            "}");
        Problem p;
        p.parseFromJson(json.toUtf8());
        QCOMPARE(p.getStatus(), 410);
        QCOMPARE(p.getTitle(), QStringLiteral("Data Requested Is Before Recorded History"));
        QCOMPARE(p.getType(), QStringLiteral("https://stellar.org/horizon-errors/before_history"));
        QVERIFY(p.getDetail().startsWith("This Horizon instance"));
    }
};

ADD_TEST(ResponseHandlerTest)
#endif // RESPONSEHANDLERTEST_H
