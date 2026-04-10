#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>

MyTcpServer* MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return &instance;
}

MyTcpServer::~MyTcpServer()
{
    if(mTcpServer) {
        mTcpServer->close();
    }
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started on port 33333";
    }
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket* client = mTcpServer->nextPendingConnection();
    qDebug() << "New client connected";
    
    connect(client, &QTcpSocket::readyRead, [this, client]() {
        QByteArray request = client->readAll();
        qDebug() << "Received:" << request;
        
        // Проверяем, HTTP ли это запрос (начинается с GET или POST)
        if (request.startsWith("GET ") || request.startsWith("POST ")) {
            // HTTP-запрос от браузера
            if (request.startsWith("GET / ")) {
                QString html = "<html><body>"
                              "<h1>TCP Server</h1>"
                              "<p>This is my custom TCP server!</p>"
                              "<form action='/echo' method='get'>"
                              "<input type='text' name='msg' placeholder='Enter message'>"
                              "<input type='submit' value='Send'>"
                              "</form>"
                              "</body></html>";
                
                QString response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "Content-Length: " + QString::number(html.toUtf8().size()) + "\r\n"
                                  "Connection: close\r\n"
                                  "\r\n" + html;
                client->write(response.toUtf8());
            }
            else if (request.startsWith("GET /echo")) {
                // Парсим параметр msg
                QString responseText = "Echo from server!";
                
                // Ищем msg= в запросе
                if (request.contains("msg=")) {
                    int start = request.indexOf("msg=") + 4;
                    int end = request.indexOf(" ", start);
                    if (end == -1) end = request.indexOf("\r\n", start);
                    QString msg = request.mid(start, end - start);
                    responseText = "You said: " + msg;
                }
                
                QString response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: " + QString::number(responseText.toUtf8().size()) + "\r\n"
                                  "Connection: close\r\n"
                                  "\r\n" + responseText;
                client->write(response.toUtf8());
            }
            else {
                // 404 Not Found
                QString response = "HTTP/1.1 404 Not Found\r\n"
                                  "Content-Type: text/html\r\n"
                                  "\r\n"
                                  "<h1>404 Not Found</h1>";
                client->write(response.toUtf8());
            }
            
            client->disconnectFromHost();
        }
        else {
            // Не HTTP - обычное TCP-соединение (telnet)
            // Отправляем приветствие
            client->write("Hello, World!!! I am echo server!\r\n");
            
            // Для простоты просто отправим обратно то, что получили
            client->write(request);
            client->disconnectFromHost();
        }
    });
    
    connect(client, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotClientDisconnected()
{
    qDebug() << "Client disconnected";
    if(mTcpSocket) {
        mTcpSocket = nullptr;
    }
}