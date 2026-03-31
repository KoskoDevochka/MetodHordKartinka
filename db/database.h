#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

// Forward declaration для класса-уничтожителя
class DatabaseDestroyer;

class Database : public QObject
{
    Q_OBJECT
    
private:
    // Статические члены для синглтона
    static Database *p_instance;
    static DatabaseDestroyer destroyer;
    
    // Объект базы данных
    QSqlDatabase db;
    
    // Приватный конструктор (синглтон)
    explicit Database(QObject *parent = nullptr);
    
    // Запрещаем копирование и присваивание
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // Деструктор (приватный, вызывается только через destroyer)
    ~Database();
    
    // Метод подключения к БД
    bool connectToDatabase();
    
    friend class DatabaseDestroyer;

public:
    // Получение экземпляра синглтона
    static Database* getInstance();
    
    // Инициализация таблиц
    bool createTables();
    
    // Методы для твоей темы (заглушки)
    bool vigenereCipher(const QString &text, const QString &key, QString &result);
    bool sha384Hash(const QString &text, QString &hash);
    bool chordMethod(double a, double b, double epsilon, double &result);
    bool hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath);
    
    // Дополнительные методы для авторизации (для будущего функционала)
    bool registerUser(const QString &username, const QString &password);
    bool loginUser(const QString &username, const QString &password);
    bool saveRequestLog(const QString &command, const QString &params, const QString &result);
};

// Класс для автоматического удаления синглтона
class DatabaseDestroyer
{
private:
    Database *p_instance;
    
public:
    ~DatabaseDestroyer()
    {
        if (p_instance) {
            delete p_instance;
            p_instance = nullptr;
        }
    }
    
    void initialize(Database *p)
    {
        p_instance = p;
    }
};

#endif // DATABASE_H