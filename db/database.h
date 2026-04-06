#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QDebug>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class Database : public QObject
{
    Q_OBJECT
public:
    static Database* getInstance();
    explicit Database(QObject *parent = nullptr);
    ~Database();
    
    // Методы для работы с БД
    bool connectToDatabase();
    bool executeQuery(const QString &query);
    bool createTables();
    
    // Методы для обработки запросов
    bool vigenereCipher(const QString &text, const QString &key, QString &encrypted);
    bool sha384Hash(const QString &text, QString &hash);
    bool chordMethod(double a, double b, double eps, double &result);
    bool hideMessageInImage(const QString &imagePath, const QString &message, const QString &outputPath);
    bool registerUser(const QString &username, const QString &password);
    bool loginUser(const QString &username, const QString &password);
    bool saveRequestLog(const QString &command, const QString &request, const QString &result);
    
private:
    static Database* m_instance;
    QSqlDatabase m_db;
};

#endif
