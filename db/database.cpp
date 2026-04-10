#include "database.h"
#include <QCryptographicHash>
#include <QFile>
#include <cmath>
#include <QDir>

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
    qDebug() << "Database constructor called";
    connectToDatabase();
}

Database::~Database()
{
    qDebug() << "Database destructor called";
    if (m_db.isOpen()) {
        m_db.close();
    }
}

// Остальные методы остаются без изменений...
bool Database::connectToDatabase()
{
    QString dbPath = QDir::current().absolutePath() + "/server.db";
    qDebug() << "Database path:" << dbPath;
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    qDebug() << "Database connected successfully";
    
    QSqlQuery query;
    
    QString createUsers = "CREATE TABLE IF NOT EXISTS users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "username TEXT UNIQUE, "
                          "password TEXT)";
    
    QString createLogs = "CREATE TABLE IF NOT EXISTS logs ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "command TEXT, "
                         "request TEXT, "
                         "result TEXT, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)";
    
    if (!query.exec(createUsers)) {
        qDebug() << "Failed to create users table:" << query.lastError().text();
        return false;
    }
    
    if (!query.exec(createLogs)) {
        qDebug() << "Failed to create logs table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Tables created successfully";
    return true;
}

bool Database::createTables()
{
    return true;
}

bool Database::executeQuery(const QString &query)
{
    QSqlQuery sqlQuery;
    return sqlQuery.exec(query);
}

bool Database::vigenereCipher(const QString &text, const QString &key, QString &encrypted)
{
    encrypted.clear();
    QString keyRepeated = key;
    while (keyRepeated.length() < text.length()) {
        keyRepeated += key;
    }
    
    for (int i = 0; i < text.length(); ++i) {
        QChar textChar = text[i];
        QChar keyChar = keyRepeated[i];
        
        if (textChar.isLetter()) {
            QChar base = textChar.isUpper() ? 'A' : 'a';
            QChar keyBase = keyChar.isUpper() ? 'A' : 'a';
            
            int textPos = textChar.toLatin1() - base.toLatin1();
            int keyPos = keyChar.toLatin1() - keyBase.toLatin1();
            int encryptedPos = (textPos + keyPos) % 26;
            
            encrypted.append(QChar(base.toLatin1() + encryptedPos));
        } else {
            encrypted.append(textChar);
        }
    }
    return true;
}

bool Database::sha384Hash(const QString &text, QString &hash)
{
    QCryptographicHash hasher(QCryptographicHash::Sha384);
    hasher.addData(text.toUtf8());
    hash = hasher.result().toHex();
    return true;
}

bool Database::chordMethod(double a, double b, double eps, double &result)
{
    auto f = [](double x) { return x*x*x - 2*x - 5; };
    
    double x0 = a, x1 = b;
    double x2 = 0;
    int maxIter = 100;
    int iter = 0;
    
    while (iter < maxIter) {
        x2 = x1 - f(x1) * (x1 - x0) / (f(x1) - f(x0));
        if (fabs(x2 - x1) < eps) {
            result = x2;
            return true;
        }
        x0 = x1;
        x1 = x2;
        iter++;
    }
    result = x2;
    return true;
}

bool Database::hideMessageInImage(const QString &imagePath, const QString &message, const QString &outputPath)
{
    Q_UNUSED(imagePath);
    Q_UNUSED(message);
    Q_UNUSED(outputPath);
    return true;
}

bool Database::registerUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    QString hashedPassword;
    sha384Hash(password, hashedPassword);
    
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    
    if (!query.exec()) {
        qDebug() << "Register failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::loginUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    QString hashedPassword;
    sha384Hash(password, hashedPassword);
    
    query.prepare("SELECT id FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    
    if (!query.exec()) {
        qDebug() << "Login query failed:" << query.lastError().text();
        return false;
    }
    
    return query.next();
}

bool Database::saveRequestLog(const QString &command, const QString &request, const QString &result)
{
    QSqlQuery query;
    query.prepare("INSERT INTO logs (command, request, result) VALUES (?, ?, ?)");
    query.addBindValue(command);
    query.addBindValue(request);
    query.addBindValue(result);
    
    if (!query.exec()) {
        qDebug() << "Failed to save log:" << query.lastError().text();
        return false;
    }
    return true;
}