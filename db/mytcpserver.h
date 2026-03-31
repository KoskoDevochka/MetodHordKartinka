#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QStringList>

#include "database.h"

class MyTcpServer : public QObject
{
    Q_OBJECT
    
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();
    
    // Отправка сообщения клиенту
    void sendToClient(QTcpSocket* socket, const QString &response);
    
public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();
    
private:
    QTcpServer *mTcpServer;
    QMap<QTcpSocket*, QString> mClients;  // Хранение клиентов
    Database *db;  // Указатель на БД (синглтон)
    
    // Парсинг и обработка запросов
    QString parseRequest(const QString &request, QTcpSocket* socket);
};

#endif // MYTCPSERVER_H