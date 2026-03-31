#include "database.h"

// Инициализация статических переменных
Database* Database::p_instance = nullptr;
DatabaseDestroyer Database::destroyer;

// ============================================
// Конструктор и деструктор
// ============================================

Database::Database(QObject *parent) : QObject(parent)
{
    qDebug() << "Initializing Database singleton...";
    
    if (connectToDatabase()) {
        createTables();
        qDebug() << "Database initialized successfully!";
    } else {
        qDebug() << "Database initialization failed!";
    }
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database connection closed";
    }
}

// ============================================
// Подключение к БД
// ============================================

bool Database::connectToDatabase()
{
    // Используем SQLite
    db = QSqlDatabase::addDatabase("QSQLITE");
    
    // Универсальный путь для разных ОС
    #ifdef Q_OS_WIN
        // Для Windows
        db.setDatabaseName("server.db");
        qDebug() << "Using Windows path: server.db";
    #else
        // Для Linux/Docker
        db.setDatabaseName("/app/data/server.db");
        qDebug() << "Using Linux path: /app/data/server.db";
    #endif
    
    if (!db.open()) {
        qDebug() << "ERROR: Database connection failed!";
        qDebug() << "Error:" << db.lastError().text();
        return false;
    }
    
    qDebug() << "Database connected successfully!";
    return true;
}

// ============================================
// Создание таблиц
// ============================================

bool Database::createTables()
{
    QSqlQuery query;
    
    // Таблица пользователей (для авторизации)
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role TEXT DEFAULT 'user',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createUsersTable)) {
        qDebug() << "ERROR: Failed to create users table:" << query.lastError().text();
        return false;
    }
    
    // Таблица для логов запросов
    QString createLogsTable = R"(
        CREATE TABLE IF NOT EXISTS request_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            command TEXT NOT NULL,
            parameters TEXT,
            result TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createLogsTable)) {
        qDebug() << "ERROR: Failed to create logs table:" << query.lastError().text();
        return false;
    }
    
    // Таблица для шифрования Виженера
    QString createVigenereTable = R"(
        CREATE TABLE IF NOT EXISTS vigenere_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            input_text TEXT NOT NULL,
            key TEXT NOT NULL,
            output_text TEXT NOT NULL,
            operation_type TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createVigenereTable)) {
        qDebug() << "ERROR: Failed to create vigenere table:" << query.lastError().text();
        return false;
    }
    
    // Таблица для SHA-384 хэшей
    QString createShaTable = R"(
        CREATE TABLE IF NOT EXISTS sha_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            input_text TEXT NOT NULL,
            hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createShaTable)) {
        qDebug() << "ERROR: Failed to create sha table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "All tables created successfully!";
    return true;
}

// ============================================
// Получение экземпляра синглтона
// ============================================

Database* Database::getInstance()
{
    if (!p_instance) {
        p_instance = new Database();
        destroyer.initialize(p_instance);
        qDebug() << "Database singleton instance created";
    }
    return p_instance;
}

// ============================================
// ЗАГЛУШКИ ДЛЯ ФУНКЦИОНАЛА
// ============================================

bool Database::vigenereCipher(const QString &text, const QString &key, QString &result)
{
    qDebug() << "Vigenere cipher called (STUB)";
    qDebug() << "  Text:" << text;
    qDebug() << "  Key:" << key;
    
    // ЗАГЛУШКА - здесь будет реальная реализация
    result = QString("[STUB] Encrypted: %1 with key: %2").arg(text).arg(key);
    
    // Сохраняем в БД
    QSqlQuery query;
    query.prepare("INSERT INTO vigenere_history (input_text, key, output_text, operation_type) VALUES (?, ?, ?, 'encrypt')");
    query.addBindValue(text);
    query.addBindValue(key);
    query.addBindValue(result);
    query.exec();
    
    return true;
}

bool Database::sha384Hash(const QString &text, QString &hash)
{
    qDebug() << "SHA-384 hash called (STUB)";
    qDebug() << "  Text:" << text;
    
    // ЗАГЛУШКА - здесь будет реальная реализация
    hash = QString("[STUB] SHA384 hash of: %1").arg(text);
    
    // Сохраняем в БД
    QSqlQuery query;
    query.prepare("INSERT INTO sha_history (input_text, hash) VALUES (?, ?)");
    query.addBindValue(text);
    query.addBindValue(hash);
    query.exec();
    
    return true;
}

bool Database::chordMethod(double a, double b, double epsilon, double &result)
{
    qDebug() << "Chord method called (STUB)";
    qDebug() << "  a:" << a << "b:" << b << "epsilon:" << epsilon;
    
    // ЗАГЛУШКА - здесь будет реальная реализация
    result = (a + b) / 2.0;  // Просто среднее значение
    
    // Сохраняем в БД
    QSqlQuery query;
    query.prepare("INSERT INTO request_logs (command, parameters, result) VALUES ('CHORD', ?, ?)");
    QString params = QString("a=%1, b=%2, eps=%3").arg(a).arg(b).arg(epsilon);
    query.addBindValue(params);
    query.addBindValue(QString::number(result));
    query.exec();
    
    return true;
}

bool Database::hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath)
{
    qDebug() << "Hide message in image called (STUB)";
    qDebug() << "  Image:" << imagePath;
    qDebug() << "  Message:" << message;
    
    // ЗАГЛУШКА - здесь будет реальная реализация
    outputPath = QString("[STUB] %1_encoded.png").arg(imagePath);
    
    return true;
}

// ============================================
// АВТОРИЗАЦИЯ (заглушки)
// ============================================

bool Database::registerUser(const QString &username, const QString &password)
{
    qDebug() << "Register user called (STUB)";
    qDebug() << "  Username:" << username;
    
    // ЗАГЛУШКА - всегда успешно
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    
    if (query.exec()) {
        qDebug() << "User registered successfully";
        return true;
    } else {
        qDebug() << "User registration failed:" << query.lastError().text();
        return false;
    }
}

bool Database::loginUser(const QString &username, const QString &password)
{
    qDebug() << "Login user called (STUB)";
    qDebug() << "  Username:" << username;
    
    // ЗАГЛУШКА - проверяем существование пользователя
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    query.exec();
    
    if (query.next()) {
        qDebug() << "Login successful";
        return true;
    } else {
        qDebug() << "Login failed - user not found";
        return false;
    }
}

bool Database::saveRequestLog(const QString &command, const QString &params, const QString &result)
{
    QSqlQuery query;
    query.prepare("INSERT INTO request_logs (command, parameters, result) VALUES (?, ?, ?)");
    query.addBindValue(command);
    query.addBindValue(params);
    query.addBindValue(result);
    
    return query.exec();
}