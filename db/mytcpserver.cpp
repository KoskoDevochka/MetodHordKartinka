#include "mytcpserver.h"

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    
    // Получаем экземпляр БД (синглтон!)
    db = Database::getInstance();
    
    // Слушаем все интерфейсы на порту 33333
    if (mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "========================================";
        qDebug() << "Server started successfully!";
        qDebug() << "Port: 33333";
        qDebug() << "Listening on all network interfaces";
        qDebug() << "========================================";
        
        // Выводим IP адреса сервера
        QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
        for (const QHostAddress& addr : addresses) {
            if (addr != QHostAddress::LocalHost && 
                addr.protocol() == QAbstractSocket::IPv4Protocol) {
                qDebug() << "Server IP:" << addr.toString();
            }
        }
        qDebug() << "========================================";
        
        connect(mTcpServer, &QTcpServer::newConnection, 
                this, &MyTcpServer::slotNewConnection);
    } else {
        qDebug() << "ERROR: Server failed to start!";
        qDebug() << "Error:" << mTcpServer->errorString();
    }
}

MyTcpServer::~MyTcpServer()
{
    if (mTcpServer) {
        mTcpServer->close();
        qDebug() << "Server stopped";
    }
    
    // Закрываем все соединения
    for (auto it = mClients.begin(); it != mClients.end(); ++it) {
        if (it.key()) {
            it.key()->close();
            it.key()->deleteLater();
        }
    }
    mClients.clear();
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket* socket = mTcpServer->nextPendingConnection();
    
    if (socket) {
        connect(socket, &QTcpSocket::readyRead, 
                this, &MyTcpServer::slotServerRead);
        connect(socket, &QTcpSocket::disconnected, 
                this, &MyTcpServer::slotClientDisconnected);
        
        QString clientInfo = QString("%1:%2")
            .arg(socket->peerAddress().toString())
            .arg(socket->peerPort());
        
        mClients.insert(socket, clientInfo);
        
        qDebug() << "========================================";
        qDebug() << "New client connected!";
        qDebug() << "Client:" << clientInfo;
        qDebug() << "Total clients:" << mClients.size();
        qDebug() << "========================================";
        
        // Отправляем приветственное сообщение
        sendToClient(socket, "WELCOME|Connected to TAM&P Server");
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QString clientInfo = mClients.value(socket, "Unknown");
        qDebug() << "Client disconnected:" << clientInfo;
        mClients.remove(socket);
        socket->deleteLater();
        qDebug() << "Total clients remaining:" << mClients.size();
    }
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    // Читаем все данные
    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data).trimmed();
    
    if (request.isEmpty()) return;
    
    QString clientInfo = mClients.value(socket, "Unknown");
    qDebug() << "----------------------------------------";
    qDebug() << "Received from" << clientInfo << ":";
    qDebug() << "  " << request;
    
    // Парсим запрос
    QString response = parseRequest(request, socket);
    
    // Отправляем ответ
    sendToClient(socket, response);
    
    qDebug() << "Response sent:" << response;
    qDebug() << "----------------------------------------";
}

void MyTcpServer::sendToClient(QTcpSocket* socket, const QString &response)
{
    if (socket && socket->isOpen()) {
        QByteArray data = response.toUtf8() + "\n";
        socket->write(data);
        socket->flush();
    }
}

QString MyTcpServer::parseRequest(const QString &request, QTcpSocket* socket)
{
    // Парсим запрос в формате: "КОМАНДА|ПАРАМЕТР1|ПАРАМЕТР2|..."
    QStringList parts = request.split('|');
    
    if (parts.isEmpty()) {
        return "ERROR|Empty request";
    }
    
    QString command = parts[0].toUpper();
    QString result;
    double numResult;
    
    // Обработка команд
    if (command == "PING" || command == "HELLO") {
        result = "OK|PONG";
    }
    else if (command == "VIGENERE" || command == "VIG") {
        // Формат: VIGENERE|text|key
        if (parts.size() >= 3) {
            QString text = parts[1];
            QString key = parts[2];
            QString encrypted;
            
            if (db->vigenereCipher(text, key, encrypted)) {
                result = "OK|" + encrypted;
            } else {
                result = "ERROR|Vigenere cipher failed";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: VIGENERE|text|key";
        }
    }
    else if (command == "SHA384" || command == "SHA") {
        // Формат: SHA384|text
        if (parts.size() >= 2) {
            QString text = parts[1];
            QString hash;
            
            if (db->sha384Hash(text, hash)) {
                result = "OK|" + hash;
            } else {
                result = "ERROR|SHA-384 hash failed";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: SHA384|text";
        }
    }
    else if (command == "CHORD") {
        // Формат: CHORD|a|b|epsilon
        if (parts.size() >= 4) {
            bool ok1, ok2, ok3;
            double a = parts[1].toDouble(&ok1);
            double b = parts[2].toDouble(&ok2);
            double eps = parts[3].toDouble(&ok3);
            
            if (ok1 && ok2 && ok3) {
                double chordResult;
                if (db->chordMethod(a, b, eps, chordResult)) {
                    result = "OK|" + QString::number(chordResult);
                } else {
                    result = "ERROR|Chord method failed";
                }
            } else {
                result = "ERROR|Invalid numeric parameters";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: CHORD|a|b|epsilon";
        }
    }
    else if (command == "HIDE_IMAGE" || command == "HIDE") {
        // Формат: HIDE_IMAGE|image_path|message
        if (parts.size() >= 3) {
            QString imagePath = parts[1];
            QString message = parts[2];
            QString outputPath;
            
            if (db->hideMessageInImage(imagePath, message, outputPath)) {
                result = "OK|" + outputPath;
            } else {
                result = "ERROR|Hide message failed";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: HIDE_IMAGE|path|message";
        }
    }
    else if (command == "REGISTER") {
        // Формат: REGISTER|username|password
        if (parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            
            if (db->registerUser(username, password)) {
                result = "OK|User registered successfully";
            } else {
                result = "ERROR|Registration failed";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: REGISTER|username|password";
        }
    }
    else if (command == "LOGIN") {
        // Формат: LOGIN|username|password
        if (parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            
            if (db->loginUser(username, password)) {
                result = "OK|Login successful";
            } else {
                result = "ERROR|Invalid credentials";
            }
        } else {
            result = "ERROR|Invalid parameters. Format: LOGIN|username|password";
        }
    }
    else if (command == "HELP") {
        result = "OK|Available commands:\n"
                 "  PING - Test connection\n"
                 "  VIGENERE|text|key - Vigenere cipher\n"
                 "  SHA384|text - SHA-384 hash\n"
                 "  CHORD|a|b|epsilon - Chord method\n"
                 "  HIDE_IMAGE|path|message - Steganography\n"
                 "  REGISTER|user|pass - Register user\n"
                 "  LOGIN|user|pass - Login user\n"
                 "  HELP - Show this help";
    }
    else if (command == "STATUS") {
        result = QString("OK|Server running. Connected clients: %1").arg(mClients.size());
    }
    else {
        result = "ERROR|Unknown command: " + command + ". Type HELP for available commands";
    }
    
    // Сохраняем запрос в лог
    db->saveRequestLog(command, request, result);
    
    return result;
}