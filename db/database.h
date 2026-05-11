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

    // ТВОИ ФУНКЦИИ (заглушки)
    bool vigenereCipher(const QString &text, const QString &key, QString &result);
    bool sha384Hash(const QString &text, QString &hash);
    bool chordMethod(double a, double b, double epsilon, double &result);
    bool hideMessageInImage(const QString &imagePath, const QString &message, QString &outputPath);

    // Авторизация и регистрация
    bool registerUser(const QString &username, const QString &password);
    bool loginUser(const QString &username, const QString &password);
    bool saveRequestLog(const QString &command, const QString &request, const QString &result);

    // ==================== РОЛИ (задание №7) ====================

    /*!
     * \brief Получить роль пользователя из БД.
     * \param username Имя пользователя
     * \return Роль: "user", "admin" или "" (если не найден)
     */
    QString getUserRole(const QString &username);

    /*!
     * \brief Установить роль пользователя.
     * \param username Имя пользователя
     * \param role Новая роль ("user" или "admin")
     * \return true если успешно
     */
    bool setUserRole(const QString &username, const QString &role);

    /*!
     * \brief Получить список всех пользователей с ролями.
     * \return Список строк вида "username: role"
     */
    QStringList getAllUsersWithRoles();
};

#endif // DATABASE_H
