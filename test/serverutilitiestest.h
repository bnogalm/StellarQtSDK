#ifndef SERVERUTILITIESTEST_H
#define SERVERUTILITIESTEST_H

#include <QObject>
#include <QtTest>
#include <QScopeGuard>
#include <QScopedPointer>
#include <QElapsedTimer>
#include <stdexcept>
#include "testcollector.h"
#include "fakeserver.h"
#include "../src/server.h"
#include "../src/httpdate.h"
#include "../src/keypair.h"
#include "../src/account.h"
#include "../src/transaction.h"
#include "../src/paymentoperation.h"
#include "../src/assettypenative.h"
#include "../src/timebounds.h"
#include "../src/responses/accountdataresponse.h"
#include "../src/responses/friendbotresponse.h"
#include "../src/responses/problem.h"
#include "../src/exception/badrequestexception.h"
#include "../src/exception/badresponseexception.h"
#include "../src/exception/connectionerrorexception.h"
#include "../src/exception/toomanyrequestsexception.h"
#include "../src/exception/unknownresponseexception.h"

/**
 * A Server whose clocks the test sets: the monotonic one that ages the
 * recorded server time, and the local time fetchTimebounds() falls back to.
 */
class ClockServer : public Server
{
public:
    explicit ClockServer(const QString& uri) : Server(uri) {}
    qint64 monotonic = 1000000;
    qint64 local = 1234567890;
protected:
    qint64 monotonicMSecs() const override { return monotonic; }
    qint64 localSecsSinceEpoch() const override { return local; }
};

/**
 * D6 (1.0.5): what js-stellar-sdk and py-stellar-sdk offer on their Horizon
 * Server that this SDK lacked: fetchBaseFee, fetchTimebounds, an account's
 * data entry and friendbot.
 */
class ServerUtilitiesTest : public QObject
{
    Q_OBJECT

    const QString ACCOUNT = QStringLiteral("GDYC2D4P2SRC5DCEDDK2OUFESSPCTZYLDOEF6NYHR2T7X5GUTEABCQC2");
    const QString HASH = QStringLiteral("2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42");

    /** 2026-10-26T08:20:07Z, and an hour later. */
    static constexpr qint64 DATE_1 = 1793002807;
    static constexpr qint64 DATE_2 = 1793006407;
    static QString date1() { return QStringLiteral("Mon, 26 Oct 2026 08:20:07 GMT"); }
    static QString date2() { return QStringLiteral("Mon, 26 Oct 2026 09:20:07 GMT"); }

    /** The class of a request's failure, most derived first, by name. */
    static QString failureKind(qstellar::exception::SdkException* ex)
    {
        using namespace qstellar::exception;
        if (!ex) return QStringLiteral("none");
        if (dynamic_cast<TooManyRequestsException*>(ex)) return QStringLiteral("rateLimited");
        if (dynamic_cast<BadResponseException*>(ex)) return QStringLiteral("badResponse");
        if (dynamic_cast<BadRequestException*>(ex)) return QStringLiteral("badRequest");
        if (dynamic_cast<UnknownResponseException*>(ex)) return QStringLiteral("unknownResponse");
        if (dynamic_cast<ClientTimeoutException*>(ex)) return QStringLiteral("timeout");
        if (dynamic_cast<ConnectionErrorException*>(ex)) return QStringLiteral("connection");
        return QStringLiteral("other");
    }

    /** "ready", or the kind of the failure, once `response` answers (within `waitMs`). */
    static QString outcome(Response* response, int waitMs = 5000)
    {
        bool ready = false, failed = false;
        QMetaObject::Connection c1 = QObject::connect(response, &Response::ready, [&ready]() { ready = true; });
        QMetaObject::Connection c2 = QObject::connect(response, &Response::error, [&failed]() { failed = true; });
        QElapsedTimer waited;
        waited.start();
        while (!ready && !failed && waited.elapsed() < waitMs)
            QTest::qWait(10);
        QObject::disconnect(c1);
        QObject::disconnect(c2);
        if (ready && !failed)
            return QStringLiteral("ready");
        if (!failed)
            return QStringLiteral("no answer");
        return failureKind(response->lastException());
    }

    static FakeServer::Reply reply(const QString& body, const QString& code = QStringLiteral("200 OK"),
                                   const QString& extraHeaders = QString())
    {
        FakeServer::Reply r;
        r.body = body;
        r.responseCode = code;
        r.extraHeaders = extraHeaders;
        return r;
    }

    static QString dateHeader(const QString& date) { return QStringLiteral("Date: ") + date + QStringLiteral("\r\n"); }

    /** /fee_stats as Horizon answers it, `baseFee` the whole last_ledger_base_fee member (or none). */
    static QString feeStats(const QString& baseFee)
    {
        const QString distribution = QStringLiteral(
            "{\"min\":\"100\",\"max\":\"100\",\"mode\":\"100\",\"p10\":\"100\",\"p20\":\"100\",\"p30\":\"100\","
            "\"p40\":\"100\",\"p50\":\"100\",\"p60\":\"100\",\"p70\":\"100\",\"p80\":\"100\",\"p90\":\"100\","
            "\"p95\":\"100\",\"p99\":\"100\"}");
        return QStringLiteral("{\"last_ledger\":\"22606298\",") + baseFee
               + (baseFee.isEmpty() ? QString() : QStringLiteral(","))
               + QStringLiteral("\"ledger_capacity_usage\":\"0.97\",\"fee_charged\":") + distribution
               + QStringLiteral(",\"max_fee\":") + distribution + QStringLiteral("}");
    }

    /** "ready <maxTime>" or the failure's kind. */
    static QString timebounds(Server* server, qint64 seconds)
    {
        QScopedPointer<TimeboundsResponse> response(server->fetchTimebounds(seconds));
        const QString result = outcome(response.data());
        if (result != QLatin1String("ready"))
            return result;
        if (response->minTime() != 0)
            return QStringLiteral("minTime %1").arg(response->minTime());
        return QStringLiteral("ready %1").arg(response->maxTime());
    }

    static QString ready(qint64 maxTime) { return QStringLiteral("ready %1").arg(maxTime); }

    /** `date` at 08:49:37 GMT in the RFC 850 form, its year cut to two digits. */
    static QByteArray rfc850(const QDate& date)
    {
        static const char* const days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
                                            "Saturday", "Sunday" };
        static const char* const months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
        return QStringLiteral("%1, %2-%3-%4 08:49:37 GMT")
            .arg(QLatin1String(days[date.dayOfWeek() - 1]))
            .arg(date.day(), 2, 10, QLatin1Char('0'))
            .arg(QLatin1String(months[date.month() - 1]))
            .arg(date.year() % 100, 2, 10, QLatin1Char('0'))
            .toLatin1();
    }

    /** `date` at 08:49:37 GMT, in seconds since the Unix epoch. */
    static qint64 secondsAt(const QDate& date)
    {
        return (date.toJulianDay() - 2440588) * 86400 + 8 * 3600 + 49 * 60 + 37;
    }

    /** What an RFC 850 `date` reads as when the current year is `currentYear`: its seconds, or -1 when refused. */
    static qint64 rfc850In(const QDate& date, int currentYear)
    {
        qint64 t = -1;
        return HttpDate::parse(rfc850(date), &t, currentYear) ? t : -1;
    }

    /** A successful friendbot answer, `extra` members added. */
    QString funded(const QString& extra = QString()) const
    {
        return QStringLiteral("{\"successful\":true,\"hash\":\"") + HASH
               + QStringLiteral("\",\"envelope_xdr\":\"AAAA\",\"result_xdr\":\"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA=\"")
               + extra + QStringLiteral("}");
    }

private slots:

    void cleanupTestCase()
    {
        qApp->processEvents();
    }

    // ---- HTTP-date ----------------------------------------------------

    void testHttpDateReadsTheThreeForms()
    {
        qint64 t = 0;
        QVERIFY(HttpDate::parse("Mon, 26 Oct 2026 08:20:07 GMT", &t));
        QCOMPARE(t, DATE_1);
        // RFC 9110's examples, one instant in its three forms.
        QVERIFY(HttpDate::parse("Sun, 06 Nov 1994 08:49:37 GMT", &t));
        QCOMPARE(t, qint64(784111777));
        t = 0;
        QVERIFY(HttpDate::parse("Sunday, 06-Nov-94 08:49:37 GMT", &t, 2026));
        QCOMPARE(t, qint64(784111777));
        t = 0;
        QVERIFY(HttpDate::parse("Sun Nov  6 08:49:37 1994", &t));
        QCOMPARE(t, qint64(784111777));
        QVERIFY(HttpDate::parse("Monday, 26-Oct-26 08:20:07 GMT", &t, 2026));
        QCOMPARE(t, DATE_1);
        QVERIFY(HttpDate::parse("Mon Oct 26 08:20:07 2026", &t));
        QCOMPARE(t, DATE_1);
        QVERIFY(HttpDate::parse("  Mon, 26 Oct 2026 08:20:07 GMT \t", &t));
        QCOMPARE(t, DATE_1);
        // A leap second reads as the next second.
        QVERIFY(HttpDate::parse("Wed, 31 Dec 2025 23:59:60 GMT", &t));
        QCOMPARE(t, qint64(1767225600));
    }

    /**
     * RFC 7231 7.1.1.1: a two-digit year is the one at most 50 years ahead
     * of the current year, else the century before; never more than 49
     * behind when the next century's fits. A date read in the wrong century
     * has the wrong weekday (100 years move it 5 or 6 days), so it is refused.
     */
    void testHttpDateReadsTwoDigitYearsByTheFiftyYearRule()
    {
        // In 2026: "76" is 2076 (50 ahead), "77" is 1977 (49 behind).
        QCOMPARE(rfc850In(QDate(2076, 12, 31), 2026), secondsAt(QDate(2076, 12, 31)));
        QCOMPARE(rfc850In(QDate(1976, 12, 31), 2026), qint64(-1));
        QCOMPARE(rfc850In(QDate(1977, 1, 1), 2026), secondsAt(QDate(1977, 1, 1)));
        QCOMPARE(rfc850In(QDate(2077, 1, 1), 2026), qint64(-1));
        QCOMPARE(rfc850In(QDate(2026, 10, 26), 2026), secondsAt(QDate(2026, 10, 26)));
        QCOMPARE(rfc850In(QDate(1994, 11, 6), 2026), secondsAt(QDate(1994, 11, 6)));
        // In 2044, "94" is 2094 (50 ahead), no longer 1994.
        QCOMPARE(rfc850In(QDate(2094, 11, 6), 2044), secondsAt(QDate(2094, 11, 6)));
        QCOMPARE(rfc850In(QDate(1994, 11, 6), 2044), qint64(-1));
        QCOMPARE(rfc850In(QDate(1995, 1, 1), 2044), secondsAt(QDate(1995, 1, 1)));
        // In 2051, "00" and "01" are the next century's (49 and 50 ahead),
        // "02" is this one's (49 behind).
        QCOMPARE(rfc850In(QDate(2100, 1, 1), 2051), secondsAt(QDate(2100, 1, 1)));
        QCOMPARE(rfc850In(QDate(2000, 1, 1), 2051), qint64(-1));
        QCOMPARE(rfc850In(QDate(2101, 6, 1), 2051), secondsAt(QDate(2101, 6, 1)));
        QCOMPARE(rfc850In(QDate(2001, 6, 1), 2051), qint64(-1));
        QCOMPARE(rfc850In(QDate(2002, 6, 1), 2051), secondsAt(QDate(2002, 6, 1)));
        QCOMPARE(rfc850In(QDate(2102, 6, 1), 2051), qint64(-1));
        // At a century's start: "50" is 2050 (50 ahead), "51" is 1951.
        QCOMPARE(rfc850In(QDate(2050, 3, 1), 2000), secondsAt(QDate(2050, 3, 1)));
        QCOMPARE(rfc850In(QDate(1951, 3, 1), 2000), secondsAt(QDate(1951, 3, 1)));
        QCOMPARE(rfc850In(QDate(2051, 3, 1), 2000), qint64(-1));
        // The other forms carry four digits: no rule applies.
        qint64 t = 0;
        QVERIFY(HttpDate::parse("Sun, 06 Nov 1994 08:49:37 GMT", &t, 2090));
        QCOMPARE(t, qint64(784111777));
        // Without a year given, the clock's: today always reads as today.
        const QDate today = QDateTime::currentDateTimeUtc().date();
        QVERIFY(HttpDate::parse(rfc850(today), &t));
        QCOMPARE(t, secondsAt(today));
    }

    void testHttpDateRefusesWhatIsNotOne()
    {
        const QList<QByteArray> refused = {
            "", "garbage",
            "Mon, 26 Oct 2026 08:20:07 UTC",      // another zone
            "Mon, 26 Oct 2026 08:20:07 +0000",
            "Tue, 26 Oct 2026 08:20:07 GMT",      // not that date's weekday
            "mon, 26 Oct 2026 08:20:07 GMT",      // names are case-sensitive
            "Mon, 26 oct 2026 08:20:07 GMT",
            "Mon, 32 Oct 2026 08:20:07 GMT",
            "Fri, 29 Feb 2019 08:20:07 GMT",      // no such day
            "Mon, 26 Oct 2026 24:00:00 GMT",
            "Mon, 26 Oct 2026 08:60:07 GMT",
            "Mon, 26 Oct 2026 08:20:61 GMT",
            "Mon, 26 Oct 2026 8:20:07 GMT",
            "Mon, 26 Oct 2026 08:20 GMT",
            "Mon, 6 Oct 2026 08:20:07 GMT",
            "Mon, 26 Oct 26 08:20:07 GMT",
            "Mon, 26-Oct-26 08:20:07 GMT",        // RFC 850 takes the long weekday
            "Monday, 26 Oct 2026 08:20:07 GMT",
            "Mon Oct 26 08:20:07 26",
            "Mon Oct 26 08:20:07 2026 GMT",
            "Mon, 26 Oct 2026 08:20:07 GMT,",
        };
        for (const QByteArray& value : refused) {
            qint64 t = 42;
            QVERIFY2(!HttpDate::parse(value, &t), value.constData());
            QCOMPARE(t, qint64(42));
        }
    }

    // ---- fetchBaseFee ---------------------------------------------------

    void testFetchBaseFee()
    {
        FakeServer* fakeServer = new FakeServer();
        Server* server = new Server(fakeServer->baseUrl());
        auto baseFee = [&](const QString& member, const QString& code = QStringLiteral("200 OK")) {
            fakeServer->addGetFull("/fee_stats", reply(feeStats(member), code));
            QScopedPointer<BaseFeeResponse> response(server->fetchBaseFee());
            const QString result = outcome(response.data());
            return result == QLatin1String("ready") ? QString::number(response->baseFee()) : result;
        };
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"100\""), QString("100"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"5000\""), QString("5000"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":250"), QString("250"));
        // No base fee to give: js answers 100, this says it could not read one.
        QCOMPARE(baseFee(QString()), QString("unknownResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":null"), QString("unknownResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"0\""), QString("unknownResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"-100\""), QString("unknownResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"cien\""), QString("unknownResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"100.5\""), QString("unknownResponse"));
        // The request's own failures keep their kind.
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"100\"", QStringLiteral("500 Internal Server Error")),
                 QString("badResponse"));
        QCOMPARE(baseFee("\"last_ledger_base_fee\":\"100\"", QStringLiteral("429 Too Many Requests")),
                 QString("rateLimited"));

        // The message says why, and that the default is the caller's.
        fakeServer->addGetFull("/fee_stats", reply(feeStats("\"last_ledger_base_fee\":\"0\"")));
        QScopedPointer<BaseFeeResponse> zero(server->fetchBaseFee());
        QCOMPARE(outcome(zero.data()), QString("unknownResponse"));
        QVERIFY2(QString::fromUtf8(zero->lastException()->what()).contains("last_ledger_base_fee"),
                 zero->lastException()->what());
        QCOMPARE(zero->baseFee(), qint64(0));
        delete server;
        fakeServer->deleteLater();
    }

    /** The caller's response outlives its Server, and says it was abandoned. */
    void testFetchBaseFeeSurvivesItsServer()
    {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHang("/fee_stats");
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<BaseFeeResponse> response(server->fetchBaseFee());
        bool failed = false;
        QObject::connect(response.data(), &Response::error, [&failed]() { failed = true; });
        QTest::qWait(200);
        delete server;
        WAIT_FOR(!failed)
        QVERIFY(failed);
        QCOMPARE(failureKind(response->lastException()), QString("connection"));
        QVERIFY2(QString::fromUtf8(response->lastException()->what()).contains("Server was deleted"),
                 response->lastException()->what());
        fakeServer->deleteLater();
    }

    // ---- fetchTimebounds -----------------------------------------------

    void testFetchTimeboundsReadsTheRootDate()
    {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date1())));
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        QScopedPointer<TimeboundsResponse> response(server->fetchTimebounds(100));
        QCOMPARE(outcome(response.data()), QString("ready"));
        QCOMPARE(response->minTime(), qint64(0));
        QCOMPARE(response->maxTime(), DATE_1 + 100);

        // Usable as is with the transaction builder, which takes it over.
        KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
        Account* account = new Account(source, 1L);
        QScopedPointer<Transaction> transaction(Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(PaymentOperation::create(KeyPair::fromAccountId(ACCOUNT), new AssetTypeNative(), "10"))
                .addTimeBounds(response->toTimeBounds())
                .setBaseFee(100)
                .build());
        QCOMPARE(transaction->getTimeBounds()->getMinTime(), qint64(0));
        QCOMPARE(transaction->getTimeBounds()->getMaxTime(), DATE_1 + 100);
        QCOMPARE(timebounds(server, 0), ready(DATE_1));
        delete server;
        fakeServer->deleteLater();
    }

    /** As js: a Date recorded within 5 minutes is used, moved on by the time since; then the root is asked again. */
    void testFetchTimeboundsUsesARecentServerTimeForFiveMinutes()
    {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date1())));
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        QCOMPARE(timebounds(server, 10), ready(DATE_1 + 10));

        // The root now says another time: not asked while the first is fresh.
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date2())));
        server->monotonic += 1999;
        QCOMPARE(timebounds(server, 10), ready(DATE_1 + 1 + 10));
        server->monotonic += 299000 - 1999;
        QCOMPARE(timebounds(server, 10), ready(DATE_1 + 299 + 10));
        server->monotonic += 1000;  // 300 s exactly: still fresh, as js's "> 300"
        QCOMPARE(timebounds(server, 10), ready(DATE_1 + 300 + 10));
        server->monotonic += 1;     // older than 5 minutes: asked again
        QCOMPARE(timebounds(server, 10), ready(DATE_2 + 10));
        // And that answer is the one kept.
        server->monotonic += 5000;
        QCOMPARE(timebounds(server, 10), ready(DATE_2 + 5 + 10));
        delete server;
        fakeServer->deleteLater();
    }

    /** Every answer from the Server's own host and port counts; another server's does not. */
    void testFetchTimeboundsRecordsAnswersFromItsOwnHostOnly()
    {
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/k");
        FakeServer* horizon = new FakeServer();
        FakeServer* other = new FakeServer();
        horizon->addGetFull("/", reply("{}", "200 OK", dateHeader(date1())));
        horizon->addGetFull(path, reply("{\"value\":\"MQ==\"}", "200 OK", dateHeader(date2())));
        other->addGetFull(path, reply("{\"value\":\"MQ==\"}", "200 OK",
                                      dateHeader(QStringLiteral("Sun, 06 Nov 1994 08:49:37 GMT"))));
        ClockServer* server = new ClockServer(horizon->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));

        // Another port: not this Horizon's clock, and the root is asked.
        AccountDataResponse* elsewhere = server->accounts().accountData(QUrl(other->baseUrl() + path));
        QCOMPARE(outcome(elsewhere), QString("ready"));
        QCOMPARE(timebounds(server, 5), ready(DATE_1 + 5));

        // An answer of its own, any request, is.
        AccountDataResponse* own = server->accounts().accountData(key.data(), "k");
        QCOMPARE(outcome(own), QString("ready"));
        QCOMPARE(timebounds(server, 5), ready(DATE_2 + 5));

        // An HTTP error's Date is the server's clock too.
        horizon->addGetFull(path, reply("{\"status\":404}", "404 Not Found", dateHeader(date1())));
        AccountDataResponse* missing = server->accounts().accountData(key.data(), "k");
        QCOMPARE(outcome(missing), QString("badRequest"));
        QCOMPARE(timebounds(server, 5), ready(DATE_1 + 5));

        // An answer without a Date leaves the last one.
        horizon->addGetFull(path, reply("{\"value\":\"MQ==\"}"));
        AccountDataResponse* undated = server->accounts().accountData(key.data(), "k");
        QCOMPARE(outcome(undated), QString("ready"));
        QCOMPARE(timebounds(server, 5), ready(DATE_1 + 5));
        delete server;
        horizon->deleteLater();
        other->deleteLater();
    }

    /** A root answer without a readable Date gives the local time, as js's retry does; its status does not matter. */
    void testFetchTimeboundsFallsBackToTheLocalTime()
    {
        FakeServer* fakeServer = new FakeServer();
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        fakeServer->addGetFull("/", reply("{}"));
        QCOMPARE(timebounds(server, 60), ready(server->local + 60));
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(QStringLiteral("Mon, 26 Oct 2026 08:20:07 UTC"))));
        QCOMPARE(timebounds(server, 60), ready(server->local + 60));
        // An HTTP error from the root still carries the server's Date.
        fakeServer->addGetFull("/", reply("{}", "503 Service Unavailable", dateHeader(date2())));
        QCOMPARE(timebounds(server, 60), ready(DATE_2 + 60));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A Date at or before 1970 is no server's clock: from the root it gives
     * the local time, from another answer it is not recorded. ready() never
     * gives a maxTime of 0 (no upper bound) or a negative one.
     */
    void testFetchTimeboundsRefusesADateAtOrBeforeTheEpoch()
    {
        const QString epoch = QStringLiteral("Thu, 01 Jan 1970 00:00:00 GMT");
        const QString before = QStringLiteral("Wed, 31 Dec 1969 23:59:59 GMT");
        FakeServer* fakeServer = new FakeServer();
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(epoch)));
        QCOMPARE(timebounds(server, 0), ready(server->local));
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(before)));
        server->monotonic += 301000;
        QCOMPARE(timebounds(server, 60), ready(server->local + 60));
        delete server;

        // Recorded from another answer: left out, the last good one kept.
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/k");
        server = new ClockServer(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        fakeServer->addGetFull(path, reply("{\"value\":\"MQ==\"}", "200 OK", dateHeader(epoch)));
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date2())));
        QCOMPARE(outcome(server->accounts().accountData(key.data(), "k")), QString("ready"));
        QCOMPARE(timebounds(server, 5), ready(DATE_2 + 5));
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date1())));
        QCOMPARE(outcome(server->accounts().accountData(key.data(), "k")), QString("ready"));
        QCOMPARE(timebounds(server, 5), ready(DATE_2 + 5));
        delete server;

        // No usable time at all (a local clock at or before 1970): an error, not a maxTime of 0.
        server = new ClockServer(fakeServer->baseUrl());
        server->local = 0;
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(epoch)));
        {
            QScopedPointer<TimeboundsResponse> none(server->fetchTimebounds(0));
            QCOMPARE(outcome(none.data()), QString("other"));
            QCOMPARE(none->maxTime(), qint64(0));
            bool threw = false;
            try { delete none->toTimeBounds(); } catch (const std::runtime_error&) { threw = true; }
            QVERIFY(threw);
        }
        server->local = -10;
        fakeServer->addGetFull("/", reply("{}"));
        QCOMPARE(timebounds(server, 30), QString("other"));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A Date is the server's time when the headers were sent: it is taken
     * when they arrive, not when the body ends, so a slow body does not
     * make it look newer than it is.
     */
    void testFetchTimeboundsTakesTheDateWhenTheHeadersArrive()
    {
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/slow");
        const QByteArray body("{\"value\":\"MQ==\"}");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHeld(path, "HTTP/1.1 200 OK\r\nContent-Type: application/hal+json\r\nContent-Length: "
                                      + QByteArray::number(body.size()) + "\r\n" + dateHeader(date1()).toLatin1()
                                      + "\r\n");
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date2())));
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        AccountDataResponse* slow = server->accounts().accountData(key.data(), "slow");
        WAIT_FOR(!fakeServer->isHeld(path))
        QVERIFY(fakeServer->isHeld(path));
        QTest::qWait(300);
        // The body ends 4 minutes after the headers.
        server->monotonic += 240000;
        fakeServer->release(path, body);
        QCOMPARE(outcome(slow), QString("ready"));
        QCOMPARE(slow->value(), QByteArray("1"));
        QCOMPARE(timebounds(server, 10), ready(DATE_1 + 240 + 10));
        // And it ages from then: 61 s later it is older than 5 minutes.
        server->monotonic += 61000;
        QCOMPARE(timebounds(server, 10), ready(DATE_2 + 10));
        delete server;
        fakeServer->deleteLater();
    }

    /**
     * A stream's Date is not recorded: its headers come once, when it opens.
     * A stream opened two hours ago that ends just before fetchTimebounds()
     * does not make its old Date the server's time; the root is asked.
     */
    void testFetchTimeboundsIgnoresAStreamThatEnds()
    {
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/k");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHeld(path, "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n"
                                  "Cache-Control: no-cache, no-store, max-age=0\r\nConnection: close\r\n"
                                  + dateHeader(date1()).toLatin1() + "\r\n"
                                  "id: 1\ndata: {\"value\":\"MQ==\"}\n\n");
        fakeServer->addGetFull("/", reply("{}", "200 OK", dateHeader(date2())));
        ClockServer* server = new ClockServer(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        AccountDataResponse* stream = server->accounts().stream().accountData(key.data(), "k");
        QList<QByteArray> values;
        QObject::connect(stream, &Response::ready, [&values, stream]() { values.append(stream->value()); });
        WAIT_FOR(values.isEmpty())
        QCOMPARE(values, QList<QByteArray>({ QByteArray("1") }));
        // Two hours on, the stream ends; at once the time bounds are asked.
        server->monotonic += 2 * 3600 * 1000;
        fakeServer->release(path, QByteArray());
        QTest::qWait(300);
        QCOMPARE(timebounds(server, 30), ready(DATE_2 + 30));
        delete server;
        fakeServer->deleteLater();
    }

    /** A root that is not answered at all is an error, as js propagates its request's. */
    void testFetchTimeboundsFailsWhenTheRootIsNotAnswered()
    {
        Response::setReplyTimeout(300);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addHang("/");
        Server* server = new Server(fakeServer->baseUrl());
        QCOMPARE(timebounds(server, 60), QString("timeout"));
        delete server;

        // Nothing listens on the port (Windows takes a while to say so).
        Response::setReplyTimeout(0);
        quint16 closedPort = 0;
        {
            FakeServer closed;
            closedPort = closed.port();
        }
        server = new Server(QStringLiteral("http://127.0.0.1:%1").arg(closedPort));
        {
            QScopedPointer<TimeboundsResponse> refused(server->fetchTimebounds(60));
            QCOMPARE(outcome(refused.data(), 20000), QString("connection"));
        }
        delete server;
        Response::setReplyTimeout(300);

        // The Server deleted while the root is asked.
        server = new Server(fakeServer->baseUrl());
        QScopedPointer<TimeboundsResponse> pending(server->fetchTimebounds(60));
        bool failed = false;
        QObject::connect(pending.data(), &Response::error, [&failed]() { failed = true; });
        QTest::qWait(100);
        delete server;
        WAIT_FOR(!failed)
        QVERIFY(failed);
        QVERIFY2(QString::fromUtf8(pending->lastException()->what()).contains("Server was deleted"),
                 pending->lastException()->what());

        // And before it started.
        server = new Server(fakeServer->baseUrl());
        QScopedPointer<TimeboundsResponse> early(server->fetchTimebounds(60));
        delete server;
        QCOMPARE(outcome(early.data()), QString("connection"));
        fakeServer->deleteLater();
    }

    void testFetchTimeboundsRefusals()
    {
        Server server(QStringLiteral("http://localhost:1"));
        bool threw = false;
        try { delete server.fetchTimebounds(-1); } catch (const std::invalid_argument&) { threw = true; }
        QVERIFY(threw);
        threw = false;
        try { delete server.fetchTimebounds(std::numeric_limits<qint64>::max()); }
        catch (const std::invalid_argument&) { threw = true; }
        QVERIFY(threw);
        // No time bounds before ready().
        QScopedPointer<TimeboundsResponse> response(server.fetchTimebounds(10));
        threw = false;
        try { delete response->toTimeBounds(); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        QCOMPARE(response->maxTime(), qint64(0));
    }

    // ---- /accounts/{id}/data/{key} -------------------------------------

    void testAccountDataReadsTheValue()
    {
        const QString base = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGet(base + "config.memo_required", "{\"value\":\"MQ==\"}");
        fakeServer->addGet(base + "sponsored",
                           "{\"value\":\"aGVsbG8=\",\"sponsor\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\"}");
        fakeServer->addGet(base + "empty", "{\"value\":\"\"}");
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));

        AccountDataResponse* memo = server->accounts().accountData(key.data(), "config.memo_required");
        QCOMPARE(outcome(memo), QString("ready"));
        QCOMPARE(memo->value(), QByteArray("1"));
        QCOMPARE(memo->valueBase64(), QString("MQ=="));
        QCOMPARE(memo->sponsor(), QString());

        AccountDataResponse* sponsored = server->accounts().accountData(key.data(), "sponsored");
        QCOMPARE(outcome(sponsored), QString("ready"));
        QCOMPARE(sponsored->value(), QByteArray("hello"));
        QCOMPARE(sponsored->sponsor(), QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));

        AccountDataResponse* empty = server->accounts().accountData(key.data(), "empty");
        QCOMPARE(outcome(empty), QString("ready"));
        QVERIFY(!empty->value().isNull());
        QVERIFY(empty->value().isEmpty());

        // Its own URL, as account(QUrl).
        AccountDataResponse* byUrl = server->accounts().accountData(QUrl(fakeServer->baseUrl() + base + "sponsored"));
        QCOMPARE(outcome(byUrl), QString("ready"));
        QCOMPARE(byUrl->value(), QByteArray("hello"));

        // An entry the account does not have: Horizon's 404.
        fakeServer->addGet(base + "nothing",
                           "{\"type\":\"https://stellar.org/horizon-errors/not_found\",\"title\":\"Resource Missing\",\"status\":404}",
                           "404 Not Found");
        AccountDataResponse* missing = server->accounts().accountData(key.data(), "nothing");
        QCOMPARE(outcome(missing), QString("badRequest"));
        QCOMPARE(missing->getStatus(), 404);
        delete server;
        fakeServer->deleteLater();
    }

    void testAccountDataRefusesAValueThatIsNotExactBase64()
    {
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/k");
        FakeServer* fakeServer = new FakeServer();
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        auto read = [&](const QString& body) {
            fakeServer->addGet(path, body);
            AccountDataResponse* response = server->accounts().accountData(key.data(), "k");
            const QString result = outcome(response);
            delete response;
            return result;
        };
        QCOMPARE(read("{\"value\":\"MQ==\"}"), QString("ready"));
        QCOMPARE(read("{\"value\":\"MQ\"}"), QString("unknownResponse"));      // no padding
        QCOMPARE(read("{\"value\":\"MR==\"}"), QString("unknownResponse"));    // stray bits
        QCOMPARE(read("{\"value\":\"M Q==\"}"), QString("unknownResponse"));
        QCOMPARE(read("{\"value\":\"!!!!\"}"), QString("unknownResponse"));
        QCOMPARE(read("{\"value\":\"MQ==\\u00e9\"}"), QString("unknownResponse"));
        QCOMPARE(read("{\"value\":\"" + QString::fromLatin1(QByteArray(64, 'x').toBase64()) + "\"}"),
                 QString("ready"));
        QCOMPARE(read("{\"value\":\"" + QString::fromLatin1(QByteArray(65, 'x').toBase64()) + "\"}"),
                 QString("unknownResponse"));                                   // more than a data entry holds
        QCOMPARE(read("{\"value\":5}"), QString("unknownResponse"));
        QCOMPARE(read("{\"value\":null}"), QString("unknownResponse"));
        QCOMPARE(read("{}"), QString("unknownResponse"));
        QCOMPARE(read("[\"MQ==\"]"), QString("unknownResponse"));
        delete server;
        fakeServer->deleteLater();
    }

    /** The key is one path segment: '/', '?', '#', '%' and spaces are percent-encoded (py breaks '/', '?', '#' and '%XX'). */
    void testAccountDataPercentEncodesTheKey()
    {
        const QString base = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addGet(base + "a%2Fb%3Fc%23d%25e%20f", "{\"value\":\"MQ==\"}");
        fakeServer->addGet(base + "x%2B%3Ay%26z%3D", "{\"value\":\"Mg==\"}");
        fakeServer->addGet(base + "a-b_c.d~e", "{\"value\":\"Mw==\"}");
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));

        AccountDataResponse* special = server->accounts().accountData(key.data(), "a/b?c#d%e f");
        QCOMPARE(outcome(special), QString("ready"));
        QCOMPARE(special->value(), QByteArray("1"));
        AccountDataResponse* reserved = server->accounts().accountData(key.data(), "x+:y&z=");
        QCOMPARE(outcome(reserved), QString("ready"));
        QCOMPARE(reserved->value(), QByteArray("2"));
        AccountDataResponse* plain = server->accounts().accountData(key.data(), "a-b_c.d~e");
        QCOMPARE(outcome(plain), QString("ready"));
        QCOMPARE(plain->value(), QByteArray("3"));
        delete server;
        fakeServer->deleteLater();
    }

    /** stellar-core's rule for a data name: 1 to 64 bytes of printable ASCII. */
    void testAccountDataRefusesANameNoEntryCanHave()
    {
        Server server(QStringLiteral("http://localhost:1"));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        const QStringList refused = {
            QString(), QString(""), QString(65, QLatin1Char('a')),
            QStringLiteral("a\tb"), QStringLiteral("a\nb"), QString(QChar(0x7f)),
            QString(QChar(0xe9)), QStringLiteral("\u20ac"), QString(1, QChar(0)) + "x",
        };
        for (const QString& name : refused) {
            bool threw = false;
            try {
                delete server.accounts().accountData(key.data(), name);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            QVERIFY2(threw, qPrintable(name));
        }
        bool threw = false;
        try { delete server.accounts().accountData(nullptr, "k"); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        // Valid for core, but dot-segments: HTTP cannot address them.
        for (const QString& dots : { QStringLiteral("."), QStringLiteral("..") }) {
            threw = false;
            try {
                delete server.accounts().accountData(key.data(), dots);
            } catch (const std::runtime_error& e) {
                threw = QString::fromUtf8(e.what()).contains(QLatin1String("dot-segment"));
            }
            QVERIFY2(threw, qPrintable(dots));
        }
        // Dots in a longer name are no dot-segment: asked.
        AccountDataResponse* dotted = server.accounts().accountData(key.data(), QStringLiteral("..."));
        QVERIFY(dotted);
        AccountDataResponse* dotName = server.accounts().accountData(key.data(), QStringLiteral(".a"));
        QVERIFY(dotName);
        // 64 bytes, every printable ASCII character: asked.
        AccountDataResponse* longest = server.accounts().accountData(key.data(), QString(64, QLatin1Char('a')));
        QVERIFY(longest);
        AccountDataResponse* printable = server.accounts().accountData(key.data(), QStringLiteral(" !~"));
        QVERIFY(printable);
    }

    /** After stream(), as py's DataCallBuilder.stream(). */
    void testAccountDataStreams()
    {
        const QString path = QStringLiteral("/accounts/") + ACCOUNT + QStringLiteral("/data/k");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream(path, QStringList()
                              << QStringLiteral("id: 1\ndata: {\"value\":\"MQ==\"}\n\nid: 2\ndata: {\"value\":\"MR==\"}\n\nid: 3\ndata: {\"value\":\"Mg==\"}\n\n"));
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        AccountDataResponse* stream = server->accounts().stream().accountData(key.data(), "k");
        QList<QByteArray> values;
        int errors = 0;
        QObject::connect(stream, &Response::ready, [&values, stream]() { values.append(stream->value()); });
        QObject::connect(stream, &Response::error, [&errors]() { ++errors; });
        WAIT_FOR(values.size() < 2)
        QCOMPARE(values, QList<QByteArray>({ QByteArray("1"), QByteArray("2") }));
        QCOMPARE(errors, 1); // the event whose value is not exact base64
        delete server;
        fakeServer->deleteLater();
    }

    // ---- friendbot ------------------------------------------------------

    /** SDF's Horizons answer /friendbot with a 307 to friendbot.stellar.org: followed. */
    void testFriendbotFollowsTheRedirect()
    {
        FakeServer* horizon = new FakeServer();
        FakeServer* friendbot = new FakeServer();
        const QString query = QStringLiteral("?addr=") + ACCOUNT;
        horizon->addGetFull("/friendbot" + query,
                            reply(QString(), "307 Temporary Redirect",
                                  QStringLiteral("Location: ") + friendbot->baseUrl() + "/" + query + "\r\n"));
        friendbot->addGet("/" + query, funded(",\"ledger\":4921035"));
        Server* server = new Server(horizon->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        FriendbotResponse* response = server->friendbot(key.data());
        QCOMPARE(outcome(response), QString("ready"));
        QCOMPARE(response->getHash(), HASH);
        QCOMPARE(response->getLedger(), qint64(4921035));
        QCOMPARE(response->getEnvelopeXdr(), QString("AAAA"));
        QCOMPARE(response->getResultXdr(), QString("AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA="));
        QVERIFY(response->parent() == server); // the Server's, as get() makes them

        // A Horizon URI with a trailing slash asks the same path.
        Server* slash = new Server(horizon->baseUrl() + "/");
        FriendbotResponse* again = slash->friendbot(key.data());
        QCOMPARE(outcome(again), QString("ready"));
        delete slash;
        delete server;
        horizon->deleteLater();
        friendbot->deleteLater();
    }

    /** An explicit friendbot URL keeps its query; addr is set in it, replacing any it had. */
    void testFriendbotAtAGivenUrl()
    {
        FakeServer* friendbot = new FakeServer();
        friendbot->addGet("/fund?token=abc&addr=" + ACCOUNT, funded());
        Server* server = new Server(QStringLiteral("http://localhost:1"));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        FriendbotResponse* response = server->friendbot(key.data(), QUrl(friendbot->baseUrl() + "/fund?token=abc&addr=GOLD"));
        QCOMPARE(outcome(response), QString("ready"));
        QCOMPARE(response->getLedger(), qint64(0)); // today's friendbot does not say
        bool threw = false;
        try { server->friendbot(key.data(), QUrl("ftp://friendbot.example/")); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        threw = false;
        try { server->friendbot(key.data(), QUrl("/friendbot")); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        threw = false;
        try { server->friendbot(nullptr); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
        delete server;
        friendbot->deleteLater();
    }

    /** Only a 200 saying "successful": true with a hash is a funded account; HTTP errors are the usual ones. */
    void testFriendbotFailureKinds()
    {
        FakeServer* fakeServer = new FakeServer();
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(ACCOUNT));
        const QString path = QStringLiteral("/friendbot?addr=") + ACCOUNT;
        auto fund = [&](const QString& body, const QString& code) {
            fakeServer->addGet(path, body, code);
            FriendbotResponse* response = server->friendbot(key.data());
            const QString result = outcome(response);
            delete response;
            return result;
        };
        QCOMPARE(fund(funded(), "200 OK"), QString("ready"));
        QCOMPARE(fund("{\"successful\":false,\"hash\":\"" + HASH + "\"}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{\"successful\":\"true\",\"hash\":\"" + HASH + "\"}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{\"hash\":\"" + HASH + "\"}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{\"successful\":true}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{\"successful\":true,\"hash\":\"" + HASH.left(63) + "\"}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{\"successful\":true,\"hash\":\"" + HASH.left(63) + "g\"}", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund(funded(",\"ledger\":0"), "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund(funded(",\"ledger\":4294967296"), "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund(funded(",\"ledger\":\"12\""), "200 OK"), QString("ready"));
        QCOMPARE(fund(funded(), "201 Created"), QString("unknownResponse"));
        QCOMPARE(fund("not json", "200 OK"), QString("unknownResponse"));
        QCOMPARE(fund("{}", "500 Internal Server Error"), QString("badResponse"));
        QCOMPARE(fund("{}", "429 Too Many Requests"), QString("rateLimited"));

        // An account friendbot funded already: its problem document, whole.
        fakeServer->addGet(path,
                           "{\"type\":\"https://stellar.org/horizon-errors/bad_request\",\"title\":\"Bad Request\","
                           "\"status\":400,\"detail\":\"createAccountAlreadyExist (AAAAAAAAAGT/////AAAAAQAAAAAAAAAA/////AAAAAA=)\"}",
                           "400 Bad Request");
        FriendbotResponse* refused = server->friendbot(key.data());
        QCOMPARE(outcome(refused), QString("badRequest"));
        QCOMPARE(refused->getStatus(), 400);
        auto* badRequest = dynamic_cast<qstellar::exception::BadRequestException*>(refused->lastException());
        QVERIFY(badRequest && badRequest->getProblem());
        QVERIFY(badRequest->getProblem()->getDetail().startsWith("createAccountAlreadyExist"));

        // No answer within replyTimeout(): the account may still be funded.
        Response::setReplyTimeout(300);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        FakeServer* silent = new FakeServer();
        silent->addHang("/friendbot");
        FriendbotResponse* late = server->friendbot(key.data(), QUrl(silent->baseUrl() + "/friendbot"));
        QCOMPARE(outcome(late), QString("timeout"));
        delete server;
        silent->deleteLater();
        fakeServer->deleteLater();
    }
};

ADD_TEST(ServerUtilitiesTest)

#endif // SERVERUTILITIESTEST_H
