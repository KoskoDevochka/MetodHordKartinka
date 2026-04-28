#include <QApplication>
#include <QThread>
#include <QTimer>
#include "mytcpserver.h"
#include "database.h"
#include "clientwindow.h"

class ServerThread : public QThread
{
public:
    void run() override {
        Database::getInstance();
        MyTcpServer::getInstance();
        exec();
    }

    ~ServerThread() {
        quit();
        wait();
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ServerThread serverThread;
    serverThread.start();

    QThread::sleep(1);

    ClientWindow *client = new ClientWindow();
    client->show();

    // Если окно клиента закрыто, но сервер ещё нужен — ничего не делаем
    // Если нужно завершить программу после закрытия клиента:
    QObject::connect(client, &ClientWindow::destroyed, [&]() {
        QTimer::singleShot(1000, &app, &QApplication::quit);
    });

    return app.exec();
}
