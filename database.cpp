#include "database.h"

// Статическая переменная
Database* Database::m_instance = nullptr;

Database* Database::getInstance()
{
    if (!m_instance) {
        m_instance = new Database();
    }
    return m_instance;
}

Database::Database(QObject *parent) : QObject(parent)
{
    connectToDatabase();
    createTables();
    qDebug() << "Database initialized";
}

Database::~Database()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    qDebug() << "Database closed";
}

bool Database::connectToDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("server.db");

    if (!m_db.open()) {
        qDebug() << "Database connection error:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully";
    return true;
}

bool Database::createTables()
{
    QSqlQuery query;

    // Таблица пользователей
    QString createUsers = "CREATE TABLE IF NOT EXISTS users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "username TEXT UNIQUE NOT NULL, "
                          "password TEXT NOT NULL)";
    if (!query.exec(createUsers)) {
        qDebug() << "Users table error:" << query.lastError().text();
        return false;
    }

    // Таблица логов
    QString createLogs = "CREATE TABLE IF NOT EXISTS request_logs ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "command TEXT, "
                         "request TEXT, "
                         "result TEXT, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)";
    if (!query.exec(createLogs)) {
        qDebug() << "Logs table error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Tables created successfully";
    return true;
}

// ==================== ТВОИ ФУНКЦИИ (ЗАГЛУШКИ) ====================

bool Database::vigenereCipher(const QString &text, const QString &key, QString &result)
{
    qDebug() << "Vigenere cipher called (STUB)";
    qDebug() << "  Text:" << text << "Key:" << key;
    result = "[STUB] Vigenere: " + text + " with key " + key;
    return true;
}

bool Database::sha384Hash(const QString &text, QString &hash)
{
    qDebug() << "SHA-384 hash called (STUB)";
    qDebug() << "  Text:" << text;
    hash = "[STUB] SHA384 hash of: " + text;
    return true;
}

bool Database::chordMethod(double a, double b, double epsilon, double &result)
{
    qDebug() << "Chord method called (STUB)";
    qDebug() << "  a:" << a << "b:" << b << "eps:" << epsilon;
    result = (a + b) / 2.0;
    return true;
}

bool Database::hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath)
{
    qDebug() << "Hide message in image called (STUB)";
    qDebug() << "  Image:" << imagePath << "Message:" << message;
    outputPath = imagePath + "_encoded.png";
    return true;
}

// ==================== ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ ====================

bool Database::registerUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec()) {
        qDebug() << "User registered:" << username;
        return true;
    } else {
        qDebug() << "Register failed:" << query.lastError().text();
        return false;
    }
}

bool Database::loginUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.exec();

    if (query.next()) {
        qDebug() << "Login success:" << username;
        return true;
    } else {
        qDebug() << "Login failed:" << username;
        return false;
    }
}

bool Database::saveRequestLog(const QString &command, const QString &request, const QString &result)
{
    QSqlQuery query;
    query.prepare("INSERT INTO request_logs (command, request, result) VALUES (?, ?, ?)");
    query.addBindValue(command);
    query.addBindValue(request);
    query.addBindValue(result);
    return query.exec();
}