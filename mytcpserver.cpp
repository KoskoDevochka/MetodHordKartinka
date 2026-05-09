/*!
 * \file mytcpserver.cpp
 * \brief Реализация класса TCP-сервера.
 */
#include "database.h"
#include "mytcpserver.h"
#include <QDebug>

// ==================== Синглтон ====================

/*!
 * \brief Получить единственный экземпляр сервера (синглтон).
 * \return Указатель на экземпляр MyTcpServer.
 */
MyTcpServer* MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return &instance;
}

/*!
 * \brief Деструктор. Закрывает серверный сокет.
 */
MyTcpServer::~MyTcpServer()
{
    if(mTcpServer) {
        mTcpServer->close();
    }
}

/*!
 * \brief Приватный конструктор. Запускает TCP-сервер на порту 33333.
 * \param parent Родительский QObject.
 */
MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "Server is not started";
    } else {
        qDebug() << "Server started on port 33333";
    }
}

// ==================== Новое подключение (задание №5) ====================

/*!
 * \brief Обрабатывает новое подключение клиента.
 *
 * Создаёт структуру ClientInfo для нового клиента,
 * добавляет его в QMap и отправляет приветственное сообщение.
 */
void MyTcpServer::slotNewConnection()
{
    QTcpSocket* clientSocket = mTcpServer->nextPendingConnection();

    ClientInfo info;
    info.socket = clientSocket;
    info.buffer = "";

    m_clients[clientSocket] = info;

    // УБИРАЕМ ЭТУ СТРОКУ, ОНА ЛОМАЕТ БРАУЗЕР
    // QString greeting = "Connected to server. Total clients: " +
    //                    QString::number(m_clients.size()) + "\r\n";
    // clientSocket->write(greeting.toUtf8());

    qDebug() << "New client connected. Total:" << m_clients.size();

    connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

// ==================== Отключение клиента (задание №5) ====================

/*!
 * \brief Обрабатывает отключение клиента.
 */
void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(clientSocket && m_clients.contains(clientSocket)) {
        qDebug() << "Client disconnected. Remaining:" << m_clients.size() - 1;
        m_clients.remove(clientSocket);
        clientSocket->deleteLater();
    }
}

// ==================== Чтение данных от клиента (задание №5) ====================

/*!
 * \brief Обрабатывает входящие данные от клиента.
 */
void MyTcpServer::slotServerRead()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket || !m_clients.contains(clientSocket)) return;

    ClientInfo& info = m_clients[clientSocket];
    QByteArray array = clientSocket->readAll();

    qDebug() << "Received from client:" << array;

    // HTTP-запросы от браузера
    if (array.startsWith("GET ") || array.startsWith("POST ")) {
        handleHttpRequest(clientSocket, array);
        return;
    }

    // Фильтрация telnet-мусора
    QByteArray cleanData;
    int i = 0;
    while(i < array.size()) {
        unsigned char c = static_cast<unsigned char>(array[i]);

        if(c == 0xFF) {
            i += 3;
            continue;
        }
        else if(c == 0x0D) {
            i++;
            continue;
        }
        else {
            cleanData.append(static_cast<char>(c));
            i++;
        }
    }

    info.buffer += QString::fromUtf8(cleanData);

    int newlinePos;
    while((newlinePos = info.buffer.indexOf('\n')) != -1) {
        QString line = info.buffer.left(newlinePos).trimmed();
        info.buffer = info.buffer.mid(newlinePos + 1);

        if(!line.isEmpty()) {
            parseRequest(clientSocket, line);
        }
    }
}

// ==================== Парсинг (задание №3 + задание №6) ====================

/*!
 * \brief Разбирает текстовую команду от клиента.
 */
void MyTcpServer::parseRequest(QTcpSocket* client, const QString& request)
{
    qDebug() << "Parsing:" << request;

    // ==================== ЗАДАНИЕ №6: АВТОРИЗАЦИЯ И РЕГИСТРАЦИЯ ====================
    if(request.startsWith("register ")) {
        // Формат: register username password
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            handleRegister(client, username, password);
        } else {
            sendToClient(client, "ERROR: Usage: register <username> <password>");
        }
    }
    else if(request.startsWith("login ")) {
        // Формат: login username password
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            handleLogin(client, username, password);
        } else {
            sendToClient(client, "ERROR: Usage: login <username> <password>");
        }
    }
    // ==================== ЗАДАНИЕ №3: ЗАГЛУШКИ ====================
    else if(request.startsWith("action1")) {
        QString data = request.mid(7).trimmed();
        stub_function1(client, data);
    }
    else if(request.startsWith("action2")) {
        QString data = request.mid(7).trimmed();
        stub_function2(client, data);
    }
    else if(request == "help") {
        stub_function3(client);
    }
    else {
        sendToClient(client, "ERROR: Unknown request. Available: register, login, action1, action2, help");
    }
}

// ==================== ЗАГЛУШКИ (задание №3) ====================

/*!
 * \brief Заглушка для команды action1.
 */
void MyTcpServer::stub_function1(QTcpSocket* client, const QString& data)
{
    sendToClient(client, "STUB: function1 called with: " + data);
    qDebug() << "Stub: function1" << data;
}

/*!
 * \brief Заглушка для команды action2.
 */
void MyTcpServer::stub_function2(QTcpSocket* client, const QString& data)
{
    sendToClient(client, "STUB: function2 called with: " + data);
    qDebug() << "Stub: function2" << data;
}

/*!
 * \brief Заглушка для команды help.
 */
void MyTcpServer::stub_function3(QTcpSocket* client)
{
    sendToClient(client, "STUB: help called. Available: action1 <data>, action2 <data>");
    qDebug() << "Stub: help";
}

// ==================== HTTP-обработка (задание подруги №4) ====================

/*!
 * \brief Обрабатывает HTTP-запросы от браузера.
 */
void MyTcpServer::handleHttpRequest(QTcpSocket* client, const QByteArray& request)
{
    qDebug() << "HTTP request:" << request;

    // Разбираем запрос
    QString requestStr = QString::fromUtf8(request);
    
    if (requestStr.startsWith("GET / ")) {
        // Корневая страница
        QString html = "<html><body>"
                       "<h1>TCP Server</h1>"
                       "<p>Connected clients: " + QString::number(m_clients.size()) + "</p>"
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
        client->flush();
    }
    else if (requestStr.startsWith("GET /echo")) {
        // Echo запрос
        QString responseText = "Echo from server!";
        
        if (requestStr.contains("msg=")) {
            int start = requestStr.indexOf("msg=") + 4;
            int end = requestStr.indexOf(" ", start);
            if (end == -1) end = requestStr.indexOf("\r\n", start);
            QString msg = requestStr.mid(start, end - start);
            responseText = "You said: " + msg;
        }

        QString response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: " + QString::number(responseText.toUtf8().size()) + "\r\n"
                           "Connection: close\r\n"
                           "\r\n" + responseText;
        client->write(response.toUtf8());
        client->flush();
    }
    else {
        // 404
        QString response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "<h1>404 Not Found</h1>";
        client->write(response.toUtf8());
        client->flush();
    }
    
    client->disconnectFromHost();
}

// ==================== Вспомогательные методы (задание №5) ====================

/*!
 * \brief Отправляет сообщение клиенту.
 */
void MyTcpServer::sendToClient(QTcpSocket* client, const QString& message)
{
    if(client && m_clients.contains(client)) {
        client->write(message.toUtf8());
        client->write("\r\n");
    }
}

/*!
 * \brief Рассылает сообщение всем клиентам.
 */
void MyTcpServer::broadcastToAll(const QString& message, QTcpSocket* exclude)
{
    for(auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if(it.key() != exclude) {
            it.key()->write(message.toUtf8());
            it.key()->write("\r\n");
        }
    }
}

//АВТОРИЗАЦИЯ И РЕГИСТРАЦИЯ (задание №6)

/*!
 * \brief Обрабатывает регистрацию пользователя.
 */
void MyTcpServer::handleRegister(QTcpSocket* client, const QString& username, const QString& password)
{
    Database* db = Database::getInstance();

    if(db->registerUser(username, password)) {
        sendToClient(client, "SUCCESS: User registered successfully");
        qDebug() << "Registration success:" << username;
    } else {
        sendToClient(client, "ERROR: Username already exists or invalid data");
        qDebug() << "Registration failed:" << username;
    }
}

/*!
 * \brief Обрабатывает вход пользователя.
 */
void MyTcpServer::handleLogin(QTcpSocket* client, const QString& username, const QString& password)
{
    Database* db = Database::getInstance();

    if(db->loginUser(username, password)) {
        sendToClient(client, "SUCCESS: Login successful");
        qDebug() << "Login success:" << username;
    } else {
        sendToClient(client, "ERROR: Invalid username or password");
        qDebug() << "Login failed:" << username;
    }
}
