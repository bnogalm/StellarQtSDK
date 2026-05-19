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
public:
    FakeServer(quint16 port=8080, QObject* parent = nullptr):QObject(parent)
    {
        m_server= new QTcpServer(this);
        m_server->setMaxPendingConnections(100);
        connect(m_server, &QTcpServer::newConnection, this, &FakeServer::incomingConnection);
        m_server->listen(QHostAddress::LocalHost, port);

    }
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

};
#endif // FAKESERVER_H
