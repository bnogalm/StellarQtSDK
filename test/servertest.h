#ifndef SERVERTEST_H
#define SERVERTEST_H
#include <QObject>
#include "../src/sep29.h"
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include <QScopeGuard>
#include "testcollector.h"
#include "../src/server.h"
#include "../src/network.h"
#include "../src/transaction.h"
#include "../src/account.h"
#include "../src/createaccountoperation.h"
#include "../src/responses/submittransactionresponse.h"
#include "../src/responses/submittransactionasyncresponse.h"
#include "../src/paymentoperation.h"
#include "../src/pathpaymentstrictreceiveoperation.h"
#include "../src/pathpaymentstrictsendoperation.h"
#include "../src/accountmergeoperation.h"
#include "../src/memo.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/assettypecreditalphanum12.h"
#include "../src/managedataoperation.h"
#include "../src/responses/operationpage.h"
#include "../src/responses/operations/paymentoperationresponse.h"
#include "../src/responses/offerresponse.h"
#include "../src/responses/ledgerresponse.h"
#include "../src/responses/transactionresponse.h"
#include "../src/responses/liquiditypoolresponse.h"
#include "../src/pathpaymentoperation.h"
#include "../src/strkey.h"
#include "../src/exception/accountrequiresmemoexception.h"
#include "../src/exception/connectionerrorexception.h"
#include "../src/exception/toomanyrequestsexception.h"
#include "../src/exception/badresponseexception.h"
#include "../src/exception/requesttimeoutexception.h"
#include "../src/exception/unknownresponseexception.h"
#include "../src/exception/badrequestexception.h"
#include "../src/responses/accountresponse.h"
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "fakeserver.h"

class ServerTest: public QObject
{
    Q_OBJECT


    Server *m_server;
    Transaction * m_transaction=nullptr;
    Transaction * m_transactionWrong=nullptr;

    void setUp()  {
        Network::useTestNetwork();
        //m_server = new Server("https://horizon.stellar.org");
        m_server = new Server("https://horizon-testnet.stellar.org");
    }


    void resetNetwork() {
        Network::use(nullptr);
    }

    /**
     * The following tests are related to SEP-0029.
     */
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_A = "GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_B = "GDUR2DMT5AQ7DJUGBIBB45NKRNQXGRJTWTQ7DPRP37EKBELSMK57RMZK";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_C = "GCS36NBLT6OKYN5EUQOQ7ZZIM6WXXNX5ME4JGTCG3HVZOYXRRMNUHNMM";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_D = "GAKQNN6GNGNPLYBVEDCD5QAIEHAZVNCQET3HAUR4YWQAP5RPBLU2W7UG";
    QString DESTINATION_ACCOUNT_NO_MEMO_REQUIRED = "GDYC2D4P2SRC5DCEDDK2OUFESSPCTZYLDOEF6NYHR2T7X5GUTEABCQC2";
    QString DESTINATION_ACCOUNT_NO_FOUND = "GD2OVSQPGD5FBJPMW4YN3FGDJ7JDFKNOMJT35T4H52FLHXJK5MFSR5RA";
    QString DESTINATION_ACCOUNT_FETCH_ERROR = "GB7WNQUTDLD6YJ4MR3KQN3Y6ZIDIGTA7GRKNH47HOGMP2ETFGRSLD6OG";
    QString DESTINATION_ACCOUNT_MEMO_ID = "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG";

    QString successTransactionResponse ="{\n"
                            "  \"_links\": {\n"
                            "    \"transaction\": {\n"
                            "      \"href\": \"/transactions/2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\"\n"
                            "    }\n"
                            "  },\n"
                            "  \"hash\": \"2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\",\n"
                            "  \"ledger\": 826150,\n"
                            "  \"envelope_xdr\": \"AAAAAKu3N77S+cHLEDfVD2eW/CqRiN9yvAKH+qkeLjHQs1u+AAAAZAAMkoMAAAADAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAbYQq8ek1GitmNBUloGnetfWxSpxlsgK48Xi66dIL3MoAAAAAC+vCAAAAAAAAAAAB0LNbvgAAAEDadQ25SNHWTg0L+2wr/KNWd8/EwSNFkX/ncGmBGA3zkNGx7lAow78q8SQmnn2IsdkD9MwICirhsOYDNbaqShwO\",\n"
                            "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA=\",\n"
                            "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAMmyYAAAAAAAAAAG2EKvHpNRorZjQVJaBp3rX1sUqcZbICuPF4uunSC9zKAAAAAAvrwgAADJsmAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAMmyYAAAAAAAAAAKu3N77S+cHLEDfVD2eW/CqRiN9yvAKH+qkeLjHQs1u+AAAAFzCfYtQADJKDAAAAAwAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAA\"\n"
                            "}";
    // A whole account, as Horizon sends one, with `data` as its data entries.
    // These were the data member alone, which 1.0.4 refuses: an account
    // without its sequence, balances, thresholds, flags or signers is not one
    // Horizon sends, and the lookup now fails instead of passing SEP-29.
    static QString accountWithData(const QString& data, const QString& weight = QStringLiteral("1"))
    {
        return QString("{\"_links\":{\"self\":{\"href\":\"/accounts/GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB\"},"
                       "\"transactions\":{\"href\":\"/accounts/GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB/transactions\"},"
                       "\"operations\":{\"href\":\"/accounts/GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB/operations\"},"
                       "\"effects\":{\"href\":\"/accounts/GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB/effects\"},"
                       "\"offers\":{\"href\":\"/accounts/GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB/offers\"}},"
                       "\"id\":\"GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB\","
                       "\"account_id\":\"GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB\","
                       "\"sequence\":\"3298702387052545\",\"subentry_count\":1,"
                       "\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},"
                       "\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false},"
                       "\"balances\":[{\"balance\":\"9999.9999900\",\"buying_liabilities\":\"0.0000000\","
                       "\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],"
                       "\"signers\":[{\"weight\":%2,\"key\":\"GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB\","
                       "\"type\":\"ed25519_public_key\"}],"
                       "\"data\":%1,\"num_sponsoring\":0,\"num_sponsored\":0,"
                       "\"paging_token\":\"GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB\"}").arg(data, weight);
    }
    QString memoRequiredResponse = accountWithData("{\"config.memo_required\":\"MQ==\"}");
    QString noMemoRequiredResponse = accountWithData("{}");

    QString resourceMissingResponse = "{\n"
                                      " \"type\": \"https://stellar.org/horizon-errors/not_found\",\n"
                                      " \"title\": \"Resource Missing\",\n"
                                      " \"status\": 404,\n"
                                      " \"detail\": \"The resource at the url requested was not found.  This usually occurs for one of two reasons:  The url requested is not valid, or no data in our database could be found with the parameters provided.\"\n"
                                    "}";

    FeeBumpTransaction* feeBump(Transaction* inner) {
        KeyPair* signer = KeyPair::fromSecretSeed(QString("SA5ZEFDVFZ52GRU7YUGR6EDPBNRU2WLA6IQFQ7S2IH2DG3VFV3DOMV2Q"));
        FeeBumpTransaction* tx =  FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                .setFeeAccount(signer->getAccountId())
                .setBaseFee(FeeBumpTransaction::MIN_BASE_FEE*10)
                .build();
        tx->sign(signer);
        return tx;
    }

    // Builds one SSE payment event: "id: <id>\ndata: {payment ...}\n\n".
    static QString paymentEvent(const QString& id, const QString& amount) {
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

    /** The class of a request's failure, most derived first, by name. */
    static QString failureKind(qstellar::exception::SdkException* ex) {
        using namespace qstellar::exception;
        if (!ex) return "none";
        if (dynamic_cast<AccountRequiresMemoException*>(ex)) return "memoRequired";
        if (dynamic_cast<TooManyRequestsException*>(ex)) return "rateLimited";
        if (dynamic_cast<RequestTimeoutException*>(ex)) return "timeout";
        if (dynamic_cast<BadResponseException*>(ex)) return "badResponse";
        if (dynamic_cast<BadRequestException*>(ex)) return "badRequest";
        if (dynamic_cast<UnknownResponseException*>(ex)) return "unknownResponse";
        if (dynamic_cast<ConnectionErrorException*>(ex)) return "connection";
        return "other";
    }

    /** How a plain lookup of `accountId` on `server` fails (see failureKind). */
    static QString lookupFailureKind(Server* server, const QString& accountId) {
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(accountId));
        AccountResponse* lookup = server->accounts().account(key.data());
        bool answered = false;
        QObject::connect(lookup, &Response::ready, [&answered]() { answered = true; });
        QObject::connect(lookup, &Response::error, [&answered]() { answered = true; });
        WAIT_FOR(!answered)
        const QString kind = failureKind(lookup->lastException());
        delete lookup;
        return kind;
    }
private slots:

    void initTestCase()
    {
        qDebug() <<"qtcpp stellar client name:"<<STELLAR_QT_SDK_CLIENT_NAME;
        qDebug() <<"qtcpp stellar client version:"<< STELLAR_QT_SDK_CLIENT_VERSION;
        this->setUp();
    }
    void cleanupTestCase()
    {
        qApp->processEvents();
    }

    void testBuildTransaction()
    {
        qint64 sequenceNumber=5026524780560386;
        KeyPair *source = KeyPair::fromSecretSeed(QString("SDPK5IBB57JY5SMMBUBOFSAHNNCETCZWLTKKDPWBDYXBX3B4ZAFOQYD5"));

        KeyPair *destination = KeyPair::fromSecretSeed(QByteArray::fromHex("268486538a268486538f268486538a268486538f268486538a268486538feedd"));


        Account* account = new Account(source, sequenceNumber);
        Transaction::Builder *builder = new Transaction::Builder(AccountConverter().enableMuxed(), account);
        builder->addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE);

        QVERIFY(1 == builder->getOperationsCount());
        Transaction *transaction = builder->build();
        delete builder;
        QVERIFY((sequenceNumber+1) == transaction->getSequenceNumber());
        QVERIFY((sequenceNumber+1) == account->getSequenceNumber());
        transaction->sign(source);
        m_transaction=transaction;

    }

    // P1.11 — first SSE streaming test (FakeServer streaming mode).
    // Uses a LOCAL Server that is deleted at the end so the stream's reconnect
    // timer is torn down (no lingering reconnect loop after the test).
    void testStreamReceivesData() {
        FakeServer* fakeServer = new FakeServer();
        QString round1 =
            "id: 1\n"
            "data: {\"_links\":{\"self\":{\"href\":\"/operations/1\"},\"transaction\":{\"href\":\"/transactions/2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\"},"
            "\"effects\":{\"href\":\"/operations/1/effects\"},\"succeeds\":{\"href\":\"/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"/effects?order=asc&cursor=1\"}},"
            "\"type\":\"payment\",\"type_i\":1,\"id\":\"1\",\"paging_token\":\"1\","
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_successful\":true,"
            "\"transaction_hash\":\"2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\","
            "\"source_account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"asset_type\":\"native\","
            "\"from\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"to\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"amount\":\"10.0000000\"}\n"
            "\n";
        fakeServer->addStream("/payments", QStringList() << round1);

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        bool gotEvent = false;
        QObject::connect(stream, &Response::ready, [&gotEvent](){ gotEvent = true; });
        WAIT_FOR(!gotEvent)

        QVERIFY(gotEvent);
        QVERIFY(stream->size() >= 1);
        PaymentOperationResponse* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement());
        QVERIFY(p != nullptr);
        QCOMPARE(p->getAmount(), QString("10.0000000"));

        delete server;            // tears down the streaming response + reconnect timer
        fakeServer->deleteLater();
    }

    // The stream closes after round 1; the SDK reconnects (after >= RECONNECT_DELAY)
    // sending `Last-Event-ID`, and round 2 is served.
    void testStreamReconnectsWithLastEventId() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << paymentEvent("5", "10.0000000")
            << paymentEvent("6", "20.0000000"));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        int events = 0;
        QString lastAmount;
        QObject::connect(stream, &Response::ready, [&](){
            events++;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        WAIT_FOR(events < 2)   // the 2nd event only arrives after a reconnect (~1s)

        QVERIFY(events >= 2);                            // reconnected
        QCOMPARE(lastAmount, QString("20.0000000"));      // round 2 served
        // The reconnect must carry the last seen id. The SDK sets the raw header
        // as "Last-Event-ID", but Qt re-cases the name on the wire and the casing
        // differs by version: Qt 6.8-6.10 send it lowercase ("last-event-id"),
        // Qt 6.11 title-cases it ("Last-Event-Id"). HTTP header names are
        // case-insensitive (RFC 7230 §3.2), so match case-insensitively rather
        // than pinning one Qt version's casing.
        QVERIFY(fakeServer->lastRequestHeaders().toLower().contains("last-event-id: 5"));

        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A connection that ended in the middle of an event left the half event
     * in the buffer; after the reconnect it was joined with the new
     * connection's first bytes and read as one event carrying the lost
     * event's id. That id was taken as seen (a parse error was all the caller
     * got), and the next reconnect asked to resume AFTER the event that was
     * never delivered. Here event 6 is cut on two connections in a row: every
     * reconnect must still ask for what follows 5, no error may be reported,
     * and 6 must arrive (js's EventSource behaves so).
     */
    void testStreamDropsAHalfEventOnReconnect() {
        const QString six = paymentEvent("6", "20.0000000");
        const QString halfSix = six.left(six.size() / 2);
        const QString preamble = QStringLiteral("retry: 1000\nevent: open\ndata: \"hello\"\n\n");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << (preamble + paymentEvent("5", "10.0000000") + halfSix)
            << (preamble + halfSix)
            << (preamble + six));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();
        QStringList amounts;
        int errors = 0;
        QObject::connect(stream, &Response::ready, [&]() {
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                amounts << p->getAmount();
        });
        QObject::connect(stream, &Response::error, [&]() { ++errors; });
        WAIT_FOR(amounts.size() < 2)

        QCOMPARE(amounts, QStringList({QStringLiteral("10.0000000"), QStringLiteral("20.0000000")}));
        QCOMPARE(errors, 0);
        const QByteArray headers = fakeServer->lastRequestHeaders().toLower();
        QCOMPARE(headers.count("last-event-id: 5"), 2);
        QCOMPARE(headers.count("last-event-id: 6"), 0);

        delete server;
        fakeServer->deleteLater();
    }

    /**
     * /trades streams, as js's and py's trades().stream(): the trade
     * Horizon pushes is the page's streamedElement(). The builder had no
     * stream(), so a trade ticker had to poll.
     */
    void testTradesStream() {
        const QString trade =
            "{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"\"},\"counter\":{\"href\":\"\"},\"operation\":{\"href\":\"\"}},"
            "\"id\":\"123456789-0\",\"paging_token\":\"123456789-0\",\"ledger_close_time\":\"2021-11-18T03:47:47Z\","
            "\"trade_type\":\"liquidity_pool\",\"liquidity_pool_fee_bp\":30,"
            "\"base_liquidity_pool_id\":\"abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789\","
            "\"base_amount\":\"10.0000000\",\"base_asset_type\":\"native\","
            "\"counter_account\":\"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\","
            "\"counter_amount\":\"2.6700000\",\"counter_asset_type\":\"credit_alphanum4\",\"counter_asset_code\":\"JPY\","
            "\"counter_asset_issuer\":\"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\","
            "\"base_is_seller\":true,\"price\":{\"n\":267,\"d\":1000}}";
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/trades", QStringList() << ("id: 123456789-0\ndata: " + trade + "\n\n"));

        Server* server = new Server(fakeServer->baseUrl());
        Page<TradeResponse>* stream = server->trades().cursor("now").stream().execute();
        QString id;
        QString tradeType;
        QObject::connect(stream, &Response::ready, [&]() {
            id = stream->streamedElement()->getId();
            tradeType = stream->streamedElement()->getTradeType();
        });
        WAIT_FOR(id.isEmpty())

        QCOMPARE(id, QString("123456789-0"));
        QCOMPARE(tradeType, QString("liquidity_pool"));
        QVERIFY(fakeServer->lastRequestHeaders().toLower().contains("accept: text/event-stream"));
        delete server;
        fakeServer->deleteLater();
    }

    // Two SSE events delivered in a single connection chunk → two `ready()`s.
    void testStreamParsesMultipleEvents() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << (paymentEvent("1", "10.0000000") + paymentEvent("2", "20.0000000")));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        int events = 0;
        QString lastAmount;
        QObject::connect(stream, &Response::ready, [&](){
            events++;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        WAIT_FOR(events < 2)

        QVERIFY(events >= 2);
        QCOMPARE(lastAmount, QString("20.0000000"));

        delete server;
        fakeServer->deleteLater();
    }

    // Horizon sends `data: "hello"` keep-alives to hold the stream open; the
    // parser must skip them (no ready()) and surface only real events.
    void testStreamSkipsKeepAlive() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << (QString("data: \"hello\"\n\n") + paymentEvent("7", "42.0000000")));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        int events = 0;
        QString lastAmount;
        QObject::connect(stream, &Response::ready, [&](){
            events++;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        WAIT_FOR(events < 1)

        QCOMPARE(events, 1);                         // keep-alive did NOT emit
        QCOMPARE(lastAmount, QString("42.0000000")); // the real payment did
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * Deleting a Server ends the requests whose responses are the caller's
     * (submitTransaction, submitTransactionAsync, checkMemoRequired) with
     * error(), a ConnectionErrorException saying the Server was deleted.
     * They waited forever: the POST's reply went with the Server's network
     * manager without finishing, and the SEP-29 lookups and the queued start
     * of checkMemoRequired went with the Server. js and py have no Server to
     * delete: their promise or call always settles.
     */
    void testDeletingTheServerEndsTheCallersRequests() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHang("/transactions");
        fakeServer->addHang("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED);
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        auto payment = [&]() {
            Account* account = new Account(source, 1L);
            Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .setBaseFee(100)
                    .build();
            transaction->sign(source);
            return transaction;
        };
        // How `response` ends once `server` is deleted `afterMs` after the request.
        auto ending = [](Response* response, Server* server, int afterMs) {
            bool done = false;
            QString outcome;
            QMetaObject::Connection c1 = QObject::connect(response, &Response::ready, [&]() {
                done = true;
                outcome = QStringLiteral("ready");
            });
            QMetaObject::Connection c2 = QObject::connect(response, &Response::error, [&]() {
                done = true;
                outcome = failureKind(response->lastException());
                if (response->lastException())
                    outcome += QStringLiteral(": ") + QString::fromUtf8(response->lastException()->what());
            });
            if (afterMs)
                QTest::qWait(afterMs);
            delete server;
            WAIT_FOR(!done)
            QObject::disconnect(c1);
            QObject::disconnect(c2);
            return outcome;
        };
        const auto abandoned = [](const QString& outcome) {
            return outcome.startsWith(QStringLiteral("connection: ")) && outcome.contains(QStringLiteral("Server was deleted"));
        };

        // The POST is out.
        Server* server = new Server(fakeServer->baseUrl());
        SubmitTransactionResponse* submitted = server->submitTransaction(payment(), true);
        QString outcome = ending(submitted, server, 300);
        delete submitted;
        QVERIFY2(abandoned(outcome), qPrintable(outcome));

        // A SEP-29 lookup is out.
        Transaction* transaction = payment();
        server = new Server(fakeServer->baseUrl());
        SubmitTransactionAsyncResponse* async = server->submitTransactionAsync(transaction);
        outcome = ending(async, server, 300);
        delete async;
        QVERIFY2(abandoned(outcome) && outcome.contains("SEP-29"), qPrintable(outcome));

        server = new Server(fakeServer->baseUrl());
        MemoRequiredCheck* check = server->checkMemoRequired(transaction);
        outcome = ending(check, server, 300);
        delete check;
        QVERIFY2(abandoned(outcome) && outcome.contains("SEP-29"), qPrintable(outcome));

        // Deleted before checkMemoRequired started.
        server = new Server(fakeServer->baseUrl());
        check = server->checkMemoRequired(transaction);
        outcome = ending(check, server, 0);
        delete check;
        QVERIFY2(abandoned(outcome) && outcome.contains("SEP-29"), qPrintable(outcome));

        delete transaction;
        fakeServer->deleteLater();
    }
#ifndef STELLAR_SKIP_LIVE_TESTS
    //it will fail because sequence number, it should be catched before creating the transaction
     void testSubmitTransactionSuccess()  {
         QVERIFY(m_transaction);
         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             qDebug() << "RECEIVED ANSWER";
             r = response;

         });         
         m_server->submitTransaction(this->m_transaction,true);         
         WAIT_FOR(!r)
         QObject::disconnect(c);
         QVERIFY(r!=nullptr);
         QVERIFY(r->isSuccess());
         QVERIFY(r->getLedger()==1676526);
         QString expected("732ac60a45c9bc86fcf14292bb7c7ebf6fec4a83767c1fc09148ac946e4b2ef1");
         QVERIFY(r->getHash() == expected);
         QVERIFY(r->getExtras().getTransactionResultCode().isEmpty());
     }
#endif
     void testBuildWrongTransaction()
     {
         qint64 sequenceNumber=1;
         KeyPair *source = KeyPair::fromSecretSeed(QString("SDPK5IBB57JY5SMMBUBOFSAHNNCETCZWLTKKDPWBDYXBX3B4ZAFOQYD5"));
         KeyPair *destination = KeyPair::fromSecretSeed(QByteArray::fromHex("268486538a268486538f268486538a268486538f268486538a268486538feedd"));


         Account* account = new Account(source, sequenceNumber);
         Transaction::Builder *builder = new Transaction::Builder(AccountConverter().enableMuxed(), account);
         builder->addOperation(new CreateAccountOperation(destination, "2000"))
                 .addMemo(Memo::text("Hello world!")).setTimeout(Transaction::Builder::TIMEOUT_INFINITE).setBaseFee(Transaction::Builder::BASE_FEE);

         QVERIFY(1 == builder->getOperationsCount());
         Transaction *transaction = builder->build();
         delete builder;
         QVERIFY((sequenceNumber+1) == transaction->getSequenceNumber());
         QVERIFY((sequenceNumber+1) == account->getSequenceNumber());
         transaction->sign(source);
         m_transactionWrong=transaction;

     }
#ifndef STELLAR_SKIP_LIVE_TESTS
     void testSubmitTransactionFail() {
         QVERIFY(m_transactionWrong);
         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
                 qDebug() << "RECEIVED ANSWER";
                 r = response;

        });
         m_server->submitTransaction(this->m_transactionWrong,true);
         WAIT_FOR(!r)
                 QObject::disconnect(c);
         QVERIFY(r!=nullptr);
         QVERIFY(!r->isSuccess());
         QVERIFY(r->getLedger()==0);
         QVERIFY(r->getHash().isEmpty());
         QVERIFY(r->getEnvelopeXdr()== "AAAAAJClcQcwAoPH4NIUph8BzrQftysWUPdiDVrFVaU5BDMxAAAAZAAAAAAAAAACAAAAAAAAAAEAAAAMSGVsbG8gd29ybGQhAAAAAQAAAAAAAAAAAAAAADpuIIngCOUxqmiav7VlDapL++s8fBxj/DwSy2GyjpInAAAABKgXyAAAAAAAAAAAATkEMzEAAABAMJ4mVb8xU+o4qr6z+y74qjEvPgcU2tigQVMc98pCMucdE4xjfV6cWlMjLPy7OdvhgTB/QcEOw6VoCS42xhMeBQ==");
         QVERIFY(r->getExtras().getResultXdr() =="AAAAAAAAAAD////7AAAAAA==");
         QVERIFY(r->getExtras().getResultCodes().getTransactionResultCode()=="tx_bad_seq");
     }
#endif



     void testCheckMemoRequiredWithMemo() {

         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .addMemo(new MemoText("Hello, Stellar."))
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)

         QVERIFY(r);

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();

     }


     void testSubmitTimeout504IsDetected() {
         // S3 (fund-safety): a Horizon 504 Gateway Timeout MUST be detectable so
         // callers resend the SAME signed envelope (idempotent) instead of
         // rebuilding with a new sequence — rebuilding after a 504 can apply the
         // payment twice. Regression guard: Response::m_status was never
         // populated from the HTTP status code (it defaulted to 0), so
         // isTimeout() always returned false until preprocessResponse() was
         // fixed to set it.

         // --- 504 path: delivered via transactionError, flagged as a timeout ---
         FakeServer* timeoutServer = new FakeServer();
         // Body intentionally has NO "status" field, so getStatusCode()==504 can
         // only come from the HTTP status line, not from JSON reflection.
         timeoutServer->addPost("/transactions",
                                QStringLiteral("{\"title\":\"Gateway Timeout\"}"),
                                QStringLiteral("504 Gateway Timeout"));
         Server* tServer = new Server(timeoutServer->baseUrl());

         KeyPair* source1 = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* a1 = new Account(source1, 1L);
         Transaction* tx1 = Transaction::Builder(AccountConverter().enableMuxed(), a1)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();
         tx1->sign(source1);

         SubmitTransactionResponse* rErr=nullptr;
         QObject::connect(tServer,&Server::transactionError,[&rErr](SubmitTransactionResponse* response){ rErr = response; });
         tServer->submitTransaction(tx1, true);
         WAIT_FOR(!rErr)
         QVERIFY(rErr);                        // arrived on the error path
         QVERIFY(rErr->isTimeout());           // 504 detected
         QCOMPARE(rErr->getStatusCode(), 504);
         // The caller's to delete. Left alive, it outlived its reply (deleted
         // with tServer) and its reply-timeout timer went off half a minute
         // later, in whichever test was running then.
         delete rErr;
         delete tServer;
         timeoutServer->deleteLater();

         // --- 2xx path: real code surfaced, NOT a timeout ---
         FakeServer* okServer = new FakeServer();
         okServer->addPost("/transactions", successTransactionResponse);
         Server* oServer = new Server(okServer->baseUrl());

         KeyPair* source2 = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* a2 = new Account(source2, 1L);
         Transaction* tx2 = Transaction::Builder(AccountConverter().enableMuxed(), a2)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();
         tx2->sign(source2);

         SubmitTransactionResponse* rOk=nullptr;
         QObject::connect(oServer,&Server::transactionResponse,[&rOk](SubmitTransactionResponse* response){ rOk = response; });
         oServer->submitTransaction(tx2, true);
         WAIT_FOR(!rOk)
         QVERIFY(rOk);
         QVERIFY(!rOk->isTimeout());           // 2xx is not a timeout
         QCOMPARE(rOk->getStatusCode(), 200);  // and surfaces the real HTTP code
         delete rOk;
         delete oServer;
         okServer->deleteLater();
     }
     void testCheckMemoRequiredWithMemoIdAddress()
     {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction* transaction =nullptr;
         try{
             transaction = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::current())
                     .addOperation(new PaymentOperation(DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeNative(), "10"))
                     .addOperation(new PathPaymentStrictReceiveOperation(new AssetTypeNative(), "10", DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                     .addOperation(new PathPaymentStrictSendOperation(new AssetTypeNative(), "10", DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                     .addOperation(new AccountMergeOperation(DESTINATION_ACCOUNT_MEMO_ID))
                     .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                     .setBaseFee(100)
                     .build();
             //QFAIL("Missing exception");
         } catch (const std::runtime_error& e) {
             //qDebug() << "EXCEPTION "<< e.what();
             //QCOMPARE("invalid address length", QString(e.what()));
             QFAIL("Muxed accounts not working");
             Q_UNUSED(e)
         }

         //QVERIFY(transaction==nullptr);//we dont even allow to build the transaction if destination is wrong.
         QVERIFY(transaction);//muxed accounts enabled
//         transaction->sign(source);

//         SubmitTransactionResponse *r=nullptr;
//         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
//             r = response;

//         });
//         m_server->submitTransaction(transaction);

//         WAIT_FOR(!r)

//         QVERIFY(r);

//         r=nullptr;

//         m_server->submitTransaction(feeBump(transaction));
//         WAIT_FOR(!r)

//         QVERIFY(r);
         fakeServer->deleteLater();

     }

     void testCheckMemoRequiredWithSkipCheck(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;

         });
         m_server->submitTransaction(transaction,true);

         WAIT_FOR(!r)


         QVERIFY(r);
         r=nullptr;

         m_server->submitTransaction(feeBump(transaction),true);
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredWithPaymentOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();

     }
     void testCheckMemoRequiredWithPathPaymentStrictReceiveOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredWithPathPaymentStrictSendOperationNoMemo(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredWithAccountMergeOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredTwoOperationsWithSameDestination(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredNoDestinationOperation() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(ManageDataOperation::create("Hello", "Stellar"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredAccountNotFound(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)



         QVERIFY(r);
         fakeServer->deleteLater();

     }

     void testCheckMemoRequiredAccountNotFoundBump(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });

         m_server->submitTransaction(feeBump(transaction));

         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredFetchAccountError() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_A,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_B,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_C,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_D,memoRequiredResponse);

         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_FETCH_ERROR), new AssetTypeNative(), "10"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);         
         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));        
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

    /**
     * SEP-29 recognised operations by C++ class, and Operation::fromXdr
     * built the deprecated PathPaymentOperation, then a separate class, for
     * a strict-receive path payment: a decoded envelope (a SEP-7 request, one relayed for signing)
     * paying a memo-required account without a memo was posted unchecked.
     * The check now reads the envelope it posts. A refusal also used to be a
     * bare error; it names the account and the operation now.
     */
    void testCheckMemoRequiredDecodedPathPaymentStrictReceive() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, noMemoRequiredResponse);
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_B, memoRequiredResponse);
        Server* server = new Server(fakeServer->baseUrl());

        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* built = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        built->sign(source);

        Transaction* decoded = dynamic_cast<Transaction*>(
            AbstractTransaction::fromEnvelopeXdr(built->toEnvelopeXdrBase64(), Network::current()));
        QVERIFY(decoded);
        // Decoded, it is the deprecated class, which is a strict-receive one
        // now (it was a separate class, and SEP-29 missed it).
        QVERIFY(dynamic_cast<PathPaymentOperation*>(decoded->getOperations().at(1)));
        QVERIFY(dynamic_cast<PathPaymentStrictReceiveOperation*>(decoded->getOperations().at(1)));

        const QList<AbstractTransaction*> submissions{ decoded, feeBump(decoded) };
        for (AbstractTransaction* tx : submissions) {
            SubmitTransactionResponse* sent = nullptr;
            SubmitTransactionResponse* refused = nullptr;
            QMetaObject::Connection c1 = QObject::connect(server, &Server::transactionResponse,
                [&sent](SubmitTransactionResponse* response) { sent = response; });
            QMetaObject::Connection c2 = QObject::connect(server, &Server::transactionError,
                [&refused](SubmitTransactionResponse* response) { refused = response; });
            server->submitTransaction(tx);
            WAIT_FOR(!sent && !refused)
            QObject::disconnect(c1);
            QObject::disconnect(c2);

            QVERIFY(!sent);
            QVERIFY(refused);
            auto* memo = dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(
                refused->lastException());
            QVERIFY(memo);
            QCOMPARE(memo->getAccountId(), DESTINATION_ACCOUNT_MEMO_REQUIRED_B);
            QCOMPARE(memo->getOperationIndex(), 1);
        }
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * Any memo answers SEP-29, id 0 and an empty text included, as js's
     * checkMemoRequired (memo.type !== "none") and py's (not NoneMemo)
     * decide: both post such a payment to an account that asks for a memo,
     * which this refused with AccountRequiresMemoException.
     */
    void testCheckMemoRequiredAnyMemoCounts() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_A, memoRequiredResponse);
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));

        auto outcome = [&](Memo* memo) {
            Account* account = new Account(source, 1L);
            Transaction::Builder builder(AccountConverter().enableMuxed(), account);
            builder.addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                   .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                   .setBaseFee(100);
            if (memo)
                builder.addMemo(memo);
            QScopedPointer<MemoRequiredCheck> check(server->checkMemoRequired(builder.build()));
            bool ready = false, failed = false;
            QObject::connect(check.data(), &Response::ready, [&ready]() { ready = true; });
            QObject::connect(check.data(), &Response::error, [&failed]() { failed = true; });
            WAIT_FOR(!ready && !failed)
            return (ready && !failed) ? QString("ready") : failureKind(check->lastException());
        };

        QCOMPARE(outcome(nullptr), QString("memoRequired"));
        QCOMPARE(outcome(Memo::id(0)), QString("ready"));
        QCOMPARE(outcome(Memo::text("")), QString("ready"));
        delete server;
        fakeServer->deleteLater();
    }

    /** SEP-29's names, public: the value is base64 of the one an account sets. */
    void testSep29ConstantsArePublic() {
        QCOMPARE(QString::fromLatin1(Sep29::DATA_NAME), QString("config.memo_required"));
        QCOMPARE(QByteArray(Sep29::DATA_VALUE).toBase64(), QByteArray(Sep29::DATA_VALUE_BASE64));
    }

    /**
     * SEP-29 on its own, as js's checkMemoRequired: the same check, and
     * nothing posted (the fake server has no /transactions to post to).
     */
    void testCheckMemoRequiredWithoutSubmitting() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, noMemoRequiredResponse);
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_B, memoRequiredResponse);
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));

        auto build = [&](const QString& second, Memo* memo) {
            Account* account = new Account(source, 1L);
            Transaction::Builder builder(AccountConverter().enableMuxed(), account);
            builder.addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                   .addOperation(PaymentOperation::create(KeyPair::fromAccountId(second), new AssetTypeNative(), "10"))
                   .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                   .setBaseFee(100);
            if (memo)
                builder.addMemo(memo);
            return builder.build();
        };
        auto outcome = [&](AbstractTransaction* tx) {
            QScopedPointer<MemoRequiredCheck> check(server->checkMemoRequired(tx));
            bool ready = false, failed = false;
            QObject::connect(check.data(), &Response::ready, [&ready]() { ready = true; });
            QObject::connect(check.data(), &Response::error, [&failed]() { failed = true; });
            WAIT_FOR(!ready && !failed)
            if (ready && !failed)
                return QString("ready");
            auto* memo = dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(check->lastException());
            return memo ? QString("%1 #%2").arg(memo->getAccountId()).arg(memo->getOperationIndex())
                        : failureKind(check->lastException());
        };

        QCOMPARE(outcome(build(DESTINATION_ACCOUNT_MEMO_REQUIRED_B, nullptr)),
                 DESTINATION_ACCOUNT_MEMO_REQUIRED_B + " #1");
        QCOMPARE(outcome(build(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, nullptr)), QString("ready"));
        // A memo makes the lookups unnecessary: answered all the same, and queued.
        QCOMPARE(outcome(build(DESTINATION_ACCOUNT_MEMO_REQUIRED_B, Memo::text("hi"))), QString("ready"));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * POST /transactions_async, as js's submitAsyncTransaction and py's
     * submit_transaction_async: core's verdict at once. PENDING (201) is
     * ready(); DUPLICATE (409), ERROR (400, with the result) and
     * TRY_AGAIN_LATER (503) are errors, as js rejects and py raises them,
     * the verdict read from the body all the same. A problem document is
     * an error with no verdict.
     */
    void testSubmitTransactionAsync() {
        const QString hash("2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42");
        FakeServer* fakeServer = new FakeServer();
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        // "ready" or the failure's kind, then the verdict, the hash and the result.
        auto submit = [&](const QString& body, const QString& status) {
            fakeServer->addPost("/transactions_async", body, status);
            QScopedPointer<SubmitTransactionAsyncResponse> response(server->submitTransactionAsync(transaction, true));
            bool ready = false, failed = false;
            QObject::connect(response.data(), &Response::ready, [&ready]() { ready = true; });
            QObject::connect(response.data(), &Response::error, [&failed]() { failed = true; });
            WAIT_FOR(!ready && !failed)
            return QStringList() << ((ready && !failed) ? QString("ready") : failureKind(response->lastException()))
                                 << response->getTxStatus() << response->getHash()
                                 << response->getErrorResultXdr();
        };

        QCOMPARE(submit("{\"tx_status\":\"PENDING\",\"hash\":\"" + hash + "\"}", "201 Created"),
                 QStringList({"ready", "PENDING", hash, ""}));
        QCOMPARE(submit("{\"tx_status\":\"DUPLICATE\",\"hash\":\"" + hash + "\"}", "409 Conflict"),
                 QStringList({"badRequest", "DUPLICATE", hash, ""}));
        // txBAD_SEQ, fee 100 (py: TransactionResult.from_xdr).
        QCOMPARE(submit("{\"tx_status\":\"ERROR\",\"hash\":\"" + hash + "\",\"error_result_xdr\":\"AAAAAAAAAGT////7AAAAAA==\"}", "400 Bad Request"),
                 QStringList({"badRequest", "ERROR", hash, "AAAAAAAAAGT////7AAAAAA=="}));
        QCOMPARE(submit("{\"tx_status\":\"TRY_AGAIN_LATER\",\"hash\":\"" + hash + "\"}", "503 Service Unavailable"),
                 QStringList({"badResponse", "TRY_AGAIN_LATER", hash, ""}));
        QCOMPARE(submit("{\"type\":\"https://stellar.org/horizon-errors/transaction_malformed\",\"title\":\"Transaction Malformed\",\"status\":400}", "400 Bad Request"),
                 QStringList({"badRequest", "", "", ""}));
        // A 201 that is not a verdict.
        QCOMPARE(submit("{\"hash\":\"" + hash + "\"}", "201 Created").first(), QString("unknownResponse"));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * transactions_async runs SEP-29 first, as js's submitAsyncTransaction
     * does unless told to skip it: nothing is posted to an account that
     * requires a memo.
     */
    void testSubmitTransactionAsyncChecksMemoRequired() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions_async", "{\"tx_status\":\"PENDING\",\"hash\":\"2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\"}", "201 Created");
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_A, memoRequiredResponse);
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        auto outcome = [&](bool skip) {
            QScopedPointer<SubmitTransactionAsyncResponse> response(server->submitTransactionAsync(transaction, skip));
            bool ready = false, failed = false;
            QObject::connect(response.data(), &Response::ready, [&ready]() { ready = true; });
            QObject::connect(response.data(), &Response::error, [&failed]() { failed = true; });
            WAIT_FOR(!ready && !failed)
            auto* memo = dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(response->lastException());
            if (memo)
                return QString("%1 #%2 %3").arg(memo->getAccountId()).arg(memo->getOperationIndex())
                                           .arg(response->getTxStatus().isEmpty() ? QString("not posted") : QString("posted"));
            return (ready && !failed) ? response->getTxStatus() : failureKind(response->lastException());
        };

        QCOMPARE(outcome(false), DESTINATION_ACCOUNT_MEMO_REQUIRED_A + " #0 not posted");
        QCOMPARE(outcome(true), QString("PENDING"));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * An M destination under a converter that disables muxing is signed as
     * its G account, which has no id of its own to stand for the memo. It
     * was skipped as "muxed" all the same, so the G account was paid with no
     * memo and no check.
     */
    void testCheckMemoRequiredMuxedDestinationSignedAsItsAccount() {
        const QString gAccount = StrKey::encodeStellarAccountId(
            StrKey::muxedAccountToAccountId(StrKey::encodeToXDRMuxedAccount(DESTINATION_ACCOUNT_MEMO_ID)));
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        fakeServer->addGet("/accounts/" + gAccount, memoRequiredResponse);
        Server* server = new Server(fakeServer->baseUrl());

        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().disableMuxed(), account)
                .addOperation(new PaymentOperation(DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        SubmitTransactionResponse* sent = nullptr;
        SubmitTransactionResponse* refused = nullptr;
        QObject::connect(server, &Server::transactionResponse,
            [&sent](SubmitTransactionResponse* response) { sent = response; });
        QObject::connect(server, &Server::transactionError,
            [&refused](SubmitTransactionResponse* response) { refused = response; });
        server->submitTransaction(transaction);
        WAIT_FOR(!sent && !refused)

        QVERIFY(!sent);
        QVERIFY(refused);
        auto* memo = dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(
            refused->lastException());
        QVERIFY(memo);
        QCOMPARE(memo->getAccountId(), gAccount);
        QCOMPARE(memo->getOperationIndex(), 0);
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A lookup that keeps failing stops the submission too, but as the
     * network failure it is: both used to be the same bare error, with no
     * way to tell "this account wants a memo" from "Horizon did not answer".
     */
    void testCheckMemoRequiredLookupFailureIsNotAMemoRefusal() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        Server* server = new Server(fakeServer->baseUrl());

        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_FETCH_ERROR), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        SubmitTransactionResponse* sent = nullptr;
        SubmitTransactionResponse* refused = nullptr;
        QObject::connect(server, &Server::transactionResponse,
            [&sent](SubmitTransactionResponse* response) { sent = response; });
        QObject::connect(server, &Server::transactionError,
            [&refused](SubmitTransactionResponse* response) { refused = response; });
        server->submitTransaction(transaction);
        WAIT_FOR(!sent && !refused)

        QVERIFY(!sent);
        QVERIFY(refused);
        QVERIFY(!dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(refused->lastException()));
        // Reported as the failure the lookup met. The fake answers the
        // unrouted lookup with a status line that has no code, which Qt reads
        // as an unreadable reply or as a dropped connection depending on its
        // version, so the expected kind is the one a plain lookup of the same
        // account meets. This expected a ConnectionErrorException because
        // every lookup failure became one (see
        // testCheckMemoRequiredLookupFailureKeepsItsKind).
        const QString lookupKind = lookupFailureKind(server, DESTINATION_ACCOUNT_FETCH_ERROR);
        QVERIFY(lookupKind != "none" && lookupKind != "other" && lookupKind != "memoRequired");
        QCOMPARE(failureKind(refused->lastException()), lookupKind);
        QVERIFY(QString::fromUtf8(refused->lastException()->what()).contains(DESTINATION_ACCOUNT_FETCH_ERROR));
        QVERIFY(QString::fromUtf8(refused->lastException()->what()).contains("SEP-29"));
        delete refused;
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A failed lookup is reported as what it was. Every failure used to become
     * a ConnectionErrorException after being asked again at once: a 429 lost
     * its rate-limit kind and its Retry-After and was retried immediately,
     * spending the allowance it had asked the client to wait for, a 5xx or a
     * 504 lost its status, and a reply that was not JSON read as a lost
     * connection. The message still names the account and SEP-29; the others
     * are still asked twice.
     */
    void testCheckMemoRequiredLookupFailureKeepsItsKind() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        FakeServer::Reply limited;
        limited.body = "{\"title\":\"Rate Limit Exceeded\",\"status\":429}";
        limited.responseCode = "429 Too Many Requests";
        limited.contentType = "application/problem+json";
        limited.extraHeaders = "Retry-After: 7\r\n";
        fakeServer->addGetFull("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_A, limited);
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_B,
                           "{\"title\":\"Internal Server Error\"}", "503 Service Unavailable");
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_C, "{}", "504 Gateway Timeout");
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, "this is not JSON");
        Server* server = new Server(fakeServer->baseUrl());

        int lookups = 0;
        const auto countLookups = [&lookups](Server* target) {
            QObject::connect(&target->manager(), &QNetworkAccessManager::finished,
                [&lookups](QNetworkReply* reply) {
                    if (reply->url().path().startsWith(QLatin1String("/accounts/")))
                        ++lookups;
                });
        };
        countLookups(server);
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        auto refusal = [&](const QString& destination) -> SubmitTransactionResponse* {
            Account* account = new Account(source, 1L);
            Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(PaymentOperation::create(KeyPair::fromAccountId(destination), new AssetTypeNative(), "10"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .setBaseFee(100)
                    .build();
            transaction->sign(source);
            SubmitTransactionResponse* sent = nullptr;
            SubmitTransactionResponse* refused = nullptr;
            QMetaObject::Connection c1 = QObject::connect(server, &Server::transactionResponse,
                [&sent](SubmitTransactionResponse* response) { sent = response; });
            QMetaObject::Connection c2 = QObject::connect(server, &Server::transactionError,
                [&refused](SubmitTransactionResponse* response) { refused = response; });
            lookups = 0;
            server->submitTransaction(transaction);
            // Not WAIT_FOR: a refused connection takes Windows a second or
            // more per attempt, which is longer than that waits.
            QElapsedTimer waited;
            waited.start();
            while (!sent && !refused && waited.elapsed() < 30000)
                QTest::qWait(20);
            QTest::qWait(100);   // long enough for a retry to have gone out
            QObject::disconnect(c1);
            QObject::disconnect(c2);
            return sent ? nullptr : refused;
        };

        SubmitTransactionResponse* refused = refusal(DESTINATION_ACCOUNT_MEMO_REQUIRED_A);
        QVERIFY(refused);
        auto* limit = dynamic_cast<qstellar::exception::TooManyRequestsException*>(refused->lastException());
        QVERIFY(limit);
        QCOMPARE(limit->getRetryAfter(), 7);
        QCOMPARE(limit->getCode(), 429);
        QVERIFY(QString::fromUtf8(limit->what()).contains(DESTINATION_ACCOUNT_MEMO_REQUIRED_A));
        QVERIFY(QString::fromUtf8(limit->what()).contains("SEP-29"));
        QCOMPARE(lookups, 1);
        delete refused;

        refused = refusal(DESTINATION_ACCOUNT_MEMO_REQUIRED_B);
        QVERIFY(refused);
        auto* bad = dynamic_cast<qstellar::exception::BadResponseException*>(refused->lastException());
        QVERIFY(bad);
        QCOMPARE(bad->getCode(), 503);
        QVERIFY(QString::fromUtf8(bad->what()).contains(DESTINATION_ACCOUNT_MEMO_REQUIRED_B));
        QVERIFY(QString::fromUtf8(bad->what()).contains("SEP-29"));
        QCOMPARE(lookups, 2);
        delete refused;

        refused = refusal(DESTINATION_ACCOUNT_MEMO_REQUIRED_C);
        QVERIFY(refused);
        auto* timeout = dynamic_cast<qstellar::exception::RequestTimeoutException*>(refused->lastException());
        QVERIFY(timeout);
        QCOMPARE(timeout->getCode(), 504);
        QVERIFY(QString::fromUtf8(timeout->what()).contains(DESTINATION_ACCOUNT_MEMO_REQUIRED_C));
        QCOMPARE(lookups, 2);
        delete refused;

        // A reply that is not JSON: an unreadable answer, not a lost connection.
        refused = refusal(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED);
        QVERIFY(refused);
        auto* unreadable = dynamic_cast<qstellar::exception::UnknownResponseException*>(refused->lastException());
        QVERIFY(unreadable);
        QCOMPARE(unreadable->getCode(), 200);
        QVERIFY(QString::fromUtf8(unreadable->what()).contains(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED));
        QVERIFY(QString::fromUtf8(unreadable->what()).contains("SEP-29"));
        QCOMPARE(lookups, 2);
        delete refused;

        // Nobody listening: the connection failure it is, asked twice too.
        delete server;
        FakeServer* gone = new FakeServer();
        const QString nowhere = gone->baseUrl();
        delete gone;
        server = new Server(nowhere);
        countLookups(server);
        refused = refusal(DESTINATION_ACCOUNT_MEMO_REQUIRED_D);
        QVERIFY(refused);
        auto* connection = dynamic_cast<qstellar::exception::ConnectionErrorException*>(refused->lastException());
        QVERIFY(connection);
        QVERIFY(QString::fromUtf8(connection->what()).contains(DESTINATION_ACCOUNT_MEMO_REQUIRED_D));
        QVERIFY(QString::fromUtf8(connection->what()).contains("SEP-29"));
        QCOMPARE(lookups, 2);
        delete refused;

        delete server;
        fakeServer->deleteLater();
    }

    // ── single-resource GET endpoints (P2.5) ──────────────────────────────
    // The request builders' single-resource forms — offer(id), ledger(seq),
    // transaction(hash), liquidityPool(id) — execute a GET and populate a
    // Response that emits ready()/error(). Previously only their buildUri()
    // was exercised (in the request-builder tests); these drive the full
    // HTTP round-trip against FakeServer so a regression in the URL the SDK
    // hits, or in the parse of the single-resource (non-paged) body, is
    // caught. A LOCAL Server is used and deleted so nothing lingers.

    void testOfferSingleResourceEndpoint() {
        FakeServer* fakeServer = new FakeServer();
        QString body =
            "{\"_links\":{\"offer_maker\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"},\"self\":{\"href\":\"https://h/offers/241\"}},"
            "\"id\":241,\"paging_token\":\"241\","
            "\"seller\":\"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\","
            "\"selling\":{\"asset_type\":\"native\"},"
            "\"buying\":{\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\","
            "\"asset_issuer\":\"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"},"
            "\"amount\":\"10.0000000\",\"price_r\":{\"n\":10,\"d\":1},"
            "\"price\":\"11.0000000\",\"last_modified_ledger\":22200794,"
            "\"last_modified_time\":\"2019-01-28T12:30:38Z\"}";
        fakeServer->addGet("/offers/241", body);

        Server* server = new Server(fakeServer->baseUrl());
        OfferResponse* resp = server->offers().offer((qint64)241);

        bool ready = false, errored = false;
        QObject::connect(resp, &Response::ready,  [&](){ ready = true; });
        QObject::connect(resp, &Response::error,  [&](){ errored = true; });
        WAIT_FOR(!ready && !errored)

        QVERIFY(ready);
        QVERIFY(!errored);
        QCOMPARE(resp->getId(), (qint64)241);
        QCOMPARE(resp->getAmount(), QString("10.0000000"));

        delete server;
        fakeServer->deleteLater();
    }

    void testLedgerSingleResourceEndpoint() {
        FakeServer* fakeServer = new FakeServer();
        QString body =
            "{\"total_coins\":\"100000000000.0000000\",\"fee_pool\":\"0.0000000\",\"max_tx_set_size\":100,\"protocol_version\":19,\"header_xdr\":\"\",\"_links\":{\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/898826/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/898826/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/898826/effects{?cursor,limit,order}\",\"templated\":true},\"self\":{\"href\":\"/ledgers/898826\"}},"
            "\"id\":\"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\","
            "\"paging_token\":\"3860428274794496\","
            "\"hash\":\"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\","
            "\"sequence\":898826,\"successful_transaction_count\":3,"
            "\"base_fee_in_stroops\":100,\"base_reserve_in_stroops\":100000000,"
            "\"failed_transaction_count\":2,\"operation_count\":10,"
            "\"closed_at\":\"2015-11-19T21:35:59Z\"}";
        fakeServer->addGet("/ledgers/898826", body);

        Server* server = new Server(fakeServer->baseUrl());
        LedgerResponse* resp = server->ledgers().ledger(898826);

        bool ready = false, errored = false;
        QObject::connect(resp, &Response::ready,  [&](){ ready = true; });
        QObject::connect(resp, &Response::error,  [&](){ errored = true; });
        WAIT_FOR(!ready && !errored)

        QVERIFY(ready);
        QVERIFY(!errored);
        QCOMPARE(resp->getSequence(), (qint64)898826);
        QCOMPARE(resp->getHash(),
                 QString("686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa"));

        delete server;
        fakeServer->deleteLater();
    }

    void testTransactionSingleResourceEndpoint() {
        const QString hash =
            "5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b";
        FakeServer* fakeServer = new FakeServer();
        QString body =
            "{\"_links\":{\"self\":{\"href\":\"/transactions/" + hash + "\"},"
            "\"account\":{\"href\":\"/accounts/GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\"},\"ledger\":{\"href\":\"/ledgers/915744\"},"
            "\"operations\":{\"href\":\"/transactions/" + hash + "/operations\"},\"effects\":{\"href\":\"/transactions/" + hash + "/effects\"},"
            "\"precedes\":{\"href\":\"/transactions?order=asc&cursor=3933090531512320\"},\"succeeds\":{\"href\":\"/transactions?order=desc&cursor=3933090531512320\"}},"
            "\"id\":\"" + hash + "\",\"paging_token\":\"3933090531512320\","
            "\"successful\":true,\"hash\":\"" + hash + "\",\"ledger\":915744,"
            "\"source_account\":\"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\","
            "\"source_account_sequence\":\"3933090531508226\",\"fee_charged\":\"100\",\"max_fee\":\"100\","
            "\"created_at\":\"2015-11-20T17:01:28Z\",\"memo_type\":\"none\","
            "\"fee_account\":\"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\"operation_count\":1,\"envelope_xdr\":\"AAAAAA==\",\"result_xdr\":\"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAABAAAAAAAAAAA=\",\"signatures\":[]}";
        fakeServer->addGet("/transactions/" + hash, body);

        Server* server = new Server(fakeServer->baseUrl());
        TransactionResponse* resp = server->transactions().transaction(hash);

        bool ready = false, errored = false;
        QObject::connect(resp, &Response::ready,  [&](){ ready = true; });
        QObject::connect(resp, &Response::error,  [&](){ errored = true; });
        WAIT_FOR(!ready && !errored)

        QVERIFY(ready);
        QVERIFY(!errored);
        QCOMPARE(resp->getHash(), hash);
        QCOMPARE(resp->getLedger(), 915744);

        delete server;
        fakeServer->deleteLater();
    }

    void testLiquidityPoolSingleResourceEndpoint() {
        FakeServer* fakeServer = new FakeServer();
        QString body =
            "{\"id\":\"abc1234\",\"paging_token\":\"abc1234\",\"fee_bp\":30,"
            "\"type\":\"constant_product\",\"total_trustlines\":\"42\","
            "\"total_shares\":\"100.0000000\",\"reserves\":["
            "{\"asset\":\"native\",\"amount\":\"50.0000000\"},"
            "{\"asset\":\"USDC:GA5\",\"amount\":\"50.0000000\"}],"
            "\"last_modified_ledger\":12345,"
            "\"_links\":{\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/liquidity_pools/abc1234/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/liquidity_pools/abc1234/operations{?cursor,limit,order}\",\"templated\":true},\"self\":{\"href\":\"https://h/liquidity_pools/abc1234\"}}}";
        fakeServer->addGet("/liquidity_pools/abc1234", body);

        Server* server = new Server(fakeServer->baseUrl());
        LiquidityPoolResponse* resp = server->liquidityPools().liquidityPool("abc1234");

        bool ready = false, errored = false;
        QObject::connect(resp, &Response::ready,  [&](){ ready = true; });
        QObject::connect(resp, &Response::error,  [&](){ errored = true; });
        WAIT_FOR(!ready && !errored)

        QVERIFY(ready);
        QVERIFY(!errored);
        QCOMPARE(resp->getId(), QString("abc1234"));
        QCOMPARE(resp->getReserves().size(), 2);

        delete server;
        fakeServer->deleteLater();
    }


    /**
     * A submission the SDK gave up waiting for (Response::replyTimeout())
     * may still be applied, like one Horizon answered 504 for. It was a
     * plain ConnectionErrorException, and isTimeout() said false: a caller
     * that took it for a request that never went out, and rebuilt the
     * transaction with a new sequence number, could pay twice.
     */
    void testSubmitClientTimeoutIsATimeout() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHang("/transactions");
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        Response::setReplyTimeout(300);
        // Restored however the test ends: a failed check used to leave 300 ms
        // for every test after it.
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        SubmitTransactionResponse* refused = nullptr;
        QObject::connect(server, &Server::transactionError,
            [&refused](SubmitTransactionResponse* response) { refused = response; });
        server->submitTransaction(transaction, true);
        WAIT_FOR(!refused)

        QVERIFY(refused);
        QVERIFY(refused->isTimeout());
        QVERIFY(dynamic_cast<qstellar::exception::ClientTimeoutException*>(refused->lastException()));
        QVERIFY(dynamic_cast<qstellar::exception::ConnectionErrorException*>(refused->lastException()));
        QCOMPARE(refused->getStatusCode(), 0);
        delete refused;
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A SEP-29 lookup that got no answer stopped the submission with a
     * ConnectionErrorException, as if the network were down. It is the
     * timeout it was now (a ClientTimeoutException, still a
     * ConnectionErrorException), naming the account and SEP-29; nothing was
     * posted, so isTimeout() stays false: this transaction cannot land.
     */
    void testCheckMemoRequiredLookupTimeoutIsATimeout() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        fakeServer->addHang("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_A);
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        Response::setReplyTimeout(300);
        // Restored however the test ends: a failed check used to leave 300 ms
        // for every test after it.
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        SubmitTransactionResponse* sent = nullptr;
        SubmitTransactionResponse* refused = nullptr;
        QObject::connect(server, &Server::transactionResponse,
            [&sent](SubmitTransactionResponse* response) { sent = response; });
        QObject::connect(server, &Server::transactionError,
            [&refused](SubmitTransactionResponse* response) { refused = response; });
        server->submitTransaction(transaction);
        WAIT_FOR(!sent && !refused)

        QVERIFY(!sent);
        QVERIFY(refused);
        auto* timeout = dynamic_cast<qstellar::exception::ClientTimeoutException*>(refused->lastException());
        QVERIFY(timeout);
        QVERIFY(QString::fromUtf8(timeout->what()).contains(DESTINATION_ACCOUNT_MEMO_REQUIRED_A));
        QVERIFY(QString::fromUtf8(timeout->what()).contains("SEP-29"));
        QVERIFY(!refused->isTimeout());
        delete refused;
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * An account whose answer cannot be read exactly (here, a data entry
     * that is not base64) stops the submission: SEP-29 is not passed on an
     * account the SDK could not read.
     */
    void testCheckMemoRequiredUnreadableAccountStopsTheSubmission() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addPost("/transactions", successTransactionResponse);
        fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_MEMO_REQUIRED_A,
                           accountWithData("{\"config.memo_required\":\"1\"}"));
        Server* server = new Server(fakeServer->baseUrl());
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        transaction->sign(source);

        SubmitTransactionResponse* sent = nullptr;
        SubmitTransactionResponse* refused = nullptr;
        QObject::connect(server, &Server::transactionResponse,
            [&sent](SubmitTransactionResponse* response) { sent = response; });
        QObject::connect(server, &Server::transactionError,
            [&refused](SubmitTransactionResponse* response) { refused = response; });
        server->submitTransaction(transaction);
        WAIT_FOR(!sent && !refused)

        QVERIFY(!sent);
        QVERIFY(refused);
        auto* unreadable = dynamic_cast<qstellar::exception::UnknownResponseException*>(refused->lastException());
        QVERIFY(unreadable);
        QVERIFY(QString::fromUtf8(unreadable->what()).contains("config.memo_required"));
        delete refused;
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A real testnet ledger stream (Horizon, captured 2026-09-28): the open
     * event, "hello", and two ledgers. Both ledgers arrive, exactly.
     */
    void testRealHorizonLedgerStream() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/ledgers", QStringList() << QString::fromUtf8(realLedgerStream()));
        Server* server = new Server(fakeServer->baseUrl());
        Page<LedgerResponse>* stream = server->ledgers().stream().execute();
        QList<qint64> sequences;
        QStringList tokens;
        int errors = 0;
        QObject::connect(stream, &Response::ready, [&]() {
            sequences << stream->streamedElement()->getSequence();
            tokens << stream->streamedElement()->getPagingToken();
        });
        QObject::connect(stream, &Response::error, [&]() { ++errors; });
        WAIT_FOR(sequences.size() < 2)
        QCOMPARE(errors, 0);
        QCOMPARE(sequences, QList<qint64>() << 4921035 << 4921036);
        QCOMPARE(tokens, QStringList() << "21135684387471360" << "21135688682438656");
        QCOMPARE(stream->streamedElement()->getBaseFeeInStroops(), QString("100"));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A destination with a signer added before protocol 10, whose weight is
     * past a byte (256 or more; Horizon sends 2^31 and past as a negative
     * int32), is paid: SEP-29's lookup of it used to fail, since weights
     * were held to 0..255, and stopped the payment.
     */
    void testCheckMemoRequiredAccountWithAHeavySignerIsPaid() {
        for (const QString& weight : { QStringLiteral("256"), QStringLiteral("2147483647"), QStringLiteral("-1294967296") }) {
            FakeServer* fakeServer = new FakeServer();
            fakeServer->addPost("/transactions", successTransactionResponse);
            fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, accountWithData("{}", weight));
            Server* server = new Server(fakeServer->baseUrl());
            KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
            Account* account = new Account(source, 1L);
            Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .setBaseFee(100)
                    .build();
            transaction->sign(source);

            SubmitTransactionResponse* sent = nullptr;
            SubmitTransactionResponse* refused = nullptr;
            QObject::connect(server, &Server::transactionResponse,
                [&sent](SubmitTransactionResponse* response) { sent = response; });
            QObject::connect(server, &Server::transactionError,
                [&refused](SubmitTransactionResponse* response) { refused = response; });
            server->submitTransaction(transaction);
            WAIT_FOR(!sent && !refused)

            QVERIFY2(!refused, refused && refused->lastException() ? refused->lastException()->what() : "refused");
            QVERIFY(sent);
            QVERIFY(sent->isSuccess());
            delete sent;
            delete server;
            fakeServer->deleteLater();
        }
    }

    /**
     * A posted submission that timed out is a timeout for as long as the
     * response lives. isTimeout() read that from the reply, which goes with
     * the Server's network manager: with the Server deleted it read false
     * while lastException() still said ClientTimeoutException, and a caller
     * checking it later took a transaction that may land for one that never
     * went out. Whichever way the POST was attached: straight away, after
     * SEP-29's lookups, or by Response::loadFromReply through a Response
     * pointer.
     */
    void testSubmitTimeoutStaysATimeoutWhenTheServerIsGone() {
        Response::setReplyTimeout(300);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        for (const bool checkMemo : { false, true }) {
            FakeServer* fakeServer = new FakeServer();
            fakeServer->addHang("/transactions");
            fakeServer->addGet("/accounts/" + DESTINATION_ACCOUNT_NO_MEMO_REQUIRED, noMemoRequiredResponse);
            Server* server = new Server(fakeServer->baseUrl());
            KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
            Account* account = new Account(source, 1L);
            Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .setBaseFee(100)
                    .build();
            transaction->sign(source);

            SubmitTransactionResponse* refused = nullptr;
            QObject::connect(server, &Server::transactionError,
                [&refused](SubmitTransactionResponse* response) { refused = response; });
            server->submitTransaction(transaction, !checkMemo);
            WAIT_FOR(!refused)
            QVERIFY(refused);
            QVERIFY2(refused->isTimeout(), checkMemo ? "after SEP-29" : "posted at once");

            // The response outlives the Server (the caller deletes it): its
            // reply goes with the Server's network manager.
            delete server;
            QCoreApplication::processEvents();
            QVERIFY(dynamic_cast<qstellar::exception::ClientTimeoutException*>(refused->lastException()));
            QVERIFY2(refused->isTimeout(), checkMemo ? "after SEP-29" : "posted at once");
            delete refused;
            fakeServer->deleteLater();
        }

        // A POST attached through a Response pointer, which does not pass
        // through SubmitTransactionResponse::loadFromReply.
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHang("/transactions");
        QNetworkAccessManager* manager = new QNetworkAccessManager();
        QNetworkRequest request(QUrl(fakeServer->baseUrl() + "/transactions"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        SubmitTransactionResponse* response = new SubmitTransactionResponse(nullptr, nullptr);
        bool failed = false;
        QObject::connect(response, &Response::error, [&failed]() { failed = true; });
        static_cast<Response*>(response)->loadFromReply(manager->post(request, QByteArray("tx=AAAA")));
        WAIT_FOR(!failed)
        QVERIFY(failed);
        QVERIFY(response->isTimeout());
        delete manager;
        QCoreApplication::processEvents();
        QVERIFY(dynamic_cast<qstellar::exception::ClientTimeoutException*>(response->lastException()));
        QVERIFY(response->isTimeout());
        delete response;
        fakeServer->deleteLater();
    }

private:
    static QByteArray realLedgerStream()
    {
        return QByteArray(
            "retry: 1000\nevent: open\ndata: \"hello\"\n\nid: 21135684387471360\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921035\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921035/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921035/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921035/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921035/effects{?cursor,limit,order}\",\"templated\":true}},\"id\":\"65ca12a0946f215a2cc0072968baf4426e55611ca876d198ec3b4a608bf14935\",\"paging_token\":\"21135684387471360\",\"hash\":\"65ca12a0946f215a2cc0072968baf4426e55611ca876d198ec3b4a608bf14935\",\"prev_hash\":\"54b1a2fb86fa4cd2977706d9ad01067f100cde5a193465189cbde54b40e9d02f\",\"sequence\":4921035,\"successful_transaction_count\":11,\"failed_transaction_count\":3,\"operation_count\":56,\"tx_set_operation_count\":59,\"closed_at\":\"2026-09-28T20:52:42Z\",\"total_coins\":\"100000000000.0000000\",\"fee_pool\":\"678657.7159240\",\"base_fee_in_stroops\":100,\"base_reserve_in_stroops\":5000000,\"max_tx_set_size\":200,\"protocol_version\":28,\"header_xdr\":\"AAAAHFSxovuG+kzSl3cG2a0BBn8QDN5aGTRlGJy95UtA6dAvelnnX5WXRxmXqjZn6BjabuetPJA+tQC6TJu1mCN/fzwAAAAAarrTmgAAAAAAAAABAAAAAKgkzRi8nXUGTSmaW1uspDvDqi8yaTgVPYwvm7XLbfAzAAAAQAt2ambpjHueFdkcxsD9vkDvpRhWRjp1dWekREpp5NVWRFLps3YM3NemtXuhDn6Obk6m2AKxJ8FB6TdGMSTQ"
            "GQN7TWJ+Rpv1OH6L4bagKejJElZGWkiKJdKu6CpUL56M2cZFPSDnH2FDdCVDnmxj6o0gHh5YgtYgJw3vDmWpu0ruAEsWyw3gtrOnZAAAAAAGLB+FVEgAAAAAAAAAAAAMt6YAAABkAExLQAAAAMjDoVKXV7hPmjmtSy3yqvPsLlH90GesHmkYlQSwrpIie++x4zR+4iv0xNnX8etn527TNTBCbPa0MgvHZ91u0mlSS7zXtzJpKyr49/IfkWIuiltIV8Vvus51zD4Jzs+T3uQAfbRzTOUFrgCd0a1NWFFVuKGt8FhQJ3iV3juXCzeRGwAAAAA=\"}\n\nid: 21135688682438656\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921036\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921036/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921036/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921036/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4921036/effects{?cursor,limit,order}\",\"templated\":true}},\"id\":\"535a33080a8aa4b699484ec70bbc509adc8ede867513ddc59c4f57c26f839c96\",\"paging_token\":\"21135688682438656\",\"hash\":\"535a33080a8aa4b699484ec70bbc509adc8ede867513ddc59c4f57c26f839c96\",\"prev_hash\":\"65ca12a0946f215a2cc0072968baf4426e55611ca876d198ec3b4a608bf14935\",\"sequence\":4921036,\"successful_transaction_count\":15,\"failed_transaction_count\":2,\"operation_count\":55,\"tx_set_operation_count\":78,\"closed_at\":\"2026-09-28T20:52:47Z\",\"total_coins\":\"100000000000.0000000\",\"fee_pool\":\"678657.7884094\",\"base_fee_in_stroops\":100,\"base_reserve_in_stroops\":5000000,\"max_tx_set_size\""
            ":200,\"protocol_version\":28,\"header_xdr\":\"AAAAHGXKEqCUbyFaLMAHKWi69EJuVWEcqHbRmOw7SmCL8Uk12I+63oAPGyxc6tBA8QpH76MlOkipe8lqVLNJpPslocIAAAAAarrTnwAAAAAAAAABAAAAANVyadliUPdJbQeb4ug1Ejbv/+jTnC4Gv6uxQh8X/GccAAAAQIzrsdrGGO98ha2Bz4YwbctgigXG8Pe71FdvRwnyQMAXONJVp7wmUNfuCQxLvJ5GC0g0eVmdUKgg/LejENmzegrFKXt0O403qu3t5Z/0E1mzjSYVe5wnxNQImEaT6Dxn5TY/B8be9WBO6rzFgFDYTPJh6U+Lsi6JRr+XXMbppc1pAEsWzA3gtrOnZAAAAAAGLB+QY74AAAAAAAAAAAAMt80AAABkAExLQAAAAMjDoVKXV7hPmjmtSy3yqvPsLlH90GesHmkYlQSwrpIie++x4zR+4iv0xNnX8etn527TNTBCbPa0MgvHZ91u0mlSS7zXtzJpKyr49/IfkWIuiltIV8Vvus51zD4Jzs+T3uQAfbRzTOUFrgCd0a1NWFFVuKGt8FhQJ3iV3juXCzeRGwAAAAA=\"}\n\n");
    }

private slots:
};

ADD_TEST(ServerTest)

#endif // SERVERTEST_H
