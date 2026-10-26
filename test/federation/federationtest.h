#ifndef FEDERATIONTEST_H
#define FEDERATIONTEST_H
#include <QUrlQuery>
#include <QObject>

#include <QtTest>
#include "../../src/exception/federationexception.h"
#include "../testcollector.h"
#include "../../src/federation/federation.h"
#include "../../src/federation/federationserver.h"
#include "../../src/federation/federationresponse.h"
#include "../../src/keypair.h"
#include "../../src/muxedaccount.h"
#include "../../src/stellartomlresolver.h"
#include "../../src/sep7origincheck.h"
#include "../../src/sep7.h"
#ifdef STELLAR_QT_QML
#include "../../src/qml/sepsqml.h"
#endif
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QPointer>
#include <QScopeGuard>
#include <QScopedPointer>
#include <QHash>
#include <QList>
#include <QNetworkAccessManager>
#include <QPair>
#include <QSet>
#include <QUrl>
#include <QThread>
#include <cstring>
#include <functional>
#include <memory>

/** A finished reply carrying `body`, as the stellar.toml request would end. */
class FakeTomlReply : public QNetworkReply
{
    QByteArray m_body;
    qint64 m_read = 0;
public:
    explicit FakeTomlReply(const QByteArray& body, NetworkError failure = NoError)
        : m_body(body)
    {
        if (failure != NoError)
            setError(failure, QStringLiteral("failed"));
        open(QIODevice::ReadOnly);
        setFinished(true);
    }
    void abort() override {}
    qint64 bytesAvailable() const override
    {
        return m_body.size() - m_read + QNetworkReply::bytesAvailable();
    }
protected:
    qint64 readData(char* data, qint64 maxSize) override
    {
        const qint64 n = qMin(maxSize, qint64(m_body.size()) - m_read);
        if (n <= 0)
            return 0;
        std::memcpy(data, m_body.constData() + m_read, size_t(n));
        m_read += n;
        return n;
    }
};

/** A federation server's answer, arrived whole with HTTP 200. */
class FakeAnswerReply : public FakeTomlReply
{
public:
    explicit FakeAnswerReply(const QByteArray& body) : FakeTomlReply(body)
    {
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    }
};

/**
 * A domain's web server and its federation server, both on this machine over
 * plain http: LocalFederationNetwork sends the SDK's https requests here with
 * the host moved into the path, so https://a.example/.well-known/stellar.toml
 * is asked for as GET /a.example/.well-known/stellar.toml. A path it has no
 * answer for is a 404; a held path is answered on release() only, so a test
 * decides when a stellar.toml "arrives".
 */
class LocalFederationHost : public QObject
{
    struct Answer { QByteArray status; QByteArray body; QByteArray headers; };
    QTcpServer m_server;
    QHash<QByteArray, Answer> m_answers;
    QSet<QByteArray> m_held;
    QList<QPair<QPointer<QTcpSocket>, QByteArray>> m_waiting;
    QHash<QByteArray, int> m_asked;
    QHash<QTcpSocket*, QByteArray> m_unread;

public:
    LocalFederationHost()
    {
        m_server.listen(QHostAddress::LocalHost, 0);
        QObject::connect(&m_server, &QTcpServer::newConnection, this, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead, this, [this, socket]() { read(socket); });
            }
        });
    }
    quint16 port() const { return m_server.serverPort(); }

    static QByteArray tomlPath(const QString& domain)
    {
        return "/" + domain.toLatin1() + "/.well-known/stellar.toml";
    }
    static QByteArray federationPath(const QString& domain)
    {
        return "/fed." + domain.toLatin1() + "/federation";
    }
    /** `domain` publishes https://fed.<domain>/federation, which answers
     *  every name with `account`. */
    void serve(const QString& domain, const QString& account)
    {
        m_answers.insert(tomlPath(domain),
                         {"200 OK", "FEDERATION_SERVER=\"https://fed." + domain.toLatin1() + "/federation\"\n"});
        m_answers.insert(federationPath(domain),
                         {"200 OK", "{\"stellar_address\":\"bob*" + domain.toLatin1()
                                        + "\",\"account_id\":\"" + account.toLatin1() + "\"}"});
    }
    /** `path` is answered with `status` ("200 OK", "301 Moved Permanently") and `body`. */
    void answer(const QByteArray& path, const QByteArray& status, const QByteArray& body)
    {
        m_answers.insert(path, {status, body});
    }
    /** `path` is answered with `status` ("301 Moved Permanently") and a Location: `location`. */
    void redirect(const QByteArray& path, const QByteArray& status, const QByteArray& location)
    {
        m_answers.insert(path, {status, QByteArray(), "Location: " + location + "\r\n"});
    }
    void hold(const QByteArray& path) { m_held.insert(path); }
    void release(const QByteArray& path)
    {
        m_held.remove(path);
        const auto waiting = m_waiting;
        m_waiting.clear();
        for (const auto& request : waiting) {
            if (request.second != path)
                m_waiting.append(request);
            else if (request.first)
                respond(request.first, path);
        }
    }
    /** How many requests for `path` arrived, answered or held. */
    int asked(const QByteArray& path) const { return m_asked.value(path); }

private:
    void read(QTcpSocket* socket)
    {
        QByteArray& unread = m_unread[socket];
        unread += socket->readAll();
        for (int end = unread.indexOf("\r\n\r\n"); end >= 0; end = unread.indexOf("\r\n\r\n")) {
            const QByteArray head = unread.left(end);
            unread.remove(0, end + 4);
            const QList<QByteArray> requestLine = head.split('\r').first().split(' ');
            if (requestLine.size() < 2)
                continue;
            const QByteArray path = requestLine.at(1).split('?').first();
            ++m_asked[path];
            if (m_held.contains(path))
                m_waiting.append({QPointer<QTcpSocket>(socket), path});
            else
                respond(socket, path);
        }
    }
    void respond(QTcpSocket* socket, const QByteArray& path)
    {
        const Answer answer = m_answers.value(path, Answer{"404 Not Found", QByteArray()});
        socket->write("HTTP/1.1 " + answer.status + "\r\n" + answer.headers + "Content-Type: text/plain\r\nContent-Length: "
                      + QByteArray::number(answer.body.size()) + "\r\n\r\n" + answer.body);
    }
};

/** Sends every request to a LocalFederationHost, and keeps each reply in sight. */
class LocalFederationNetwork : public QNetworkAccessManager
{
public:
    quint16 port = 0;
    QList<QPointer<QNetworkReply>> replies;

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& original,
                                 QIODevice* outgoingData) override
    {
        QUrl url = original.url();
        url.setPath(QLatin1Char('/') + url.host() + url.path());
        url.setScheme(QStringLiteral("http"));
        url.setHost(QStringLiteral("127.0.0.1"));
        url.setPort(port);
        QNetworkRequest request(original);
        request.setUrl(url);
        QNetworkReply* reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
        replies.append(reply);
        return reply;
    }
};

/**
 * Federation lookups against a LocalFederationHost, for as long as it lives:
 * then the cache is off again, whatever the cache deleted later is gone, and
 * the SDK's own network manager is back.
 */
class LocalFederation
{
public:
    LocalFederationHost host;
    LocalFederationNetwork network;
    QNetworkAccessManager* previous;

    LocalFederation()
    {
        network.port = host.port();
        previous = FederationServer::setNetworkAccessManager(&network);
    }
    ~LocalFederation()
    {
        FederationServer::activeCache(false);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        FederationServer::setNetworkAccessManager(previous);
    }
};

class FederationTest: public QObject
{
    Q_OBJECT

    //TODO use offline test...
//    const QByteArray successResponse =
//   "{\"stellar_address\":\"bob*stellar.org\",\"account_id\":\"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\"}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }


#ifndef STELLAR_SKIP_LIVE_TESTS
    void testResolveSuccess()  {
//        FederationResponse response(0);
//        response.loadFromJson(successResponse);

        FederationResponse* response = Federation::resolve("bank*funtracker.site");

        WAIT_FOR(response->getStellarAddress().isNull())

        QCOMPARE(response->getStellarAddress(), QString("bank*funtracker.site"));
        QCOMPARE(response->getAccountId(), QString("GBUYUAI75XXWDZEKLY66CFYKQPET5JR4EENXZBUZ3YXZ7DS56Z4OKOFU"));
        QVERIFY(response->getMemoType().isNull());
        QVERIFY(response->getMemo().isNull());
    }
#endif


    void testMalformedAddress() {
        try {
            FederationResponse* response = Federation::resolve("bob*stellar.org*test");
            Q_UNUSED(response)
            QFAIL("Expected exception");
        } catch (...) {

       }
    }

    /**
     * resolve("") came back as the account id "" with no reply behind it, so
     * whoever waited for an answer waited for ever. "*example.com" and "bob*"
     * were looked up, "bob*good.example@evil.example" at evil.example, and
     * "hello" handed back as an account id. All are refused before any
     * request now; an account id, G or M, is still its own answer.
     */
    void testResolveRefusesWhatItCannotAnswer()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        QString corrupt = key->getAccountId();
        corrupt[20] = corrupt.at(20) == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        const QStringList bad{QString(), QStringLiteral(" \t "), QStringLiteral("*example.com"),
                              QStringLiteral("bob*"), QStringLiteral("hello"),
                              QStringLiteral("bob*good.example@evil.example"),
                              QStringLiteral("bob*example.com/.well-known?"),
                              QStringLiteral("bob*example.com:http"), QStringLiteral("bob*example.com:"),
                              QStringLiteral("bob*exa mple.com"),
                              key->getSecretSeed(), corrupt, key->getAccountId().toLower()};
        for (const QString& value : bad) {
            try {
                delete Federation::resolve(value);
                QFAIL(qPrintable(QStringLiteral("Missing exception for \"%1\"").arg(value)));
            } catch (const std::runtime_error& e) {
                // A seed pasted by mistake is not repeated into a log.
                QVERIFY(!QString(e.what()).contains(key->getSecretSeed()));
            }
        }

        QScopedPointer<FederationResponse> own(Federation::resolve(key->getAccountId()));
        QCOMPARE(own->getAccountId(), key->getAccountId());
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(key->getAccountId(), 9).toAddress();
        QScopedPointer<FederationResponse> sub(Federation::resolve(QStringLiteral(" ") + muxed));
        QCOMPARE(sub->getAccountId(), muxed);
        Federation::validate(QStringLiteral("bob*example.com"));
        Federation::validate(QStringLiteral("bob*example.com:8080"));
    }

    /**
     * The toml's FEDERATION_SERVER was used as given, http included, so anyone
     * on the path could answer with their own account for the name being paid;
     * js refuses it, as the constructor taking a QUrl does. And a failed
     * request was read as the toml unless its code was 300 or more: a 404
     * page or an unreachable host, whatever the body said.
     */
    void testTomlFederationServerIsHttpsFromARealAnswer()
    {
        auto outcome = [](const QByteArray& toml, QNetworkReply::NetworkError failure) {
            FederationServer server;
            QString reason;
            bool ready = false;
            QObject::connect(&server, &FederationServer::error, [&](QString r) { reason = r; });
            QObject::connect(&server, &FederationServer::ready, [&]() { ready = true; });
            auto* reply = new FakeTomlReply(toml, failure);
            QObject::connect(reply, SIGNAL(finished()), &server, SLOT(initialize()));
            emit reply->finished();
            QCoreApplication::sendPostedEvents(reply, QEvent::DeferredDelete);
            return ready ? QStringLiteral("ready") : reason;
        };
        const QByteArray https("FEDERATION_SERVER=\"https://fed.example/federation\"\n");
        QCOMPARE(outcome(https, QNetworkReply::NoError), QStringLiteral("ready"));
        QCOMPARE(outcome("FEDERATION_SERVER=\"http://fed.example/federation\"\n", QNetworkReply::NoError),
                 QStringLiteral("Invalid federation server"));
        QCOMPARE(outcome("FEDERATION_SERVER=\"fed.example/federation\"\n", QNetworkReply::NoError),
                 QStringLiteral("Invalid federation server"));
        for (QNetworkReply::NetworkError failure : {QNetworkReply::ContentNotFoundError,
                                                   QNetworkReply::HostNotFoundError,
                                                   QNetworkReply::OperationCanceledError}) {
            QVERIFY(outcome(https, failure).startsWith(QStringLiteral("the stellar.toml could not be read")));
        }
        QVERIFY(outcome("FEDERATION_SERVER=", QNetworkReply::NoError)
                    .startsWith(QStringLiteral("the stellar.toml is not valid TOML")));
    }

    /**
     * The stellar.toml request had no timeout: a host that accepted the
     * connection and never answered left createForDomain(), and every
     * Federation::resolve or Seps.resolveFederation waiting on it, pending for
     * ever. It is given up after the toml timeout (STELLAR_QT_REPLY_TIMEOUT
     * unless set), with error().
     */
    void testTheTomlRequestGivesUpOnAHostThatNeverAnswers()
    {
        LocalFederation local;
        local.host.hold(LocalFederationHost::tomlPath(QStringLiteral("silent.example")));   // never answered
        QCOMPARE(FederationServer::tomlTimeout(), STELLAR_QT_REPLY_TIMEOUT);
        FederationServer::setTomlTimeout(300);
        QPointer<FederationServer> server(FederationServer::createForDomain(QStringLiteral("silent.example")));
        QSignalSpy failed(server.data(), &FederationServer::error);
        QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 10000);
        QVERIFY(failed.first().first().toString().startsWith(QStringLiteral("the stellar.toml could not be read")));
        FederationServer::setTomlTimeout(0);
        QCOMPARE(FederationServer::tomlTimeout(), STELLAR_QT_REPLY_TIMEOUT);
        delete server.data();
    }

    /**
     * The name went into the query through QUrlQuery, which leaves '+', '#'
     * and '%' as they are: a server read "john+pay@gmail.com*d" as "john
     * pay@gmail.com*d", "john#x*d" was cut at the '#' (a fragment, never
     * sent) and "a%2B*d" decoded to "a+*d". Each character is sent encoded
     * now, as js and py send it, and the server's own query is kept, as py
     * keeps it (js drops it).
     */
    void testTheNameIsSentEncoded()
    {
        class Recorder : public QNetworkAccessManager
        {
        public:
            QList<QUrl> urls;
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
            {
                urls.append(request.url());
                return new FakeAnswerReply(QByteArray("{}"));
            }
        };
        Recorder recorder;
        QNetworkAccessManager* previous = FederationServer::setNetworkAccessManager(&recorder);
        FederationServer server(QUrl(QStringLiteral("https://fed.example.com/federation")),
                                QStringLiteral("example.com"));
        FederationServer withQuery(QUrl(QStringLiteral("https://fed.example.com/api?v=2")),
                                   QStringLiteral("example.com"));
        const QList<QPair<QString, QString>> names = {
            {QStringLiteral("john+pay@gmail.com*example.com"), QStringLiteral("john%2Bpay%40gmail.com%2Aexample.com")},
            {QStringLiteral("john#x*example.com"), QStringLiteral("john%23x%2Aexample.com")},
            {QStringLiteral("a%2B*example.com"), QStringLiteral("a%252B%2Aexample.com")},
            {QStringLiteral("a&type=id*example.com"), QStringLiteral("a%26type%3Did%2Aexample.com")},
            {QStringLiteral("jo hn*example.com"), QStringLiteral("jo%20hn%2Aexample.com")},
        };
        for (const auto& name : names) {
            recorder.urls.clear();
            delete server.resolveAddress(name.first);
            QCOMPARE(recorder.urls.size(), 1);
            const QUrl sent = recorder.urls.first();
            QVERIFY(!sent.hasFragment());
            QCOMPARE(sent.query(QUrl::FullyEncoded), QStringLiteral("type=name&q=") + name.second);
            QCOMPARE(QUrlQuery(sent).queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded), name.first);
        }
        recorder.urls.clear();
        delete withQuery.resolveAddress(QStringLiteral("bob*example.com"));
        QCOMPARE(recorder.urls.size(), 1);
        QCOMPARE(recorder.urls.first().query(QUrl::FullyEncoded), QStringLiteral("v=2&type=name&q=bob%2Aexample.com"));
        FederationServer::setNetworkAccessManager(previous);
    }

    /**
     * The three SEP-2 requests (type=name, id and txid) against what py
     * 14.1.0 (resolve_stellar_address, resolve_account_id: the URL requests
     * prepares) and js 17.1.0 (FederationServer.resolveAddress,
     * resolveAccountId, resolveTransactionId, its fetch client stubbed) send
     * for the same values. The URL is py's byte for byte, but for a space,
     * which py and js send as '+' and this server as %20; js leaves '*' as it
     * is. q reads back as js reads it, every time; a name alone is the name
     * at the server's domain, as in js (py takes a full address only).
     */
    void testTheFederationRequestsAreJsAndPys()
    {
        class Recorder : public QNetworkAccessManager
        {
        public:
            QList<QUrl> urls;
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
            {
                urls.append(request.url());
                return new FakeAnswerReply(QByteArray("{}"));
            }
        };
        Recorder recorder;
        QNetworkAccessManager* previous = FederationServer::setNetworkAccessManager(&recorder);
        auto restore = qScopeGuard([previous] { FederationServer::setNetworkAccessManager(previous); });
        FederationServer server(QUrl(QStringLiteral("https://fed.example.com/federation")),
                                QStringLiteral("example.com"));
        // name, py's URL (this server's where it differs), js's q read back.
        struct Vector { const char* name; const char* url; const char* q; };
        const Vector names[] = {
            {"bob*example.com", "https://fed.example.com/federation?type=name&q=bob%2Aexample.com", "bob*example.com"},
            {"john+pay@gmail.com*example.com",
             "https://fed.example.com/federation?type=name&q=john%2Bpay%40gmail.com%2Aexample.com",
             "john+pay@gmail.com*example.com"},
            // py: q=jo+hn%2Aexample.com
            {"jo hn*example.com", "https://fed.example.com/federation?type=name&q=jo%20hn%2Aexample.com", "jo hn*example.com"},
            {"a%2B*example.com", "https://fed.example.com/federation?type=name&q=a%252B%2Aexample.com", "a%2B*example.com"},
            {"a&type=id*example.com", "https://fed.example.com/federation?type=name&q=a%26type%3Did%2Aexample.com",
             "a&type=id*example.com"},
            {"j\xc3\xb6rg*example.com", "https://fed.example.com/federation?type=name&q=j%C3%B6rg%2Aexample.com",
             "j\xc3\xb6rg*example.com"},
            // js only.
            {"bob", "https://fed.example.com/federation?type=name&q=bob%2Aexample.com", "bob*example.com"},
        };
        for (const Vector& vector : names) {
            recorder.urls.clear();
            delete server.resolveAddress(QString::fromUtf8(vector.name));
            QCOMPARE(recorder.urls.size(), 1);
            const QUrl sent = recorder.urls.first();
            QCOMPARE(sent.toString(QUrl::FullyEncoded), QString::fromUtf8(vector.url));
            QCOMPARE(QUrlQuery(sent).queryItemValue(QStringLiteral("type")), QStringLiteral("name"));
            QCOMPARE(QUrlQuery(sent).queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded), QString::fromUtf8(vector.q));
        }

        // py and js alike.
        recorder.urls.clear();
        delete server.resolveAccountId(QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY"));
        // js (py has no transaction lookup).
        delete server.resolveTransactionId(QStringLiteral("3389e9f0f1a65f19736cacf544c2e825313e8447f569233bb8db39aa607c8889"));
        QCOMPARE(recorder.urls.size(), 2);
        QCOMPARE(recorder.urls.at(0).toString(QUrl::FullyEncoded),
                 QStringLiteral("https://fed.example.com/federation?type=id&q=GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY"));
        QCOMPARE(recorder.urls.at(1).toString(QUrl::FullyEncoded),
                 QStringLiteral("https://fed.example.com/federation?type=txid&q=3389e9f0f1a65f19736cacf544c2e825313e8447f569233bb8db39aa607c8889"));
    }

    /**
     * The federation server's answer was taken as it came: an account_id that
     * is not an address, a memo_type nobody defines, a memo invalid for its
     * type (a text memo too long to fit, an id past uint64 or sent as a JSON
     * number and rounded on its way into a string, a hash that is not 32
     * bytes) were handed to the wallet to pay. Such an answer is an error now,
     * and nothing of it is kept. An empty text memo was refused as "a
     * memo_type without a memo": core takes MEMO_TEXT "", and js and py hand
     * it back, so it is kept now.
     */
    void testAFederationAnswerIsCheckedBeforeItIsKept()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        const QString account = key->getAccountId();
        auto outcome = [](const QString& body, QString* accountId = nullptr, QString* memo = nullptr) {
            auto* reply = new FakeAnswerReply(body.toUtf8());
            FederationResponse response(reply);
            bool ready = false, failed = false;
            QObject::connect(&response, &Response::ready, [&]() { ready = true; });
            QObject::connect(&response, &Response::error, [&]() { failed = true; });
            emit reply->finished();
            if (accountId) *accountId = response.getAccountId();
            if (memo) *memo = response.getMemo();
            return ready && !failed ? QStringLiteral("ready") : failed ? QStringLiteral("error") : QStringLiteral("none");
        };
        auto answer = [](const QString& fields) {
            return QStringLiteral("{\"stellar_address\":\"bob*example.com\",%1}").arg(fields);
        };
        QString got, memo;
        QCOMPARE(outcome(answer(QStringLiteral("\"account_id\":\"%1\"").arg(account)), &got), QStringLiteral("ready"));
        QCOMPARE(got, account);
        QCOMPARE(outcome(answer(QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"18446744073709551615\"")
                                    .arg(account)), &got, &memo),
                 QStringLiteral("ready"));
        QCOMPARE(memo, QStringLiteral("18446744073709551615"));
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(account, 5).toAddress();
        QCOMPARE(outcome(answer(QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"order 42\"").arg(muxed))),
                 QStringLiteral("ready"));
        QCOMPARE(outcome(answer(QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"%2\"")
                                    .arg(account, QString::fromLatin1(QByteArray(32, '\x09').toBase64())))),
                 QStringLiteral("ready"));
        memo = QStringLiteral("unset");
        QCOMPARE(outcome(answer(QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"\"").arg(account)),
                         &got, &memo),
                 QStringLiteral("ready"));
        QVERIFY(memo.isEmpty());

        for (const QString& fields : {QStringLiteral("\"memo\":\"1\""),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(key->getSecretSeed()),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(account.toLower()),
                                      QStringLiteral("\"account_id\":[\"%1\"]").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"%2\"")
                                          .arg(account, QString(29, QLatin1Char('x'))),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"18446744073709551616\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":12345678901234567").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\" 42\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"%2\"")
                                          .arg(account, QString::fromLatin1(QByteArray(33, '\x09').toBase64())),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"MEMO_TEXT\",\"memo\":\"x\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo\":\"x\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":null").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"\"").arg(account)}) {
            got = QStringLiteral("unset");
            QCOMPARE(outcome(answer(fields), &got), QStringLiteral("error"));
            QVERIFY2(got.isEmpty(), qPrintable(fields));
        }
        QCOMPARE(outcome(QStringLiteral("[1, 2]")), QStringLiteral("error"));
    }

    /**
     * activeCache(false) deleted the cache without forgetting it: the cache
     * still read as active, createForDomain() looked domains up in freed
     * memory, and turning the cache on again kept the dangling pointer, to be
     * deleted a second time.
     */
    void testTurningTheCacheOffForgetsIt()
    {
        QVERIFY(!FederationServer::isCacheActive());
        FederationServer::activeCache(true);
        QVERIFY(FederationServer::isCacheActive());
        FederationServer::activeCache(false);
        QVERIFY(!FederationServer::isCacheActive());
        FederationServer::activeCache(true);
        QVERIFY(FederationServer::isCacheActive());
        FederationServer::activeCache(false);
        QVERIFY(!FederationServer::isCacheActive());
    }

    /**
     * The SDK's own network manager (StellarTomlResolver::resolve and
     * FederationServer use it when given none) is the calling thread's, as a
     * QNetworkAccessManager belongs to the thread that made it. There was one
     * for the process, parented to qApp and made by whichever thread asked
     * first: made from a worker, it served the GUI thread from the worker's
     * thread, and nothing once the worker had ended. The federation cache is
     * the calling thread's too; it had no lock. (js and py have no thread
     * affinity: each request has its own fetch or session.)
     */
    void testTheDefaultNetworkManagerIsTheCallingThreads()
    {
        class Worker : public QThread
        {
        public:
            std::function<void()> job;
        protected:
            void run() override { job(); }
        };
        FederationServer::activeCache(true);
        auto cacheOff = qScopeGuard([] { FederationServer::activeCache(false); });
        bool workerOwnsItsManager = false;
        bool workerHasNoCache = false;
        Worker worker;
        worker.job = [&]() {
            QNetworkAccessManager& network = StellarTomlResolver::defaultNetworkAccessManager();
            workerOwnsItsManager = network.thread() == QThread::currentThread()
                                   && &network == &StellarTomlResolver::defaultNetworkAccessManager();
            workerHasNoCache = !FederationServer::isCacheActive();
        };
        worker.start();
        QVERIFY(worker.wait(10000));
        QVERIFY(workerOwnsItsManager);
        QVERIFY(workerHasNoCache);
        QNetworkAccessManager& network = StellarTomlResolver::defaultNetworkAccessManager();
        QCOMPARE(network.thread(), QThread::currentThread());
        QVERIFY(&network == &StellarTomlResolver::defaultNetworkAccessManager());
        QVERIFY(FederationServer::isCacheActive());
    }

    /**
     * A thread's federation cache was a thread_local raw pointer that nothing
     * freed: a worker that turned the cache on and ended without
     * activeCache(false) leaked the hash and every server in it, QObjects of
     * a thread that was gone. The cache and its servers go with the thread
     * now, as the thread's network manager does. (js and py keep no cache of
     * federation servers.)
     */
    void testAThreadsCacheGoesWithTheThread()
    {
        class Worker : public QThread
        {
        public:
            std::function<void()> job;
        protected:
            void run() override { job(); }
        };
        // The stellar.toml request is never answered (the worker runs no event
        // loop), so the server is still reading it when the thread ends.
        class Silent : public QNetworkAccessManager
        {
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest&, QIODevice*) override
            {
                QNetworkReply* reply = new FakeTomlReply(QByteArray());
                reply->setParent(this);
                return reply;
            }
        };
        QVERIFY(!FederationServer::isCacheActive());
        QPointer<FederationServer> server;
        bool cachedInTheWorker = false;
        Worker worker;
        worker.job = [&]() {
            Silent silent;
            QNetworkAccessManager* previous = FederationServer::setNetworkAccessManager(&silent);
            FederationServer::activeCache(true);
            server = FederationServer::createForDomain(QStringLiteral("worker.example"));
            cachedInTheWorker = FederationServer::isCacheActive()
                                && FederationServer::createForDomain(QStringLiteral("worker.example")) == server.data();
            FederationServer::setNetworkAccessManager(previous);
        };
        worker.start();
        QVERIFY(worker.wait(10000));
        QVERIFY(cachedInTheWorker);
        QVERIFY(server.isNull());
        QVERIFY(!FederationServer::isCacheActive());
    }

    /**
     * FederationServer(QString, QString) took an http server (anyone on the
     * path could answer with their own account for the name being paid) while
     * the QUrl constructor refused one. Both take https with a host only.
     */
    void testBothConstructorsAreHttpsOnly()
    {
        auto fromText = [](const QString& uri) {
            try {
                FederationServer server(uri, QStringLiteral("example.com"));
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        auto fromUrl = [](const QUrl& uri) {
            try {
                FederationServer server(uri, QStringLiteral("example.com"));
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        QVERIFY(fromText(QStringLiteral("https://fed.example/federation")));
        QVERIFY(fromUrl(QUrl(QStringLiteral("https://fed.example/federation"))));
        for (const QString& uri : {QStringLiteral("http://fed.example/federation"),
                                   QStringLiteral("ftp://fed.example/federation"),
                                   QStringLiteral("fed.example/federation"),
                                   QStringLiteral("https:///federation")}) {
            QVERIFY2(!fromText(uri), qPrintable(uri));
            QVERIFY2(!fromUrl(QUrl(uri)), qPrintable(uri));
        }
    }

    /**
     * SEP-1's fetcher, as js's StellarToml.Resolver: the domain's
     * /.well-known/stellar.toml over https, read as TOML; no redirect
     * followed; refused over 100 KB; a domain that is not a host name (an
     * optional port aside) refused before any request.
     */
    void testStellarTomlResolverReadsADomainsToml()
    {
        LocalFederation local;
        local.host.serve(QStringLiteral("good.example"), QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY"));
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("moved.example")),
                          "301 Moved Permanently", QByteArray());
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("big.example")), "200 OK",
                          "# " + QByteArray(StellarTomlResolver::MaxSize, 'x') + "\nVERSION=\"2.0.0\"\n");
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("junk.example")), "200 OK", "VERSION=");

        auto outcome = [&](const QString& domain) {
            QPointer<StellarTomlResolver> resolver(StellarTomlResolver::resolve(domain, &local.network, 10000));
            QString result;
            bool done = false;
            QObject::connect(resolver, &StellarTomlResolver::resolved, [&](const QVariantMap& toml) {
                result = QStringLiteral("toml: ") + toml.value(QStringLiteral("FEDERATION_SERVER")).toString();
                done = true;
            });
            QObject::connect(resolver, &StellarTomlResolver::failed, [&](const QString& why) {
                result = why;
                done = true;
            });
            if (!QTest::qWaitFor([&]() { return done; }, 10000))
                return QStringLiteral("no answer");
            if (!QTest::qWaitFor([&]() { return resolver.isNull(); }, 5000))
                return QStringLiteral("not deleted");   // it deletes itself
            return result;
        };
        QCOMPARE(outcome(QStringLiteral("good.example")), QStringLiteral("toml: https://fed.good.example/federation"));
        QVERIFY(outcome(QStringLiteral("missing.example")).startsWith(QStringLiteral("the stellar.toml could not be read")));
        QVERIFY(outcome(QStringLiteral("moved.example")).startsWith(QStringLiteral("the stellar.toml could not be read")));
        QCOMPARE(outcome(QStringLiteral("big.example")), QStringLiteral("the stellar.toml is larger than 100 KB"));
        QVERIFY(outcome(QStringLiteral("junk.example")).startsWith(QStringLiteral("the stellar.toml is not valid TOML")));

        for (const char* domain : {"good.example@evil.example", "good.example:0", "https://good.example",
                                   "good.example/x", ""}) {
            QVERIFY_EXCEPTION_THROWN(StellarTomlResolver::resolve(QString::fromLatin1(domain), &local.network),
                                     std::runtime_error);
        }
    }

    /**
     * A domain with a port ("anchor.example:8443") was refused, by
     * StellarTomlResolver and in a federation address, where js and py take
     * it. It is taken now, and the resolver reads a domain as js does,
     * surrounding whitespace and a trailing '/' dropped. An IPv4 or IDN host
     * is taken as py and js's resolver take it, and so is 3m.com (js's
     * federation check refuses all three); an IPv6 literal is not.
     */
    void testADomainMayCarryAPort()
    {
        for (const QString& domain : {QStringLiteral("anchor.example:8443"), QStringLiteral("localhost:8000"),
                                      QStringLiteral("example.com:443"), QStringLiteral("example.com"),
                                      QStringLiteral("3m.com"), QStringLiteral("1.2.3.4"),
                                      QString::fromUtf8("b\xc3\xbc" "cher.de")}) {
            QVERIFY2(StellarTomlResolver::isHostName(domain), qPrintable(domain));
        }
        for (const QString& domain : {QStringLiteral("example.com:"), QStringLiteral("example.com:0"),
                                      QStringLiteral("example.com:65536"), QStringLiteral("example.com:+80"),
                                      QStringLiteral("example.com: 80"), QStringLiteral("example.com:80:80"),
                                      QStringLiteral(":8443"), QStringLiteral("[::1]:8443"),
                                      QStringLiteral(" example.com"), QStringLiteral("example.com/"),
                                      QStringLiteral("user@example.com:8443")}) {
            QVERIFY2(!StellarTomlResolver::isHostName(domain), qPrintable(domain));
        }

        class Recorder : public QNetworkAccessManager
        {
        public:
            QList<QUrl> urls;
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
            {
                urls.append(request.url());
                return new FakeTomlReply(QByteArray());
            }
        };
        Recorder recorder;
        delete StellarTomlResolver::request(QStringLiteral("anchor.example:8443"), recorder);
        delete StellarTomlResolver::request(QStringLiteral(" anchor.example:8443/ "), recorder);
        QVERIFY_EXCEPTION_THROWN(StellarTomlResolver::request(QStringLiteral("anchor.example /"), recorder),
                                 std::runtime_error);
        QCOMPARE(recorder.urls.size(), 2);
        for (const QUrl& url : recorder.urls)
            QCOMPARE(url, QUrl(QStringLiteral("https://anchor.example:8443/.well-known/stellar.toml")));
        Federation::validate(QStringLiteral("bob*anchor.example:8443"));
        QVERIFY_EXCEPTION_THROWN(Federation::validate(QStringLiteral("bob*anchor.example:8443/")), std::runtime_error);
    }

    /**
     * The stellar.toml URL against the one js 17.1.0's
     * StellarToml.Resolver.resolve asks for (its fetch client stubbed) for
     * the same domains: whitespace around and a trailing '/' or '\' dropped,
     * the host in lower case and an IDN in its ACE form. py 14.1.0's
     * fetch_stellar_toml agrees on example.com, anchor.example:8443 and
     * 1.2.3.4; it pastes the rest as given (" example.com/ " is asked for at
     * "https:// example.com/ /.well-known/stellar.toml", and "EXAMPLE.com"
     * keeps its case, which names the same host).
     */
    void testTheStellarTomlUrlIsJss()
    {
        class Recorder : public QNetworkAccessManager
        {
        public:
            QList<QUrl> urls;
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
            {
                urls.append(request.url());
                return new FakeTomlReply(QByteArray());
            }
        };
        Recorder recorder;
        const QList<QPair<QString, QString>> vectors = {
            {QStringLiteral("example.com"), QStringLiteral("https://example.com/.well-known/stellar.toml")},
            {QStringLiteral("anchor.example:8443"), QStringLiteral("https://anchor.example:8443/.well-known/stellar.toml")},
            {QStringLiteral(" example.com/ "), QStringLiteral("https://example.com/.well-known/stellar.toml")},
            {QStringLiteral("example.com\\"), QStringLiteral("https://example.com/.well-known/stellar.toml")},
            {QStringLiteral("EXAMPLE.com"), QStringLiteral("https://example.com/.well-known/stellar.toml")},
            {QStringLiteral("1.2.3.4"), QStringLiteral("https://1.2.3.4/.well-known/stellar.toml")},
            {QString::fromUtf8("b\xc3\xbc" "cher.de"), QStringLiteral("https://xn--bcher-kva.de/.well-known/stellar.toml")},
        };
        for (const auto& vector : vectors) {
            recorder.urls.clear();
            delete StellarTomlResolver::request(vector.first, recorder);
            QCOMPARE(recorder.urls.size(), 1);
            QCOMPARE(recorder.urls.first().toString(QUrl::FullyEncoded), vector.second);
        }
    }

    /**
     * SEP-2's other two lookups, as js's resolveAccountId / resolveTransactionId
     * and py's resolve_account_id (py has no txid lookup): type=id with an
     * account id, type=txid with a transaction hash. What is not one is
     * refused before any request.
     */
    void testReverseAndTransactionLookups()
    {
        LocalFederation local;
        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        local.host.serve(QStringLiteral("rev.example"), account);
        FederationServer server(QUrl(QStringLiteral("https://fed.rev.example/federation")), QStringLiteral("rev.example"));

        auto sent = [&]() {
            return local.network.replies.isEmpty() || !local.network.replies.last()
                       ? QString() : local.network.replies.last()->request().url().query(QUrl::FullyEncoded);
        };
        QScopedPointer<FederationResponse> reverse(server.resolveAccountId(account));
        QCOMPARE(sent(), QStringLiteral("type=id&q=") + account);
        QSignalSpy reverseReady(reverse.data(), &Response::ready);
        QTRY_VERIFY_WITH_TIMEOUT(reverseReady.count() == 1, 10000);
        QCOMPARE(reverse->getStellarAddress(), QStringLiteral("bob*rev.example"));
        QCOMPARE(reverse->getAccountId(), account);

        const QString txid = QString(64, QLatin1Char('a'));
        QScopedPointer<FederationResponse> tx(server.resolveTransactionId(txid));
        QCOMPARE(sent(), QStringLiteral("type=txid&q=") + txid);
        QSignalSpy txReady(tx.data(), &Response::ready);
        QTRY_VERIFY_WITH_TIMEOUT(txReady.count() == 1, 10000);
        QCOMPARE(tx->getAccountId(), account);

        const int requests = local.network.replies.size();
        std::unique_ptr<KeyPair> random(KeyPair::random());
        for (const QString& junk : {random->getSecretSeed(), QStringLiteral("bob*rev.example"),
                                    account.toLower(), QString()}) {
            QVERIFY_EXCEPTION_THROWN(delete server.resolveAccountId(junk), std::runtime_error);
        }
        for (const QString& junk : {QString(63, QLatin1Char('a')), QString(65, QLatin1Char('a')),
                                    QString(64, QLatin1Char('g')), QString()}) {
            QVERIFY_EXCEPTION_THROWN(delete server.resolveTransactionId(junk), std::runtime_error);
        }
        QCOMPARE(local.network.replies.size(), requests);
    }

    /**
     * A reverse lookup's answer was checked as a forward one: an answer
     * without account_id was refused (py reads it as the record of the
     * account asked for, js hands it back), and one without stellar_address,
     * the one thing a reverse lookup is for, was accepted (py refuses it).
     * Both are read as py reads them now, and an answer naming another
     * account than the one asked for is refused. An empty account_id names
     * no account: it was refused as "about another account", and is the
     * account asked for now, as py reads it (`account_id or
     * data.get("account_id")`; js hands back "").
     */
    void testAReverseLookupAnswerIsTheRecordOfTheAccountAskedFor()
    {
        LocalFederation local;
        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        const QString other = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const QByteArray path = LocalFederationHost::federationPath(QStringLiteral("rev.example"));
        FederationServer server(QUrl(QStringLiteral("https://fed.rev.example/federation")), QStringLiteral("rev.example"));
        QScopedPointer<FederationResponse> lookup;
        auto outcome = [&](const QByteArray& body) {
            local.host.answer(path, "200 OK", body);
            lookup.reset(server.resolveAccountId(account));
            QSignalSpy ready(lookup.data(), &Response::ready);
            QSignalSpy failed(lookup.data(), &Response::error);
            if (!QTest::qWaitFor([&]() { return ready.count() + failed.count() > 0; }, 10000))
                return QStringLiteral("no answer");
            return ready.count() == 1 && failed.isEmpty() ? QStringLiteral("ready") : QStringLiteral("error");
        };
        QCOMPARE(outcome("{\"stellar_address\":\"bob*rev.example\"}"), QStringLiteral("ready"));
        QCOMPARE(lookup->getStellarAddress(), QStringLiteral("bob*rev.example"));
        QCOMPARE(lookup->getAccountId(), account);
        QCOMPARE(lookup->json().value(QStringLiteral("account_id")).toString(), account);
        QCOMPARE(outcome("{\"stellar_address\":\"bob*rev.example\",\"account_id\":\"" + account.toLatin1() + "\"}"),
                 QStringLiteral("ready"));
        QCOMPARE(lookup->getAccountId(), account);
        for (const QByteArray& body : {QByteArray("{\"stellar_address\":\"bob*rev.example\",\"account_id\":\"\"}"),
                                       QByteArray("{\"stellar_address\":\"bob*rev.example\",\"account_id\":null}")}) {
            QCOMPARE(outcome(body), QStringLiteral("ready"));
            QCOMPARE(lookup->getAccountId(), account);
            QCOMPARE(lookup->json().value(QStringLiteral("account_id")).toString(), account);
        }

        const QList<QByteArray> refused = {
            "{\"account_id\":\"" + account.toLatin1() + "\"}",
            "{\"stellar_address\":\"\",\"account_id\":\"" + account.toLatin1() + "\"}",
            "{\"stellar_address\":\"bob*rev.example\",\"account_id\":\"" + other.toLatin1() + "\"}",
        };
        for (const QByteArray& body : refused) {
            QCOMPARE(outcome(body), QStringLiteral("error"));
            QVERIFY2(lookup->getAccountId().isEmpty(), body.constData());
        }
    }

    /**
     * Sep7OriginCheck reads the origin_domain's stellar.toml and checks the
     * request against its URI_REQUEST_SIGNING_KEY (it lives here for the
     * local fake host). A request with no origin, or one that is not a fully
     * qualified domain name, is refused before any request; a toml that
     * cannot be read says so.
     */
    void testSep7OriginCheckReadsTheOriginsToml()
    {
        LocalFederation local;
        std::unique_ptr<KeyPair> origin(KeyPair::random());
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("shop.example")), "200 OK",
                          "URI_REQUEST_SIGNING_KEY=\"" + origin->getAccountId().toLatin1() + "\"\n");
        const QString destination = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");

        auto outcome = [&](const QString& uri) {
            QPointer<Sep7OriginCheck> check(Sep7OriginCheck::verify(uri, &local.network, 10000));
            QString result;
            QObject::connect(check, &Sep7OriginCheck::verified, [&](const QString& key) { result = "verified " + key; });
            QObject::connect(check, &Sep7OriginCheck::failed, [&](const QString& why) { result = why; });
            if (!QTest::qWaitFor([&]() { return !result.isEmpty(); }, 10000))
                return QStringLiteral("no answer");
            return result;
        };
        Sep7Pay pay(destination);
        pay.originDomain = QStringLiteral("shop.example");
        pay.sign(origin.get());
        QCOMPARE(outcome(pay.toUri()), "verified " + origin->getAccountId());

        std::unique_ptr<KeyPair> impostor(KeyPair::random());
        pay.sign(impostor.get());
        QVERIFY(outcome(pay.toUri()).contains(QStringLiteral("not signed by its origin")));

        pay.originDomain = QStringLiteral("gone.example");
        pay.sign(origin.get());
        QVERIFY(outcome(pay.toUri()).startsWith(QStringLiteral("Sep7: the origin_domain's stellar.toml could not be read")));

        const int requests = local.network.replies.size();
        Sep7Pay plain(destination);
        QVERIFY_EXCEPTION_THROWN(Sep7OriginCheck::verify(plain.toUri(), &local.network), std::runtime_error);
        for (const char* host : {"localhost", "127.0.0.1"}) {
            plain.originDomain = QString::fromLatin1(host);
            plain.sign(origin.get());
            QVERIFY_EXCEPTION_THROWN(Sep7OriginCheck::verify(plain.toUri(), &local.network), std::runtime_error);
        }
        QCOMPARE(local.network.replies.size(), requests);
    }

    /**
     * A member SEP-2 does not name (a server's own) is kept: json() is the
     * whole answer, as js and py return it.
     */
    void testAFederationAnswerKeepsEveryMember()
    {
        LocalFederation local;
        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        local.host.serve(QStringLiteral("extra.example"), account);
        local.host.answer(LocalFederationHost::federationPath(QStringLiteral("extra.example")), "200 OK",
                          "{\"stellar_address\":\"bob*extra.example\",\"account_id\":\"" + account.toLatin1()
                              + "\",\"display_name\":\"Bob\",\"limits\":{\"daily\":\"100\"}}");
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*extra.example")));
        QSignalSpy ready(lookup.data(), &Response::ready);
        QTRY_VERIFY_WITH_TIMEOUT(ready.count() == 1, 10000);
        QCOMPARE(lookup->getAccountId(), account);
        QCOMPARE(lookup->json().value(QStringLiteral("display_name")).toString(), QStringLiteral("Bob"));
        QCOMPARE(lookup->json().value(QStringLiteral("limits")).toObject().value(QStringLiteral("daily")).toString(),
                 QStringLiteral("100"));
    }

    /**
     * The federation request set no redirect policy, so it had Qt's default:
     * Qt 6 followed a redirect, and Qt 5.15 stopped at it and the 3xx was
     * refused as an answer. It is
     * followed on both now, as js (fetch) and py (requests) follow it.
     */
    void testAFederationServerRedirectIsFollowed()
    {
        LocalFederation local;
        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        const QString domain = QStringLiteral("moved.example");
        const QByteArray federation = LocalFederationHost::federationPath(domain);
        local.host.serve(domain, account);
        local.host.redirect(federation, "301 Moved Permanently", federation + "/v2");
        local.host.answer(federation + "/v2", "200 OK",
                          "{\"stellar_address\":\"bob*moved.example\",\"account_id\":\"" + account.toLatin1() + "\"}");
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*moved.example")));
        QSignalSpy ready(lookup.data(), &Response::ready);
        QSignalSpy failed(lookup.data(), &Response::error);
        QTRY_VERIFY_WITH_TIMEOUT(ready.count() + failed.count() > 0, 10000);
        QCOMPARE(failed.count(), 0);
        QCOMPARE(lookup->getAccountId(), account);
        QCOMPARE(local.host.asked(federation), 1);
        QCOMPARE(local.host.asked(federation + "/v2"), 1);
    }

    /**
     * A federation answer over 100 KB is refused, as js refuses it, and not
     * read whole: it is stopped while it arrives, with a FederationException
     * (whose header said it was for stellar.toml failures only).
     */
    void testAFederationAnswerOver100KBIsRefused()
    {
        LocalFederation local;
        local.host.serve(QStringLiteral("big.example"), QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY"));
        local.host.answer(LocalFederationHost::federationPath(QStringLiteral("big.example")), "200 OK",
                          "{\"account_id\":\"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\",\"pad\":\""
                              + QByteArray(FederationResponse::MaxSize, 'x') + "\"}");
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*big.example")));
        QSignalSpy failed(lookup.data(), &Response::error);
        QSignalSpy ready(lookup.data(), &Response::ready);
        QTRY_VERIFY_WITH_TIMEOUT(failed.count() > 0, 10000);
        QCOMPARE(failed.count(), 1);
        QCOMPARE(ready.count(), 0);
        QVERIFY(lookup->getAccountId().isEmpty());
        QVERIFY(QString::fromUtf8(lookup->lastException()->what()).contains(QStringLiteral("larger than 100 KB")));
        QVERIFY(dynamic_cast<qstellar::exception::FederationException*>(lookup->lastException()));
    }

    /**
     * A lookup that fails at the domain's stellar.toml says why: the
     * response's lastException() is a FederationException naming the reason
     * (it was a bare error(), "no toml" and "no FEDERATION_SERVER" alike). And
     * an account id given to resolve() emits ready() like any other lookup.
     */
    void testAFailedLookupSaysWhyAndAnAccountIdIsReady()
    {
        LocalFederation local;   // nothing served: every stellar.toml is a 404
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*missing.example")));
        QSignalSpy failed(lookup.data(), &Response::error);
        QTRY_VERIFY_WITH_TIMEOUT(failed.count() > 0, 10000);
        auto* why = dynamic_cast<qstellar::exception::FederationException*>(lookup->lastException());
        QVERIFY(why);
        QVERIFY(QString::fromUtf8(why->what()).contains(QStringLiteral("the stellar.toml could not be read")));

        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        QScopedPointer<FederationResponse> passthrough(Federation::resolve(account));
        QSignalSpy ready(passthrough.data(), &Response::ready);
        QTRY_COMPARE(ready.count(), 1);
        QCOMPARE(passthrough->getAccountId(), account);
    }

#ifdef STELLAR_QT_QML
    /**
     * Seps.resolveFederation says why a lookup failed, as the C++ caller
     * hears it: a stellar.toml that is a 404, one with no FEDERATION_SERVER
     * and a federation server that does not know the name were all one
     * fixed "the lookup failed: ..." text, and the reason the response
     * carried in lastException() was never read.
     */
    void testSepsResolveFederationSaysWhyALookupFailed()
    {
        LocalFederation local;   // what is not answered below is a 404
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("nofed.example")),
                          "200 OK", "VERSION=\"2.0.0\"\n");
        local.host.answer(LocalFederationHost::tomlPath(QStringLiteral("noname.example")),
                          "200 OK", "FEDERATION_SERVER=\"https://fed.noname.example/federation\"\n");
        SepsQml seps;
        QSignalSpy failed(&seps, &SepsQml::failed);
        const QStringList addresses{QStringLiteral("bob*missing.example"),
                                    QStringLiteral("bob*nofed.example"),
                                    QStringLiteral("bob*noname.example")};
        const QStringList reasons{QStringLiteral("the stellar.toml could not be read"),
                                  QStringLiteral("No federation server"),
                                  QStringLiteral("the federation server answered HTTP 404")};
        for (int i = 0; i < addresses.size(); ++i) {
            failed.clear();
            QVERIFY(!seps.resolveFederation(addresses.at(i)).contains(QStringLiteral("error")));
            QTRY_COMPARE_WITH_TIMEOUT(failed.count(), 1, 10000);
            const QVariantMap error = failed.at(0).at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("connection"));
            const QString message = error.value(QStringLiteral("message")).toString();
            QVERIFY2(message.startsWith(QStringLiteral("the lookup failed: ")), qPrintable(message));
            QVERIFY2(message.contains(reasons.at(i)), qPrintable(message));
        }
        QCOMPARE(seps.pending(), 0);
    }
#endif

    /**
     * Federation::resolve(G...) answered with an empty json(), where js hands
     * back the record {account_id}. json() is that record now, an M address's
     * included (js takes a G address only; here an M address is an account
     * id too).
     */
    void testAnAccountIdIsItsOwnRecord()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(key->getAccountId(), 9).toAddress();
        for (const QString& account : {key->getAccountId(), muxed}) {
            QScopedPointer<FederationResponse> own(Federation::resolve(account));
            QSignalSpy ready(own.data(), &Response::ready);
            QTRY_COMPARE(ready.count(), 1);
            QCOMPARE(own->getAccountId(), account);
            QVERIFY(own->getStellarAddress().isEmpty());
            QVERIFY(own->getMemoType().isEmpty());
            QCOMPARE(own->json().keys(), QStringList{QStringLiteral("account_id")});
            QCOMPARE(own->json().value(QStringLiteral("account_id")).toString(), account);
        }
    }

    /**
     * Federation::resolve forwarded the stellar.toml failure to the response
     * a second time (resolveAddress() already forwards it), so every lookup
     * that failed there reported error() twice. It reports it once.
     */
    void testAFailedLookupReportsItsErrorOnce()
    {
        FederationServer::setTomlTimeout(2000);
        auto restore = qScopeGuard([] { FederationServer::setTomlTimeout(0); });
        // Nothing on this machine serves a stellar.toml over https at
        // localhost: the request is refused, fails its TLS handshake or times
        // out, and the lookup fails whichever it is.
        QScopedPointer<FederationResponse> response(Federation::resolve(QStringLiteral("bob*localhost")));
        QSignalSpy failed(response.data(), &Response::error);
        QTRY_VERIFY_WITH_TIMEOUT(failed.count() > 0, 10000);
        QTest::qWait(100);
        QCOMPARE(failed.count(), 1);
        QVERIFY(response->getAccountId().isEmpty());
    }

    /**
     * With the cache on, a server was cached only once its stellar.toml had
     * been read, and in the default build (USE_CPPTOML off) never: every
     * lookup read the toml again and left its FederationServer behind, owned
     * by nobody. Two lookups made before the first toml arrived made two
     * servers, the second replacing the first in the cache. A domain's toml
     * is read once now, by one server that every lookup for it waits on, and
     * the cache deletes that server, once.
     */
    void testTheCacheReadsADomainsTomlOnce()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        LocalFederation local;
        const QString domain = QStringLiteral("cache.example");
        const QByteArray toml = LocalFederationHost::tomlPath(domain);
        local.host.serve(domain, key->getAccountId());
        local.host.hold(toml);
        FederationServer::activeCache(true);

        QScopedPointer<FederationResponse> first(Federation::resolve(QStringLiteral("bob*cache.example")));
        QPointer<FederationServer> server(FederationServer::createForDomain(domain));
        QSignalSpy deleted(server.data(), &QObject::destroyed);
        QTRY_COMPARE(local.host.asked(toml), 1);
        QVERIFY(!server->valid());   // still waiting for its toml
        QScopedPointer<FederationResponse> second(Federation::resolve(QStringLiteral("alice*cache.example")));
        QCOMPARE(FederationServer::createForDomain(domain), server.data());
        local.host.release(toml);
        QTRY_COMPARE(first->getAccountId(), key->getAccountId());
        QTRY_COMPARE(second->getAccountId(), key->getAccountId());

        QScopedPointer<FederationResponse> third(Federation::resolve(QStringLiteral("carol*cache.example")));
        QTRY_COMPARE(third->getAccountId(), key->getAccountId());
        QCOMPARE(local.host.asked(toml), 1);
        QCOMPARE(local.host.asked(LocalFederationHost::federationPath(domain)), 3);
        QCOMPARE(FederationServer::createForDomain(domain), server.data());

        FederationServer::clearCached();
        QTRY_VERIFY(server.isNull());
        QCOMPARE(deleted.count(), 1);
        // Forgotten with it: the next lookup reads the toml again.
        QScopedPointer<FederationResponse> fourth(Federation::resolve(QStringLiteral("dave*cache.example")));
        QTRY_COMPARE(fourth->getAccountId(), key->getAccountId());
        QCOMPARE(local.host.asked(toml), 2);
    }

    /**
     * createForDomain(" trim.example/ ") read trim.example's stellar.toml
     * (the request drops the whitespace and the '/') but kept the domain as
     * given: resolveAddress("bob") asked for "bob* trim.example/", and the
     * cache held it apart from "trim.example", reading the toml twice. The
     * domain is read once now, and that is the server's domain and its key
     * in the cache. (js refuses such a domain in createForDomain, and trims
     * it in StellarToml.Resolver.)
     */
    void testCreateForDomainReadsTheDomainOnce()
    {
        const QString account = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        LocalFederation local;
        const QString domain = QStringLiteral("trim.example");
        const QByteArray federation = LocalFederationHost::federationPath(domain);
        local.host.serve(domain, account);
        local.host.hold(federation);
        FederationServer::activeCache(true);

        QPointer<FederationServer> server(FederationServer::createForDomain(QStringLiteral(" trim.example/ ")));
        QCOMPARE(server->getDomain(), domain);
        QCOMPARE(FederationServer::createForDomain(domain), server.data());
        QCOMPARE(FederationServer::createForDomain(QStringLiteral("trim.example\\")), server.data());
        QScopedPointer<FederationResponse> lookup(server->resolveAddress(QStringLiteral("bob")));
        QTRY_COMPARE(local.host.asked(federation), 1);
        QVERIFY(!local.network.replies.isEmpty() && local.network.replies.last());
        QCOMPARE(local.network.replies.last()->request().url().query(QUrl::FullyEncoded),
                 QStringLiteral("type=name&q=bob%2Atrim.example"));
        local.host.release(federation);
        QTRY_COMPARE(lookup->getAccountId(), account);
        QCOMPARE(local.host.asked(LocalFederationHost::tomlPath(domain)), 1);
    }

    /**
     * With the cache on, a lookup whose stellar.toml failed left its server
     * behind for good, in no cache and owned by nobody. It is deleted once
     * every lookup waiting on it has heard (once each), and nothing is kept:
     * the next lookup asks again.
     */
    void testAFailedTomlLeavesNoServerBehind()
    {
        LocalFederation local;   // nothing served: every stellar.toml is a 404
        const QString domain = QStringLiteral("missing.example");
        const QByteArray toml = LocalFederationHost::tomlPath(domain);
        local.host.hold(toml);
        FederationServer::activeCache(true);

        QPointer<FederationServer> server(FederationServer::createForDomain(domain));
        QSignalSpy deleted(server.data(), &QObject::destroyed);
        QScopedPointer<FederationResponse> direct(server->resolveAddress(QStringLiteral("bob*missing.example")));
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("alice*missing.example")));
        QSignalSpy directFailed(direct.data(), &Response::error);
        QSignalSpy lookupFailed(lookup.data(), &Response::error);
        QTRY_COMPARE(local.host.asked(toml), 1);
        local.host.release(toml);
        QTRY_VERIFY(server.isNull());
        QCOMPARE(deleted.count(), 1);
        QCOMPARE(directFailed.count(), 1);
        QCOMPARE(lookupFailed.count(), 1);

        QPointer<FederationServer> again(FederationServer::createForDomain(domain));
        QTRY_VERIFY(again.isNull());
        QCOMPARE(local.host.asked(toml), 2);
    }

    /**
     * The cache owns what it caches, and a server deleted by somebody else
     * leaves no pointer behind in it: the next lookup gets a new server
     * rather than the freed one.
     */
    void testADeletedServerLeavesNoPointerInTheCache()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        LocalFederation local;
        const QString domain = QStringLiteral("deleted.example");
        local.host.serve(domain, key->getAccountId());
        FederationServer::activeCache(true);

        QPointer<FederationServer> server(FederationServer::createForDomain(domain));
        QSignalSpy ready(server.data(), &FederationServer::ready);
        QTRY_COMPARE(ready.count(), 1);
        delete server.data();
        QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*deleted.example")));
        QTRY_COMPARE(lookup->getAccountId(), key->getAccountId());
        QCOMPARE(local.host.asked(LocalFederationHost::tomlPath(domain)), 2);
    }

    /**
     * clearCached() and activeCache(false) delete the servers they drop. One
     * still reading its stellar.toml has lookups waiting on it, which were
     * then never answered (and before its toml arrived it was not in the
     * cache at all, but leaked). It is dropped at once, so the next lookup
     * reads the toml again, answers the lookups it has, and then deletes
     * itself, once.
     */
    void testDroppingTheCacheWhileLookupsArePending()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        LocalFederation local;
        const QString domain = QStringLiteral("pending.example");
        const QByteArray toml = LocalFederationHost::tomlPath(domain);
        local.host.serve(domain, key->getAccountId());
        for (const bool turnedOff : {false, true}) {
            const int asked = local.host.asked(toml);
            local.host.hold(toml);
            FederationServer::activeCache(true);
            QPointer<FederationServer> server(FederationServer::createForDomain(domain));
            QSignalSpy deleted(server.data(), &QObject::destroyed);
            QScopedPointer<FederationResponse> lookup(Federation::resolve(QStringLiteral("bob*pending.example")));
            QScopedPointer<FederationResponse> direct(server->resolveAddress(QStringLiteral("eve*pending.example")));
            QTRY_COMPARE(local.host.asked(toml), asked + 1);

            if (turnedOff) {
                FederationServer::activeCache(false);
            } else {
                FederationServer::clearCached();
                // Not the dropped one: a server of its own, reading the toml.
                QPointer<FederationServer> next(FederationServer::createForDomain(domain));
                QVERIFY(next.data() != server.data());
                QTRY_COMPARE(local.host.asked(toml), asked + 2);
            }
            QTest::qWait(50);
            QVERIFY(!server.isNull());
            local.host.release(toml);
            QTRY_COMPARE(lookup->getAccountId(), key->getAccountId());
            QTRY_COMPARE(direct->getAccountId(), key->getAccountId());
            QTRY_VERIFY(server.isNull());
            QCOMPARE(deleted.count(), 1);
            FederationServer::activeCache(false);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }
    }

    /**
     * Without the cache a lookup's server is its response's, and goes with
     * it. A response deleted before the stellar.toml arrived took its server
     * along and left the toml request running, and then in the network
     * manager for good.
     */
    void testALookupDeletedEarlyTakesItsRequestWithIt()
    {
        LocalFederation local;
        const QString domain = QStringLiteral("early.example");
        const QByteArray toml = LocalFederationHost::tomlPath(domain);
        local.host.hold(toml);
        QVERIFY(!FederationServer::isCacheActive());

        FederationResponse* lookup = Federation::resolve(QStringLiteral("bob*early.example"));
        QPointer<FederationServer> server(lookup->findChild<FederationServer*>());
        QVERIFY(!server.isNull());
        QTRY_COMPARE(local.host.asked(toml), 1);
        QCOMPARE(local.network.replies.size(), 1);
        const QPointer<QNetworkReply> request = local.network.replies.first();
        delete lookup;
        QVERIFY(server.isNull());
        QTRY_VERIFY(request.isNull());
    }

    /**
     * resolveAddress() on a server still reading its stellar.toml took any
     * address and built the request when the toml arrived: a malformed one
     * threw from inside ready(), cutting that signal short for the other
     * lookups waiting on the same server (with the cache on, every lookup
     * for the domain). It is refused at once, as a ready server refuses it.
     * A name alone is not malformed: it is the name at the server's domain.
     */
    void testAMalformedAddressIsRefusedBeforeTheTomlArrives()
    {
        QScopedPointer<KeyPair> key(KeyPair::random());
        LocalFederation local;
        const QString domain = QStringLiteral("strict.example");
        const QByteArray toml = LocalFederationHost::tomlPath(domain);
        local.host.serve(domain, key->getAccountId());
        local.host.hold(toml);
        FederationServer::activeCache(true);

        QPointer<FederationServer> server(FederationServer::createForDomain(domain));
        QScopedPointer<FederationResponse> good(server->resolveAddress(QStringLiteral("bob*strict.example")));
        QScopedPointer<FederationResponse> bare(server->resolveAddress(QStringLiteral("carol")));
        for (const QString& address : {QString(), QStringLiteral("bob*strict.example*x"), QStringLiteral("carol**")}) {
            bool refused = false;
            try {
                delete server->resolveAddress(address);
            } catch (const std::runtime_error&) {
                refused = true;
            }
            QVERIFY2(refused, qPrintable(address));
        }
        local.host.release(toml);
        QTRY_COMPARE(good->getAccountId(), key->getAccountId());
        QTRY_COMPARE(bare->getAccountId(), key->getAccountId());
    }

    /**
     * resolveAddress("bob") threw even on a server that knows its domain;
     * js sends it as bob*<domain> (createForDomain("acme.com"), then
     * resolveAddress("bob")), and so does this server now. A server with no
     * domain refuses a name alone, saying why, as js does.
     */
    void testANameAloneIsLookedUpAtTheServersDomain()
    {
        class Recorder : public QNetworkAccessManager
        {
        public:
            QList<QUrl> urls;
        protected:
            QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override
            {
                urls.append(request.url());
                return new FakeAnswerReply(QByteArray("{}"));
            }
        };
        Recorder recorder;
        QNetworkAccessManager* previous = FederationServer::setNetworkAccessManager(&recorder);
        auto restore = qScopeGuard([previous] { FederationServer::setNetworkAccessManager(previous); });
        FederationServer server(QUrl(QStringLiteral("https://fed.example.com/federation")),
                                QStringLiteral("example.com"));
        delete server.resolveAddress(QStringLiteral("bob"));
        delete server.resolveAddress(QStringLiteral("bob*other.example"));
        QCOMPARE(recorder.urls.size(), 2);
        QCOMPARE(recorder.urls.at(0).query(QUrl::FullyEncoded), QStringLiteral("type=name&q=bob%2Aexample.com"));
        QCOMPARE(recorder.urls.at(1).query(QUrl::FullyEncoded), QStringLiteral("type=name&q=bob%2Aother.example"));

        FederationServer anonymous(QUrl(QStringLiteral("https://fed.example.com/federation")), QString());
        QVERIFY_EXCEPTION_THROWN(delete anonymous.resolveAddress(QStringLiteral("bob")), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(delete server.resolveAddress(QString()), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(delete server.resolveAddress(QStringLiteral("bob*example.com*x")), std::runtime_error);
        QCOMPARE(recorder.urls.size(), 2);
    }

};
// The live lookup is behind STELLAR_SKIP_LIVE_TESTS, so as not to spam
// funtracker.site; the rest is offline and always runs.
ADD_TEST(FederationTest)
#endif // FEDERATIONTEST_H



