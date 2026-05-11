#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QHostAddress>

/*!
 * \brief Класс TCP-сервера для обработки клиентских подключений.
 *
 * MyTcpServer реализует TCP-сервер, который слушает указанный порт,
 * принимает входящие соединения и обрабатывает данные от клиентов.
 * Реализован как синглтон.
 *
 * Поддерживает:
 * - HTTP-запросы от браузера (задание подруги)
 * - Telnet-команды action1, action2, help (задание №3)
 * - Несколько клиентов одновременно (задание №5)
 * - Авторизацию и регистрацию (задание №6)
 * - Механизм ролей: guest / user / admin (задание №7)
 */
class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Деструктор. Закрывает серверный сокет.
     */
    ~MyTcpServer();

    /*!
     * \brief Запрещаем копирование (синглтон).
     */
    MyTcpServer(const MyTcpServer&) = delete;
    MyTcpServer& operator=(const MyTcpServer&) = delete;

    /*!
     * \brief Получить единственный экземпляр сервера (синглтон).
     * \return Указатель на экземпляр MyTcpServer.
     */
    static MyTcpServer* getInstance();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

private:
    explicit MyTcpServer(QObject *parent = nullptr);

    //  РОЛИ (задание №7)
    /*!
     * \brief Перечисление ролей клиента.
     *
     * - Guest  : подключился, но не авторизован
     * - User   : вошёл как обычный пользователь
     * - Admin  : вошёл как администратор
     */
    enum class Role {
        Guest,  ///< Не авторизован
        User,   ///< Обычный пользователь
        Admin   ///< Администратор
    };

    /*!
     * \brief Структура для хранения данных каждого клиента.
     */
    struct ClientInfo {
        QTcpSocket* socket;   ///< Сокет клиента
        QString buffer;       ///< Буфер для накопления команды
        Role    role;         ///< Текущая роль (задание №7)
        QString username;     ///< Имя пользователя после логина (задание №7)
    };

    QTcpServer* mTcpServer;
    QTcpSocket* mTcpSocket;
    QMap<QTcpSocket*, ClientInfo> m_clients;

    // Парсинг и обработка команд
    void parseRequest(QTcpSocket* client, const QString& request);
    void stub_function1(QTcpSocket* client, const QString& data);
    void stub_function2(QTcpSocket* client, const QString& data);
    void stub_function3(QTcpSocket* client);
    void handleHttpRequest(QTcpSocket* client, const QByteArray& request);

    // Авторизация
    void handleRegister(QTcpSocket* client, const QString& username, const QString& password);
    void handleLogin(QTcpSocket* client, const QString& username, const QString& password);

    // ==================== РОЛИ (задание №7) ====================

    /*!
     * \brief Проверяет, имеет ли клиент требуемую роль или выше.
     * \param client  Сокет клиента
     * \param required Минимально необходимая роль
     * \return true если роль клиента >= required
     */
    bool hasRole(QTcpSocket* client, Role required);

    /*!
     * \brief Назначает роль другому пользователю (только Admin).
     * \param client    Сокет клиента-администратора
     * \param username  Целевой пользователь
     * \param role      Новая роль ("user" или "admin")
     */
    void handleSetRole(QTcpSocket* client, const QString& username, const QString& role);

    /*!
     * \brief Выводит список всех пользователей и их ролей (только Admin).
     * \param client Сокет клиента-администратора
     */
    void handleListUsers(QTcpSocket* client);

    // Утилиты
    void sendToClient(QTcpSocket* client, const QString& message);
    void broadcastToAll(const QString& message, QTcpSocket* exclude = nullptr);

    /*!
     * \brief Преобразует Role в строку для отображения.
     */
    static QString roleToString(Role role);
};

#endif // MYTCPSERVER_H
