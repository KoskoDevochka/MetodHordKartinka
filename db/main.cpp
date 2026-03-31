#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "TAM&P Server Starting...";
    qDebug() << "========================================";
    
    // Инициализация сервера (БД автоматически инициализируется как синглтон)
    MyTcpServer server;
    
    qDebug() << "Server is running...";
    qDebug() << "Press Ctrl+C to stop";
    qDebug() << "========================================";
    
    return a.exec();
}