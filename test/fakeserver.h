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
    QTcpServer* m_server;
    QHash<QString,QPair<QString, QString> > m_getResponses;
    QHash<QString,QPair<QString, QString> > m_postResponses;
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
        m_getResponses.insert(key,QPair<QString, QString> (value,responseCode));
    }
    void addPost(QString key, QString value, QString responseCode="200 OK")
    {
        m_postResponses.insert(key,QPair<QString, QString> (value,responseCode));
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
            QRegularExpression exp(QRegularExpression::anchoredPattern(QLatin1String("[ \r\n][ \r\n]*")));
            QStringList tokens = QString(socket->readLine()).split(exp);
#endif
            //qDebug() << "TOKENS "<< tokens;
            if(tokens.size()>=2){

                QPair<QString, QString>  response("","");
                if (tokens[0] == "GET")
                    response=m_getResponses.value(tokens[1]);
                else if (tokens[0].endsWith("POST"))
                    response=m_postResponses.value(tokens[1]);
                else
                {

                    continue;
                }
                if(response.second=="")
                {
                    socket->write(QString("HTTP/1.1 %1\r\n"
                                          "Cache-Control: no-cache, no-store, max-age=0\r\n"
                                          "Vary: Origin\r\n"
                                          "Content-Length: %2\r\n"
                                          "Content-Type: application/hal+json; charset=\"utf-8\"\r\n"
                                          "\r\n%3").arg(response.second).arg(0).arg(response.first).toUtf8());
                    socket->waitForBytesWritten(1000);
                    continue;
                }
                  //  qDebug()<< QString("No response for %1").arg(tokens.join(","));

                int contentLength = response.first.toUtf8().size();

                while (socket->canReadLine()) {
                    socket->readLine();
                }
                socket->write(QString("HTTP/1.1 %1\r\n"
                                      "Cache-Control: no-cache, no-store, max-age=0\r\n"
                                      "Vary: Origin\r\n"
                                      "Content-Length: %2\r\n"
                                      "Content-Type: application/hal+json; charset=\"utf-8\"\r\n"
                                      "\r\n%3").arg(response.second).arg(contentLength).arg(response.first).toUtf8());
                socket->waitForBytesWritten(1000);
                //qDebug() << "FAKE SERVER REPLY "<<response.first;


            }
        }
    }

};
#endif // FAKESERVER_H
