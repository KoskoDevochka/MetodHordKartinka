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

//  Новое подключение

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
    info.authenticated = false;
    info.isAdmin = false;

    m_clients[clientSocket] = info;

    qDebug() << "New client connected. Total:" << m_clients.size();

    connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

//Отключение клиента

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

// Чтение данных от клиента

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

//  Парсинг

/*!
 * \brief Разбирает текстовую команду от клиента.
 */
void MyTcpServer::parseRequest(QTcpSocket* client, const QString& request)
{
    qDebug() << "Parsing:" << request;

    // Регистрация и логин не требуют авторизации
    if(request.startsWith("register ")) {
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            handleRegister(client, username, password);
        } else {
            sendToClient(client, "ERROR: Usage: register <username> <password>");
        }
        return;
    }
    else if(request.startsWith("login ")) {
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            QString username = parts[1];
            QString password = parts[2];
            handleLogin(client, username, password);
        } else {
            sendToClient(client, "ERROR: Usage: login <username> <password>");
        }
        return;
    }

    // Все остальные команды требуют авторизации
    auto it = m_clients.find(client);
    if(it == m_clients.end() || !it->authenticated) {
        sendToClient(client, "ERROR: You must login first");
        return;
    }

    // Обработка админских команд (требуют isAdmin)
    if(request == "admin_list_users") {
        if(it->isAdmin) handleListUsers(client);
        else sendToClient(client, "ERROR: Access denied. Admin required.");
        return;
    }
    else if(request == "admin_list_logs") {
        if(it->isAdmin) handleListLogs(client);
        else sendToClient(client, "ERROR: Access denied. Admin required.");
        return;
    }
    else if(request == "admin_clear_logs") {
        if(it->isAdmin) handleClearLogs(client);
        else sendToClient(client, "ERROR: Access denied. Admin required.");
        return;
    }
    else if(request.startsWith("admin_delete_user ")) {
        if(it->isAdmin) {
            QStringList parts = request.split(' ');
            if(parts.size() >= 2) {
                handleDeleteUser(client, parts[1]);
            } else {
                sendToClient(client, "ERROR: Usage: admin_delete_user <username>");
            }
        } else {
            sendToClient(client, "ERROR: Access denied. Admin required.");
        }
        return;
    }

    // Обработка вызова функций (формат "funcId|inputData", inputData может содержать дополнительные '|')
    if(request.contains('|')) {
        int sepIndex = request.indexOf('|');   // находим первый разделитель
        QString funcIdStr = request.left(sepIndex);
        QString inputData = request.mid(sepIndex + 1); // всё остальное (может содержать '|')
        bool ok;
        int funcId = funcIdStr.toInt(&ok);
        if(ok && !inputData.isEmpty()) {
            handleFunctionCall(client, funcId, inputData);
        } else {
            sendToClient(client, "ERROR: Invalid request format. Expected: functionId|data");
        }
        return;
    }

    // Старые заглушки для совместимости с telnet
    if(request.startsWith("action1")) {
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
        sendToClient(client, "ERROR: Unknown request.");
    }
}

// ЗАГЛУШКИ

void MyTcpServer::stub_function1(QTcpSocket* client, const QString& data)
{
    sendToClient(client, "STUB: function1 called with: " + data);
    qDebug() << "Stub: function1" << data;
}

void MyTcpServer::stub_function2(QTcpSocket* client, const QString& data)
{
    sendToClient(client, "STUB: function2 called with: " + data);
    qDebug() << "Stub: function2" << data;
}

void MyTcpServer::stub_function3(QTcpSocket* client)
{
    sendToClient(client, "STUB: help called. Available: action1 <data>, action2 <data>, login, register, admin_*");
    qDebug() << "Stub: help";
}

// HTTP-обработка

void MyTcpServer::handleHttpRequest(QTcpSocket* client, const QByteArray& request)
{
    qDebug() << "HTTP request:" << request;

    QString requestStr = QString::fromUtf8(request);

    if (requestStr.startsWith("GET / ")) {
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

//  Вспомогательные методы

void MyTcpServer::sendToClient(QTcpSocket* client, const QString& message)
{
    if(client && m_clients.contains(client)) {
        client->write(message.toUtf8());
        client->write("\r\n");
    }
}

void MyTcpServer::broadcastToAll(const QString& message, QTcpSocket* exclude)
{
    for(auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if(it.key() != exclude) {
            it.key()->write(message.toUtf8());
            it.key()->write("\r\n");
        }
    }
}

//  АВТОРИЗАЦИЯ И РЕГИСТРАЦИЯ

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

void MyTcpServer::handleLogin(QTcpSocket* client, const QString& username, const QString& password)
{
    Database* db = Database::getInstance();
    bool isAdmin = false;

    if(db->loginUser(username, password, isAdmin)) {
        auto it = m_clients.find(client);
        if(it != m_clients.end()) {
            it->authenticated = true;
            it->isAdmin = isAdmin;
        }
        sendToClient(client, "SUCCESS: Login successful");
        qDebug() << "Login success:" << username << "isAdmin:" << isAdmin;
        emit logMessage(QString("[LOGIN] User %1 (admin=%2)").arg(username).arg(isAdmin));
    } else {
        sendToClient(client, "ERROR: Invalid username or password");
        qDebug() << "Login failed:" << username;
    }
}

//НОВЫЕ АДМИНСКИЕ КОМАНДЫ

void MyTcpServer::handleListUsers(QTcpSocket* client)
{
    Database* db = Database::getInstance();
    QStringList users = db->getAllUsers();
    if(users.isEmpty()) {
        sendToClient(client, "No users found.");
    } else {
        sendToClient(client, "=== Users list ===");
        for(const QString& u : users) {
            sendToClient(client, u);
        }
        sendToClient(client, "==================");
    }
    emit logMessage("Admin listed users");
}

void MyTcpServer::handleListLogs(QTcpSocket* client)
{
    Database* db = Database::getInstance();
    QStringList logs = db->getAllLogs();
    if(logs.isEmpty()) {
        sendToClient(client, "No logs found.");
    } else {
        sendToClient(client, "=== Request logs ===");
        for(const QString& log : logs) {
            sendToClient(client, log);
        }
        sendToClient(client, "====================");
    }
    emit logMessage("Admin listed logs");
}

void MyTcpServer::handleClearLogs(QTcpSocket* client)
{
    Database* db = Database::getInstance();
    if(db->clearLogs()) {
        sendToClient(client, "SUCCESS: All logs cleared.");
        emit logMessage("Admin cleared logs");
    } else {
        sendToClient(client, "ERROR: Failed to clear logs.");
    }
}

void MyTcpServer::handleDeleteUser(QTcpSocket* client, const QString& username)
{
    Database* db = Database::getInstance();
    if(db->deleteUser(username)) {
        sendToClient(client, "SUCCESS: User " + username + " deleted (if existed and was not admin).");
        emit logMessage(QString("Admin deleted user: %1").arg(username));
    } else {
        sendToClient(client, "ERROR: Could not delete user (maybe admin or not exists).");
    }
}

//ОБРАБОТКА ОСНОВНЫХ ФУНКЦИЙ

void MyTcpServer::handleFunctionCall(QTcpSocket* client, int funcId, const QString& inputData)
{
    Database* db = Database::getInstance();
    QString result;
    bool success = false;

    switch(funcId) {
    case 0: { // Шифр Виженера (для упрощения используем ключ "secret")
        QString key = "secret";
        success = db->vigenereCipher(inputData, key, result);
        break;
    }
    case 1: { // SHA-384
        success = db->sha384Hash(inputData, result);
        break;
    }
    case 2: { // Метод хорд (ожидает числа a,b,epsilon через пробел)
        QStringList params = inputData.split(' ', Qt::SkipEmptyParts);
        if(params.size() >= 3) {
            double a = params[0].toDouble();
            double b = params[1].toDouble();
            double eps = params[2].toDouble();
            double res;
            success = db->chordMethod(a, b, eps, res);
            result = QString::number(res);
        } else {
            result = "ERROR: Need 3 numbers: a b epsilon";
            success = false;
        }
        break;
    }
    case 3: { // Стеганография (ожидает путь к картинке и сообщение через | )
        QStringList parts = inputData.split('|');
        if(parts.size() >= 2) {
            QString imagePath = parts[0];
            QString message = parts[1];
            QString outputPath;
            success = db->hideMessageInImage(imagePath, message, outputPath);
            result = "Image saved to: " + outputPath;
        } else {
            result = "ERROR: Need imagePath|message";
            success = false;
        }
        break;
    }
    default:
        result = "ERROR: Unknown function ID";
        success = false;
    }

    if(success) {
        sendToClient(client, "RESULT: " + result);
        // Логируем в БД
        QString command = QString("Function%1").arg(funcId);
        db->saveRequestLog(command, inputData, result);
        emit logMessage(QString("Function %1 called by %2, result: %3").arg(funcId)
                            .arg(client->peerAddress().toString()).arg(result.left(50)));
    } else {
        sendToClient(client, "ERROR: " + result);
        emit logMessage(QString("Function %1 failed: %2").arg(funcId).arg(result));
    }
}