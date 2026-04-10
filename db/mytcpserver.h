#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

/*!
 * \brief Класс TCP-сервера для обработки клиентских подключений.
 * 
 * MyTcpServer реализует TCP-сервер, который слушает указанный порт,
 * принимает входящие соединения и обрабатывает данные от клиентов.
 * Реализован как синглтон.
 * 
 * \author Student
 * \date 2026
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

private:
    /*!
     * \brief Приватный конструктор (синглтон).
     * \param parent Родительский QObject.
     */
    explicit MyTcpServer(QObject *parent = nullptr);
    
    QTcpServer* mTcpServer;  ///< Серверный TCP-сокет
    QTcpSocket* mTcpSocket;  ///< Сокет текущего клиента
};

#endif