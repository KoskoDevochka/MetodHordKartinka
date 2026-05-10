#include "database.h"
#include <QCryptographicHash>
#include <QImage>
#include <QFileInfo>
#include <QBitArray>
#include <cmath>

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


    QSqlQuery checkColumn;
    checkColumn.exec("PRAGMA table_info(users)");
    bool hasIsAdmin = false;
    while (checkColumn.next()) {
        if (checkColumn.value(1).toString() == "is_admin") {
            hasIsAdmin = true;
            break;
        }
    }
    if (!hasIsAdmin) {
        qDebug() << "Adding is_admin column to users table...";
        if (!query.exec("ALTER TABLE users ADD COLUMN is_admin INTEGER DEFAULT 0")) {
            qDebug() << "Failed to add is_admin column:" << query.lastError().text();
        } else {
            qDebug() << "is_admin column added successfully";
        }
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

    // Добавить тестового админа, если его нет
    QSqlQuery checkAdmin;
    checkAdmin.prepare("SELECT id FROM users WHERE username = 'admin'");
    if (!checkAdmin.exec() || !checkAdmin.next()) {
        QSqlQuery insertAdmin;
        insertAdmin.prepare("INSERT INTO users (username, password, is_admin) VALUES ('admin', 'admin', 1)");
        if (!insertAdmin.exec()) {
            qDebug() << "Failed to create admin user:" << insertAdmin.lastError().text();
        } else {
            qDebug() << "Default admin user created: admin/admin (is_admin=1)";
        }
    } else {
        QSqlQuery updateAdmin;
        updateAdmin.prepare("UPDATE users SET is_admin = 1 WHERE username = 'admin'");
        updateAdmin.exec();
    }

    qDebug() << "Tables created successfully";
    return true;
}

//ОСНОВНЫЕ ФУНКЦИИ (РЕАЛЬНЫЕ РЕАЛИЗАЦИИ)

bool Database::vigenereCipher(const QString &text, const QString &key, QString &result)
{
    if (key.isEmpty() || text.isEmpty()) return false;

    QString keyUpper = key.toUpper();
    QString textUpper = text.toUpper();
    QString encrypted;

    int keyLen = keyUpper.length();
    for (int i = 0, j = 0; i < textUpper.length(); ++i) {
        QChar ch = textUpper[i];
        if (ch.isLetter()) {
            int shift = keyUpper[j % keyLen].unicode() - 'A';
            char encryptedChar = 'A' + (ch.unicode() - 'A' + shift) % 26;
            encrypted.append(encryptedChar);
            j++;
        } else {
            encrypted.append(ch);
        }
    }
    result = encrypted;
    return true;
}

bool Database::sha384Hash(const QString &text, QString &hash)
{
    QByteArray data = text.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(data, QCryptographicHash::Sha384);
    hash = hashBytes.toHex();
    return true;
}

bool Database::chordMethod(double a, double b, double epsilon, double &result)
{
    // Решаем уравнение x^2 - 2 = 0 (корень sqrt(2))
    auto f = [](double x) { return x * x - 2.0; };

    double fa = f(a);
    double fb = f(b);
    if (fa * fb >= 0) {
        result = 0;
        return false;
    }

    double x_prev = a;
    double x_curr = b;
    double f_prev = fa;
    double f_curr = fb;
    int iter = 0;
    const int maxIter = 1000;

    while (fabs(x_curr - x_prev) > epsilon && iter < maxIter) {
        double x_next = x_curr - f_curr * (x_curr - x_prev) / (f_curr - f_prev);
        x_prev = x_curr;
        f_prev = f_curr;
        x_curr = x_next;
        f_curr = f(x_curr);
        iter++;
    }
    result = x_curr;
    return true;
}

bool Database::hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath)
{
    QImage img(imagePath);
    if (img.isNull()) {
        qDebug() << "Cannot load image:" << imagePath;
        return false;
    }

    QImage working = img.convertToFormat(QImage::Format_ARGB32);
    if (working.isNull()) return false;

    QByteArray data = message.toUtf8();
    data.append('\0'); // терминатор

    int maxBytes = working.width() * working.height();
    if (data.size() > maxBytes) {
        qDebug() << "Message too long. Max bytes:" << maxBytes;
        return false;
    }

    int bitIndex = 0;
    for (int y = 0; y < working.height() && bitIndex < data.size() * 8; ++y) {
        for (int x = 0; x < working.width() && bitIndex < data.size() * 8; ++x) {
            QRgb pixel = working.pixel(x, y);
            int red = qRed(pixel);

            int bytePos = bitIndex / 8;
            int bitPos = bitIndex % 8;
            int bit = (data[bytePos] >> (7 - bitPos)) & 1;

            red = (red & 0xFE) | bit;
            QRgb newPixel = qRgba(red, qGreen(pixel), qBlue(pixel), qAlpha(pixel));
            working.setPixel(x, y, newPixel);
            bitIndex++;
        }
    }

    QFileInfo info(imagePath);
    outputPath = info.path() + "/" + info.baseName() + "_embedded.png";
    if (!working.save(outputPath, "PNG")) {
        qDebug() << "Failed to save image to" << outputPath;
        return false;
    }
    return true;
}

//  ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ

bool Database::registerUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, is_admin) VALUES (?, ?, 0)");
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

bool Database::loginUser(const QString &username, const QString &password, bool &isAdmin)
{
    QSqlQuery query;
    query.prepare("SELECT is_admin FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.exec();
    if (query.next()) {
        isAdmin = query.value(0).toBool();
        qDebug() << "Login success:" << username << "isAdmin:" << isAdmin;
        return true;
    } else {
        qDebug() << "Login failed:" << username;
        isAdmin = false;
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

// АДМИНСКИЕ МЕТОДЫ

QStringList Database::getAllUsers()
{
    QStringList users;
    QSqlQuery query("SELECT username FROM users ORDER BY id");
    while (query.next()) {
        users << query.value(0).toString();
    }
    return users;
}

QStringList Database::getAllLogs()
{
    QStringList logs;
    QSqlQuery query("SELECT id, command, request, result, timestamp FROM request_logs ORDER BY id DESC");
    while (query.next()) {
        QString line = QString("[%1] %2 | %3 -> %4")
        .arg(query.value(4).toString())
            .arg(query.value(1).toString())
            .arg(query.value(2).toString())
            .arg(query.value(3).toString());
        logs << line;
    }
    return logs;
}

bool Database::clearLogs()
{
    QSqlQuery query("DELETE FROM request_logs");
    return query.exec();
}

bool Database::deleteUser(const QString &username)
{
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE username = ? AND is_admin = 0");
    query.addBindValue(username);
    return query.exec();
}