#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QDebug>
#include "database.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer* getInstance();
    ~MyTcpServer();
    
protected:
    void incomingConnection(qintptr socketDescriptor) override;
    
private:
    explicit MyTcpServer(QObject *parent = nullptr);
    static MyTcpServer* m_instance;
    
    void processRequest(QTcpSocket* socket, const QString &request);
    QString parseRequest(const QString &request, QTcpSocket* socket);
};

#endif
