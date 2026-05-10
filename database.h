#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Database : public QObject
{
    Q_OBJECT

private:
    static Database* m_instance;
    QSqlDatabase m_db;

    explicit Database(QObject *parent = nullptr);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

public:
    static Database* getInstance();
    ~Database();

    // Инициализация
    bool connectToDatabase();
    bool createTables();

    // функции
    bool vigenereCipher(const QString &text, const QString &key, QString &result);
    bool sha384Hash(const QString &text, QString &hash);
    bool chordMethod(double a, double b, double epsilon, double &result);
    bool hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath);

    // Дополнительные методы
    bool registerUser(const QString &username, const QString &password);
    bool loginUser(const QString &username, const QString &password, bool &isAdmin); // изменён
    bool saveRequestLog(const QString &command, const QString &request, const QString &result);

    // АДМИНСКИЕ МЕТОДЫ
    QStringList getAllUsers();                  // список имён пользователей
    QStringList getAllLogs();                   // список строк логов (id, команда, запрос, результат, время)
    bool clearLogs();                           // очистка таблицы логов
    bool deleteUser(const QString &username);   // удаление пользователя
};

#endif // DATABASE_H