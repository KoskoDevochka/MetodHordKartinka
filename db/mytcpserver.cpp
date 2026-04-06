#include "mytcpserver.h"
#include <QThread>

MyTcpServer* MyTcpServer::m_instance = nullptr;

MyTcpServer* MyTcpServer::getInstance()
{
    if (!m_instance) {
        m_instance = new MyTcpServer();
    }
    return m_instance;
}

MyTcpServer::MyTcpServer(QObject *parent) : QTcpServer(parent)
{
    if (listen(QHostAddress::Any, 12345)) {
        qDebug() << "Server started on port 12345";
    } else {
        qDebug() << "Failed to start server";
    }
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "Server stopped";
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    
    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
        QByteArray data = socket->readAll();
        QString request = QString::fromUtf8(data);
        processRequest(socket, request);
    });
    
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void MyTcpServer::processRequest(QTcpSocket* socket, const QString &request)
{
    qDebug() << "Received request:" << request;
    
    QString response = parseRequest(request, socket);
    socket->write(response.toUtf8());
    socket->flush();
    socket->disconnectFromHost();
}

QString MyTcpServer::parseRequest(const QString &request, QTcpSocket* socket)
{
    Q_UNUSED(socket);
    
    // Убираем пробелы, табы, переводы строк
    QString cmd = request.trimmed();
    
    qDebug() << "Parsed command:" << cmd;
    
    Database* db = Database::getInstance();
    QString response;
    
    // Обработка команд
    if (cmd == "PING" || cmd == "ping") {
        response = "OK:PONG\n";
    }
    else if (cmd.startsWith("HASH:") || cmd.startsWith("hash:")) {
        QString text = cmd.mid(5);
        QString hash;
        if (db->sha384Hash(text, hash)) {
            response = "OK:" + hash + "\n";
        } else {
            response = "ERROR:Hash failed\n";
        }
    }
    else if (cmd.startsWith("REGISTER:") || cmd.startsWith("register:")) {
        QStringList parts = cmd.mid(9).split(":");
        if (parts.size() >= 2) {
            if (db->registerUser(parts[0], parts[1])) {
                response = "OK:User registered\n";
            } else {
                response = "ERROR:Registration failed\n";
            }
        } else {
            response = "ERROR:Invalid format. Use: REGISTER:username:password\n";
        }
    }
    else if (cmd.startsWith("LOGIN:") || cmd.startsWith("login:")) {
        QStringList parts = cmd.mid(6).split(":");
        if (parts.size() >= 2) {
            if (db->loginUser(parts[0], parts[1])) {
                response = "OK:Login successful\n";
            } else {
                response = "ERROR:Invalid credentials\n";
            }
        } else {
            response = "ERROR:Invalid format. Use: LOGIN:username:password\n";
        }
    }
    else if (cmd.startsWith("CIPHER:") || cmd.startsWith("cipher:")) {
        QStringList parts = cmd.mid(7).split(":");
        if (parts.size() >= 2) {
            QString encrypted;
            if (db->vigenereCipher(parts[0], parts[1], encrypted)) {
                response = "OK:" + encrypted + "\n";
            } else {
                response = "ERROR:Cipher failed\n";
            }
        } else {
            response = "ERROR:Invalid format. Use: CIPHER:text:key\n";
        }
    }
    else if (cmd.startsWith("CHORD:") || cmd.startsWith("chord:")) {
        QStringList parts = cmd.mid(6).split(":");
        if (parts.size() >= 3) {
            double a = parts[0].toDouble();
            double b = parts[1].toDouble();
            double eps = parts[2].toDouble();
            double result;
            if (db->chordMethod(a, b, eps, result)) {
                response = "OK:" + QString::number(result) + "\n";
            } else {
                response = "ERROR:Chord method failed\n";
            }
        } else {
            response = "ERROR:Invalid format. Use: CHORD:a:b:eps\n";
        }
    }
    else {
        response = "ERROR:Unknown command. Available: PING, HASH:text, REGISTER:user:pass, LOGIN:user:pass, CIPHER:text:key, CHORD:a:b:eps\n";
    }
    
    db->saveRequestLog("parseRequest", cmd, response);
    return response;
}
