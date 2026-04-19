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
    /*!
     * \brief Обрабатывает новое подключение клиента.
     */
    void slotNewConnection();

    /*!
     * \brief Обрабатывает отключение клиента.
     */
    void slotClientDisconnected();

    /*!
     * \brief Обрабатывает входящие данные от клиента.
     */
    void slotServerRead();

private:
    /*!
     * \brief Приватный конструктор (синглтон).
     * \param parent Родительский QObject.
     */
    explicit MyTcpServer(QObject *parent = nullptr);

    /*!
     * \brief Структура для хранения данных каждого клиента (задание №5)
     */
    struct ClientInfo {
        QTcpSocket* socket;   ///< Сокет клиента
        QString buffer;       ///< Буфер для накопления команды
    };

    QTcpServer* mTcpServer;                       ///< Серверный TCP-сокет
    QTcpSocket* mTcpSocket; 
    QMap<QTcpSocket*, ClientInfo> m_clients;      ///< Список всех подключённых клиентов

    /*!
     * \brief Разбирает текстовую команду от клиента.
     * \param client Сокет клиента
     * \param request Строка запроса (например "action1 Hello")
     */
    void parseRequest(QTcpSocket* client, const QString& request);

    /*!
     * \brief Заглушка для команды action1 (задание №3)
     * \param client Сокет клиента
     * \param data Данные, переданные с командой
     */
    void stub_function1(QTcpSocket* client, const QString& data);

    /*!
     * \brief Заглушка для команды action2 (задание №3)
     * \param client Сокет клиента
     * \param data Данные, переданные с командой
     */
    void stub_function2(QTcpSocket* client, const QString& data);

    /*!
     * \brief Заглушка для команды help (задание №3)
     * \param client Сокет клиента
     */
    void stub_function3(QTcpSocket* client);

    /*!
     * \brief Обрабатывает HTTP-запросы (задание подруги)
     * \param client Сокет клиента
     * \param request Сырой HTTP-запрос
     */
    void handleHttpRequest(QTcpSocket* client, const QByteArray& request);

    /*!
     * \brief Отправляет сообщение клиенту.
     * \param client Сокет клиента
     * \param message Текст сообщения
     */
    void sendToClient(QTcpSocket* client, const QString& message);

    /*!
     * \brief Рассылает сообщение всем клиентам.
     * \param message Текст сообщения
     * \param exclude Сокет клиента, которому не нужно отправлять (опционально)
     */
    void broadcastToAll(const QString& message, QTcpSocket* exclude = nullptr);

   // АВТОРИЗАЦИЯ И РЕГИСТРАЦИЯ (задание №6) 
    /*!
     * \brief Обрабатывает регистрацию пользователя.
     * \param client Сокет клиента
     * \param username Имя пользователя
     * \param password Пароль
     */
    void handleRegister(QTcpSocket* client, const QString& username, const QString& password);

    /*!
     * \brief Обрабатывает вход пользователя.
     * \param client Сокет клиента
     * \param username Имя пользователя
     * \param password Пароль
     */
    void handleLogin(QTcpSocket* client, const QString& username, const QString& password);
};



#endif // MYTCPSERVER_H
