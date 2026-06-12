#ifndef FAKESERVER_H
#define FAKESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QThread>

/**
 * @brief The FakeServer class
 *
 * Implements a simple server for tests
 */
class FakeServer : public QObject{

    Q_OBJECT
public:
    struct Reply {
        QString body;
        QString responseCode = "200 OK";
        QString contentType = "application/hal+json; charset=\"utf-8\"";
        QString extraHeaders; // each line must end with \r\n
    };
private:
    QTcpServer* m_server;
    QHash<QString, Reply> m_getResponses;
    QHash<QString, Reply> m_postResponses;
    QList<QTcpSocket*> m_clients;

    // --- SSE / streaming support ---
    struct Stream { QStringList rounds; int served = 0; };
    QHash<QString, Stream> m_streams;   // keyed by path (query string stripped)
    QByteArray m_lastRequestHeaders;    // accumulated raw headers of all stream requests
public:
    /**
     * Default port = 0 → ask the OS for any free ephemeral port. The actually
     * bound port is exposed via `port()`. Pass an explicit port if a test
     * still wants a fixed one (no longer recommended; ports may stay in
     * TIME_WAIT between consecutive tests on Windows).
     *
     * If `port != 0` and that port is already taken, falls back to 0 so the
     * test still gets a working listener instead of binding silently failing.
     */
    FakeServer(quint16 port=0, QObject* parent = nullptr):QObject(parent)
    {
        m_server= new QTcpServer(this);
        m_server->setMaxPendingConnections(100);
        connect(m_server, &QTcpServer::newConnection, this, &FakeServer::incomingConnection);
        if (!m_server->listen(QHostAddress::LocalHost, port)) {
            // Requested port busy — retry with ephemeral.
            m_server->listen(QHostAddress::LocalHost, 0);
        }
    }

    /** Returns the actual port the listener is bound to (resolved when port=0). */
    quint16 port() const { return m_server->serverPort(); }

    /** Returns the canonical base URL (`http://localhost:<port>`) — pass to Server. */
    QString baseUrl() const { return QStringLiteral("http://localhost:%1").arg(m_server->serverPort()); }
    virtual ~FakeServer()
    {
        for(QTcpSocket * c :m_clients)
        {
            c->disconnectFromHost();
            if(c->state()!=QTcpSocket::UnconnectedState)
            {
                c->waitForDisconnected();
            }
        }
        m_server->close();
        delete m_server;
    }

    void addGet(QString key, QString value, QString responseCode="200 OK")
    {
        Reply r;
        r.body = value;
        r.responseCode = responseCode;
        m_getResponses.insert(key, r);
    }
    void addPost(QString key, QString value, QString responseCode="200 OK")
    {
        Reply r;
        r.body = value;
        r.responseCode = responseCode;
        m_postResponses.insert(key, r);
    }
    /** Add GET reply with custom Content-Type and additional headers (each line ending in \r\n). */
    void addGetFull(QString key, Reply reply)
    {
        m_getResponses.insert(key, reply);
    }
    void addPostFull(QString key, Reply reply)
    {
        m_postResponses.insert(key, reply);
    }

    /**
     * Register an SSE (`text/event-stream`) endpoint. Each entry in `rounds` is the
     * full SSE body served on ONE connection; the SDK reconnects after the stream
     * closes (sending `Last-Event-ID`), so successive connections get successive
     * rounds (rounds beyond the list send an empty keep-alive close). Matched by
     * path (query string ignored). Compose bodies as `id: <n>\ndata: <json>\n\n`.
     */
    void addStream(const QString& path, const QStringList& rounds)
    {
        Stream s; s.rounds = rounds;
        m_streams.insert(path, s);
    }
    /** Raw headers of all stream requests, accumulated (lets a test assert that a
     *  reconnect sent `Last-Event-ID`, regardless of how many reconnects followed). */
    QByteArray lastRequestHeaders() const { return m_lastRequestHeaders; }

private slots:
    void incomingConnection()
    {
        while(m_server->hasPendingConnections())
        {
            //qDebug() << "FAKE SERVER OPEN A CONNECTION ";
            QTcpSocket* s =m_server->nextPendingConnection();
            connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
            s->setParent(this);
            m_clients.append(s);
        }
    }
    void readClient()
    {        
        QTcpSocket* socket = (QTcpSocket*)sender();
        while (socket->canReadLine()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
#else            
            QStringList tokens = QString(socket->readLine()).split(QRegularExpression("[ \\r\\n][ \\r\\n]*"));
#endif
            if(tokens.size()>=2){

                QString reqPath = tokens[1].section('?', 0, 0);
                if (tokens[0] == "GET" && m_streams.contains(reqPath)) {
                    serveStream(socket, reqPath);
                    return; // socket is being closed; stop processing it
                }

                Reply response;
                response.responseCode = "";
                if (tokens[0] == "GET")
                    response = m_getResponses.value(tokens[1]);
                else if (tokens[0].endsWith("POST"))
                    response = m_postResponses.value(tokens[1]);
                else
                {
                    continue;
                }
                if(response.responseCode.isEmpty())
                {
                    // 404-like default
                    socket->write(QString("HTTP/1.1 %1\r\n"
                                          "Cache-Control: no-cache, no-store, max-age=0\r\n"
                                          "Vary: Origin\r\n"
                                          "Content-Length: %2\r\n"
                                          "Content-Type: %3\r\n"
                                          "%4"
                                          "\r\n%5")
                                  .arg(QString(""))
                                  .arg(0)
                                  .arg(response.contentType)
                                  .arg(response.extraHeaders)
                                  .arg(response.body)
                                  .toUtf8());
                    socket->waitForBytesWritten(1000);
                    continue;
                }

                int contentLength = response.body.toUtf8().size();

                while (socket->canReadLine()) {
                    socket->readLine();
                }
                socket->write(QString("HTTP/1.1 %1\r\n"
                                      "Cache-Control: no-cache, no-store, max-age=0\r\n"
                                      "Vary: Origin\r\n"
                                      "Content-Length: %2\r\n"
                                      "Content-Type: %3\r\n"
                                      "%4"
                                      "\r\n%5")
                              .arg(response.responseCode)
                              .arg(contentLength)
                              .arg(response.contentType)
                              .arg(response.extraHeaders)
                              .arg(response.body)
                              .toUtf8());
                socket->waitForBytesWritten(1000);
                //qDebug() << "FAKE SERVER REPLY "<<response.body;


            }
        }
    }

private:
    void serveStream(QTcpSocket* socket, const QString& path)
    {
        // Read the rest of the request headers (until the blank line), waiting for
        // fragmented data, and ACCUMULATE across requests so a later reconnect
        // doesn't hide an earlier one's `Last-Event-ID`.
        QByteArray headers;
        bool sawBlank = false;
        for (int guard = 0; guard < 50 && !sawBlank; ++guard) {
            while (socket->canReadLine()) {
                QByteArray line = socket->readLine();
                headers.append(line);
                if (line == "\r\n" || line == "\n") { sawBlank = true; break; }
            }
            if (!sawBlank && !socket->waitForReadyRead(100))
                break;
        }
        m_lastRequestHeaders.append(headers);

        Stream& s = m_streams[path];
        QString body = (s.served < s.rounds.size()) ? s.rounds.at(s.served) : QString();
        s.served++;

        socket->write(QByteArray(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/event-stream\r\n"
            "Cache-Control: no-cache, no-store, max-age=0\r\n"
            "Connection: close\r\n"
            "\r\n"));
        if (!body.isEmpty())
            socket->write(body.toUtf8());
        socket->waitForBytesWritten(1000);
        socket->disconnectFromHost();
    }

};
#endif // FAKESERVER_H
