#ifndef STELLARQMLTEST_H
#define STELLARQMLTEST_H

#include <QObject>
#include <QtTest>
#include <QScopeGuard>
#include "../testcollector.h"

#include "../../src/qml/stellarqml.h"
#include "../../src/strkey.h"
#include "../../src/qml/assetregistry.h"
#include "../../src/qml/txbuilderqml.h"
#include "../../src/qml/xdrqml.h"
#include "../../src/qml/horizonqml.h"
#include "../../src/asset.h"
#include "../../src/scval/scv.h"
#include "../../src/account.h"
#include "../../src/assettypenative.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/paymentoperation.h"
#include "../../src/server.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../xdr/stellartransaction.h"
#include "../fakeserver.h"
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QNetworkCookieJar>
#include <QPointer>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTimer>
#include <QUrlQuery>
#include <atomic>
#include <type_traits>
#include "../../src/memo.h"
#include "../../src/responses/response.h"

/**
 * A Horizon that reads every request whole before it does anything with it,
 * and counts what reached it: GET / (its root, which names the network it
 * serves) answered with the root set (the real testnet one unless told
 * otherwise), any other GET (/accounts/{id}: a SEP-29 lookup, or a
 * loadAccount) answered with the account set, POST /transactions answered
 * with the answer set for it — or any of them held (read whole, not
 * answered) while its hold is on, until answerHeld(). It keeps the `tx` of
 * every POST it read whole, so a test knows a transaction reached it, and
 * every request line in the order read. The counts, envelopes and requests
 * can be read, the holds and answers set, from any thread; answerHeld()
 * runs on the endpoint's own.
 *
 * `dropPosts` closes the connection on a POST once it is read whole, with no
 * answer (a connection lost midway): aborted, or closed in good order with
 * `closeGracefully`; with `refuseAfterDrop` it stops listening first, as a
 * Horizon that went away, so the connection Qt 5 opens to send the POST
 * again is refused. `dropFirstPosts` drops only that many POSTs, the first
 * ones read, the same way, and treats the rest as ever: Horizon read the
 * first whole and the connection closed on it, and Qt 5, sending it again,
 * gets an answer. `refuseAfterRoot` answers the root with
 * "Connection: close" and then stops listening, so the next connection, the
 * POST's, is refused.
 */
class HeldHorizon
{
public:
    enum Kind { Root, Lookup, Post };

    HeldHorizon()
    {
        m_root = testnetRoot();
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead, [this, socket]() { read(socket); });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
        m_port = m_server.serverPort();
    }

    /** Horizon's root on testnet, as horizon-testnet.stellar.org sent it. */
    static QByteArray testnetRoot()
    {
        return QByteArray(
            "{\n"
            "  \"_links\": {\n"
            "    \"account\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/{account_id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"accounts\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts{?signer,sponsor,asset,liquidity_pool,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"account_transactions\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/{account_id}/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"claimable_balances\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/claimable_balances{?asset,sponsor,claimant,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"assets\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/assets{?asset_code,asset_issuer,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"effects\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/effects{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"fee_stats\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/fee_stats\"\n"
            "    },\n"
            "    \"friendbot\": {\n"
            "      \"href\": \"https://friendbot.stellar.org/{?addr}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"ledger\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/ledgers/{sequence}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"ledgers\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/ledgers{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"liquidity_pools\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/liquidity_pools{?reserves,account,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"offer\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/offers/{offer_id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"offers\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/offers{?selling,buying,seller,sponsor,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operation\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/operations/{id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operations\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/operations{?cursor,limit,order,include_failed}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"order_book\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/order_book{?selling_asset_type,selling_asset_code,selling_asset_issuer,buying_asset_type,buying_asset_code,buying_asset_issuer,limit}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"payments\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/payments{?cursor,limit,order,include_failed}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/\"\n"
            "    },\n"
            "    \"strict_receive_paths\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/paths/strict-receive{?source_assets,source_account,destination_account,destination_asset_type,destination_asset_issuer,destination_asset_code,destination_amount}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"strict_send_paths\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/paths/strict-send{?destination_account,destination_assets,source_asset_type,source_asset_issuer,source_asset_code,source_amount}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"trade_aggregations\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/trade_aggregations?base_asset_type={base_asset_type}\\u0026base_asset_code={base_asset_code}\\u0026base_asset_issuer={base_asset_issuer}\\u0026counter_asset_type={counter_asset_type}\\u0026counter_asset_code={counter_asset_code}\\u0026counter_asset_issuer={counter_asset_issuer}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"trades\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/trades?base_asset_type={base_asset_type}\\u0026base_asset_code={base_asset_code}\\u0026base_asset_issuer={base_asset_issuer}\\u0026counter_asset_type={counter_asset_type}\\u0026counter_asset_code={counter_asset_code}\\u0026counter_asset_issuer={counter_asset_issuer}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"transaction\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/transactions/{hash}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"transactions\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    }\n"
            "  },\n"
            "  \"horizon_version\": \"29.0.0-c9e48b645f0777f19135109d3de5c46160cea737\",\n"
            "  \"core_version\": \"stellar-core 29.0.0 (4eb83337380a29ad0907f4e32196ce97b8dc7649)\",\n"
            "  \"ingest_latest_ledger\": 4941086,\n"
            "  \"history_latest_ledger\": 4941086,\n"
            "  \"history_latest_ledger_closed_at\": \"2026-09-30T00:43:37Z\",\n"
            "  \"history_elder_ledger\": 128,\n"
            "  \"core_latest_ledger\": 4941086,\n"
            "  \"network_passphrase\": \"Test SDF Network ; September 2015\",\n"
            "  \"current_protocol_version\": 29,\n"
            "  \"supported_protocol_version\": 29,\n"
            "  \"core_supported_protocol_version\": 29\n"
            "}");
    }

    /** Horizon's root on the public network, as horizon.stellar.org sent it. */
    static QByteArray pubnetRoot()
    {
        return QByteArray(
            "{\n"
            "  \"_links\": {\n"
            "    \"account\": {\n"
            "      \"href\": \"https://horizon.stellar.org/accounts/{account_id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"accounts\": {\n"
            "      \"href\": \"https://horizon.stellar.org/accounts{?signer,sponsor,asset,liquidity_pool,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"account_transactions\": {\n"
            "      \"href\": \"https://horizon.stellar.org/accounts/{account_id}/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"claimable_balances\": {\n"
            "      \"href\": \"https://horizon.stellar.org/claimable_balances{?asset,sponsor,claimant,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"assets\": {\n"
            "      \"href\": \"https://horizon.stellar.org/assets{?asset_code,asset_issuer,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"effects\": {\n"
            "      \"href\": \"https://horizon.stellar.org/effects{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"fee_stats\": {\n"
            "      \"href\": \"https://horizon.stellar.org/fee_stats\"\n"
            "    },\n"
            "    \"ledger\": {\n"
            "      \"href\": \"https://horizon.stellar.org/ledgers/{sequence}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"ledgers\": {\n"
            "      \"href\": \"https://horizon.stellar.org/ledgers{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"liquidity_pools\": {\n"
            "      \"href\": \"https://horizon.stellar.org/liquidity_pools{?reserves,account,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"offer\": {\n"
            "      \"href\": \"https://horizon.stellar.org/offers/{offer_id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"offers\": {\n"
            "      \"href\": \"https://horizon.stellar.org/offers{?selling,buying,seller,sponsor,cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operation\": {\n"
            "      \"href\": \"https://horizon.stellar.org/operations/{id}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operations\": {\n"
            "      \"href\": \"https://horizon.stellar.org/operations{?cursor,limit,order,include_failed}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"order_book\": {\n"
            "      \"href\": \"https://horizon.stellar.org/order_book{?selling_asset_type,selling_asset_code,selling_asset_issuer,buying_asset_type,buying_asset_code,buying_asset_issuer,limit}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"payments\": {\n"
            "      \"href\": \"https://horizon.stellar.org/payments{?cursor,limit,order,include_failed}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"https://horizon.stellar.org/\"\n"
            "    },\n"
            "    \"strict_receive_paths\": {\n"
            "      \"href\": \"https://horizon.stellar.org/paths/strict-receive{?source_assets,source_account,destination_account,destination_asset_type,destination_asset_issuer,destination_asset_code,destination_amount}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"strict_send_paths\": {\n"
            "      \"href\": \"https://horizon.stellar.org/paths/strict-send{?destination_account,destination_assets,source_asset_type,source_asset_issuer,source_asset_code,source_amount}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"trade_aggregations\": {\n"
            "      \"href\": \"https://horizon.stellar.org/trade_aggregations?base_asset_type={base_asset_type}\\u0026base_asset_code={base_asset_code}\\u0026base_asset_issuer={base_asset_issuer}\\u0026counter_asset_type={counter_asset_type}\\u0026counter_asset_code={counter_asset_code}\\u0026counter_asset_issuer={counter_asset_issuer}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"trades\": {\n"
            "      \"href\": \"https://horizon.stellar.org/trades?base_asset_type={base_asset_type}\\u0026base_asset_code={base_asset_code}\\u0026base_asset_issuer={base_asset_issuer}\\u0026counter_asset_type={counter_asset_type}\\u0026counter_asset_code={counter_asset_code}\\u0026counter_asset_issuer={counter_asset_issuer}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"transaction\": {\n"
            "      \"href\": \"https://horizon.stellar.org/transactions/{hash}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"transactions\": {\n"
            "      \"href\": \"https://horizon.stellar.org/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    }\n"
            "  },\n"
            "  \"horizon_version\": \"29.0.0-c9e48b645f0777f19135109d3de5c46160cea737\",\n"
            "  \"core_version\": \"stellar-core 29.0.0 (4eb83337380a29ad0907f4e32196ce97b8dc7649)\",\n"
            "  \"ingest_latest_ledger\": 64688647,\n"
            "  \"history_latest_ledger\": 64688647,\n"
            "  \"history_latest_ledger_closed_at\": \"2026-09-30T00:43:37Z\",\n"
            "  \"history_elder_ledger\": 58381201,\n"
            "  \"core_latest_ledger\": 64688647,\n"
            "  \"network_passphrase\": \"Public Global Stellar Network ; September 2015\",\n"
            "  \"current_protocol_version\": 28,\n"
            "  \"supported_protocol_version\": 29,\n"
            "  \"core_supported_protocol_version\": 29\n"
            "}");
    }

    /** No trailing slash: the SDK appends each path to it. */
    QString url() const { return QStringLiteral("http://127.0.0.1:%1").arg(m_port); }

    std::atomic<bool> holdRoots{false};
    std::atomic<bool> holdLookups{false};
    std::atomic<bool> holdPosts{false};
    std::atomic<bool> dropPosts{false};
    std::atomic<bool> refuseAfterDrop{false};
    std::atomic<bool> closeGracefully{false};
    std::atomic<bool> refuseAfterRoot{false};
    std::atomic<int> dropFirstPosts{0};

    void setAccount(const QByteArray& body) { QMutexLocker lock(&m_mutex); m_account = body; }
    void setApplied(const QByteArray& body) { setPostReply("200 OK", "application/hal+json; charset=utf-8", body); }
    /** What a POST is answered with: its status line, Content-Type and body. */
    void setPostReply(const QByteArray& status, const QByteArray& contentType, const QByteArray& body)
    {
        QMutexLocker lock(&m_mutex);
        m_postStatus = status;
        m_postType = contentType;
        m_applied = body;
    }
    /** What the root is answered with (the real testnet root, 200, by default). */
    void setRootReply(const QByteArray& status, const QByteArray& body)
    {
        QMutexLocker lock(&m_mutex);
        m_rootStatus = status;
        m_root = body;
    }
    void setRoot(const QByteArray& body) { setRootReply("200 OK", body); }
    /** Header lines ("Name: value\r\n" each) added to every answer. */
    void setExtraHeaders(const QByteArray& lines) { QMutexLocker lock(&m_mutex); m_extraHeaders = lines; }

    int roots() const { return m_roots.loadAcquire(); }
    int lookups() const { return m_lookups.loadAcquire(); }
    int posts() const { return m_posts.loadAcquire(); }
    QStringList posted() const { QMutexLocker lock(&m_mutex); return m_posted; }
    /** "METHOD path" of every request read whole, in order. */
    QStringList requests() const { QMutexLocker lock(&m_mutex); return m_requests; }

    /** Answers every request held so far. */
    void answerHeld()
    {
        const QList<QPair<QPointer<QTcpSocket>, Kind>> held = m_held;
        m_held.clear();
        for (const auto& request : held)
            respond(request.first, request.second);
    }

private:
    void read(QTcpSocket* socket)
    {
        QByteArray& buffer = m_buffers[socket];
        buffer += socket->readAll();
        for (;;) {
            const int end = int(buffer.indexOf("\r\n\r\n"));
            if (end < 0)
                return;
            const QList<QByteArray> head = buffer.left(end).split('\n');
            int length = 0;
            for (const QByteArray& line : head) {
                if (line.toLower().startsWith("content-length:"))
                    length = line.mid(15).trimmed().toInt();
            }
            if (buffer.size() < end + 4 + length)
                return;
            const QList<QByteArray> requestLine = head.value(0).trimmed().split(' ');
            const QByteArray method = requestLine.value(0);
            const QByteArray path = requestLine.value(1);
            const QByteArray body = buffer.mid(end + 4, length);
            buffer.remove(0, end + 4 + length);
            const Kind kind = method == "POST" && path == "/transactions" ? Post
                              : method == "GET" && path == "/"         ? Root
                                                                       : Lookup;
            {
                QMutexLocker lock(&m_mutex);
                m_requests.append(QString::fromLatin1(method + ' ' + path));
            }
            if (kind == Post) {
                const QString tx = QUrlQuery(QString::fromLatin1(body))
                                       .queryItemValue(QStringLiteral("tx"), QUrl::FullyDecoded);
                {
                    QMutexLocker lock(&m_mutex);
                    m_posted.append(tx);
                }
                m_posts.fetchAndAddOrdered(1);
                bool drop = dropPosts.load();
                if (!drop && dropFirstPosts.load() > 0) {
                    --dropFirstPosts;
                    drop = true;
                }
                if (drop) {
                    if (refuseAfterDrop.load())
                        m_server.close();   // gone: a POST sent again is refused
                    if (closeGracefully.load())
                        socket->disconnectFromHost();
                    else
                        socket->abort();    // read whole, never answered
                    return;
                }
            } else if (kind == Root) {
                m_roots.fetchAndAddOrdered(1);
            } else {
                m_lookups.fetchAndAddOrdered(1);
            }
            const bool hold = kind == Post ? holdPosts.load()
                              : kind == Root ? holdRoots.load() : holdLookups.load();
            if (hold)
                m_held.append(qMakePair(QPointer<QTcpSocket>(socket), kind));
            else
                respond(socket, kind);
        }
    }

    void respond(const QPointer<QTcpSocket>& socket, Kind kind)
    {
        if (!socket || socket->state() != QAbstractSocket::ConnectedState)
            return;
        QByteArray status = "200 OK";
        QByteArray type = "application/hal+json; charset=utf-8";
        QByteArray body;
        QByteArray extra;
        {
            QMutexLocker lock(&m_mutex);
            extra = m_extraHeaders;
            if (kind == Post) {
                status = m_postStatus;
                type = m_postType;
                body = m_applied;
            } else if (kind == Root) {
                status = m_rootStatus;
                body = m_root;
            } else {
                body = m_account;
            }
        }
        const bool closing = kind == Root && refuseAfterRoot.load();
        socket->write("HTTP/1.1 " + status + "\r\nContent-Type: " + type + "\r\n" + extra
                      + (closing ? QByteArray("Connection: close\r\n") : QByteArray())
                      + "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body);
        socket->flush();
        if (closing) {
            m_server.close();               // the next connection is refused
            socket->disconnectFromHost();
        }
    }

    QTcpServer m_server;
    quint16 m_port = 0;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QList<QPair<QPointer<QTcpSocket>, Kind>> m_held;
    mutable QMutex m_mutex;
    QByteArray m_account;
    QByteArray m_applied;
    QByteArray m_postStatus = "200 OK";
    QByteArray m_postType = "application/hal+json; charset=utf-8";
    QByteArray m_root;
    QByteArray m_rootStatus = "200 OK";
    QByteArray m_extraHeaders;
    QStringList m_posted;
    QStringList m_requests;
    QAtomicInt m_roots;
    QAtomicInt m_lookups;
    QAtomicInt m_posts;
};

/**
 * The optional QML layer (STELLAR_QT_QML).
 *
 * Its whole contract is: nothing throws into QML, and no SDK pointer crosses
 * the boundary. Both are easy to break by accident later, so they are pinned
 * here rather than left to the app to discover at runtime.
 */
class StellarQmlTest: public QObject
{
    Q_OBJECT

    static const char* ACCOUNT() { return "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"; }
    static const char* SECRET()  { return "SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"; }
    static const char* ISSUER()  { return "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"; }

    /**
     * A whole Horizon account for `id`, holding the data entries `data` (a
     * JSON object), its one signer weighing `weight`.
     */
    static QString accountJson(const QString& id, const QString& data, const QString& weight = QStringLiteral("1"))
    {
        return QStringLiteral("{\"_links\":{\"self\":{\"href\":\"/accounts/%1\"},\"transactions\":{\"href\":\"/accounts/%1/transactions\"},"
                              "\"operations\":{\"href\":\"/accounts/%1/operations\"},\"effects\":{\"href\":\"/accounts/%1/effects\"},"
                              "\"offers\":{\"href\":\"/accounts/%1/offers\"}},"
                              "\"id\":\"%1\",\"account_id\":\"%1\",\"sequence\":\"5\",\"subentry_count\":1,"
                              "\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},"
                              "\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,"
                              "\"auth_clawback_enabled\":false},"
                              "\"balances\":[{\"balance\":\"100.0000000\",\"buying_liabilities\":\"0.0000000\","
                              "\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],"
                              "\"signers\":[{\"weight\":%3,\"key\":\"%1\",\"type\":\"ed25519_public_key\"}],"
                              "\"data\":%2,\"num_sponsoring\":0,\"num_sponsored\":0,\"paging_token\":\"%1\"}").arg(id, data, weight);
    }

    /** A funded friendbot's answer, as friendbot sends it today (no ledger). */
    static QString friendbotAnswer()
    {
        return QStringLiteral("{\n  \"successful\": true,\n"
                              "  \"hash\": \"c737c3f100abdee0a3721dd4f93f14bc16860cebc3b5bb76874789114b85606b\",\n"
                              "  \"envelope_xdr\": \"AAAAAgAAAAC0omeVy1ifudxFDQs6POcQGsw37D2nt0OeSW+O+cILygAPQkAANsVjAAAB0AAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAABAAAAABB90WssODNIgi6BHveqzxTRmIpvAFRyVNM+Hm2GVuCcAAAAAAAAAADU9D4zZtDUB80GLxe2rhi8o/agogmQZYNZQSOdxrszeQAAABdIdugAAAAAAAAAAAL5wgvKAAAAQD5a7YiCza9jT+asFT67JAqKAdc2wsDi28QEDQmGWMlyFHXLzUX57Gn90U9dr8tL1bkyA8bmXK1Wwrnu6LC16ASGVuCcAAAAQCeYp26SNTOQYq5+xFRDvKLdGuVbmgbVrFgetUNL4pL0Di62wg/mSX7wLHRaD7ASkBJsCKGF6qwfYGyzS2dgrw4=\"\n}");
    }

    /**
     * What fundAccount() emits for friendbot answering `code` with `body`
     * (or not answering, with `hang`): the funded result, or the failure.
     */
    static QVariantMap fundWith(const QString& code, const QString& body, bool hang, bool* funded)
    {
        const QString account = QStringLiteral("GDKPIPRTM3INIB6NAYXRPNVODC6KH5VAUIEZAZMDLFASHHOGXMZXTGR2");
        FakeServer* fake = new FakeServer();
        if (hang) {
            fake->addHang(QStringLiteral("/"));
        } else {
            FakeServer::Reply reply;
            reply.body = body;
            reply.responseCode = code;
            reply.contentType = code.startsWith(QLatin1String("200"))
                    ? QStringLiteral("application/hal+json; charset=utf-8")
                    : QStringLiteral("application/problem+json; charset=utf-8");
            fake->addGetFull(QStringLiteral("/?addr=") + account, reply);
        }
        HorizonQml horizon;
        // fundAccount() asks friendbot only for a test network; nothing is
        // asked of this Horizon.
        horizon.setEndpoint(QStringLiteral("https://horizon-testnet.stellar.org"));
        horizon.setFriendbotUrl(fake->baseUrl() + QStringLiteral("/"));
        QVariantMap out;
        int answers = 0;
        *funded = false;
        QObject::connect(&horizon, &HorizonQml::funded, [&](int, const QVariantMap& result) {
            ++answers; *funded = true; out = result;
        });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int, const QVariantMap& error) {
            ++answers; out = error;
        });
        horizon.fundAccount(account);
        WAIT_FOR(answers == 0)
        QTest::qWait(20);
        out.insert(QStringLiteral("_answers"), answers);
        out.insert(QStringLiteral("_pending"), horizon.pending());
        fake->deleteLater();
        return out;
    }

    /** One { type, value } contract argument. */
    static QVariantMap arg(const QString& type, const QVariant& value)
    {
        QVariantMap a;
        a.insert(QStringLiteral("type"), type);
        a.insert(QStringLiteral("value"), value);
        return a;
    }

    /** A transaction spec invoking `functionName` with `args` (a list or JSON text). */
    static QVariantMap invokeSpec(const QVariant& args, const QString& functionName = QStringLiteral("f"))
    {
        QVariantMap op;
        op.insert(QStringLiteral("type"), QStringLiteral("invokeContract"));
        op.insert(QStringLiteral("contractId"),
                  QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        op.insert(QStringLiteral("functionName"), functionName);
        op.insert(QStringLiteral("args"), args);
        QVariantMap spec;
        spec.insert(QStringLiteral("sourceAccount"), QString::fromLatin1(ACCOUNT()));
        spec.insert(QStringLiteral("sequence"), QStringLiteral("5"));
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        return spec;
    }

    /** The arguments of the one invokeContract a built envelope carries. */
    static QList<stellar::SCVal> invokedArgs(const QVariantMap& built)
    {
        const QByteArray raw =
            QByteArray::fromBase64(built.value(QStringLiteral("xdr")).toString().toLatin1());
        QDataStream in(raw);
        stellar::TransactionEnvelope envelope;
        in >> envelope;
        return envelope.v1.tx.operations.value.at(0)
            .operationInvokeHostFunction.hostFunction.invokeContract.args;
    }

    static QString keyText(const stellar::SCVal& key)
    {
        switch (key.type) {
        case stellar::SCValType::SCV_SYMBOL: return Scv::fromSymbol(key);
        case stellar::SCValType::SCV_STRING: return Scv::fromString(key);
        case stellar::SCValType::SCV_I32:    return QString::number(Scv::fromInt32(key));
        case stellar::SCValType::SCV_I64:    return QString::number(Scv::fromInt64(key));
        case stellar::SCValType::SCV_BYTES:  return QString::fromLatin1(Scv::fromBytes(key).toHex());
        default:                             return QStringLiteral("?");
        }
    }

    /**
     * A signed testnet (or `network`) payment of 10 XLM from SECRET()'s
     * account to `destination` at `sequence`, with a text memo — so no
     * SEP-29 lookup is made, and it is posted as soon as the endpoint's
     * network is known — or none; its hash on that network in `hash`.
     */
    static QString paymentTo(const QString& destination, bool withMemo, qint64 sequence, QString* hash,
                             Network* network = Network::testnetNetwork())
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString::fromLatin1(SECRET())));
        Account source(signer->getAccountId(), sequence);
        TransactionBuilder builder(&source, network);
        builder.addOperation(new PaymentOperation(destination, new AssetTypeNative(), QStringLiteral("10")))
               .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
               .setBaseFee(100);
        if (withMemo)
            builder.addMemo(Memo::text(QStringLiteral("invoice 7")));
        QScopedPointer<Transaction> tx(builder.build());
        tx->sign(signer.data());
        if (hash)
            *hash = tx->hashHex().toLower();
        return tx->toEnvelopeXdrBase64();
    }

    /**
     * A failure of a submit that says its transaction was not sent:
     * mayHaveReachedServer false, and no hash to look up.
     */
    static bool saysNotSent(const QVariantMap& error)
    {
        return error.value(QStringLiteral("mayHaveReachedServer")) == QVariant(false)
               && !error.contains(QStringLiteral("hash"));
    }

    /**
     * A failure of a submit that says its transaction may have reached
     * Horizon, and names `hash` to look it up by.
     */
    static bool saysMayHaveLanded(const QVariantMap& error, const QString& hash)
    {
        return error.value(QStringLiteral("mayHaveReachedServer")) == QVariant(true)
               && error.value(QStringLiteral("hash")).toString() == hash;
    }

    /**
     * What Horizon answers a POST /transactions the network refused for its
     * sequence number (400, a problem document with the result codes).
     */
    static QByteArray badSeqAnswer(const QString& envelope)
    {
        return QStringLiteral(
            "{\n  \"type\": \"https://stellar.org/horizon-errors/transaction_failed\",\n"
            "  \"title\": \"Transaction Failed\",\n  \"status\": 400,\n"
            "  \"detail\": \"The transaction failed when submitted to the stellar network. The "
            "`extras.result_codes` field on this response contains further details.  Descriptions "
            "of each code can be found at: https://developers.stellar.org/docs/data/apis/horizon/"
            "api-reference/errors/http-status-codes/horizon-specific/transaction-failed\",\n"
            "  \"extras\": {\n    \"envelope_xdr\": \"%1\",\n"
            "    \"result_codes\": {\n      \"transaction\": \"tx_bad_seq\"\n    },\n"
            "    \"result_xdr\": \"AAAAAAAAAGT////7AAAAAA==\"\n  }\n}").arg(envelope).toUtf8();
    }

    /** What Horizon answers a POST /transactions it applied, as it sends it. */
    static QByteArray appliedAnswer(const QString& hash, const QString& envelope)
    {
        QJsonObject answer;
        answer.insert(QStringLiteral("hash"), hash);
        answer.insert(QStringLiteral("ledger"), 4917043);
        answer.insert(QStringLiteral("envelope_xdr"), envelope);
        answer.insert(QStringLiteral("result_xdr"), QStringLiteral("AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAABAAAAAAAAAAA="));
        answer.insert(QStringLiteral("successful"), true);
        return QJsonDocument(answer).toJson(QJsonDocument::Compact);
    }

    /** A HeldHorizon on a thread of its own, created and deleted there. */
    struct HorizonThread
    {
        QThread thread;
        QObject* context = nullptr;
        HeldHorizon* horizon = nullptr;

        HorizonThread()
        {
            context = new QObject;
            context->moveToThread(&thread);
            QObject::connect(&thread, &QThread::finished, context, &QObject::deleteLater);
            thread.start();
            QMetaObject::invokeMethod(context, [this]() { horizon = new HeldHorizon; },
                                      Qt::BlockingQueuedConnection);
        }
        ~HorizonThread()
        {
            // Told to quit from its own thread, inside its event loop: quit()
            // from this one could reach a loop just going idle and not wake
            // it (Qt 5.15 on Windows, about one run in 150: wait() never
            // returned, with the endpoint's thread idle in its loop).
            QMetaObject::invokeMethod(context, [this]() {
                delete horizon;
                QThread::currentThread()->quit();
            }, Qt::BlockingQueuedConnection);
            thread.wait();
        }
        /** Has the endpoint answer what it holds, on its thread; returns at once. */
        void answerHeld()
        {
            HeldHorizon* held = horizon;
            QMetaObject::invokeMethod(context, [held]() { held->answerHeld(); }, Qt::QueuedConnection);
        }
    };

    static QString base64Of(const QByteArray& raw) { return QString::fromLatin1(raw.toBase64()); }

    static QByteArray xdrOf(const stellar::SCVal& v)
    {
        QByteArray raw;
        QDataStream s(&raw, QIODevice::WriteOnly);
        s << v;
        return raw;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── The contract: hostile input never throws ────────────────────

    /** Every invokable must survive garbage, because QML has no way to catch
     *  a C++ exception and the engine's behaviour if one escapes is undefined.
     *  This is the single most important test in the file. */
    void testNothingThrowsOnGarbageInput()
    {
        StellarQml s;
        const QStringList garbage = {
            QString(), QStringLiteral(""), QStringLiteral("   "),
            QStringLiteral("not an address"), QStringLiteral("GARBAGE"),
            QStringLiteral("G"), QString(600, QChar('X')),
            QStringLiteral("1,000.5"), QStringLiteral("-0"),
            QStringLiteral("../../etc/passwd"), QStringLiteral("%00%01"),
            QString::fromUtf8("\xF0\x9F\x92\xA9")
        };
        for (const QString& g : garbage) {
            s.isValidAccountId(g);
            s.isValidSecretSeed(g);
            s.isValidMuxedAccount(g);
            s.isValidContractId(g);
            s.isValidClaimableBalanceId(g);
            s.isValidLiquidityPoolId(g);
            s.strKeyKind(g);
            s.validateAmount(g);
            s.amountToStroops(g);
            s.parsePrice(g);
            s.isValidAsset(g);
            s.describeAsset(g);
            s.validateMnemonic(g);
            s.keyPairFromMnemonic(g);
            s.accountIdFromSecretSeed(g);
            s.networkPassphrase(g);
        }
        QVERIFY(true);   // reaching here without an exception IS the assertion
    }

    // ─── Keys ────────────────────────────────────────────────────────

    void testGenerateKeyPairRoundTrips()
    {
        StellarQml s;
        const QVariantMap kp = s.generateKeyPair();
        QVERIFY(!kp.contains(QStringLiteral("error")));
        const QString account = kp.value(QStringLiteral("accountId")).toString();
        const QString secret  = kp.value(QStringLiteral("secretSeed")).toString();
        QVERIFY(s.isValidAccountId(account));
        QVERIFY(s.isValidSecretSeed(secret));
        QCOMPARE(s.accountIdFromSecretSeed(secret), account);
    }

    void testAccountIdFromBadSeedIsEmptyNotAThrow()
    {
        StellarQml s;
        QVERIFY(s.accountIdFromSecretSeed(QStringLiteral("nope")).isEmpty());
    }

    // ─── strkey ──────────────────────────────────────────────────────

    void testStrKeyKindDiscriminates()
    {
        StellarQml s;
        QCOMPARE(s.strKeyKind(QString(ACCOUNT())), QStringLiteral("account"));
        QCOMPARE(s.strKeyKind(QString(SECRET())),  QStringLiteral("secret"));
        QCOMPARE(s.strKeyKind(QStringLiteral("nonsense")), QString());
        // The three signer-key forms are strkeys too (they read as none).
        const QByteArray hash(32, '\x5a');
        QCOMPARE(s.strKeyKind(StrKey::encodePreAuthTx(hash)), QStringLiteral("preAuthTx"));
        QCOMPARE(s.strKeyKind(StrKey::encodeSha256Hash(hash)), QStringLiteral("sha256Hash"));
        QCOMPARE(s.strKeyKind(StrKey::encodeSignedPayload(hash, QByteArray("pay"))),
                 QStringLiteral("signedPayload"));
        // An account id is not a contract id and vice versa.
        QVERIFY(!s.isValidContractId(QString(ACCOUNT())));
    }

    // ─── Amounts: the SDK is the source of truth ─────────────────────

    /** The amount field binds to this. The point is that the SDK's own message
     *  reaches the UI verbatim — including for input the SDK deliberately
     *  rejects, like a thousands separator. */
    void testValidateAmountReportsTheSdkMessage()
    {
        StellarQml s;
        QVERIFY(s.validateAmount(QStringLiteral("10.5")).isEmpty());
        QVERIFY(s.validateAmount(QStringLiteral("0")).isEmpty());

        const QString err = s.validateAmount(QStringLiteral("1,000.5"));
        QVERIFY(!err.isEmpty());
        QVERIFY(!s.validateAmount(QStringLiteral("abc")).isEmpty());

        // Text, both ways. Stroops are an int64 and pass 2^53 inside the range
        // the network really uses, so a number would round silently; and a bad
        // amount comes back empty rather than as -1, because a sentinel that
        // looks like a value is how a bad amount gets sent.
        QCOMPARE(s.amountToStroops(QStringLiteral("10.5")), QStringLiteral("105000000"));
        QCOMPARE(s.amountToStroops(QStringLiteral("abc")), QString());
        QCOMPARE(s.stroopsToAmount(QStringLiteral("105000000")), QStringLiteral("10.5"));
        // The case the old signature could not express: 92,233,720,368.5477580
        // XLM is the largest amount there is, and its stroop count needs all 63
        // bits.
        QCOMPARE(s.amountToStroops(QStringLiteral("92233720368.5477580")),
                 QStringLiteral("922337203685477580"));
        QCOMPARE(s.stroopsToAmount(QStringLiteral("922337203685477580")),
                 QStringLiteral("92233720368.547758"));
        QCOMPARE(s.stroopsToAmount(QStringLiteral("not a number")), QString());
    }

    // ─── Prices: show the fraction, not just the decimal ─────────────

    void testParsePriceExposesTheFraction()
    {
        StellarQml s;
        const QVariantMap half = s.parsePrice(QStringLiteral("0.5"));
        QVERIFY(!half.contains(QStringLiteral("error")));
        QCOMPARE(half.value(QStringLiteral("n")).toInt(), 1);
        QCOMPARE(half.value(QStringLiteral("d")).toInt(), 2);

        // Showing n/d is how a user sees which fraction a decimal became. A
        // decimal that is itself an int32 fraction stays exactly that — "0.333"
        // is 333/1000, not one third — and one that is not is approximated:
        // "0.3333333333" (3333333333/10^10 does not fit) becomes 1/3.
        const QVariantMap exact = s.parsePrice(QStringLiteral("0.333"));
        QCOMPARE(exact.value(QStringLiteral("n")).toInt(), 333);
        QCOMPARE(exact.value(QStringLiteral("d")).toInt(), 1000);
        const QVariantMap third = s.parsePrice(QStringLiteral("0.3333333333"));
        QVERIFY(!third.contains(QStringLiteral("error")));
        QCOMPARE(third.value(QStringLiteral("n")).toInt(), 1);
        QCOMPARE(third.value(QStringLiteral("d")).toInt(), 3);

        QVERIFY(s.parsePrice(QStringLiteral("not a price")).contains(QStringLiteral("error")));
        QCOMPARE(s.priceToString(1, 2), QStringLiteral("0.5"));
    }

    // ─── Assets: strings in, no pointers out ─────────────────────────

    void testDescribeAssetByCanonicalForm()
    {
        StellarQml s;
        const QVariantMap native = s.describeAsset(QStringLiteral("native"));
        QCOMPARE(native.value(QStringLiteral("type")).toString(), QStringLiteral("native"));

        const QVariantMap usd = s.describeAsset(QStringLiteral("USD:") + QString(ISSUER()));
        QCOMPARE(usd.value(QStringLiteral("code")).toString(), QStringLiteral("USD"));
        QCOMPARE(usd.value(QStringLiteral("issuer")).toString(), QString(ISSUER()));

        QVERIFY(s.describeAsset(QStringLiteral("::::")).contains(QStringLiteral("error")));
        QVERIFY(s.isValidAsset(QStringLiteral("native")));
        QVERIFY(!s.isValidAsset(QStringLiteral("::::")));
    }

    /** The registry caches, so an operation built now and one built later get
     *  the same pointer and neither has to think about freeing it. */
    void testAssetRegistryCachesAndOwns()
    {
        AssetRegistry reg;
        Asset* a = reg.assetFor(QStringLiteral("native"));
        Asset* b = reg.assetFor(QStringLiteral("native"));
        QCOMPARE(a, b);                       // same instance, not a copy
        QCOMPARE(reg.count(), 1);

        const QString usd = QStringLiteral("USD:") + QString(ISSUER());
        Asset* c = reg.assetFor(usd);
        QVERIFY(c != a);
        QCOMPARE(reg.count(), 2);
        QCOMPARE(AssetRegistry::canonicalFormOf(c), usd);

        // copyOf() is the escape hatch for the few APIs that DO take ownership.
        Asset* owned = reg.copyOf(usd);
        QVERIFY(owned != c);
        delete owned;                          // caller's to free; must not crash
    }

    void testAssetRegistryRejectsMalformed()
    {
        AssetRegistry reg;
        bool threw = false;
        try { reg.assetFor(QStringLiteral("::::")); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);                        // C++ side may throw...
        QVERIFY(!AssetRegistry::isValid(QStringLiteral("::::")));   // ...the query does not
    }

    // ─── Mnemonics ───────────────────────────────────────────────────

    /**
     * The index was an int: the QML engine narrows a JS number to it with
     * ToInt32, so 2^31 became -2^31, which KeyPair::fromBip39Seed takes as an
     * already hardened 0 — account 0's keys, and 2^32 + 100 gave account
     * 100's, with nothing in the map to say so. It is read exactly now, from
     * 0 to 2^31 - 1, and the map says which index and path it derived.
     * A JS number arrives as a double, which is what these QVariants hold.
     */
    void testKeyPairFromMnemonicReadsTheIndexExactly()
    {
        StellarQml s;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QVariantMap first = s.keyPairFromMnemonic(words, 0);
        QCOMPARE(first.value(QStringLiteral("index")).toInt(), 0);
        QCOMPARE(first.value(QStringLiteral("path")).toString(), QStringLiteral("m/44'/148'/0'"));
        const QVariantMap hundred = s.keyPairFromMnemonic(words, QVariant(100.0));
        QCOMPARE(hundred.value(QStringLiteral("path")).toString(), QStringLiteral("m/44'/148'/100'"));
        QCOMPARE(s.keyPairFromMnemonic(words, QStringLiteral("100")).value(QStringLiteral("accountId")),
                 hundred.value(QStringLiteral("accountId")));
        QVERIFY(first.value(QStringLiteral("accountId")) != hundred.value(QStringLiteral("accountId")));
        const QVariantMap last = s.keyPairFromMnemonic(words, QVariant(2147483647.0));
        QVERIFY2(!last.contains(QStringLiteral("error")), qPrintable(last.value(QStringLiteral("error")).toString()));
        // py stellar_sdk 14.1.0, the same phrase, account 2^31 - 1.
        QCOMPARE(last.value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GAYW4AVOGT2533ITZT6SUNUVCIQFIRFONQZN2GY3FX27LSF4K2DMNYGU"));
        for (const QVariant& bad : {QVariant(2147483648.0), QVariant(4294967396.0), QVariant(-1),
                                    QVariant(int(-2147483647 - 1)), QVariant(1.5), QVariant(true),
                                    QVariant(QStringLiteral("1e2")), QVariant(qint64(2147483648LL))}) {
            QVERIFY2(s.keyPairFromMnemonic(words, bad).contains(QStringLiteral("error")),
                     qPrintable(bad.toString()));
            QVERIFY2(s.keyPairFromMnemonicLegacyPassphrase(words, bad).contains(QStringLiteral("error")),
                     qPrintable(bad.toString()));
        }
        QCOMPARE(s.generateMnemonic(QVariant(128.0)).split(QLatin1Char(' ')).size(), 12);
        QVERIFY(s.generateMnemonic(QVariant(128.5)).isEmpty());
        QVERIFY(s.generateMnemonic(QVariant(4294967424.0)).isEmpty());
    }

    void testMnemonicGenerateValidateAndDerive()
    {
        StellarQml s;
        const QString phrase = s.generateMnemonic(128);
        QCOMPARE(phrase.split(' ').size(), 12);
        QVERIFY(s.validateMnemonic(phrase));
        QVERIFY(!s.validateMnemonic(QStringLiteral("clearly not a mnemonic")));

        const QVariantMap kp = s.keyPairFromMnemonic(phrase, 0);
        QVERIFY(!kp.contains(QStringLiteral("error")));
        QVERIFY(s.isValidAccountId(kp.value(QStringLiteral("accountId")).toString()));

        // Different derivation index, different account.
        const QVariantMap kp1 = s.keyPairFromMnemonic(phrase, 1);
        QVERIFY(kp1.value(QStringLiteral("accountId")).toString()
                != kp.value(QStringLiteral("accountId")).toString());

        QVERIFY(s.keyPairFromMnemonic(QStringLiteral("bad")).contains(QStringLiteral("error")));
    }

    // ─── Networks ────────────────────────────────────────────────────

    void testNetworkPassphrases()
    {
        StellarQml s;
        QCOMPARE(s.networkPassphrase(QStringLiteral("testnet")),
                 QStringLiteral("Test SDF Network ; September 2015"));
        QCOMPARE(s.networkPassphrase(QStringLiteral("public")),
                 QStringLiteral("Public Global Stellar Network ; September 2015"));
        QVERIFY(s.networkPassphrase(QStringLiteral("futurenet")).contains(QStringLiteral("Future")));
        QVERIFY(s.networkPassphrase(QStringLiteral("nope")).isEmpty());
    }

    // ─── Contract arguments ──────────────────────────────────────────

    /**
     * A "map" argument is signed with its keys in the host's order. Keys given
     * as amount, to were signed as {to, amount}, -1, 1 as {1, -1} and "aa",
     * "b" as {"b", "aa"}: a map the host refuses as unsorted, fee charged.
     */
    void testContractMapKeysAreInTheHostOrder()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto signedKeys = [&](const QString& keyType, const QVariantList& keys) {
            QVariantList pairs;
            for (int i = 0; i < keys.size(); ++i) {
                QVariantMap pair;
                pair.insert(QStringLiteral("key"), arg(keyType, keys.at(i)));
                pair.insert(QStringLiteral("value"), arg(QStringLiteral("u32"), i));
                pairs.append(pair);
            }
            const QVariantMap built =
                builder.build(invokeSpec(QVariantList{arg(QStringLiteral("map"), pairs)}));
            if (built.contains(QStringLiteral("error")))
                return QStringList{built.value(QStringLiteral("error")).toString()};
            QStringList seen;
            for (const stellar::SCMapEntry& e : Scv::fromMap(invokedArgs(built).at(0)))
                seen << keyText(e.key);
            return seen;
        };
        const QStringList amountTo{QStringLiteral("amount"), QStringLiteral("to")};
        QCOMPARE(signedKeys(QStringLiteral("symbol"), {QStringLiteral("amount"), QStringLiteral("to")}), amountTo);
        QCOMPARE(signedKeys(QStringLiteral("symbol"), {QStringLiteral("to"), QStringLiteral("amount")}), amountTo);
        QCOMPARE(signedKeys(QStringLiteral("symbol"), {QStringLiteral("signature"), QStringLiteral("public_key")}),
                 QStringList({QStringLiteral("public_key"), QStringLiteral("signature")}));
        QCOMPARE(signedKeys(QStringLiteral("i32"), {-1, 1}), QStringList({QStringLiteral("-1"), QStringLiteral("1")}));
        QCOMPARE(signedKeys(QStringLiteral("i64"), {QStringLiteral("5"), QStringLiteral("-5")}),
                 QStringList({QStringLiteral("-5"), QStringLiteral("5")}));
        QCOMPARE(signedKeys(QStringLiteral("string"), {QStringLiteral("aa"), QStringLiteral("b")}),
                 QStringList({QStringLiteral("aa"), QStringLiteral("b")}));
        QCOMPARE(signedKeys(QStringLiteral("bytes"), {QStringLiteral("02"), QStringLiteral("0101")}),
                 QStringList({QStringLiteral("0101"), QStringLiteral("02")}));
    }

    /**
     * Contract arguments nest at most as deep as the XDR reader reads
     * (MAX_SCVAL_DEPTH). 250 nested "vec" arguments — 7 KB of JSON — used to
     * overflow the stack in Tx.build, a crash QML has no way to catch.
     */
    void testContractArgumentNestingIsLimited()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const int max = stellar::MAX_SCVAL_DEPTH;
        const QString tooDeep = QStringLiteral("nest at most 128");
        auto nestedArg = [](int depth) {
            QVariantMap node = arg(QStringLiteral("u32"), 7);
            for (int i = 1; i < depth; ++i)
                node = arg(QStringLiteral("vec"), QVariantList{node});
            return node;
        };
        auto errorOf = [&](const QVariant& args) {
            return builder.build(invokeSpec(args)).value(QStringLiteral("error")).toString();
        };

        const QVariantMap atLimit = builder.build(invokeSpec(QVariantList{nestedArg(max)}));
        QVERIFY2(!atLimit.contains(QStringLiteral("error")),
                 qPrintable(atLimit.value(QStringLiteral("error")).toString()));
        // ...and what was built reads back: the two limits are the same one.
        XdrQml xdr;
        const QVariantMap read = xdr.decode(QStringLiteral("envelope"), atLimit.value(QStringLiteral("xdr")).toString());
        QVERIFY2(!read.contains(QStringLiteral("error")), qPrintable(read.value(QStringLiteral("error")).toString()));
        const QString refused = errorOf(QVariantList{nestedArg(max + 1)});
        QVERIFY2(refused.contains(tooDeep), qPrintable(refused));

        // As JSON text, the deepest argument list there is: maps all the way
        // down to an empty vec, 3 * 128 levels of JSON. One level more is
        // refused before Qt parses it.
        auto nestedMapJson = [](int levels) {
            QString json = QStringLiteral("{\"type\":\"vec\",\"value\":[]}");
            for (int i = 1; i < levels; ++i)
                json = QStringLiteral("{\"type\":\"map\",\"value\":[{\"key\":{\"type\":\"u32\",\"value\":\"0\"},\"value\":%1}]}").arg(json);
            return QStringLiteral("[%1]").arg(json);
        };
        const QString deepestJson = errorOf(nestedMapJson(max));
        QVERIFY2(deepestJson.isEmpty(), qPrintable(deepestJson));
        const QString deeperJson = errorOf(nestedMapJson(max + 1));
        QVERIFY2(deeperJson.contains(tooDeep), qPrintable(deeperJson));

        auto nestedVecJson = [](int levels) {
            QString json = QStringLiteral("{\"type\":\"u32\",\"value\":7}");
            for (int i = 1; i < levels; ++i)
                json = QStringLiteral("{\"type\":\"vec\",\"value\":[%1]}").arg(json);
            return QStringLiteral("[%1]").arg(json);
        };
        // Refused at the 129th level with 380 levels of parsed JSON still to
        // free, the most the JSON check lets through: freeing them while the
        // refusal unwound is what overflowed the stack.
        const QString atUnwind = errorOf(nestedVecJson(190));
        QVERIFY2(atUnwind.contains(tooDeep), qPrintable(atUnwind));
        const QString fromJson = errorOf(nestedVecJson(250));
        QVERIFY2(fromJson.contains(tooDeep), qPrintable(fromJson));
        // Close to the most Qt's parser takes (1024 levels), and brackets in a
        // string do not count.
        const QString atQtLimit =
            errorOf(QString(1000, QLatin1Char('[')) + QString(1000, QLatin1Char(']')));
        QVERIFY2(atQtLimit.contains(tooDeep), qPrintable(atQtLimit));
        const QString inString = errorOf(QStringLiteral("[{\"type\":\"string\",\"value\":\"%1\"}]")
                                             .arg(QString(1000, QLatin1Char('['))));
        QVERIFY2(inString.isEmpty(), qPrintable(inString));
    }

    /** The QML invokeContract takes a function name only when it is a symbol;
     *  "trans-fer" and "a b" were built and signed, then refused by the host. */
    void testContractFunctionNameMustBeASymbol()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVERIFY(builder.build(invokeSpec(QVariantList{}, QStringLiteral("trans-fer"))).contains(QStringLiteral("error")));
        QVERIFY(builder.build(invokeSpec(QVariantList{}, QStringLiteral("a b"))).contains(QStringLiteral("error")));
        QVERIFY(builder.build(invokeSpec(QVariantList{}, QString::fromUtf8("h\xC3\xA9llo"))).contains(QStringLiteral("error")));
        QVERIFY(!builder.build(invokeSpec(QVariantList{}, QStringLiteral("transfer"))).contains(QStringLiteral("error")));
    }

    /** Xdr.decode gives 128/256-bit integers, timepoints and durations a value
     *  row, exact: they had only a type row, so a token balance (an i128) could
     *  not be read at all. */
    void testXdrDecodeShowsWideIntegersExactly()
    {
        XdrQml xdr;
        auto shown = [&](const stellar::SCVal& v) {
            const QVariantMap out = xdr.decode(QStringLiteral("scval"), base64Of(xdrOf(v)));
            for (const QVariant& row : out.value(QStringLiteral("rows")).toList()) {
                if (row.toMap().value(QStringLiteral("field")).toString() == QLatin1String("value"))
                    return row.toMap().value(QStringLiteral("value")).toString();
            }
            return QStringLiteral("no value row");
        };
        QCOMPARE(shown(Scv::toInt128(qint64(100000000000LL))), QStringLiteral("100000000000"));
        QCOMPARE(shown(Scv::toInt128(-1)), QStringLiteral("-1"));
        QCOMPARE(shown(Scv::toInt128Parts(qint64(-9223372036854775807LL - 1), quint64(0))),
                 QStringLiteral("-170141183460469231731687303715884105728"));
        QCOMPARE(shown(Scv::toUint128Parts(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL)),
                 QStringLiteral("340282366920938463463374607431768211455"));
        QCOMPARE(shown(Scv::toUint256(QByteArray(32, '\xff'))),
                 QStringLiteral("115792089237316195423570985008687907853269984665640564039457584007913129639935"));
        QCOMPARE(shown(Scv::toInt256(QByteArray(1, '\xfb'))), QStringLiteral("-5"));
        QCOMPARE(shown(Scv::toTimePoint(quint64(1700000000))), QStringLiteral("1700000000"));
        QCOMPARE(shown(Scv::toDuration(quint64(86400))), QStringLiteral("86400"));
    }

    /**
     * Xdr.decode reads line-wrapped base64, as js-stellar-sdk and
     * py-stellar-sdk do: a space, tab, CR or LF inside the text was "not
     * valid base64". Any other character outside the alphabet, a
     * non-breaking space or a URL-safe '-', is still refused.
     */
    void testXdrDecodeReadsLineWrappedBase64()
    {
        XdrQml xdr;
        // SCV_BYTES of 32 0xfb bytes, as js and py encode it: 40 bytes.
        const QString value = QStringLiteral("AAAADQAAACD7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+w==");
        const QStringList gaps{QStringLiteral("\n"), QStringLiteral("\r\n"),
                               QStringLiteral(" "), QStringLiteral("\t")};
        for (const QString& gap : gaps) {
            const QString wrapped = value.left(20) + gap + value.mid(20) + gap;
            QVERIFY(xdr.looksLikeBase64(wrapped));
            const QVariantMap out = xdr.decode(QStringLiteral("scval"), wrapped);
            QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(out.value(QStringLiteral("error")).toString()));
            QCOMPARE(out.value(QStringLiteral("bytes")).toInt(), 40);
        }
        for (const QString& gap : {QString(QChar(0x00A0)), QStringLiteral("-")}) {
            const QString damaged = value.left(20) + gap + value.mid(20);
            QVERIFY(!xdr.looksLikeBase64(damaged));
            QVERIFY(xdr.decode(QStringLiteral("scval"), damaged).contains(QStringLiteral("error")));
        }
    }

    /**
     * Xdr.decode and looksLikeBase64 skip the whitespace js 17.1.0
     * (xdr.ScVal.fromXDR(text, 'base64')) and py 14.1.0
     * (xdr.SCVal.from_xdr(text)) both skip: space, tab, LF, form feed and
     * CR, at the ends or inside. A form feed was "not valid base64". A
     * vertical tab, a non-breaking space, U+0085 or U+2028 is refused
     * wherever it is, as js refuses it ("invalid base64 input"; py skips any
     * character outside the alphabet): at the ends they were trimmed off.
     */
    void testXdrDecodeSkipsTheWhitespaceJsAndPySkip()
    {
        XdrQml xdr;
        // SCV_BYTES of 32 0xfb bytes, as js and py encode it: 40 bytes.
        const QString value = QStringLiteral("AAAADQAAACD7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+w==");
        const QStringList read{
            value.left(20) + QStringLiteral("\f") + value.mid(20),
            QStringLiteral("\f") + value + QStringLiteral("\f"),
            QStringLiteral(" \t\n\f\r") + value.left(20) + QStringLiteral("\r\n\f\t ") + value.mid(20)
                + QStringLiteral("\n"),
        };
        for (const QString& text : read) {
            QVERIFY(xdr.looksLikeBase64(text));
            const QVariantMap out = xdr.decode(QStringLiteral("scval"), text);
            QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(out.value(QStringLiteral("error")).toString()));
            QCOMPARE(out.value(QStringLiteral("bytes")).toInt(), 40);
        }
        const QStringList refused{
            value.left(20) + QStringLiteral("\v") + value.mid(20),
            value + QStringLiteral("\v"),
            QString(QChar(0x00A0)) + value,
            value + QString(QChar(0x0085)),
            value + QString(QChar(0x2028)),
        };
        for (const QString& text : refused) {
            QVERIFY(!xdr.looksLikeBase64(text));
            const QString error = xdr.decode(QStringLiteral("scval"), text).value(QStringLiteral("error")).toString();
            QVERIFY2(error.startsWith(QStringLiteral("that is not valid base64")), qPrintable(error));
        }
        // Only the skipped whitespace is nothing pasted; a vertical tab or a
        // non-breaking space alone is not base64, as anywhere else.
        const QString blank = xdr.decode(QStringLiteral("scval"), QStringLiteral(" \t\n\f\r"))
                                  .value(QStringLiteral("error")).toString();
        QVERIFY2(blank.startsWith(QStringLiteral("paste some base64 first")), qPrintable(blank));
        for (const QString& text : {QStringLiteral("\v"), QString(QChar(0x00A0))}) {
            const QString error = xdr.decode(QStringLiteral("scval"), text).value(QStringLiteral("error")).toString();
            QVERIFY2(error.startsWith(QStringLiteral("that is not valid base64")), qPrintable(error));
        }
    }

    /** Hostile XDR pasted into Xdr.decode is an error, not a crash: 400 nested
     *  vectors overflowed the stack, and 12 bytes claiming 3,000,000 elements
     *  committed 3 GB before failing. */
    void testXdrDecodeRefusesHostileValues()
    {
        XdrQml xdr;
        auto be32 = [](qint32 n) {
            QByteArray out;
            QDataStream s(&out, QIODevice::WriteOnly);
            s << n;
            return out;
        };
        QByteArray deep;
        for (int i = 1; i < 400; ++i) deep += be32(16) + be32(1) + be32(1);
        deep += be32(3) + be32(7);
        QVERIFY(xdr.decode(QStringLiteral("scval"), base64Of(deep)).value(QStringLiteral("error"))
                    .toString().contains(QStringLiteral("nesting too deep")));
        QVERIFY(xdr.decode(QStringLiteral("scval"), base64Of(be32(16) + be32(1) + be32(3000000)))
                    .value(QStringLiteral("error")).toString().contains(QStringLiteral("exceeds the remaining input")));
        // A whole value followed by other bytes was decoded as if they were
        // not there.
        const QString u32 = base64Of(be32(3) + be32(7));
        QVERIFY(!xdr.decode(QStringLiteral("scval"), u32).contains(QStringLiteral("error")));
        QVERIFY(xdr.decode(QStringLiteral("scval"), base64Of(be32(3) + be32(7) + be32(0)))
                    .value(QStringLiteral("error")).toString().contains(QStringLiteral("followed by other bytes")));
    }

    /**
     * The passphrase reaches BIP-39 in NFKD whatever form QML hands it over
     * in. It went in as typed, so a precomposed passphrase opened a wallet
     * no other BIP-39 software would find. Expected: py stellar_sdk 14.1.0
     * Keypair.from_mnemonic_phrase(words, passphrase=..., index=0).
     */
    void testKeyPairFromMnemonicNormalisesThePassphrase()
    {
        StellarQml s;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QString typed = QStringLiteral("P") + QChar(0x00E4) + QStringLiteral("ssw")
                              + QChar(0x00F6) + QStringLiteral("rd ") + QChar(0x334D)
                              + QStringLiteral(" ") + QChar(0xFB01) + QStringLiteral(" ")
                              + QChar(0x212B);
        const QVariantMap key = s.keyPairFromMnemonic(words, 0, typed);
        QCOMPARE(key.value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GCKHOYVAHPJFPAE54WVDN6K42HG7ZG3AU7P46DOKRGUQYWBVHE66ARHZ"));
        QCOMPARE(key.value(QStringLiteral("secretSeed")).toString(),
                 QStringLiteral("SAKCFV4SR4ODQTXPFHYXK4A66JCNK2SLGQHC7D47JIESBSIRPEBVNAUC"));
        QVERIFY(s.keyPairFromMnemonic(words, 0, QStringLiteral("x") + QChar(0xD800))
                    .contains(QStringLiteral("error")));
    }

    /**
     * A wallet created with 1.0.0-1.0.3 and that passphrase lives at the
     * accounts derived from it as typed. With the passphrase now normalised
     * nothing in the QML layer reached them, so the upgrade left its funds
     * out of reach. keyPairFromMnemonicLegacyPassphrase does, and says it is
     * the legacy derivation; for an ASCII passphrase both give the same key.
     * Expected: PBKDF2 over the passphrase as typed (python hashlib, as the
     * 1.0.3 code did) and py stellar_sdk 14.1.0's SEP-5 derivation.
     */
    void testKeyPairFromMnemonicLegacyPassphraseRecoversOldWallets()
    {
        StellarQml s;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QString typed = QStringLiteral("P") + QChar(0x00E4) + QStringLiteral("ssw")
                              + QChar(0x00F6) + QStringLiteral("rd ") + QChar(0x334D)
                              + QStringLiteral(" ") + QChar(0xFB01) + QStringLiteral(" ")
                              + QChar(0x212B);
        QVERIFY(s.legacyPassphraseDerivesDifferently(typed));
        const QVariantMap old = s.keyPairFromMnemonicLegacyPassphrase(words, 0, typed);
        QCOMPARE(old.value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GB5OYYOSOUO52LZI6TOMCJIZ726BNNZZFI3RKISAUZWFZ25MQFZLHKYZ"));
        QCOMPARE(old.value(QStringLiteral("secretSeed")).toString(),
                 QStringLiteral("SDKSJVB6YMUIOTKAHBF7EVX4ZC5ZMVWRLNAHO4V4NLZX3ZUXV6QNDBGW"));
        QVERIFY(old.value(QStringLiteral("legacy")).toBool());
        QCOMPARE(s.keyPairFromMnemonicLegacyPassphrase(words, 1, typed)
                     .value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GA4VE6QLOCX4BMGCH6JRNVNNK2BVNWTM7OGKOTITTCVYGHGOLUWEALL4"));
        QCOMPARE(s.keyPairFromMnemonic(words, 0, typed).value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GCKHOYVAHPJFPAE54WVDN6K42HG7ZG3AU7P46DOKRGUQYWBVHE66ARHZ"));

        const QString ascii = QStringLiteral("TREZOR");
        QVERIFY(!s.legacyPassphraseDerivesDifferently(ascii));
        QCOMPARE(s.keyPairFromMnemonicLegacyPassphrase(words, 0, ascii)
                     .value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GBXISQLLM6OGWWZU7SCNF2GOJSQLHJU3I76P23J6IDSJGKEEGM2AJYG5"));
        QCOMPARE(s.keyPairFromMnemonic(words, 0, ascii).value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GBXISQLLM6OGWWZU7SCNF2GOJSQLHJU3I76P23J6IDSJGKEEGM2AJYG5"));

        // Failures come back as values, as everywhere in this layer.
        QVERIFY(s.keyPairFromMnemonicLegacyPassphrase(QStringLiteral("bad"))
                    .contains(QStringLiteral("error")));
        const QString unpaired = QStringLiteral("x") + QChar(0xD800);
        QVERIFY(s.keyPairFromMnemonicLegacyPassphrase(words, 0, unpaired)
                    .contains(QStringLiteral("error")));
        QVERIFY(!s.legacyPassphraseDerivesDifferently(unpaired));
    }

    /**
     * Every result code the XDR defines is shown by name. The codes this
     * release added were read and then shown as "unknown (-18)" or
     * "unknown (-6)", trust-line codes after -5 as a bare number, and every
     * operation but payments, account creation and trust lines as a dash,
     * whatever the network said about it.
     */
    void testXdrDecodeNamesEveryResultCode()
    {
        XdrQml xdr;
        auto be32 = [](qint32 n) {
            QByteArray out;
            QDataStream s(&out, QIODevice::WriteOnly);
            s << n;
            return out;
        };
        auto shown = [&](const QByteArray& raw) {
            QMap<QString, QString> fields;
            const QVariantMap decoded = xdr.decode(QStringLiteral("result"), base64Of(raw));
            const QVariantList rows = decoded.value(QStringLiteral("rows")).toList();
            for (const QVariant& row : rows)
                fields.insert(row.toMap().value(QStringLiteral("field")).toString(),
                              row.toMap().value(QStringLiteral("value")).toString());
            return fields;
        };
        const QByteArray fee = be32(0) + be32(100);   // int64 feeCharged
        const QByteArray ext = be32(0);

        QCOMPARE(shown(fee + be32(-15) + ext).value(QStringLiteral("result")),
                 QStringLiteral("tx_bad_min_seq_age_or_gap"));
        QCOMPARE(shown(fee + be32(-16) + ext).value(QStringLiteral("result")),
                 QStringLiteral("tx_malformed"));
        QCOMPARE(shown(fee + be32(-17) + ext).value(QStringLiteral("result")),
                 QStringLiteral("tx_soroban_invalid"));
        QCOMPARE(shown(fee + be32(-18) + ext).value(QStringLiteral("result")),
                 QStringLiteral("tx_frozen_key_accessed"));

        const QMap<QString, QString> failed = shown(fee + be32(-1) + be32(6)
            + be32(0) + be32(6) + be32(-7)       // CHANGE_TRUST_CANNOT_DELETE
            + be32(-6)                           // opTOO_MANY_SPONSORING
            + be32(0) + be32(24) + be32(-2)      // INVOKE_HOST_FUNCTION_TRAPPED
            + be32(0) + be32(15) + be32(-6)      // CLAIM_CLAIMABLE_BALANCE_TRUSTLINE_FROZEN
            + be32(0) + be32(13) + be32(-12)     // PATH_PAYMENT_STRICT_SEND_UNDER_DESTMIN
            + be32(0) + be32(3) + be32(-12)      // MANAGE_SELL_OFFER_LOW_RESERVE
            + ext);
        QCOMPARE(failed.value(QStringLiteral("result")), QStringLiteral("tx_failed"));
        QCOMPARE(failed.value(QStringLiteral("operation 1")),
                 QStringLiteral("change_trust \u2014 change_trust_cannot_delete"));
        QCOMPARE(failed.value(QStringLiteral("operation 2")), QStringLiteral("op_too_many_sponsoring"));
        QCOMPARE(failed.value(QStringLiteral("operation 3")),
                 QStringLiteral("invoke_host_function \u2014 invoke_host_function_trapped"));
        QCOMPARE(failed.value(QStringLiteral("operation 4")),
                 QStringLiteral("claim_claimable_balance \u2014 claim_claimable_balance_trustline_frozen"));
        QCOMPARE(failed.value(QStringLiteral("operation 5")),
                 QStringLiteral("path_payment_strict_send \u2014 path_payment_strict_send_under_destmin"));
        QCOMPARE(failed.value(QStringLiteral("operation 6")),
                 QStringLiteral("manage_sell_offer \u2014 manage_sell_offer_low_reserve"));
    }

    // ─── Horizon ─────────────────────────────────────────────────────

    /**
     * HorizonQml was the one QML class without QSTELLAR_ALIAS: code on the
     * unqualified names (STELLAR_QT_LEGACY_GLOBAL_NS, as this suite is) could
     * name every facade but this one. Naming it here is the test.
     */
    void testHorizonQmlHasTheLegacyAlias()
    {
        static_assert(std::is_same<HorizonQml, qstellar::HorizonQml>::value,
                      "the alias names the qstellar class");
        HorizonQml horizon;
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * The "assets" query built its table from "amount" and "num_accounts",
     * which Horizon 29 no longer sends: every asset read supply "" and 0
     * holders. A real Horizon testnet record (2026-09-30).
     */
    void testHorizonAssetsQueryShowsSupplyAndHolders()
    {
        FakeServer* fake = new FakeServer();
        fake->addGet(QStringLiteral("/assets?limit=20"), QStringLiteral(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/assets?cursor=&limit=20&order=asc\"}},\"_embedded\":{\"records\":[{\"_"
            "links\":{\"toml\":{\"href\":\"\"}},\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"001\",\"asset_issuer\":\"GCY2WELAIODVHDNJQFZVDSVK3OC"
            "CS6EQGRXLAEBC5M2KCNCIJSEZYTHU\",\"paging_token\":\"001_GCY2WELAIODVHDNJQFZVDSVK3OCCS6EQGRXLAEBC5M2KCNCIJSEZYTHU_credit_alphanum4\",\"num_cla"
            "imable_balances\":0,\"num_liquidity_pools\":0,\"num_contracts\":0,\"accounts\":{\"authorized\":14,\"authorized_to_maintain_liabilities\":0,\""
            "unauthorized\":0},\"claimable_balances_amount\":\"0.0000000\",\"liquidity_pools_amount\":\"0.0000000\",\"contracts_amount\":\"0.0000000\",\""
            "balances\":{\"authorized\":\"100.0000000\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\""
            "auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false}}]}}"));
        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());
        QVariantMap table;
        int answers = 0;
        QObject::connect(&horizon, &HorizonQml::queried, [&](int, const QVariantMap& out) { ++answers; table = out; });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int, const QVariantMap& out) { ++answers; table = out; });
        QVariantMap spec;
        spec.insert(QStringLiteral("query"), QStringLiteral("assets"));
        horizon.runQuery(spec);
        WAIT_FOR(answers == 0)
        QCOMPARE(answers, 1);
        QVERIFY2(!table.contains(QStringLiteral("kind")), qPrintable(table.value(QStringLiteral("message")).toString()));
        const QVariantList rows = table.value(QStringLiteral("rows")).toList();
        QCOMPARE(rows.size(), 1);
        const QVariantMap row = rows.at(0).toMap();
        QCOMPARE(row.value(QStringLiteral("asset")).toString(),
                 QStringLiteral("001:GCY2WELAIODVHDNJQFZVDSVK3OCCS6EQGRXLAEBC5M2KCNCIJSEZYTHU"));
        QCOMPARE(row.value(QStringLiteral("supply")).toString(), QStringLiteral("100.0000000"));
        QCOMPARE(row.value(QStringLiteral("holders")).toString(), QStringLiteral("14"));
        fake->deleteLater();
    }

    /**
     * A reply the SDK cannot read threw out of the handler Qt called with
     * it: an account holding a credit line whose issuer is not an account id
     * made getAssetIssuer() throw inside a signal dispatch, which ends the
     * app. It is one "badResponse" failure now, for loadAccount and runQuery
     * alike, and the request is no longer pending.
     */
    void testHorizonUnreadableReplyIsAFailure()
    {
        const QString account = QString::fromLatin1(ACCOUNT());
        FakeServer* fake = new FakeServer();
        fake->addGet(QStringLiteral("/accounts/") + account, QStringLiteral(
            "{\"id\":\"%1\",\"account_id\":\"%1\",\"sequence\":\"1\",\"subentry_count\":0,"
            "\"balances\":[{\"balance\":\"1.0000000\",\"limit\":\"100.0000000\","
            "\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\","
            "\"asset_issuer\":\"GNOTANACCOUNT\"},"
            "{\"balance\":\"5.0000000\",\"asset_type\":\"native\"}],"
            "\"signers\":[],\"data\":{}}").arg(account));
        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());

        int answers = 0;
        int failedId = -1;
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::failed, [&](int id, const QVariantMap& error) {
            ++answers; failedId = id; failure = error;
        });
        QObject::connect(&horizon, &HorizonQml::accountLoaded, [&](int, const QVariantMap&) { ++answers; });
        QObject::connect(&horizon, &HorizonQml::queried, [&](int, const QVariantMap&) { ++answers; });

        const int load = horizon.loadAccount(account);
        WAIT_FOR(answers == 0)
        QTest::qWait(50);
        QCOMPARE(answers, 1);
        QCOMPARE(failedId, load);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QVERIFY(!failure.value(QStringLiteral("message")).toString().isEmpty());
        QCOMPARE(horizon.pending(), 0);

        answers = 0;
        failedId = -1;
        QVariantMap spec;
        spec.insert(QStringLiteral("query"), QStringLiteral("account"));
        spec.insert(QStringLiteral("account"), account);
        const int query = horizon.runQuery(spec);
        WAIT_FOR(answers == 0)
        QTest::qWait(50);
        QCOMPARE(answers, 1);
        QCOMPARE(failedId, query);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QCOMPARE(horizon.pending(), 0);
        fake->deleteLater();
    }

    /**
     * A submit stopped by SEP-29 came back as kind "connection": nothing said
     * which destination wanted a memo, and "check your network" is the wrong
     * advice for a payment to an exchange. It is "memoRequired" now, with the
     * destination and the operation; a lookup that fails is reported as the
     * failure it was, the kind loadAccount reports for the same account. (The
     * fake answers the unrouted lookup with a status line that has no code,
     * which Qt reads as an unreadable reply, "badResponse", or as a dropped
     * connection depending on its version. It was "connection" whatever the
     * lookup met, because every lookup failure was.) Neither was posted, and
     * each says so: mayHaveReachedServer false, and no hash to look up.
     */
    void testHorizonMemoRequiredIsReportedAsSuch()
    {
        const QString exchange = QString::fromLatin1(ISSUER());
        const QString unreachable = QString::fromLatin1(ACCOUNT());
        FakeServer* fake = new FakeServer();
        // A whole account, as Horizon sends one: 1.0.4 refuses an account
        // that is its data member alone (it was the whole fixture here).
        fake->addGet(QStringLiteral("/accounts/") + exchange, accountJson(exchange,
                     QStringLiteral("{\"config.memo_required\":\"MQ==\"}")));
        // No GET for `unreachable`: the fake answers it with a broken reply.
        fake->addPost(QStringLiteral("/transactions"), QStringLiteral("{\"hash\":\"00\",\"ledger\":1}"));
        fake->addGet(QStringLiteral("/"), QString::fromLatin1(HeldHorizon::testnetRoot()));
        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());

        const auto envelopeTo = [](const QString& destination) {
            QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString::fromLatin1(SECRET())));
            Account source(signer->getAccountId(), 5);
            QScopedPointer<Transaction> tx(TransactionBuilder(&source, Network::testnetNetwork())
                .addOperation(new PaymentOperation(destination, new AssetTypeNative(), QStringLiteral("10")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
            tx->sign(signer.data());
            return tx->toEnvelopeXdrBase64();
        };

        bool submitted = false;
        int failedId = -1;
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::submitted, [&](int, const QVariantMap&) { submitted = true; });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int id, const QVariantMap& error) {
            failedId = id; failure = error;
        });

        const int id = horizon.submit(envelopeTo(exchange), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, id);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("memoRequired"));
        QCOMPARE(failure.value(QStringLiteral("destination")).toString(), exchange);
        QCOMPARE(failure.value(QStringLiteral("operationIndex")).toInt(), 0);
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(exchange));
        QVERIFY2(saysNotSent(failure), "a submit SEP-29 stopped says it was not sent");
        QCOMPARE(horizon.pending(), 0);

        // What loading that account meets.
        failedId = -1;
        failure.clear();
        const int load = horizon.loadAccount(unreachable);
        WAIT_FOR(failedId < 0)
        QCOMPARE(failedId, load);
        const QString lookupKind = failure.value(QStringLiteral("kind")).toString();
        QVERIFY(lookupKind == QLatin1String("badResponse") || lookupKind == QLatin1String("connection"));

        failedId = -1;
        failure.clear();
        const int second = horizon.submit(envelopeTo(unreachable), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, second);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), lookupKind);
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("SEP-29")));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(unreachable));
        QVERIFY2(saysNotSent(failure), "a submit whose SEP-29 lookup failed says it was not sent");
        QCOMPARE(horizon.pending(), 0);
        fake->deleteLater();
    }

    /**
     * A SEP-29 lookup that Horizon rate-limited, answered with a 5xx or
     * answered with something that is not JSON stopped the submit as kind
     * "connection" with no Retry-After: the user was told to check a network
     * that was working, and the app had no idea how long to wait. It is
     * "rateLimited" with `retryAfter`, and "badResponse", now, still naming
     * the account and SEP-29. A reply that is not JSON was "unknown" for
     * loadAccount too, although the class comment calls it "badResponse".
     * None of those submits was posted, and each failure says so.
     */
    void testHorizonMemoLookupFailureKeepsItsKind()
    {
        const QString limitedAccount = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const QString failingAccount = QStringLiteral("GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB");
        const QString garbledAccount = QString::fromLatin1(ACCOUNT());
        FakeServer* fake = new FakeServer();
        fake->addGet(QStringLiteral("/accounts/") + garbledAccount, QStringLiteral("this is not JSON"));
        FakeServer::Reply limited;
        limited.body = QStringLiteral("{\"title\":\"Rate Limit Exceeded\",\"status\":429}");
        limited.responseCode = QStringLiteral("429 Too Many Requests");
        limited.contentType = QStringLiteral("application/problem+json");
        limited.extraHeaders = QStringLiteral("Retry-After: 12\r\n");
        fake->addGetFull(QStringLiteral("/accounts/") + limitedAccount, limited);
        fake->addGet(QStringLiteral("/accounts/") + failingAccount, QStringLiteral("{}"),
                     QStringLiteral("503 Service Unavailable"));
        fake->addPost(QStringLiteral("/transactions"), QStringLiteral("{\"hash\":\"00\",\"ledger\":1}"));
        fake->addGet(QStringLiteral("/"), QString::fromLatin1(HeldHorizon::testnetRoot()));
        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());

        const auto envelopeTo = [](const QString& destination) {
            QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString::fromLatin1(SECRET())));
            Account source(signer->getAccountId(), 5);
            QScopedPointer<Transaction> tx(TransactionBuilder(&source, Network::testnetNetwork())
                .addOperation(new PaymentOperation(destination, new AssetTypeNative(), QStringLiteral("10")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
            tx->sign(signer.data());
            return tx->toEnvelopeXdrBase64();
        };

        bool submitted = false;
        int failedId = -1;
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::submitted, [&](int, const QVariantMap&) { submitted = true; });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int id, const QVariantMap& error) {
            failedId = id; failure = error;
        });

        const int first = horizon.submit(envelopeTo(limitedAccount), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, first);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("rateLimited"));
        QCOMPARE(failure.value(QStringLiteral("retryAfter")).toInt(), 12);
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(limitedAccount));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("SEP-29")));
        QVERIFY(saysNotSent(failure));

        failedId = -1;
        failure.clear();
        const int second = horizon.submit(envelopeTo(failingAccount), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, second);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(failingAccount));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("SEP-29")));
        QVERIFY(saysNotSent(failure));

        failedId = -1;
        failure.clear();
        const int third = horizon.submit(envelopeTo(garbledAccount), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, third);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(garbledAccount));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("SEP-29")));
        QVERIFY(saysNotSent(failure));

        failedId = -1;
        failure.clear();
        const int load = horizon.loadAccount(garbledAccount);
        WAIT_FOR(failedId < 0)
        QCOMPARE(failedId, load);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QCOMPARE(horizon.pending(), 0);
        fake->deleteLater();
    }

    /**
     * A request the SDK gave up waiting for (Response::replyTimeout()) was
     * kind "connection", which says the request never went out: for a
     * submit that is the dangerous misreading, since the transaction may
     * still land. It is "timeout", as Horizon's 504 is, for loadAccount and
     * for a SEP-29 lookup ahead of a submit alike. (The submit whose lookup
     * timed out was not posted, and says so.)
     */
    void testHorizonClientTimeoutIsATimeout()
    {
        const QString account = QString::fromLatin1(ACCOUNT());
        FakeServer* fake = new FakeServer();
        fake->addHang(QStringLiteral("/accounts/") + account);
        fake->addPost(QStringLiteral("/transactions"), QStringLiteral("{\"hash\":\"00\",\"ledger\":1}"));
        fake->addGet(QStringLiteral("/"), QString::fromLatin1(HeldHorizon::testnetRoot()));
        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());

        bool submitted = false;
        int failedId = -1;
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::submitted, [&](int, const QVariantMap&) { submitted = true; });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int id, const QVariantMap& error) {
            failedId = id; failure = error;
        });

        // The endpoint's network is learned first, in the SDK's own time
        // (a submit asks it before its SEP-29 lookups): the 300 ms below are
        // the lookup's. A submit with a memo makes no lookup; how it ends
        // does not matter here.
        horizon.submit(paymentTo(account, /*memo*/ true, 4, nullptr), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        failedId = -1;
        failure.clear();
        submitted = false;

        Response::setReplyTimeout(300);
        // Restored however the test ends: a failed check used to leave 300 ms
        // for every test after it.
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        const int load = horizon.loadAccount(account);
        WAIT_FOR(failedId < 0)
        QCOMPARE(failedId, load);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("timeout"));

        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString::fromLatin1(SECRET())));
        Account source(signer->getAccountId(), 5);
        QScopedPointer<Transaction> tx(TransactionBuilder(&source, Network::testnetNetwork())
            .addOperation(new PaymentOperation(account, new AssetTypeNative(), QStringLiteral("10")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        tx->sign(signer.data());
        failedId = -1;
        failure.clear();
        const int submit = horizon.submit(tx->toEnvelopeXdrBase64(), Network::TESTNET_NETWORK_PASSPHRASE());
        WAIT_FOR(failedId < 0 && !submitted)
        QVERIFY(!submitted);
        QCOMPARE(failedId, submit);
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("timeout"));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("SEP-29")));
        QVERIFY(saysNotSent(failure));
        QCOMPARE(horizon.pending(), 0);
        fake->deleteLater();
    }

    // ─── Cancelling: a submit that may land is always reported ───────

    /**
     * cancelAll(), or an endpoint change, while a submit is on the wire —
     * Horizon has read its POST whole and holds its answer — reports it,
     * once: failed() with kind "cancelled", mayHaveReachedServer true and
     * the transaction's hash, and `pending` counts it until then. Horizon's
     * answer, when it comes afterwards, is no second report. That report
     * used to be dropped, and nothing aborted the POST: a wallet that
     * cancelled, or switched endpoint, never heard that its payment may
     * land, and could pay again. A load beside it is forgotten at once, as
     * documented; after the endpoint change the next submit goes to the new
     * endpoint, and is answered from there.
     */
    void testHorizonCancelStillReportsASubmitOnTheWire()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        for (const bool byEndpointChange : { false, true }) {
            const char* how = byEndpointChange ? "endpoint change" : "cancelAll()";
            HeldHorizon wire;
            wire.holdPosts = true;
            wire.holdLookups = true;
            wire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
            HeldHorizon elsewhere;
            QString hash;
            const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
            wire.setApplied(appliedAnswer(hash, envelope));

            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            QSignalSpy loaded(&horizon, &HorizonQml::accountLoaded);

            horizon.loadAccount(destination);
            const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(wire.posts(), 1, 15000);      // read whole, held
            QTRY_COMPARE_WITH_TIMEOUT(wire.lookups(), 1, 15000);
            QCOMPARE(wire.posted().value(0), envelope);
            QCOMPARE(horizon.pending(), 2);

            if (byEndpointChange)
                horizon.setEndpoint(elsewhere.url());
            else
                horizon.cancelAll();
            // The submit, until it is reported; the load is forgotten.
            QVERIFY2(horizon.pending() == 1, how);
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(horizon.pending(), 0);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), how);
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
            QVERIFY2(error.value(QStringLiteral("message")).toString().contains(QStringLiteral("look it up")),
                     qPrintable(error.value(QStringLiteral("message")).toString()));

            // Horizon's answers, when they come now, are no second report.
            wire.answerHeld();
            QTest::qWait(300);
            QCOMPARE(submitted.size(), 0);
            QCOMPARE(failed.size(), 1);
            QCOMPARE(loaded.size(), 0);
            QCOMPARE(elsewhere.posts() + elsewhere.lookups(), 0);
            QCOMPARE(wire.posts(), 1);

            if (byEndpointChange) {
                QString nextHash;
                const QString next = paymentTo(destination, /*memo*/ true, 6, &nextHash);
                elsewhere.setApplied(appliedAnswer(nextHash, next));
                const int after = horizon.submit(next, Network::TESTNET_NETWORK_PASSPHRASE());
                QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 1, 15000);
                QCOMPARE(submitted.first().at(0).toInt(), after);
                QCOMPARE(submitted.first().at(1).toMap().value(QStringLiteral("hash")).toString(), nextHash);
                QCOMPARE(elsewhere.posts(), 1);
                QCOMPARE(wire.posts(), 1);
                QCOMPARE(failed.size(), 1);
                QCOMPARE(horizon.pending(), 0);
            }
        }
    }

    /**
     * A HorizonQml deleted with a submit on the wire (its QML component torn
     * down mid-upload) hears nothing more from that POST. Since I8 a deleted
     * Server's manager and replies are deleted later, from the event loop,
     * so the reply outlives the HorizonQml; the upload watch was connected
     * with the reply as context, and the reply saying it had written its
     * body wrote to the deleted HorizonQml (a use-after-free; a crash with
     * MSVC's debug heap). Here the reply says so after the delete, before
     * the event loop runs again, and then goes.
     */
    void testHorizonDeletedWithASubmitOnTheWireHearsNothingMore()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        HeldHorizon wire;
        wire.holdPosts = true;
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
        wire.setApplied(appliedAnswer(hash, envelope));

        auto* horizon = new HorizonQml;
        horizon->setEndpoint(wire.url());
        horizon->submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        QTRY_COMPARE_WITH_TIMEOUT(wire.posts(), 1, 15000);      // read whole, held
        QPointer<QNetworkReply> post;
        const QList<QNetworkReply*> replies = horizon->findChildren<QNetworkReply*>();
        for (QNetworkReply* reply : replies) {
            if (reply->operation() == QNetworkAccessManager::PostOperation)
                post = reply;
        }
        QVERIFY(post);

        delete horizon;
        // Deleted later (I8): still there, and it may still say what it wrote.
        QVERIFY(post);
        emit post->uploadProgress(1, 1);
        wire.answerHeld();
        QTRY_VERIFY_WITH_TIMEOUT(!post, 15000);
        QCOMPARE(wire.posts(), 1);
    }

    /**
     * A submit whose answer had already reached this thread when
     * cancelAll() came is reported as that answer — submitted(), with the
     * hash and ledger Horizon gave — as without the cancel, and `pending`
     * counts it until then. The endpoint runs on a thread of its own, and
     * this thread takes no event between having Horizon answer and
     * cancelAll(), so the answer is queued here by then, given the time to
     * arrive: the wait grows over up to three submits, one of which must be
     * reported so. Every one of them is reported exactly once, and one
     * reported as cancelled says that it may have reached Horizon, with its
     * hash. The answer used to be dropped.
     */
    void testHorizonCancelAllStillReportsASubmitThatHadEnded()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        HorizonThread endpoint;
        endpoint.horizon->holdPosts = true;
        HorizonQml horizon;
        horizon.setEndpoint(endpoint.horizon->url());
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        int attempts = 0;
        bool ended = false;
        for (const int margin : { 300, 1000, 3000 }) {
            ++attempts;
            QString hash;
            const QString envelope = paymentTo(destination, /*memo*/ true, 10 + attempts, &hash);
            endpoint.horizon->setApplied(appliedAnswer(hash, envelope));
            submitted.clear();
            failed.clear();
            const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(endpoint.horizon->posts(), attempts, 15000);   // read whole, held
            endpoint.answerHeld();
            QThread::msleep(margin);            // takes no event: the answer queues up here
            horizon.cancelAll();
            QCOMPARE(horizon.pending(), 1);     // posted: kept until reported
            QTRY_COMPARE_WITH_TIMEOUT(horizon.pending(), 0, 15000);
            QTest::qWait(50);
            QCOMPARE(submitted.size() + failed.size(), 1);
            if (!submitted.isEmpty()) {
                QCOMPARE(submitted.first().at(0).toInt(), id);
                const QVariantMap result = submitted.first().at(1).toMap();
                QCOMPARE(result.value(QStringLiteral("hash")).toString(), hash);
                QCOMPARE(result.value(QStringLiteral("ledger")).toString(), QStringLiteral("4917043"));
                QVERIFY(result.value(QStringLiteral("successful")).toBool());
                ended = true;
                break;
            }
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            QVERIFY(error.value(QStringLiteral("mayHaveReachedServer")).toBool());
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
        }
        QVERIFY2(ended, "an answer already on its way when cancelAll() came was reported as cancelled");
    }

    /**
     * A submit cancelled while still in its SEP-29 lookup was not posted, and
     * never is: it leaves `pending` at once, nothing is reported of it, and
     * Horizon never sees its POST once the lookup is answered (the account
     * asks for no memo) — by cancelAll() or an endpoint change, and when the
     * lookup's answer had already reached this thread when cancelAll() came
     * (the endpoint on a thread of its own, this one taking no event in
     * between). It used to be posted all the same once the lookup answered,
     * as a submit nobody was waiting for: the payment was made and its
     * outcome dropped. Not cancelled, the same submit is posted once its
     * lookup passes, so the ones above were stopped, not refused.
     */
    void testHorizonASubmitCancelledInItsLookupIsNeverPosted()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        const QByteArray account = accountJson(destination, QStringLiteral("{}")).toUtf8();
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ false, 5, &hash);

        for (const bool byEndpointChange : { false, true }) {
            const char* how = byEndpointChange ? "endpoint change" : "cancelAll()";
            HeldHorizon wire;
            wire.holdLookups = true;
            wire.setAccount(account);
            wire.setApplied(appliedAnswer(hash, envelope));
            HeldHorizon elsewhere;
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(wire.lookups(), 1, 15000);   // read whole, held
            QCOMPARE(wire.posts(), 0);
            if (byEndpointChange)
                horizon.setEndpoint(elsewhere.url());
            else
                horizon.cancelAll();
            QVERIFY2(horizon.pending() == 0, how);                  // dropped at once
            wire.answerHeld();
            QTest::qWait(500);
            QVERIFY2(wire.posts() == 0, how);
            QCOMPARE(elsewhere.posts() + elsewhere.lookups(), 0);
            QVERIFY2(submitted.size() + failed.size() == 0, how);
            QCOMPARE(horizon.pending(), 0);
        }

        {
            HorizonThread endpoint;
            endpoint.horizon->holdLookups = true;
            endpoint.horizon->setAccount(account);
            endpoint.horizon->setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(endpoint.horizon->url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(endpoint.horizon->lookups(), 1, 15000);
            endpoint.answerHeld();
            QThread::msleep(500);               // takes no event: the answer queues up here
            horizon.cancelAll();
            QCOMPARE(horizon.pending(), 0);
            QTest::qWait(500);
            QCOMPARE(endpoint.horizon->posts(), 0);
            QCOMPARE(submitted.size() + failed.size(), 0);
        }

        // cancelAll() from a slot, followed by an event loop nested in it, as
        // a modal dialog or a wait for an answer runs one: the lookup is
        // answered in that loop, which deletes nothing deleted later (Qt
        // holds those for the loop that asked). A cancelled submit deleted
        // later was posted from there.
        {
            HeldHorizon wire;
            wire.holdLookups = true;
            wire.setAccount(account);
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(wire.lookups(), 1, 15000);
            QEventLoop outer;
            int pendingAfter = -1;
            QTimer::singleShot(0, &horizon, [&]() {
                horizon.cancelAll();
                pendingAfter = horizon.pending();
                wire.answerHeld();
                QEventLoop nested;
                QTimer::singleShot(1000, &nested, &QEventLoop::quit);
                nested.exec();
                outer.quit();
            });
            outer.exec();
            QCOMPARE(pendingAfter, 0);
            QCOMPARE(wire.posts(), 0);
            QCOMPARE(submitted.size() + failed.size(), 0);
        }

        HeldHorizon wire;
        wire.setAccount(account);
        wire.setApplied(appliedAnswer(hash, envelope));
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 1, 15000);
        QCOMPARE(submitted.first().at(0).toInt(), id);
        QCOMPARE(submitted.first().at(1).toMap().value(QStringLiteral("hash")).toString(), hash);
        QCOMPARE(wire.lookups(), 1);
        QCOMPARE(wire.posts(), 1);
        QCOMPARE(wire.posted().value(0), envelope);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * A cancel made from a handler of the pendingChanged a call emits as it
     * starts (cancelAll(), or a new endpoint) cancels that call before it has
     * asked anything: nothing reaches Horizon, nothing is reported, and
     * `pending` is 0 when the call returns. submit() used to go on regardless,
     * build a Server of its own and post: the transaction reached Horizon,
     * its outcome was dropped as a cancelled request's, and the Server stayed
     * until the facade died. (The first check is the reviewer's.)
     * loadAccount(), runQuery() and fundAccount() used to go on and ask
     * Horizon, or friendbot, all the same.
     */
    void testHorizonCancelFromPendingChangedInsideSubmit()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        for (const bool byEndpointChange : { false, true }) {
            const char* how = byEndpointChange ? "endpoint change" : "cancelAll()";
            HeldHorizon wire;
            wire.holdPosts = true;
            HeldHorizon elsewhere;
            QString hash;
            const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            bool armed = true;
            QObject::connect(&horizon, &HorizonQml::pendingChanged, &horizon, [&]() {
                if (armed && horizon.pending() > 0) {
                    armed = false;
                    if (byEndpointChange)
                        horizon.setEndpoint(elsewhere.url());
                    else
                        horizon.cancelAll();
                }
            });
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QVERIFY2(horizon.pending() == 0, how);
            // No Server was built for it: one was, and was never released.
            QVERIFY2(horizon.findChildren<Server*>().isEmpty(), how);
            QTest::qWait(1000);
            const int postsBefore = wire.posts();
            wire.answerHeld();
            QTest::qWait(500);
            QVERIFY2(postsBefore == 0 || submitted.size() + failed.size() == 1,
                     "a submit cancelled inside submit() was posted and never reported");
            QVERIFY2(wire.requests().isEmpty(), qPrintable(wire.requests().join(QStringLiteral(", "))));
            QVERIFY2(elsewhere.requests().isEmpty(), qPrintable(elsewhere.requests().join(QStringLiteral(", "))));
            QVERIFY2(submitted.size() + failed.size() == 0, how);
            QCOMPARE(horizon.pending(), 0);
        }

        // The other requests, the same way.
        HeldHorizon wire;
        wire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy loaded(&horizon, &HorizonQml::accountLoaded);
        QSignalSpy queried(&horizon, &HorizonQml::queried);
        QSignalSpy funded(&horizon, &HorizonQml::funded);
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        bool armed = false;
        QObject::connect(&horizon, &HorizonQml::pendingChanged, &horizon, [&]() {
            if (armed && horizon.pending() > 0) {
                armed = false;
                horizon.cancelAll();
            }
        });
        armed = true;
        horizon.loadAccount(destination);
        QCOMPARE(horizon.pending(), 0);
        QVariantMap network;
        network.insert(QStringLiteral("query"), QStringLiteral("network"));
        armed = true;
        horizon.runQuery(network);
        QCOMPARE(horizon.pending(), 0);
        // Friendbot is asked only for a test network's endpoint; this one is
        // never reached, and the friendbot is `wire`.
        horizon.setEndpoint(QStringLiteral("https://horizon-testnet.stellar.org"));
        horizon.setFriendbotUrl(wire.url() + QStringLiteral("/"));
        armed = true;
        horizon.fundAccount(destination);
        QCOMPARE(horizon.pending(), 0);
        QTest::qWait(500);
        QVERIFY2(wire.requests().isEmpty(), qPrintable(wire.requests().join(QStringLiteral(", "))));
        QCOMPARE(loaded.size() + queried.size() + funded.size() + failed.size(), 0);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * The passphrase given to submit() is checked against the network the
     * endpoint serves, as its root (GET /) names it, before anything else is
     * asked and before the transaction is posted; one that is not that
     * network's is refused: "validation", nothing sent, and no hash (the
     * roots are Horizon's own, testnet and public). A testnet envelope given
     * the public passphrase used to be posted to a testnet Horizon and, on a
     * cancel, reported under the hash the public passphrase gives it, which
     * testnet never files it under (the reviewer's case, first).
     *
     * On its own network it goes out, the root first, then SEP-29, then the
     * POST, and the network is remembered: the next submit asks no root. A
     * passphrase that differs from the one remembered has Horizon asked again
     * before it is refused (the Horizon behind the URL now serves another
     * network, and is right), and refused only on that fresh answer. An
     * endpoint change forgets it: the new endpoint is asked. An empty
     * passphrase is refused before anything is asked, and says that nothing
     * was sent.
     */
    void testHorizonSubmitIsRefusedForAnotherNetworkBeforePosting()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        const QString testnet = Network::TESTNET_NETWORK_PASSPHRASE();
        const QString pubnet = Network::PUBLIC_NETWORK_PASSPHRASE();
        const QString account = accountJson(destination, QStringLiteral("{}"));

        {
            HeldHorizon wire;
            QString hash;
            const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            const int id = horizon.submit(envelope, QString());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
            QVERIFY(saysNotSent(error));
            QTest::qWait(200);
            QVERIFY2(wire.requests().isEmpty(), qPrintable(wire.requests().join(QStringLiteral(", "))));
            QCOMPARE(submitted.size(), 0);
            QCOMPARE(horizon.pending(), 0);
        }

        struct Refused { QByteArray root; QString served; QString given; const char* what; };
        const QList<Refused> refused = {
            { HeldHorizon::testnetRoot(), testnet, pubnet, "a testnet Horizon given the public passphrase" },
            { HeldHorizon::pubnetRoot(), pubnet, testnet, "a public Horizon given the testnet passphrase" },
        };
        for (const Refused& c : refused) {
            HeldHorizon wire;
            wire.setRoot(c.root);
            wire.setAccount(account.toUtf8());
            QString hash;
            // Signed for testnet, and no memo: SEP-29 would look the
            // destination up, after the network, and does not get that far.
            const QString envelope = paymentTo(destination, /*memo*/ false, 5, &hash);
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            const int id = horizon.submit(envelope, c.given);
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            const QString message = error.value(QStringLiteral("message")).toString();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String("validation"), c.what);
            QVERIFY2(saysNotSent(error), c.what);
            QVERIFY2(message.contains(c.served) && message.contains(c.given), qPrintable(message));
            QCOMPARE(wire.requests(), QStringList{ QStringLiteral("GET /") });
            QCOMPARE(submitted.size(), 0);
            QCOMPARE(horizon.pending(), 0);
        }

        HeldHorizon wire;
        wire.setRoot(HeldHorizon::pubnetRoot());
        wire.setAccount(account.toUtf8());
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ false, 5, &hash, Network::publicNetwork());
        wire.setApplied(appliedAnswer(hash, envelope));
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        const int id = horizon.submit(envelope, pubnet);
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 1, 15000);
        QCOMPARE(submitted.first().at(0).toInt(), id);
        QCOMPARE(submitted.first().at(1).toMap().value(QStringLiteral("hash")).toString(), hash);
        QCOMPARE(wire.requests(), (QStringList{ QStringLiteral("GET /"),
                                                QStringLiteral("GET /accounts/") + destination,
                                                QStringLiteral("POST /transactions") }));
        QCOMPARE(wire.posted().value(0), envelope);

        // Remembered: no root asked this time.
        QString secondHash;
        const QString second = paymentTo(destination, /*memo*/ true, 6, &secondHash, Network::publicNetwork());
        wire.setApplied(appliedAnswer(secondHash, second));
        horizon.submit(second, pubnet);
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 2, 15000);
        QCOMPARE(wire.roots(), 1);

        // The Horizon behind the URL serves testnet now: a testnet submit
        // differs from the network remembered, so Horizon is asked again,
        // and says testnet.
        wire.setRoot(HeldHorizon::testnetRoot());
        QString thirdHash;
        const QString third = paymentTo(destination, /*memo*/ true, 7, &thirdHash);
        wire.setApplied(appliedAnswer(thirdHash, third));
        horizon.submit(third, testnet);
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 3, 15000);
        QCOMPARE(wire.roots(), 2);
        QCOMPARE(submitted.last().at(1).toMap().value(QStringLiteral("hash")).toString(), thirdHash);

        // A public submit now differs from testnet, remembered: asked once
        // more, still testnet, refused.
        horizon.submit(paymentTo(destination, /*memo*/ true, 8, nullptr, Network::publicNetwork()), pubnet);
        QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
        QCOMPARE(wire.roots(), 3);
        QCOMPARE(failed.first().at(1).toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("validation"));
        QVERIFY(saysNotSent(failed.first().at(1).toMap()));
        QCOMPARE(wire.posts(), 3);

        // A new endpoint is asked which network it serves.
        HeldHorizon other;
        QString fourthHash;
        const QString fourth = paymentTo(destination, /*memo*/ true, 9, &fourthHash);
        other.setApplied(appliedAnswer(fourthHash, fourth));
        horizon.setEndpoint(other.url());
        horizon.submit(fourth, testnet);
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 4, 15000);
        QCOMPARE(other.requests(), (QStringList{ QStringLiteral("GET /"), QStringLiteral("POST /transactions") }));
        QCOMPARE(failed.size(), 1);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * A submit whose endpoint's network cannot be read is not posted: its
     * root answered 503, 429, something that is not JSON, a root without a
     * network passphrase or with an empty one, or nothing within
     * Response::replyTimeout(). It fails with that lookup's kind, saying so,
     * and that nothing was sent. (No root was asked: it was posted.)
     */
    void testHorizonSubmitWhoseNetworkCannotBeReadIsNotPosted()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        const QByteArray named = "\"network_passphrase\": \"Test SDF Network ; September 2015\"";
        QByteArray empty = HeldHorizon::testnetRoot();
        empty.replace(named, "\"network_passphrase\": \"\"");
        QByteArray missing = HeldHorizon::testnetRoot();
        missing.replace("  " + named + ",\n", "");
        QVERIFY(empty != HeldHorizon::testnetRoot() && missing != HeldHorizon::testnetRoot());

        struct Case { QByteArray status; QByteArray body; const char* kind; };
        const QList<Case> cases = {
            { "503 Service Unavailable", "{}", "badResponse" },
            { "429 Too Many Requests", "{}", "rateLimited" },
            { "200 OK", "this is not JSON", "badResponse" },
            { "200 OK", missing, "badResponse" },
            { "200 OK", empty, "badResponse" },
            { "200 OK", QByteArray(), "timeout" },        // held: never answered
        };
        for (const Case& c : cases) {
            const bool hang = c.body.isNull();
            Response::setReplyTimeout(hang ? 500 : 0);
            auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
            HeldHorizon wire;
            wire.setRootReply(c.status, c.body);
            wire.holdRoots = hang;
            QString hash;
            const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            const QString message = error.value(QStringLiteral("message")).toString();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String(c.kind), qPrintable(message));
            QVERIFY2(saysNotSent(error), qPrintable(message));
            QVERIFY2(message.startsWith(QLatin1String("not submitted")) && message.contains(QLatin1String("GET /")),
                     qPrintable(message));
            QCOMPARE(wire.requests(), QStringList{ QStringLiteral("GET /") });
            wire.answerHeld();
            QTest::qWait(100);
            QCOMPARE(wire.posts(), 0);
            QCOMPARE(submitted.size() + failed.size(), 1);
            QCOMPARE(horizon.pending(), 0);
        }
    }

    /**
     * A submit cancelled while it asks the endpoint's network was not
     * posted, and never is: it leaves `pending` at once, nothing is reported,
     * and no POST reaches Horizon once the root is answered — by cancelAll()
     * or an endpoint change, with the root's answer already on this thread
     * when cancelAll() came (the endpoint on a thread of its own), and when
     * cancelAll() is followed by an event loop nested in the same slot. Not
     * cancelled, the same submit is posted once the root is answered.
     */
    void testHorizonASubmitCancelledInItsNetworkLookupIsNeverPosted()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);

        for (const bool byEndpointChange : { false, true }) {
            const char* how = byEndpointChange ? "endpoint change" : "cancelAll()";
            HeldHorizon wire;
            wire.holdRoots = true;
            wire.setApplied(appliedAnswer(hash, envelope));
            HeldHorizon elsewhere;
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(wire.roots(), 1, 15000);     // read whole, held
            if (byEndpointChange)
                horizon.setEndpoint(elsewhere.url());
            else
                horizon.cancelAll();
            QVERIFY2(horizon.pending() == 0, how);
            wire.answerHeld();
            QTest::qWait(500);
            QVERIFY2(wire.posts() == 0, how);
            QVERIFY2(elsewhere.requests().isEmpty(), how);
            QVERIFY2(submitted.size() + failed.size() == 0, how);
        }

        {
            HorizonThread endpoint;
            endpoint.horizon->holdRoots = true;
            endpoint.horizon->setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(endpoint.horizon->url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(endpoint.horizon->roots(), 1, 15000);
            endpoint.answerHeld();
            QThread::msleep(500);               // takes no event: the answer queues up here
            horizon.cancelAll();
            QCOMPARE(horizon.pending(), 0);
            QTest::qWait(500);
            QCOMPARE(endpoint.horizon->posts(), 0);
            QCOMPARE(submitted.size() + failed.size(), 0);
        }

        {
            HeldHorizon wire;
            wire.holdRoots = true;
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(wire.roots(), 1, 15000);
            QEventLoop outer;
            int pendingAfter = -1;
            QTimer::singleShot(0, &horizon, [&]() {
                horizon.cancelAll();
                pendingAfter = horizon.pending();
                wire.answerHeld();
                QEventLoop nested;
                QTimer::singleShot(1000, &nested, &QEventLoop::quit);
                nested.exec();
                outer.quit();
            });
            outer.exec();
            QCOMPARE(pendingAfter, 0);
            QCOMPARE(wire.posts(), 0);
            QCOMPARE(submitted.size() + failed.size(), 0);
        }

        HeldHorizon wire;
        wire.holdRoots = true;
        wire.setApplied(appliedAnswer(hash, envelope));
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        QTRY_COMPARE_WITH_TIMEOUT(wire.roots(), 1, 15000);
        QCOMPARE(wire.posts(), 0);
        wire.answerHeld();
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 1, 15000);
        QCOMPARE(submitted.first().at(0).toInt(), id);
        QCOMPARE(wire.posted().value(0), envelope);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * Every failure of a submit that was posted says it may have reached
     * Horizon, with the transaction's hash to look it up by, as a cancel's
     * does: Horizon's 504, a 500 or 503, an answer that cannot be read (a
     * 200 included), Horizon's own refusals (tx_bad_seq, a 429), the SDK's
     * own timeout (Horizon read the POST whole and never answered), and a
     * connection lost after the POST was read. They carried neither, so a
     * wallet could not tell a payment that may have landed from one that
     * never left. A POST whose connection never opened is known not to have
     * left, and says so, with no hash.
     */
    void testHorizonEveryFailureOfAPostedSubmitSaysItMayHaveLanded()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
        const QByteArray problem = "application/problem+json; charset=utf-8";
        const QByteArray hal = "application/hal+json; charset=utf-8";

        struct Case { const char* what; QByteArray status; QByteArray type; QByteArray body; const char* kind; };
        const QList<Case> answers = {
            { "Horizon's 504", "504 Gateway Timeout", problem,
              "{\"type\":\"https://stellar.org/horizon-errors/timeout\",\"title\":\"Timeout\",\"status\":504,"
              "\"detail\":\"Your request timed out before completing.\"}", "timeout" },
            { "a 500", "500 Internal Server Error", problem,
              "{\"type\":\"https://stellar.org/horizon-errors/server_error\",\"title\":\"Internal Server Error\","
              "\"status\":500}", "badResponse" },
            { "a 503", "503 Service Unavailable", hal, "{}", "badResponse" },
            { "an unreadable 200", "200 OK", hal, "this is not JSON", "badResponse" },
            { "tx_bad_seq", "400 Bad Request", problem, badSeqAnswer(envelope), "txFailed" },
            { "a 429", "429 Too Many Requests", problem,
              "{\"type\":\"https://stellar.org/horizon-errors/rate_limit_exceeded\",\"title\":\"Rate Limit Exceeded\","
              "\"status\":429}", "rateLimited" },
        };
        for (const Case& c : answers) {
            HeldHorizon wire;
            wire.setPostReply(c.status, c.type, c.body);
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String(c.kind), c.what);
            QVERIFY2(saysMayHaveLanded(error, hash), c.what);
            // The answer to the one send: final for "txFailed" and "rateLimited".
            QVERIFY2(!error.contains(QStringLiteral("resent")), c.what);
            if (QLatin1String(c.kind) == QLatin1String("txFailed"))
                QCOMPARE(error.value(QStringLiteral("txResultCode")).toString(), QStringLiteral("tx_bad_seq"));
            QCOMPARE(wire.posts(), 1);
            QCOMPARE(submitted.size(), 0);
            QCOMPARE(horizon.pending(), 0);
        }

        {
            // Read whole, never answered: the SDK gives up.
            Response::setReplyTimeout(500);
            auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
            HeldHorizon wire;
            wire.holdPosts = true;
            wire.setApplied(appliedAnswer(hash, envelope));
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy submitted(&horizon, &HorizonQml::submitted);
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("timeout"));
            QVERIFY(saysMayHaveLanded(error, hash));
            QCOMPARE(wire.posts(), 1);
            wire.answerHeld();
            QTest::qWait(200);
            QCOMPARE(submitted.size() + failed.size(), 1);
        }

        {
            // The connection closed on it once Horizon had read it whole.
            HeldHorizon wire;
            wire.dropPosts = true;
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            const QVariantMap error = failed.first().at(1).toMap();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String("connection"),
                     qPrintable(error.value(QStringLiteral("message")).toString()));
            QVERIFY(saysMayHaveLanded(error, hash));
            QVERIFY(wire.posts() >= 1);
            // Qt 5 sends it again, and each is read whole: said, whatever the kind.
            if (wire.posts() > 1)
                QCOMPARE(error.value(QStringLiteral("resent")), QVariant(true));
            else
                QVERIFY(!error.contains(QStringLiteral("resent")));
            QCOMPARE(horizon.pending(), 0);
        }

        {
            // Posted to a connection that was refused: it never left.
            HeldHorizon wire;
            wire.refuseAfterRoot = true;
            HorizonQml horizon;
            horizon.setEndpoint(wire.url());
            QSignalSpy failed(&horizon, &HorizonQml::failed);
            horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            const QVariantMap error = failed.first().at(1).toMap();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String("connection"),
                     qPrintable(error.value(QStringLiteral("message")).toString()));
            QVERIFY(saysNotSent(error));
            QCOMPARE(wire.requests(), QStringList{ QStringLiteral("GET /") });
            QCOMPARE(horizon.pending(), 0);
        }
    }

    /**
     * Horizon read the POST whole, then its connection closed (aborted, or
     * closed in good order) with no answer, and Horizon went away (stopped
     * listening). Qt 5 sends the POST again by itself, on a new connection,
     * which is refused, so the request ends in "connection refused"; Qt 6
     * reports the connection closed. Either way the transaction reached
     * Horizon and may land: the failure says so, with its hash. That holds
     * for the POST made at once (a memo: no SEP-29 lookup) and for the one
     * the Server makes itself once a SEP-29 lookup has passed. On Qt 5 it
     * said "not sent", with no hash: a refused connection was taken to mean
     * the POST never left, and a wallet told so may pay again.
     */
    void testHorizonAPostRefusedWhenSentAgainMayHaveLanded_data()
    {
        QTest::addColumn<bool>("graceful");
        QTest::addColumn<bool>("memo");
        QTest::newRow("abort, memo") << false << true;
        QTest::newRow("close, memo") << true << true;
        QTest::newRow("abort, sep29") << false << false;
    }
    void testHorizonAPostRefusedWhenSentAgainMayHaveLanded()
    {
        QFETCH(bool, graceful);
        QFETCH(bool, memo);
        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, memo, 5, &hash);
        HeldHorizon wire;
        wire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
        wire.dropPosts = true;
        wire.refuseAfterDrop = true;
        wire.closeGracefully = graceful;
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        QTRY_COMPARE_WITH_TIMEOUT(failed.size() + submitted.size(), 1, 15000);
        QCOMPARE(failed.size(), 1);
        QCOMPARE(failed.first().at(0).toInt(), id);
        const QVariantMap error = failed.first().at(1).toMap();
        const QString message = error.value(QStringLiteral("message")).toString();
        // Read whole by Horizon, once; the second connection never opened.
        QCOMPARE(wire.posts(), 1);
        QCOMPARE(wire.posted().value(0), envelope);
        const QStringList expected = memo
            ? QStringList{ QStringLiteral("GET /"), QStringLiteral("POST /transactions") }
            : QStringList{ QStringLiteral("GET /"), QStringLiteral("GET /accounts/") + destination,
                           QStringLiteral("POST /transactions") };
        QCOMPARE(wire.requests(), expected);
        QVERIFY2(error.value(QStringLiteral("kind")).toString() == QLatin1String("connection"),
                 qPrintable(message));
        QVERIFY2(saysMayHaveLanded(error, hash),
                 qPrintable(QStringLiteral("Horizon read the POST whole, yet mayHaveReachedServer=%1 hash=%2: %3")
                                .arg(error.value(QStringLiteral("mayHaveReachedServer")).toString(),
                                     error.value(QStringLiteral("hash")).toString(), message)));
        // Qt connected again but wrote nothing more: not `resent`.
        QVERIFY2(!error.contains(QStringLiteral("resent")), qPrintable(message));
        QTest::qWait(200);
        QCOMPARE(failed.size() + submitted.size(), 1);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * Horizon read the POST whole and its connection closed with no answer,
     * so Horizon may have applied it; Qt 5 sends it again, by itself, and
     * Horizon answers that second POST. A 429, or a 400 tx_bad_seq (what
     * Horizon answers a transaction sent again once the first has been
     * applied, before it has taken in that ledger), answers the second send
     * only: the failure keeps Horizon's kind and codes, and says `resent`,
     * with mayHaveReachedServer true, the hash, and a message that starts
     * "not final" and names the hash, so the wallet looks the transaction up
     * before anything else. A 200 is the transaction's own outcome, and is
     * submitted(). A cancel while the second POST is on the wire says
     * `resent` too, and its message starts "not final" as well (it was the
     * cancel's alone, and the others named no hash). That
     * holds for the POST made at once (a memo) and for the one the Server
     * makes once a SEP-29 lookup has passed. The 429 and the tx_bad_seq were
     * reported as Horizon's final word ("this submit will not make it
     * succeed"), and a wallet that built the payment again with a fresh
     * sequence number paid twice. Qt 6 does not send it again: the failure
     * is then "connection", with the hash, and not `resent`.
     */
    void testHorizonAnAnswerToAPostQtSentAgainIsNotFinal_data()
    {
        QTest::addColumn<QString>("answer");
        QTest::addColumn<bool>("memo");
        QTest::newRow("429, memo") << QStringLiteral("429") << true;
        QTest::newRow("tx_bad_seq, memo") << QStringLiteral("tx_bad_seq") << true;
        QTest::newRow("200, memo") << QStringLiteral("200") << true;
        QTest::newRow("cancel, memo") << QStringLiteral("cancel") << true;
        QTest::newRow("429, sep29") << QStringLiteral("429") << false;
        QTest::newRow("tx_bad_seq, sep29") << QStringLiteral("tx_bad_seq") << false;
        QTest::newRow("200, sep29") << QStringLiteral("200") << false;
    }
    void testHorizonAnAnswerToAPostQtSentAgainIsNotFinal()
    {
        QFETCH(QString, answer);
        QFETCH(bool, memo);
        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, memo, 5, &hash);
        HeldHorizon wire;
        wire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
        wire.dropFirstPosts = 1;
        const QByteArray problem = "application/problem+json; charset=utf-8";
        if (answer == QLatin1String("429"))
            wire.setPostReply("429 Too Many Requests", problem,
                              "{\"type\":\"https://stellar.org/horizon-errors/rate_limit_exceeded\","
                              "\"title\":\"Rate Limit Exceeded\",\"status\":429}");
        else if (answer == QLatin1String("tx_bad_seq"))
            wire.setPostReply("400 Bad Request", problem, badSeqAnswer(envelope));
        else
            wire.setApplied(appliedAnswer(hash, envelope));
        const bool cancel = answer == QLatin1String("cancel");
        wire.holdPosts = cancel;        // the second POST, read whole and held
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        if (cancel) {
            QTRY_VERIFY_WITH_TIMEOUT(wire.posts() == 2 || failed.size() + submitted.size() > 0, 15000);
            if (wire.posts() == 2) {
                QTest::qWait(100);      // what the connection said it wrote, delivered
                QCOMPARE(failed.size() + submitted.size(), 0);
                horizon.cancelAll();
            }
        }
        QTRY_COMPARE_WITH_TIMEOUT(failed.size() + submitted.size(), 1, 15000);
        QTest::qWait(200);
        QCOMPARE(failed.size() + submitted.size(), 1);
        QCOMPARE(horizon.pending(), 0);

        const int sends = wire.posts();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCOMPARE(sends, 2);             // Qt 5 sends it again, and the second is answered
#endif
        QStringList expected{ QStringLiteral("GET /") };
        if (!memo)
            expected << QStringLiteral("GET /accounts/") + destination;
        for (int i = 0; i < sends; ++i)
            expected << QStringLiteral("POST /transactions");
        QCOMPARE(wire.requests(), expected);
        for (const QString& tx : wire.posted())
            QCOMPARE(tx, envelope);

        if (sends == 1) {
            // Not sent again: the connection closed on it, and it may land.
            QCOMPARE(failed.size(), 1);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("connection"));
            QVERIFY(saysMayHaveLanded(error, hash));
            QVERIFY(!error.contains(QStringLiteral("resent")));
            return;
        }
        QCOMPARE(sends, 2);

        if (answer == QLatin1String("200")) {
            // Horizon says the transaction is in a ledger: that is final.
            QCOMPARE(failed.size(), 0);
            QCOMPARE(submitted.first().at(0).toInt(), id);
            const QVariantMap result = submitted.first().at(1).toMap();
            QCOMPARE(result.value(QStringLiteral("hash")).toString(), hash);
            QCOMPARE(result.value(QStringLiteral("successful")), QVariant(true));
            return;
        }

        QCOMPARE(failed.size(), 1);
        QCOMPARE(failed.first().at(0).toInt(), id);
        const QVariantMap error = failed.first().at(1).toMap();
        const QString message = error.value(QStringLiteral("message")).toString();
        QVERIFY2(error.value(QStringLiteral("resent")) == QVariant(true),
                 qPrintable(QStringLiteral("Horizon read the first POST whole, and answered Qt's second, "
                                           "yet the %1 was not said to be resent: %2")
                                .arg(error.value(QStringLiteral("kind")).toString(), message)));
        QVERIFY2(saysMayHaveLanded(error, hash), qPrintable(message));
        // Whatever the kind, the message says it is not final and names the
        // hash to look up (a cancel's said neither, and the rest no hash).
        QVERIFY2(message.startsWith(QLatin1String("not final: "))
                     && message.contains(QStringLiteral("by its hash, ") + hash),
                 qPrintable(message));
        if (answer == QLatin1String("cancel")) {
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            QVERIFY2(message.contains(QLatin1String("cancelled while it was on the wire")),
                     qPrintable(message));
            return;
        }
        if (answer == QLatin1String("429")) {
            // Horizon's kind and status are kept.
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("rateLimited"));
            QCOMPARE(error.value(QStringLiteral("status")).toInt(), 429);
        } else {
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("txFailed"));
            QCOMPARE(error.value(QStringLiteral("txResultCode")).toString(), QStringLiteral("tx_bad_seq"));
            QVERIFY2(message.endsWith(QLatin1String("tx_bad_seq")), qPrintable(message));
        }
    }

    /**
     * An answer that sets a cookie (as a proxy in front of Horizon may, on
     * every answer) has the submit's QNetworkAccessManager make its cookie
     * jar: a child of the manager that is not a reply. The watch on what the
     * connection writes of the manager's replies passes it over without a
     * warning, and the submit goes through as ever.
     */
    void testHorizonSubmitToAHorizonThatSetsACookie()
    {
        static QStringList connectWarnings;
        static QtMessageHandler previous = nullptr;
        connectWarnings.clear();
        previous = qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context,
                                             const QString& message) {
            if (message.contains(QLatin1String("QObject::connect")))
                connectWarnings.append(message);
            if (previous)
                previous(type, context, message);
        });
        auto restore = qScopeGuard([] { qInstallMessageHandler(previous); });

        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ false, 5, &hash);
        HeldHorizon wire;
        wire.setExtraHeaders("Set-Cookie: session=abc123; Path=/; HttpOnly\r\n");
        wire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
        wire.setApplied(appliedAnswer(hash, envelope));
        wire.holdPosts = true;
        HorizonQml horizon;
        horizon.setEndpoint(wire.url());
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        const int id = horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());
        QTRY_COMPARE_WITH_TIMEOUT(wire.posts(), 1, 15000);
        // The manager has made its jar by now, the root's answer set a cookie.
        QVERIFY(!horizon.findChildren<QNetworkCookieJar*>().isEmpty());
        wire.answerHeld();
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size(), 1, 15000);
        QCOMPARE(submitted.first().at(0).toInt(), id);
        QCOMPARE(failed.size(), 0);
        QCOMPARE(wire.requests(), (QStringList{ QStringLiteral("GET /"),
                                                QStringLiteral("GET /accounts/") + destination,
                                                QStringLiteral("POST /transactions") }));
        QVERIFY2(connectWarnings.isEmpty(), qPrintable(connectWarnings.join(QStringLiteral("; "))));
    }

    /**
     * An endpoint change forgets the network the old endpoint serves before
     * it cancels, so a submit started from a handler of that cancel's
     * pendingChanged asks the new endpoint which network it serves, even
     * with the passphrase remembered for the old one. Here the old endpoint
     * is testnet (remembered from a submit), the new one serves the public
     * network (its real root), and the submit is a testnet one: the new
     * endpoint is asked GET / and nothing else, and the submit is refused
     * ("validation", not sent). With the cancel first, the submit found
     * testnet remembered and posted a testnet transaction to the public
     * network's Horizon at once.
     */
    void testHorizonSubmitFromAnEndpointChangesCancelAsksTheNewEndpoint()
    {
        const QString destination = QString::fromLatin1(ISSUER());
        QString hash;
        const QString envelope = paymentTo(destination, /*memo*/ true, 5, &hash);
        HeldHorizon oldWire;                                // testnet
        oldWire.setApplied(appliedAnswer(hash, envelope));
        oldWire.setAccount(accountJson(destination, QStringLiteral("{}")).toUtf8());
        HeldHorizon newWire;
        newWire.setRoot(HeldHorizon::pubnetRoot());         // the public network
        HorizonQml horizon;
        horizon.setEndpoint(oldWire.url());
        QSignalSpy failed(&horizon, &HorizonQml::failed);
        QSignalSpy submitted(&horizon, &HorizonQml::submitted);
        horizon.submit(envelope, Network::TESTNET_NETWORK_PASSPHRASE());   // testnet, remembered
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size() + failed.size(), 1, 15000);
        QCOMPARE(submitted.size(), 1);
        QCOMPARE(oldWire.roots(), 1);
        failed.clear();
        submitted.clear();

        // A request under way, so that the endpoint change's cancel changes
        // `pending`.
        oldWire.holdLookups = true;
        horizon.loadAccount(destination);
        QTRY_COMPARE_WITH_TIMEOUT(oldWire.lookups(), 1, 15000);
        bool armed = true;
        int id = -1;
        QString secondHash;
        const QString second = paymentTo(destination, /*memo*/ true, 6, &secondHash);
        QObject::connect(&horizon, &HorizonQml::pendingChanged, &horizon, [&]() {
            if (armed && horizon.pending() == 0) {
                armed = false;
                id = horizon.submit(second, Network::TESTNET_NETWORK_PASSPHRASE());
            }
        });
        horizon.setEndpoint(newWire.url());
        QVERIFY(!armed);
        QTRY_COMPARE_WITH_TIMEOUT(submitted.size() + failed.size(), 1, 15000);
        QTest::qWait(200);
        QVERIFY2(newWire.requests() == QStringList{ QStringLiteral("GET /") },
                 qPrintable(newWire.requests().join(QStringLiteral(", "))));
        QCOMPARE(newWire.posts(), 0);
        QCOMPARE(submitted.size(), 0);
        QCOMPARE(failed.size(), 1);
        QCOMPARE(failed.first().at(0).toInt(), id);
        const QVariantMap error = failed.first().at(1).toMap();
        QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
        QVERIFY(saysNotSent(error));
        QCOMPARE(oldWire.posts(), 1);
        QCOMPARE(oldWire.roots(), 1);
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * An account with a signer added before protocol 10, whose weight is past
     * a byte (Horizon sends 2^31 and past as a negative int32), loads, and its
     * weight reaches QML as it is. Such an account used to fail as
     * "badResponse": weights were held to 0..255.
     */
    void testHorizonAccountWithAHeavySignerLoads()
    {
        const QString account = QString::fromLatin1(ACCOUNT());
        for (const qint64 weight : { Q_INT64_C(256), Q_INT64_C(2147483647), Q_INT64_C(-1294967296) }) {
            FakeServer* fake = new FakeServer();
            fake->addGet(QStringLiteral("/accounts/") + account,
                         accountJson(account, QStringLiteral("{}"), QString::number(weight)));
            HorizonQml horizon;
            horizon.setEndpoint(fake->baseUrl());
            int answers = 0;
            QVariantMap loaded, failure;
            QObject::connect(&horizon, &HorizonQml::accountLoaded, [&](int, const QVariantMap& a) { ++answers; loaded = a; });
            QObject::connect(&horizon, &HorizonQml::failed, [&](int, const QVariantMap& e) { ++answers; failure = e; });
            horizon.loadAccount(account);
            WAIT_FOR(answers == 0)
            QVERIFY2(failure.isEmpty(), qPrintable(failure.value(QStringLiteral("message")).toString()));
            const QVariantList signers = loaded.value(QStringLiteral("signers")).toList();
            QCOMPARE(signers.size(), 1);
            QCOMPARE(signers.at(0).toMap().value(QStringLiteral("weight")).toLongLong(), weight);
            // Horizon sends no is_authorized for XLM; it read authorized: false.
            const QVariantMap native = loaded.value(QStringLiteral("balances")).toList().at(0).toMap();
            QCOMPARE(native.value(QStringLiteral("assetType")).toString(), QStringLiteral("native"));
            QVERIFY(!native.contains(QStringLiteral("authorized")));
            fake->deleteLater();
        }
    }

    /**
     * The friendbot is the one of the endpoint's network, chosen by its host:
     * "contains testnet" refused futurenet and a local quickstart and sent
     * any URL with "testnet" in it to SDF's friendbot. friendbotUrl, now a
     * public property, sets another; a public-network endpoint has none, and
     * any other is asked at its own /friendbot.
     */
    void testFriendbotFollowsTheEndpointsNetwork()
    {
        HorizonQml horizon;
        QCOMPARE(horizon.friendbotUrlFor(QStringLiteral("https://horizon-testnet.stellar.org")),
                 QStringLiteral("https://friendbot.stellar.org"));
        QCOMPARE(horizon.friendbotUrlFor(QStringLiteral("https://horizon-futurenet.stellar.org/")),
                 QStringLiteral("https://friendbot-futurenet.stellar.org"));
        QVERIFY(horizon.friendbotUrlFor(QStringLiteral("https://horizon.stellar.org")).isEmpty());
        QCOMPARE(horizon.friendbotUrlFor(QStringLiteral("https://my-testnet-mirror.example")),
                 QStringLiteral("https://my-testnet-mirror.example/friendbot"));
        QCOMPARE(horizon.friendbotUrlFor(QStringLiteral("http://localhost:8000/")),
                 QStringLiteral("http://localhost:8000/friendbot"));
        QVERIFY(horizon.friendbotUrlFor(QStringLiteral("not a url")).isEmpty());

        horizon.setEndpoint(QStringLiteral("https://horizon.stellar.org"));
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::failed, [&](int, const QVariantMap& e) { failure = e; });
        horizon.fundAccount(QString::fromLatin1(ACCOUNT()));
        QTRY_VERIFY(!failure.isEmpty());
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
        QVERIFY(failure.value(QStringLiteral("message")).toString().contains(QStringLiteral("friendbotUrl")));
        QCOMPARE(horizon.pending(), 0);
    }

    /**
     * fundAccount() on an endpoint that is no SDF Horizon asks that Horizon's
     * own /friendbot, as js-stellar-sdk's Horizon.Server.friendbot() does: a
     * local quickstart serves one there, and was refused unless friendbotUrl
     * was set. A query friendbotUrl carries is kept (it was replaced by addr
     * alone), and a contract address is refused, not sent.
     */
    void testFundAccountAsksTheEndpointsOwnFriendbot()
    {
        const QString account = QStringLiteral("GDKPIPRTM3INIB6NAYXRPNVODC6KH5VAUIEZAZMDLFASHHOGXMZXTGR2");
        FakeServer* fake = new FakeServer();
        FakeServer::Reply reply;
        reply.body = friendbotAnswer();
        reply.responseCode = QStringLiteral("200 OK");
        reply.contentType = QStringLiteral("application/hal+json; charset=utf-8");
        fake->addGetFull(QStringLiteral("/friendbot?addr=") + account, reply);
        fake->addGetFull(QStringLiteral("/fund?token=x&addr=") + account, reply);

        HorizonQml horizon;
        horizon.setEndpoint(fake->baseUrl());
        int funded = 0;
        QVariantMap failure;
        QObject::connect(&horizon, &HorizonQml::funded, [&](int, const QVariantMap&) { ++funded; });
        QObject::connect(&horizon, &HorizonQml::failed, [&](int, const QVariantMap& e) { failure = e; });
        horizon.fundAccount(account);
        QTRY_VERIFY(funded == 1 || !failure.isEmpty());
        QVERIFY2(failure.isEmpty(), qPrintable(failure.value(QStringLiteral("message")).toString()));

        horizon.setFriendbotUrl(fake->baseUrl() + QStringLiteral("/fund?token=x"));
        horizon.fundAccount(account);
        QTRY_VERIFY(funded == 2 || !failure.isEmpty());
        QVERIFY2(failure.isEmpty(), qPrintable(failure.value(QStringLiteral("message")).toString()));

        horizon.fundAccount(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        QTRY_VERIFY(!failure.isEmpty());
        QCOMPARE(failure.value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
        QCOMPARE(funded, 2);
        QCOMPARE(horizon.pending(), 0);
        fake->deleteLater();
    }

    /**
     * Friendbot's answer, as it sends it: funded, with the transaction's
     * hash, and no ledger (today's friendbot does not wait for one), which
     * used to be reported as ledger "0". A ledger it does name is read
     * exactly; one that is not a whole number is a "badResponse".
     */
    void testFriendbotAnswerIsRead()
    {
        bool funded = false;
        QVariantMap out = fundWith(QStringLiteral("200 OK"), friendbotAnswer(), false, &funded);
        QVERIFY2(funded, qPrintable(out.value(QStringLiteral("message")).toString()));
        QCOMPARE(out.value(QStringLiteral("_answers")).toInt(), 1);
        QCOMPARE(out.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("c737c3f100abdee0a3721dd4f93f14bc16860cebc3b5bb76874789114b85606b"));
        QCOMPARE(out.value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GDKPIPRTM3INIB6NAYXRPNVODC6KH5VAUIEZAZMDLFASHHOGXMZXTGR2"));
        QVERIFY(!out.contains(QStringLiteral("ledger")));
        QCOMPARE(out.value(QStringLiteral("_pending")).toInt(), 0);

        QString withLedger = friendbotAnswer();
        withLedger.replace(QStringLiteral("\"successful\": true,"), QStringLiteral("\"successful\": true, \"ledger\": 4928166,"));
        out = fundWith(QStringLiteral("200 OK"), withLedger, false, &funded);
        QVERIFY(funded);
        QCOMPARE(out.value(QStringLiteral("ledger")).toString(), QStringLiteral("4928166"));

        QString badLedger = friendbotAnswer();
        badLedger.replace(QStringLiteral("\"successful\": true,"), QStringLiteral("\"successful\": true, \"ledger\": \"4928166th\","));
        out = fundWith(QStringLiteral("200 OK"), badLedger, false, &funded);
        QVERIFY(!funded);
        QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
    }

    /**
     * A 200 that is not friendbot's report of a funded account (no hash, a
     * hash that is not one, not successful, not JSON) was reported as funded,
     * with an empty hash. It is a "badResponse" now.
     */
    void testFriendbotAnswerWithoutItsTransactionIsNotFunded()
    {
        const QString hash = QStringLiteral("\"hash\": \"c737c3f100abdee0a3721dd4f93f14bc16860cebc3b5bb76874789114b85606b\"");
        const QStringList bodies = QStringList()
            << QStringLiteral("{}")
            << QStringLiteral("{\"successful\": true}")
            << QStringLiteral("{\"successful\": true, \"hash\": \"c737c3f1\"}")
            << QStringLiteral("{\"successful\": true, \"hash\": 5}")
            << QStringLiteral("{%1}").arg(hash)
            << QStringLiteral("{\"successful\": false, %1}").arg(hash)
            << QStringLiteral("{\"successful\": \"true\", %1}").arg(hash)
            << QStringLiteral("funded!")
            << QString();
        for (const QString& body : bodies) {
            bool funded = true;
            const QVariantMap out = fundWith(QStringLiteral("200 OK"), body, false, &funded);
            QVERIFY2(!funded, qPrintable(body));
            QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
            QCOMPARE(out.value(QStringLiteral("_answers")).toInt(), 1);
            QCOMPARE(out.value(QStringLiteral("_pending")).toInt(), 0);
        }
    }

    /**
     * Friendbot's refusals keep their detail, and a failure of friendbot
     * itself is a "badResponse" (it was "connection", which says the request
     * never went out).
     */
    void testFriendbotRefusalsAreClassified()
    {
        bool funded = true;
        QVariantMap out = fundWith(QStringLiteral("400 Bad Request"),
            QStringLiteral("{\"type\": \"https://stellar.org/friendbot-errors/bad_request\", \"title\": \"Bad Request\","
                           " \"status\": 400, \"detail\": \"account already funded to starting balance\"}"),
            false, &funded);
        QVERIFY(!funded);
        QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("badRequest"));
        QCOMPARE(out.value(QStringLiteral("message")).toString(), QStringLiteral("account already funded to starting balance"));
        out = fundWith(QStringLiteral("500 Internal Server Error"), QStringLiteral("{}"), false, &funded);
        QVERIFY(!funded);
        QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("badResponse"));
        QCOMPARE(out.value(QStringLiteral("status")).toInt(), 500);
        out = fundWith(QStringLiteral("429 Too Many Requests"), QStringLiteral("{}"), false, &funded);
        QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("rateLimited"));
    }

    /**
     * A friendbot that takes the request and never answers is given up on
     * after Response::replyTimeout(), as a "timeout" (the account may still
     * be funded). It used to stay pending for ever.
     */
    void testFriendbotThatNeverAnswersTimesOut()
    {
        Response::setReplyTimeout(300);
        auto restoreTimeout = qScopeGuard([] { Response::setReplyTimeout(0); });
        bool funded = true;
        const QVariantMap out = fundWith(QString(), QString(), true, &funded);
        QVERIFY(!funded);
        QCOMPARE(out.value(QStringLiteral("_answers")).toInt(), 1);
        QCOMPARE(out.value(QStringLiteral("kind")).toString(), QStringLiteral("timeout"));
        QCOMPARE(out.value(QStringLiteral("_pending")).toInt(), 0);
    }
};

ADD_TEST(StellarQmlTest)
#endif // STELLARQMLTEST_H
