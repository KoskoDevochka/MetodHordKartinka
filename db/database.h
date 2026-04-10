/*!
 * \file database.h
 * \brief Заголовочный файл класса Database.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

/*!
 * \brief Класс для работы с SQLite базой данных.
 * 
 * Database предоставляет методы для аутентификации пользователей,
 * шифрования данных и сохранения логов. Реализован как синглтон.
 */
class Database : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Получить единственный экземпляр базы данных.
     * \return Указатель на Database.
     */
    static Database* getInstance();
    
    /*!
     * \brief Деструктор. Закрывает соединение с БД.
     */
    ~Database();
    
    /*!
     * \brief Регистрация нового пользователя.
     * \param username Имя пользователя.
     * \param password Пароль (будет захэширован SHA-384).
     * \return true при успешной регистрации, false при ошибке.
     */
    bool registerUser(const QString &username, const QString &password);
    
    /*!
     * \brief Аутентификация пользователя.
     * \param username Имя пользователя.
     * \param password Пароль.
     * \return true если пользователь существует и пароль верен.
     */
    bool loginUser(const QString &username, const QString &password);
    
    /*!
     * \brief Метод хорд для решения уравнений.
     * \param a Левая граница интервала.
     * \param b Правая граница интервала.
     * \param eps Точность вычислений.
     * \param result Ссылка для сохранения результата.
     * \return true при успешном вычислении.
     */
    bool chordMethod(double a, double b, double eps, double &result);
    
private:
    /*!
     * \brief Приватный конструктор (синглтон).
     * \param parent Родительский QObject.
     */
    explicit Database(QObject *parent = nullptr);
    
    static Database* m_instance;  ///< Единственный экземпляр класса
    QSqlDatabase m_db;            ///< Объект подключения к БД
};

#endif