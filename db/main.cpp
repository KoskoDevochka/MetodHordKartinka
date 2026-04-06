#include <QCoreApplication>
#include <QDebug>
#include "database.h"
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    qDebug() << "Starting TCP Server...";
    
    // Инициализация базы данных
    Database* db = Database::getInstance();
    if (db->connectToDatabase()) {
        qDebug() << "Database connected successfully";
    } else {
        qDebug() << "Failed to connect to database";
        return 1;
    }
    
    // Запуск TCP сервера
    MyTcpServer* server = MyTcpServer::getInstance();
    if (server) {
        qDebug() << "Server started successfully on port 12345";
    } else {
        qDebug() << "Failed to start server";
        return 1;
    }
    
    return a.exec();
}
