#ifndef RESPONSEHANDLERTEST_H
#define RESPONSEHANDLERTEST_H
// First, before anything else: the moc file of this header is a translation
// unit of its own, so responses/common.h, an installed public header, is
// compiled here with nothing in front of it. It used QVariantMap without
// including <QVariant>, and compiled only after another header had pulled
// that in.
#include "../../src/responses/common.h"
#include <QObject>
#include <QtTest>
#include <QScopeGuard>
#include <QElapsedTimer>
#include <QPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
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
#include "../../src/bumpsequenceoperation.h"
#include "../../src/account.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../src/responses/submittransactionresponse.h"
#include "../../src/exception/connectionerrorexception.h"
#include "../../src/responses/operationpage.h"
#include "../../src/responses/operations/paymentoperationresponse.h"

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
        m_fakeServer = new FakeServer();
        m_server = new Server(m_fakeServer->baseUrl());
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

    /**
     * A failed submission (Horizon's 400 transaction_failed) keeps its
     * result: getExtras() and getResultXdr() return what Horizon sent, as
     * they are documented to. Both were empty and the codes reachable only
     * through the exception's Problem (found by the live interop test against
     * js-stellar-sdk, which hands them over on its error).
     */
    void testFailedSubmissionKeepsItsResult()
    {
        setUp();
        FakeServer::Reply reply;
        reply.body = QStringLiteral(
            "{\"type\":\"https://stellar.org/horizon-errors/transaction_failed\","
            "\"title\":\"Transaction Failed\",\"status\":400,"
            "\"detail\":\"The transaction failed when submitted to the stellar network.\","
            "\"extras\":{\"envelope_xdr\":\"AAAAAg==\","
            "\"result_codes\":{\"transaction\":\"tx_fee_bump_inner_failed\","
            "\"inner_transaction\":\"tx_failed\",\"operations\":[\"op_no_destination\"]},"
            "\"result_xdr\":\"AAAAAAAAAGT////3AAAAAA==\"}}");
        reply.responseCode = QStringLiteral("400 Bad Request");
        reply.contentType = QStringLiteral("application/problem+json");
        m_fakeServer->addPostFull(QStringLiteral("/transactions"), reply);

        Account* account = new Account(KeyPair::fromSecretSeed(
            QStringLiteral("SDUIFD6PZX6RKBLW2FJBLGYENH7OVJJ6NPY5JEY4O23VIXJYGVRKXAPM")), 41);
        Transaction* tx = TransactionBuilder(account, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(50)))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(
            QStringLiteral("SDUIFD6PZX6RKBLW2FJBLGYENH7OVJJ6NPY5JEY4O23VIXJYGVRKXAPM")));
        tx->sign(signer.data());
        delete account;

        SubmitTransactionResponse* r = m_server->submitTransaction(tx, true);
        bool done = false;
        connect(r, &Response::ready, [&]{ done = true; });
        connect(r, &Response::error, [&]{ done = true; });
        WAIT_FOR(!done);
        QVERIFY(!r->isSuccess());
        QVERIFY(dynamic_cast<qstellar::exception::BadRequestException*>(r->lastException()));
        QCOMPARE(r->getResultXdr(), QStringLiteral("AAAAAAAAAGT////3AAAAAA=="));
        QCOMPARE(r->getExtras().getEnvelopeXdr(), QStringLiteral("AAAAAg=="));
        QCOMPARE(r->getExtras().getResultCodes().getTransactionResultCode(), QStringLiteral("tx_fee_bump_inner_failed"));
        QCOMPARE(r->getExtras().getResultCodes().getInnerTransactionResultCode(), QStringLiteral("tx_failed"));
        QCOMPARE(r->getExtras().getResultCodes().getOperationsResultCodes(), QStringList() << QStringLiteral("op_no_destination"));
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

private:
    // One SSE payment event: "id: <id>\ndata: {payment ...}\n\n".
    static QString paymentEvent(const QString& id, const QString& amount)
    {
        return "id: " + id + "\n"
               "data: {\"_links\":{\"self\":{\"href\":\"/operations/" + id + "\"},\"transaction\":{\"href\":\"/transactions/2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\"},"
               "\"effects\":{\"href\":\"/operations/" + id + "/effects\"},\"succeeds\":{\"href\":\"/effects?order=desc&cursor=" + id + "\"},\"precedes\":{\"href\":\"/effects?order=asc&cursor=" + id + "\"}},"
               "\"type\":\"payment\",\"type_i\":1,\"id\":\"" + id + "\",\"paging_token\":\"" + id + "\","
               "\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_successful\":true,"
               "\"transaction_hash\":\"2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\","
               "\"source_account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"asset_type\":\"native\","
               "\"from\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"to\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"amount\":\"" + amount + "\"}\n"
               "\n";
    }

    /**
     * Answers each connection to `listener` with `head` (a status line and
     * headers, no length: the body ends with the connection), then sends
     * `drip` every 50 ms for as long as the client stays, counting the
     * connections in `*connections`. A body that never ends.
     */
    static void serveDripping(QTcpServer* listener, const QByteArray& head, const QByteArray& drip,
                              int* connections)
    {
        QObject::connect(listener, &QTcpServer::newConnection, listener, [=]() {
            while (QTcpSocket* socket = listener->nextPendingConnection()) {
                ++*connections;
                QObject::connect(socket, &QTcpSocket::readyRead, socket, [=]() {
                    const QByteArray request = socket->property("request").toByteArray() + socket->readAll();
                    socket->setProperty("request", request);
                    if (!request.contains("\r\n\r\n") || socket->property("answered").toBool())
                        return;
                    socket->setProperty("answered", true);
                    socket->write(head);
                    QTimer* timer = new QTimer(socket);
                    QObject::connect(timer, &QTimer::timeout, socket, [=]() { socket->write(drip); });
                    timer->start(50);
                });
                QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            }
        });
    }

private slots:
    /**
     * A request the SDK gave up on (Response::replyTimeout()) was a plain
     * ConnectionErrorException, which reads as "the request never went
     * out", while it may well have reached Horizon. It is a
     * ClientTimeoutException now, which is still a ConnectionErrorException,
     * so code that catches that one keeps catching it.
     */
    void testClientTimeoutIsATypedTimeout()
    {
        setUp();
        m_fakeServer->addHang(QString("/accounts/") + sampleAccountId());
        Response::setReplyTimeout(300);
        // Restored however the test ends: a failed check used to leave 300 ms
        // for every test after it.
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        AccountResponse* r = requestAccount();
        QVERIFY(r->lastException() != nullptr);
        auto* timeout = dynamic_cast<qstellar::exception::ClientTimeoutException*>(r->lastException());
        QVERIFY2(timeout != nullptr, r->lastException()->what());
        QCOMPARE(timeout->getTimeout(), 300);
        QVERIFY(dynamic_cast<qstellar::exception::ConnectionErrorException*>(r->lastException()) != nullptr);
        QVERIFY(dynamic_cast<qstellar::exception::RequestTimeoutException*>(r->lastException()) == nullptr);
        QCOMPARE(r->getStatus(), 0);
        bool caught = false;
        try {
            throw qstellar::exception::ClientTimeoutException(300);
        } catch (const qstellar::exception::ConnectionErrorException&) {
            caught = true;
        }
        QVERIFY(caught);
        delete r;
        tearDown();
    }

    /** The limit is the build's STELLAR_QT_REPLY_TIMEOUT until set; 0 or less restores it. */
    void testReplyTimeoutCanBeSet()
    {
        QCOMPARE(Response::replyTimeout(), STELLAR_QT_REPLY_TIMEOUT);
        // Restored however the test ends: a failed check used to leave 1234 ms
        // for every test after it.
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        Response::setReplyTimeout(1234);
        QCOMPARE(Response::replyTimeout(), 1234);
        Response::setReplyTimeout(-5);
        QCOMPARE(Response::replyTimeout(), STELLAR_QT_REPLY_TIMEOUT);
    }

    /**
     * The X-Ratelimit-* headers, exactly. A header that was not sent (the
     * Horizons that do not limit leave them out), or is not a whole number
     * that fits an int, reads -1; both read 0 with toInt(), which for
     * X-Ratelimit-Remaining says that no request is left.
     */
    void testRateLimitHeadersAreReadExactly()
    {
        struct Case { const char* headers; int limit; int remaining; int reset; };
        const Case cases[] = {
            { "X-Ratelimit-Limit: 3600\r\nX-Ratelimit-Remaining: 0\r\nX-Ratelimit-Reset: 12\r\n", 3600, 0, 12 },
            { "X-RateLimit-Limit: 72\r\nX-RateLimit-Remaining: 71\r\nX-RateLimit-Reset: 3599\r\n", 72, 71, 3599 },
            { "X-Ratelimit-Limit: 36x0\r\nX-Ratelimit-Remaining: -1\r\nX-Ratelimit-Reset: 1.5\r\n", -1, -1, -1 },
            { "X-Ratelimit-Limit: 99999999999\r\nX-Ratelimit-Remaining: 0x10\r\nX-Ratelimit-Reset: +5\r\n", -1, -1, -1 },
            { "", -1, -1, -1 },
        };
        for (const Case& c : cases) {
            setUp();
            FakeServer::Reply reply;
            reply.responseCode = QStringLiteral("404 Not Found");
            reply.contentType = QStringLiteral("text/plain");
            reply.extraHeaders = QString::fromLatin1(c.headers);
            m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);
            AccountResponse* r = requestAccount();
            QCOMPARE(r->getStatus(), 404);
            QCOMPARE(r->getRateLimitLimit(), c.limit);
            QCOMPARE(r->getRateLimitRemaining(), c.remaining);
            QCOMPARE(r->getRateLimitReset(), c.reset);
            delete r;
            tearDown();
        }
        // Nothing sent yet.
        AccountResponse fresh(nullptr);
        QCOMPARE(fresh.getRateLimitLimit(), -1);
        QCOMPARE(fresh.getRateLimitRemaining(), -1);
        QCOMPARE(fresh.getRateLimitReset(), -1);
    }

    /**
     * A streamed event that cannot be read is reported with error(), and
     * the stream goes on. It used to be retried with the events after it
     * appended, which never parsed either: the stream went silent, and
     * nothing said why.
     */
    void testUnreadableStreamEventIsReportedAndTheStreamGoesOn()
    {
        FakeServer* fake = new FakeServer();
        const QString good1 = paymentEvent("1", "10.0000000");
        QString bad = paymentEvent("2", "20.0000000");
        bad.replace("\"amount\":\"20.0000000\"", "\"amount\":20");
        const QString good2 = paymentEvent("3", "30.0000000");
        fake->addStream("/payments", QStringList() << (good1 + bad + good2));
        Server* server = new Server(fake->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        int ready = 0, errors = 0;
        bool unreadable = false;
        QString lastAmount, errorMessage;
        QObject::connect(stream, &Response::ready, [&]() {
            ++ready;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        QObject::connect(stream, &Response::error, [&]() {
            ++errors;
            if (stream->lastException())
                errorMessage = QString::fromUtf8(stream->lastException()->what());
            unreadable = dynamic_cast<qstellar::exception::UnknownResponseException*>(stream->lastException()) != nullptr;
        });
        WAIT_FOR(ready < 2)
        QCOMPARE(errors, 1);
        QCOMPARE(ready, 2);
        QCOMPARE(lastAmount, QString("30.0000000"));
        QVERIFY2(errorMessage.contains("amount"), qPrintable(errorMessage));
        QVERIFY(unreadable);
        delete server;
        fake->deleteLater();
    }

    /**
     * Each streamed event is the whole record. A member one event had and
     * the next left out (an account's home domain, once cleared) kept the
     * earlier event's value.
     */
    void testStreamedRecordKeepsNothingFromTheEventBefore()
    {
        FakeServer* fake = new FakeServer();
        const QString account = sampleAccountId();
        const QString withDomain = QString(
            "{\"id\":\"%1\",\"account_id\":\"%1\",\"sequence\":\"%2\",\"subentry_count\":0,%3"
            "\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},"
            "\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false},"
            "\"balances\":[{\"balance\":\"5.0000000\",\"buying_liabilities\":\"0.0000000\","
            "\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],"
            "\"signers\":[{\"weight\":1,\"key\":\"%1\",\"type\":\"ed25519_public_key\"}],"
            "\"data\":{},\"paging_token\":\"%1\",\"num_sponsoring\":0,\"num_sponsored\":0,"
            "\"_links\":{\"self\":{\"href\":\"/accounts/%1\"},\"transactions\":{\"href\":\"/accounts/%1/transactions\"},\"operations\":{\"href\":\"/accounts/%1/operations\"},"
            "\"effects\":{\"href\":\"/accounts/%1/effects\"},\"offers\":{\"href\":\"/accounts/%1/offers\"}}}");
        const QString first = withDomain.arg(account, "7", "\"home_domain\":\"example.com\",");
        const QString second = withDomain.arg(account, "8", "");
        fake->addStream(QString("/accounts/") + account, QStringList()
            << ("id: 1\ndata: " + first + "\n\nid: 2\ndata: " + second + "\n\n"));
        Server* server = new Server(fake->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(account));
        AccountResponse* stream = server->accounts().stream().account(key.data());
        int ready = 0;
        QStringList domains;
        QList<qint64> sequences;
        QObject::connect(stream, &Response::ready, [&]() {
            ++ready;
            domains << stream->getHomeDomain();
            sequences << stream->getSequenceNumber();
        });
        WAIT_FOR(ready < 2)
        QCOMPARE(ready, 2);
        QCOMPARE(domains, QStringList() << "example.com" << QString());
        QCOMPARE(sequences, QList<qint64>() << 7 << 8);
        delete server;
        fake->deleteLater();
    }

    /**
     * A stream answered 429 reconnects after the Retry-After Horizon sent,
     * in seconds. It was compared with the reconnect delay, which is in
     * milliseconds, so "Retry-After: 2" waited 1 s and a rate-limited
     * stream kept asking once a second.
     */
    void testStreamWaitsTheRetryAfterOfA429()
    {
        FakeServer* fake = new FakeServer();
        FakeServer::Reply limited;
        limited.body = QStringLiteral("{\"type\":\"https://stellar.org/horizon-errors/rate_limit_exceeded\","
                                      "\"title\":\"Rate Limit Exceeded\",\"status\":429}");
        limited.responseCode = QStringLiteral("429 Too Many Requests");
        limited.contentType = QStringLiteral("application/problem+json");
        limited.extraHeaders = QStringLiteral("Retry-After: 2\r\n");
        fake->addGetFull(QStringLiteral("/payments"), limited);
        Server* server = new Server(fake->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        QElapsedTimer clock;
        clock.start();
        QList<qint64> at;
        QList<int> retryAfter;
        QObject::connect(stream, &Response::error, [&]() {
            at << clock.elapsed();
            auto* limit = dynamic_cast<qstellar::exception::TooManyRequestsException*>(stream->lastException());
            retryAfter << (limit ? limit->getRetryAfter() : -2);
        });
        while (at.size() < 2 && clock.elapsed() < 10000)
            QTest::qWait(20);
        delete server;
        fake->deleteLater();
        QCOMPARE(at.size(), 2);
        QCOMPARE(retryAfter, QList<int>() << 2 << 2);
        QVERIFY2(at.at(1) - at.at(0) >= 1900, qPrintable(QString::number(at.at(1) - at.at(0))));
    }

    /**
     * A stream that cannot connect says so, once per attempt, and keeps
     * trying, as js's EventSource does: js 17.1.0 calls onerror with
     * "fetch failed: connect ECONNREFUSED" once per attempt, 3 s apart. The
     * reply finished without a byte read, and the stream reconnected every
     * second with no error(): a stream pointed at a Horizon that is down
     * waited forever.
     */
    void testAStreamThatCannotConnectSaysSo()
    {
        QTcpServer probe;
        QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
        const quint16 port = probe.serverPort();
        probe.close(); // nothing listens there now: the connection is refused
        Server* server = new Server(QStringLiteral("http://127.0.0.1:%1").arg(port));
        OperationPage* stream = server->payments().stream().execute();
        int errors = 0;
        bool refused = true;
        QObject::connect(stream, &Response::error, [&]() {
            ++errors;
            refused = refused && stream->lastErrorCode() == QNetworkReply::ConnectionRefusedError
                      && dynamic_cast<qstellar::exception::ConnectionErrorException*>(stream->lastException())
                      && stream->getStatus() == 0;
        });
        // Not WAIT_FOR: a refused connection takes Windows a second or more.
        QElapsedTimer waited;
        waited.start();
        while (errors < 2 && waited.elapsed() < 30000)
            QTest::qWait(20);
        delete server;
        QVERIFY2(errors >= 2, qPrintable(QString::number(errors)));
        QVERIFY(refused);
    }

    /**
     * A stream answered with an HTTP error, or with a 2xx that is not an
     * event stream, reports it once per connection. An empty 503 never
     * reached the code that reports errors (it ran on readyRead only), and a
     * 200 web page was read as a stream without events: both reconnected
     * every second saying nothing. js 17.1.0 reports them as "Non-200 status
     * code (503)" and 'Invalid content type, expected "text/event-stream"'.
     */
    void testAStreamAnsweredWithAnErrorSaysSoOncePerConnection()
    {
        FakeServer* fake = new FakeServer();
        FakeServer::Reply busy;
        busy.responseCode = QStringLiteral("503 Service Unavailable");
        busy.contentType = QStringLiteral("text/plain");
        fake->addGetFull(QStringLiteral("/payments"), busy);
        FakeServer::Reply page;
        page.body = QStringLiteral("<html><body>Sign in to use the Wi-Fi</body></html>");
        page.contentType = QStringLiteral("text/html");
        fake->addGetFull(QString("/accounts/") + sampleAccountId(), page);
        Server* server = new Server(fake->baseUrl());
        OperationPage* payments = server->payments().stream().execute();
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(sampleAccountId()));
        AccountResponse* account = server->accounts().stream().account(key.data());
        QElapsedTimer clock;
        clock.start();
        QList<qint64> busyAt, pageAt;
        bool busyKind = true, pageKind = true;
        QObject::connect(payments, &Response::error, [&]() {
            busyAt << clock.elapsed();
            auto* bad = dynamic_cast<qstellar::exception::BadResponseException*>(payments->lastException());
            busyKind = busyKind && bad && bad->getCode() == 503;
        });
        QObject::connect(account, &Response::error, [&]() {
            pageAt << clock.elapsed();
            auto* unknown = dynamic_cast<qstellar::exception::UnknownResponseException*>(account->lastException());
            pageKind = pageKind && unknown && QString::fromUtf8(unknown->what()).contains("text/html");
        });
        while ((busyAt.size() < 2 || pageAt.size() < 2) && clock.elapsed() < 10000)
            QTest::qWait(20);
        delete server;
        fake->deleteLater();
        QVERIFY(busyAt.size() >= 2);
        QVERIFY(pageAt.size() >= 2);
        QVERIFY(busyKind);
        QVERIFY(pageKind);
        // One error() per connection: each a reconnect delay (1 s) after the one before.
        for (int i = 1; i < busyAt.size(); ++i)
            QVERIFY2(busyAt.at(i) - busyAt.at(i - 1) >= 900, qPrintable(QString::number(busyAt.at(i) - busyAt.at(i - 1))));
        for (int i = 1; i < pageAt.size(); ++i)
            QVERIFY2(pageAt.at(i) - pageAt.at(i - 1) >= 900, qPrintable(QString::number(pageAt.at(i) - pageAt.at(i - 1))));
    }

    /**
     * A stream that gets no answer within Response::replyTimeout() says so
     * with a ClientTimeoutException, and tries again. It was closed and
     * reconnected with no error().
     */
    void testAStreamWithNoAnswerTimesOutAndSaysSo()
    {
        FakeServer* fake = new FakeServer();
        fake->addHang(QStringLiteral("/payments"));
        Response::setReplyTimeout(300);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        Server* server = new Server(fake->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        int errors = 0;
        bool timedOut = true;
        QObject::connect(stream, &Response::error, [&]() {
            ++errors;
            timedOut = timedOut && dynamic_cast<qstellar::exception::ClientTimeoutException*>(stream->lastException());
        });
        QElapsedTimer waited;
        waited.start();
        while (errors < 2 && waited.elapsed() < 10000)
            QTest::qWait(20);
        delete server;
        fake->deleteLater();
        QCOMPARE(errors, 2);
        QVERIFY(timedOut);
    }

    /**
     * A stream answered with an HTTP error whose body keeps coming (a slow
     * drip, or one with no end) says so too: the 500, within
     * Response::replyTimeout() of the request and without its body, and it
     * reconnects. Each piece of that body restarted the timeout, so the
     * reply never finished: no error(), ever, and Qt kept buffering the
     * body. js 17.1.0's EventSource, served the same, reports "Non-200
     * status code (500)" on the status alone, and py's requests_sse raises
     * InvalidStatusCodeError; neither reads the body.
     */
    void testAStreamAnsweredWithAnEndlessErrorSaysSo()
    {
        int connections = 0;
        QTcpServer listener;
        QVERIFY(listener.listen(QHostAddress::LocalHost, 0));
        serveDripping(&listener, "HTTP/1.1 500 Internal Server Error\r\n"
                                 "Content-Type: text/plain\r\nConnection: close\r\n\r\n",
                      "x", &connections);
        Response::setReplyTimeout(400);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        Server* server = new Server(QStringLiteral("http://127.0.0.1:%1").arg(listener.serverPort()));
        OperationPage* stream = server->payments().stream().execute();
        QList<int> codes;
        QObject::connect(stream, &Response::error, [&]() {
            auto* bad = dynamic_cast<qstellar::exception::BadResponseException*>(stream->lastException());
            codes << (bad ? bad->getCode() : -1);
        });
        QElapsedTimer waited;
        waited.start();
        while (codes.size() < 2 && waited.elapsed() < 10000)
            QTest::qWait(20);
        delete server;
        QCOMPARE(codes, QList<int>() << 500 << 500);
        QVERIFY2(connections >= 2, qPrintable(QString::number(connections)));
    }

    /**
     * A stream answered 200 with a body that is not an event stream and
     * never ends (a proxy that relabels Horizon's stream as text/plain)
     * says so and reconnects, once per connection. It said so once and then
     * stayed on that connection forever, throwing every byte away: the
     * bytes kept its timeout from firing, and it never reconnected. js
     * 17.1.0's EventSource, served the same, reports 'Invalid content type,
     * expected "text/event-stream"' and reads no more of it; py's
     * requests_sse raises InvalidContentTypeError.
     */
    void testAStreamAnsweredWithAnEndlessPageReconnects()
    {
        int connections = 0;
        QTcpServer listener;
        QVERIFY(listener.listen(QHostAddress::LocalHost, 0));
        serveDripping(&listener, "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\nConnection: close\r\n\r\n",
                      ": keep-alive\n\n", &connections);
        Server* server = new Server(QStringLiteral("http://127.0.0.1:%1").arg(listener.serverPort()));
        OperationPage* stream = server->payments().stream().execute();
        QElapsedTimer clock;
        clock.start();
        QList<qint64> at;
        bool kind = true;
        QObject::connect(stream, &Response::error, [&]() {
            at << clock.elapsed();
            auto* unknown = dynamic_cast<qstellar::exception::UnknownResponseException*>(stream->lastException());
            kind = kind && unknown && QString::fromUtf8(unknown->what()).contains("text/plain");
        });
        while (at.size() < 2 && clock.elapsed() < 10000)
            QTest::qWait(20);
        delete server;
        QCOMPARE(at.size(), 2);
        QVERIFY(kind);
        QVERIFY2(connections >= 2, qPrintable(QString::number(connections)));
        // One error() per connection: the reconnect delay (1 s) between them.
        QVERIFY2(at.at(1) - at.at(0) >= 900, qPrintable(QString::number(at.at(1) - at.at(0))));
    }

    /**
     * A stream, or its Server, can be deleted from the stream's own ready().
     * ~Response deleted the reply on the spot, while the reply was still
     * inside its readyRead emission, and Qt went on using it once the slot
     * returned: a use-after-free (Qt's rule for a reply is deleteLater()).
     * Deleting the Server did the same through its network manager, which
     * deleted every reply it had. js's close() and py's generator have no
     * such trap.
     */
    void testAStreamOrItsServerCanBeDeletedFromItsSlots()
    {
        FakeServer* fake = new FakeServer();
        fake->addStream(QStringLiteral("/payments"), QStringList()
            << (paymentEvent("1", "10.0000000") + paymentEvent("2", "20.0000000"))
            << paymentEvent("3", "30.0000000"));
        Server* server = new Server(fake->baseUrl());
        QPointer<OperationPage> stream = server->payments().stream().execute();
        int ready = 0;
        QObject::connect(stream.data(), &Response::ready, [&]() {
            ++ready;
            delete stream.data();
        });
        WAIT_FOR(stream)
        QVERIFY(!stream);
        QCOMPARE(ready, 1); // the event after it goes with it
        QTest::qWait(200);  // the reply's deferred deletion

        QPointer<Server> owner(server);
        QPointer<OperationPage> second = server->payments().stream().execute();
        QString amount;
        QObject::connect(second.data(), &Response::ready, [&]() {
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(second->streamedElement()))
                amount = p->getAmount();
            delete owner.data();
        });
        WAIT_FOR(owner)
        QVERIFY(!owner);
        QVERIFY(!second); // the Server's responses go with it
        QCOMPARE(amount, QString("30.0000000"));
        QTest::qWait(200); // its network manager's deferred deletion
        fake->deleteLater();
    }

    /**
     * The SSE event type is read as js's EventSource reads it. An "error"
     * event carries its message as text; it was parsed as a record and
     * reported as "JSON parse failed", the message lost. It is reported
     * with its message now, and the stream goes on. An event of another
     * type ("open", "close", or one the SDK does not know) is not a record.
     * js 17.1.0, fed this stream, calls onerror with "problem streaming:
     * boom" and onmessage with 7 (and 9, whose amount it does not check),
     * never with 8. streamedElement() is nullptr before the first record
     * and after an event that could not be read: it indexed an empty list.
     */
    void testStreamEventTypesAreReadAsJsReadsThem()
    {
        QString other = paymentEvent("8", "80.0000000");
        other.replace("id: 8\n", "id: 8\nevent: notice\n");
        QString bad = paymentEvent("9", "90.0000000");
        bad.replace("\"amount\":\"90.0000000\"", "\"amount\":90");
        FakeServer* fake = new FakeServer();
        fake->addStream(QStringLiteral("/payments"), QStringList()
            << (QStringLiteral("retry: 1000\nevent: open\ndata: \"hello\"\n\n"
                               "event: error\ndata: problem streaming: boom\n\n")
                + paymentEvent("7", "70.0000000") + other + bad
                + QStringLiteral("event: close\ndata: \"byebye\"\n\n")));
        Server* server = new Server(fake->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        QStringList amounts, errors;
        QList<bool> noRecord;
        QObject::connect(stream, &Response::ready, [&]() {
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                amounts << p->getAmount();
        });
        QObject::connect(stream, &Response::error, [&]() {
            errors << (stream->lastException() ? QString::fromUtf8(stream->lastException()->what()) : QString());
            noRecord << (stream->streamedElement() == nullptr);
        });
        WAIT_FOR(errors.size() < 2)
        delete server;
        fake->deleteLater();
        QCOMPARE(amounts, QStringList() << "70.0000000");
        QCOMPARE(errors.size(), 2);
        QVERIFY2(errors.at(0).contains("problem streaming: boom"), qPrintable(errors.at(0)));
        QVERIFY2(errors.at(1).contains("amount"), qPrintable(errors.at(1)));
        QCOMPARE(noRecord, QList<bool>() << true << true);
        OperationPage empty(nullptr);
        QVERIFY(empty.streamedElement() == nullptr);
    }

    /**
     * A line may end with a lone CR, as the SSE spec allows and js's
     * eventsource reads (and CRLF, and LF before a CRLF blank line). Only
     * LF and CRLF were known: an event written with CRs never ended, and
     * the stream sat until its timeout. js 17.1.0, fed this stream,
     * delivers both payments.
     */
    void testStreamLinesMayEndWithACarriageReturn()
    {
        QString eight = paymentEvent("8", "80.0000000");
        eight.replace("\n", "\r");
        QString nine = paymentEvent("9", "90.0000000");
        nine.chop(2);
        nine.replace("\n", "\r\n");
        nine += QStringLiteral("\n\r\n:x\r");
        FakeServer* fake = new FakeServer();
        fake->addStream(QStringLiteral("/payments"), QStringList()
            << (QStringLiteral("retry: 1000\r\r") + eight + nine));
        Server* server = new Server(fake->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        QStringList amounts;
        int errors = 0;
        QObject::connect(stream, &Response::ready, [&]() {
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                amounts << p->getAmount();
        });
        QObject::connect(stream, &Response::error, [&]() { ++errors; });
        WAIT_FOR(amounts.size() < 2)
        delete server;
        fake->deleteLater();
        QCOMPARE(amounts, QStringList() << "80.0000000" << "90.0000000");
        QCOMPARE(errors, 0);
    }

    /**
     * Horizon's problem document is read whatever the Content-Type it came
     * with, as js and py read it: behind a proxy that rewrote it to
     * application/json or text/plain, getProblem() was null. py 14.1.0
     * (raise_request_exception) reads title "Transaction Failed" and these
     * result codes for all three types, and js 17.1.0 hands the parsed
     * object over on its error. A body that is not JSON has no problem
     * (py: title None).
     */
    void testProblemIsReadWhateverTheContentType()
    {
        const QString body = QStringLiteral(
            "{\"type\":\"https://stellar.org/horizon-errors/transaction_failed\","
            "\"title\":\"Transaction Failed\",\"status\":400,\"detail\":\"d\","
            "\"extras\":{\"result_codes\":{\"transaction\":\"tx_failed\",\"operations\":[\"op_underfunded\"]}}}");
        const QStringList types = { QStringLiteral("application/problem+json"),
                                    QStringLiteral("application/json"),
                                    QStringLiteral("text/plain; charset=utf-8") };
        for (const QString& type : types) {
            setUp();
            FakeServer::Reply reply;
            reply.body = body;
            reply.responseCode = QStringLiteral("400 Bad Request");
            reply.contentType = type;
            m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), reply);
            AccountResponse* r = requestAccount();
            auto* bad = dynamic_cast<qstellar::exception::BadRequestException*>(r->lastException());
            QVERIFY2(bad && bad->getProblem(), qPrintable(type));
            QCOMPARE(bad->getProblem()->getTitle(), QStringLiteral("Transaction Failed"));
            QCOMPARE(bad->getProblem()->getExtras().getResultCodes().getTransaction(), QStringLiteral("tx_failed"));
            QCOMPARE(bad->getProblem()->getExtras().getResultCodes().getOperations(),
                     QStringList() << QStringLiteral("op_underfunded"));
            delete r;
            tearDown();
        }
        setUp();
        FakeServer::Reply page;
        page.body = QStringLiteral("<html>busy</html>");
        page.responseCode = QStringLiteral("503 Service Unavailable");
        page.contentType = QStringLiteral("text/html");
        m_fakeServer->addGetFull(QString("/accounts/") + sampleAccountId(), page);
        AccountResponse* r = requestAccount();
        auto* busy = dynamic_cast<qstellar::exception::BadResponseException*>(r->lastException());
        QVERIFY(busy);
        QCOMPARE(busy->getCode(), 503);
        QVERIFY(busy->getProblem() == nullptr);
        delete r;
        tearDown();
    }

    /**
     * A problem document's members are what Horizon sends, or the document
     * is not read (tryParse returns nullptr). An operation's result code that
     * was not text was dropped, and the codes after it moved onto the
     * operations before: op_underfunded reported for the wrong operation.
     */
    void testProblemIsReadExactly()
    {
        const QByteArray shifted = "{\"title\":\"Transaction Failed\",\"status\":400,"
            "\"extras\":{\"result_codes\":{\"transaction\":\"tx_failed\","
            "\"operations\":[5,\"op_underfunded\"]}}}";
        QVERIFY(Problem::tryParse(shifted) == nullptr);
        QVERIFY(Problem::tryParse("{\"title\":\"x\",\"status\":\"400\"}") == nullptr);
        QVERIFY(Problem::tryParse("{\"title\":\"x\",\"status\":400.5}") == nullptr);
        QVERIFY(Problem::tryParse("{\"title\":5,\"status\":400}") == nullptr);
        QVERIFY(Problem::tryParse("{\"title\":\"x\",\"extras\":{\"result_codes\":\"tx_failed\"}}") == nullptr);
        std::unique_ptr<Problem> good(Problem::tryParse(
            "{\"title\":\"Transaction Failed\",\"status\":400,\"detail\":null,"
            "\"extras\":{\"result_codes\":{\"transaction\":\"tx_failed\","
            "\"operations\":[\"op_success\",\"op_underfunded\"]}}}"));
        QVERIFY(good != nullptr);
        QCOMPARE(good->getStatus(), 400);
        QCOMPARE(good->getExtras().getResultCodes().getOperations(),
                 QStringList() << "op_success" << "op_underfunded");
    }
};

ADD_TEST(ResponseHandlerTest)
#endif // RESPONSEHANDLERTEST_H
