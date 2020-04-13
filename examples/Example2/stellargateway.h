#ifndef STELLARGATEWAY_H
#define STELLARGATEWAY_H

#include <QObject>
#include "server.h"



class StellarGateway : public QObject
{
    Q_OBJECT


    Server *m_server;


public:
    explicit StellarGateway(QObject *parent = nullptr);

    Server * server() const;


};

#endif // STELLARGATEWAY_H
