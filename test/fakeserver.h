#ifndef FAKESERVER_H
#define FAKESERVER_H

#include <QTcpServer>
#include <QTcpSocket>

/**
 * @brief The FakeServer class
 *
 * Implements a simple server for tests
 */
class FakeServer : public QTcpServer{

    Q_OBJECT
    QHash<QString,QPair<QString, QString> > m_getResponses;
    QHash<QString,QPair<QString, QString> > m_postResponses;
public:
    FakeServer(quint16 port=8080, QObject* parent = 0)
        : QTcpServer(parent)
    {
        listen(QHostAddress::LocalHost, port);
    }

    void incomingConnection(qintptr socket)
    {
        QTcpSocket* s = new QTcpSocket(this);
        connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
        connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));        
        s->setSocketDescriptor(socket);
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
    void readClient()
    {

        QTcpSocket* socket = (QTcpSocket*)sender();
        if (socket->canReadLine()) {
            QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
            //qDebug() << "TOKENS "<< tokens;
            if(tokens.size()>=2){

                QPair<QString, QString>  response("","");
                if (tokens[0] == "GET")
                    response=m_getResponses.value(tokens[1]);
                else if (tokens[0] == "POST")
                    response=m_postResponses.value(tokens[1]);
                //if(response.second=="")
                  //  qDebug()<< QString("No response for %1").arg(tokens.join(","));

                int contentLength = response.first.toUtf8().size();

                socket->write(QString("HTTP/1.1 %1\r\n"
                                      "Cache-Control: no-cache, no-store, max-age=0\r\n"
                                      "Vary: Origin\r\n"
                                      "Content-Length: %2\r\n"
                                      "Content-Type: application/hal+json; charset=\"utf-8\"\r\n"
                                      "\r\n%3").arg(response.second).arg(contentLength).arg(response.first).toUtf8());

                socket->waitForBytesWritten();
                socket->close();

            }
        }
    }
    void discardClient()
    {
        QTcpSocket* socket = (QTcpSocket*)sender();
        socket->deleteLater();
    }
};
#endif // FAKESERVER_H
