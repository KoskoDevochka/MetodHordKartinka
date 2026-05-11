/*!
 * \file mytcpserver.cpp
 * \brief Реализация класса TCP-сервера.
 */

#include "database.h"
#include "mytcpserver.h"
#include <QDebug>

// ==================== Синглтон ====================

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
        qDebug() << "Server is not started";
    } else {
        qDebug() << "Server started on port 33333";
    }
}

// ==================== Новое подключение ====================

void MyTcpServer::slotNewConnection()
{
    QTcpSocket* clientSocket = mTcpServer->nextPendingConnection();

    ClientInfo info;
    info.socket   = clientSocket;
    info.buffer   = "";
    info.role     = Role::Guest;   // По умолчанию — гость (задание №7)
    info.username = "";

    m_clients[clientSocket] = info;

    qDebug() << "New client connected. Total:" << m_clients.size();

    // Отправляем приветствие с инструкцией
    sendToClient(clientSocket,
        "Welcome! You are connected as Guest.\r\n"
        "Commands: register <user> <pass>, login <user> <pass>, help");

    connect(clientSocket, &QTcpSocket::readyRead,    this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}

// ==================== Отключение клиента ====================

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if(clientSocket && m_clients.contains(clientSocket)) {
        qDebug() << "Client disconnected:" << m_clients[clientSocket].username
                 << "Remaining:" << m_clients.size() - 1;
        m_clients.remove(clientSocket);
        clientSocket->deleteLater();
    }
}

// ==================== Чтение данных ====================

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
        if(c == 0xFF) { i += 3; continue; }
        else if(c == 0x0D) { i++; continue; }
        else { cleanData.append(static_cast<char>(c)); i++; }
    }

    info.buffer += QString::fromUtf8(cleanData);

    int newlinePos;
    while((newlinePos = info.buffer.indexOf('\n')) != -1) {
        QString line = info.buffer.left(newlinePos).trimmed();
        info.buffer  = info.buffer.mid(newlinePos + 1);
        if(!line.isEmpty()) {
            parseRequest(clientSocket, line);
        }
    }
}

// ==================== Парсинг команд ====================

void MyTcpServer::parseRequest(QTcpSocket* client, const QString& request)
{
    qDebug() << "Parsing:" << request;

    // ---------- Регистрация / Логин (доступно всем) ----------
    if(request.startsWith("register ")) {
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            handleRegister(client, parts[1], parts[2]);
        } else {
            sendToClient(client, "ERROR: Usage: register <username> <password>");
        }
    }
    else if(request.startsWith("login ")) {
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            handleLogin(client, parts[1], parts[2]);
        } else {
            sendToClient(client, "ERROR: Usage: login <username> <password>");
        }
    }

    // ---------- Команды только для авторизованных (User + Admin) ----------
    else if(request.startsWith("action1")) {
        if(!hasRole(client, Role::User)) {
            sendToClient(client, "ERROR: Access denied. Please login first.");
            return;
        }
        QString data = request.mid(7).trimmed();
        stub_function1(client, data);
    }
    else if(request.startsWith("action2")) {
        if(!hasRole(client, Role::User)) {
            sendToClient(client, "ERROR: Access denied. Please login first.");
            return;
        }
        QString data = request.mid(7).trimmed();
        stub_function2(client, data);
    }

    // ---------- Команды только для Admin ----------
    else if(request.startsWith("setrole ")) {
        // Формат: setrole <username> <role>
        if(!hasRole(client, Role::Admin)) {
            sendToClient(client, "ERROR: Access denied. Admin only.");
            return;
        }
        QStringList parts = request.split(' ');
        if(parts.size() >= 3) {
            handleSetRole(client, parts[1], parts[2]);
        } else {
            sendToClient(client, "ERROR: Usage: setrole <username> <role>");
        }
    }
    else if(request == "listusers") {
        if(!hasRole(client, Role::Admin)) {
            sendToClient(client, "ERROR: Access denied. Admin only.");
            return;
        }
        handleListUsers(client);
    }

    // ---------- Help (доступно всем) ----------
    else if(request == "help") {
        stub_function3(client);
    }

    // ---------- Whoami ----------
    else if(request == "whoami") {
        ClientInfo& info = m_clients[client];
        if(info.role == Role::Guest) {
            sendToClient(client, "You are: Guest (not logged in)");
        } else {
            sendToClient(client, "You are: " + info.username +
                         " [" + roleToString(info.role) + "]");
        }
    }

    // ---------- Logout ----------
    else if(request == "logout") {
        ClientInfo& info = m_clients[client];
        if(info.role == Role::Guest) {
            sendToClient(client, "ERROR: You are not logged in.");
        } else {
            QString name = info.username;
            info.role     = Role::Guest;
            info.username = "";
            sendToClient(client, "SUCCESS: Logged out. Goodbye, " + name + "!");
        }
    }

    else {
        sendToClient(client, "ERROR: Unknown command. Type 'help' for commands list.");
    }
}

// ==================== Заглушки ====================

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
    ClientInfo& info = m_clients[client];
    QString helpText =
        "=== Available commands ===\r\n"
        "  register <user> <pass>  - Register new account\r\n"
        "  login <user> <pass>     - Login\r\n"
        "  logout                  - Logout\r\n"
        "  whoami                  - Show current user and role\r\n"
        "  action1 <data>          - [User/Admin] Function 1\r\n"
        "  action2 <data>          - [User/Admin] Function 2\r\n"
        "  setrole <user> <role>   - [Admin] Set user role (user/admin)\r\n"
        "  listusers               - [Admin] List all users with roles\r\n"
        "  help                    - Show this message\r\n"
        "Your role: " + roleToString(info.role);

    sendToClient(client, helpText);
}

// ==================== HTTP ====================

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
            int end   = requestStr.indexOf(" ", start);
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
                           "\r\n<h1>404 Not Found</h1>";
        client->write(response.toUtf8());
        client->flush();
    }

    client->disconnectFromHost();
}

// ==================== Авторизация ====================

void MyTcpServer::handleRegister(QTcpSocket* client, const QString& username, const QString& password)
{
    Database* db = Database::getInstance();
    if(db->registerUser(username, password)) {
        sendToClient(client, "SUCCESS: User '" + username + "' registered. Now use: login " + username + " <pass>");
        qDebug() << "Registration success:" << username;
    } else {
        sendToClient(client, "ERROR: Username already exists or invalid data");
        qDebug() << "Registration failed:" << username;
    }
}

void MyTcpServer::handleLogin(QTcpSocket* client, const QString& username, const QString& password)
{
    Database* db = Database::getInstance();
    if(db->loginUser(username, password)) {
        // Получаем роль из БД и сохраняем в ClientInfo (задание №7)
        QString roleStr = db->getUserRole(username);

        ClientInfo& info = m_clients[client];
        info.username = username;

        if(roleStr == "admin") {
            info.role = Role::Admin;
        } else {
            info.role = Role::User;
        }

        sendToClient(client, "SUCCESS: Welcome, " + username +
                     "! Your role: " + roleToString(info.role));
        qDebug() << "Login success:" << username << "role:" << roleStr;
    } else {
        sendToClient(client, "ERROR: Invalid username or password");
        qDebug() << "Login failed:" << username;
    }
}

// ==================== РОЛИ (задание №7) ====================

/*!
 * \brief Проверяет, имеет ли клиент требуемую роль или выше.
 *
 * Иерархия: Guest < User < Admin
 */
bool MyTcpServer::hasRole(QTcpSocket* client, Role required)
{
    if(!m_clients.contains(client)) return false;
    Role clientRole = m_clients[client].role;

    // Guest = 0, User = 1, Admin = 2
    return static_cast<int>(clientRole) >= static_cast<int>(required);
}

/*!
 * \brief Назначает роль другому пользователю (только Admin).
 */
void MyTcpServer::handleSetRole(QTcpSocket* client, const QString& username, const QString& role)
{
    if(role != "user" && role != "admin") {
        sendToClient(client, "ERROR: Invalid role. Use 'user' or 'admin'.");
        return;
    }

    Database* db = Database::getInstance();
    if(db->setUserRole(username, role)) {
        sendToClient(client, "SUCCESS: Role of '" + username + "' set to '" + role + "'.");

        // Если целевой пользователь сейчас онлайн — обновляем его ClientInfo
        for(auto it = m_clients.begin(); it != m_clients.end(); ++it) {
            if(it.value().username == username) {
                it.value().role = (role == "admin") ? Role::Admin : Role::User;
                sendToClient(it.key(), "NOTICE: Your role has been changed to '" + role + "' by admin.");
                break;
            }
        }
    } else {
        sendToClient(client, "ERROR: User '" + username + "' not found.");
    }
}

/*!
 * \brief Выводит список всех пользователей и их ролей (только Admin).
 */
void MyTcpServer::handleListUsers(QTcpSocket* client)
{
    Database* db = Database::getInstance();
    QStringList users = db->getAllUsersWithRoles();

    if(users.isEmpty()) {
        sendToClient(client, "No users found.");
        return;
    }

    sendToClient(client, "=== User list ===");
    for(const QString& line : users) {
        sendToClient(client, "  " + line);
    }
    sendToClient(client, "=================");
}

// ==================== Утилиты ====================

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

/*!
 * \brief Преобразует Role в строку для отображения.
 */
QString MyTcpServer::roleToString(Role role)
{
    switch(role) {
        case Role::Guest: return "Guest";
        case Role::User:  return "User";
        case Role::Admin: return "Admin";
        default:          return "Unknown";
    }
}
