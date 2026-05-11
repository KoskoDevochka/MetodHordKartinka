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

    // Таблица пользователей с полем role
    // role: "user" (обычный) или "admin" (администратор)
    QString createUsers = "CREATE TABLE IF NOT EXISTS users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "username TEXT UNIQUE NOT NULL, "
                          "password TEXT NOT NULL, "
                          "role TEXT NOT NULL DEFAULT 'user')";

    if (!query.exec(createUsers)) {
        qDebug() << "Users table error:" << query.lastError().text();
        return false;
    }

    // Миграция: добавляем колонку role если её нет (для существующей БД)
    query.exec("ALTER TABLE users ADD COLUMN role TEXT NOT NULL DEFAULT 'user'");
    // Ошибка игнорируется — если колонка уже есть, это нормально

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

    // Создаём admin по умолчанию, если его нет
    query.prepare("INSERT OR IGNORE INTO users (username, password, role) VALUES (?, ?, ?)");
    query.addBindValue("admin");
    query.addBindValue("admin123");
    query.addBindValue("admin");
    query.exec();

    qDebug() << "Tables created successfully";
    return true;
}

// ==================== ЗАГЛУШКИ ====================

bool Database::vigenereCipher(const QString &text, const QString &key, QString &result)
{
    qDebug() << "Vigenere cipher called (STUB)";
    result = "[STUB] Vigenere: " + text + " with key " + key;
    return true;
}

bool Database::sha384Hash(const QString &text, QString &hash)
{
    qDebug() << "SHA-384 hash called (STUB)";
    hash = "[STUB] SHA384 hash of: " + text;
    return true;
}

bool Database::chordMethod(double a, double b, double epsilon, double &result)
{
    qDebug() << "Chord method called (STUB)";
    result = (a + b) / 2.0;
    return true;
}

bool Database::hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath)
{
    qDebug() << "Hide message in image called (STUB)";
    outputPath = imagePath + "_encoded.png";
    return true;
}

// ==================== АВТОРИЗАЦИЯ ====================

bool Database::registerUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    // При регистрации роль всегда "user"
    query.prepare("INSERT INTO users (username, password, role) VALUES (?, ?, 'user')");
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

// ==================== РОЛИ (задание №7) ====================

/*!
 * \brief Получить роль пользователя из БД.
 */
QString Database::getUserRole(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE username = ?");
    query.addBindValue(username);
    query.exec();

    if (query.next()) {
        return query.value(0).toString();
    }
    return QString(); // Пользователь не найден
}

/*!
 * \brief Установить роль пользователя.
 */
bool Database::setUserRole(const QString &username, const QString &role)
{
    if (role != "user" && role != "admin") {
        qDebug() << "Invalid role:" << role;
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE users SET role = ? WHERE username = ?");
    query.addBindValue(role);
    query.addBindValue(username);

    if (query.exec() && query.numRowsAffected() > 0) {
        qDebug() << "Role updated:" << username << "->" << role;
        return true;
    } else {
        qDebug() << "setUserRole failed:" << query.lastError().text();
        return false;
    }
}

/*!
 * \brief Получить список всех пользователей с ролями.
 */
QStringList Database::getAllUsersWithRoles()
{
    QStringList result;
    QSqlQuery query("SELECT username, role FROM users ORDER BY username");

    while (query.next()) {
        QString line = query.value(0).toString() + ": " + query.value(1).toString();
        result.append(line);
    }
    return result;
}
