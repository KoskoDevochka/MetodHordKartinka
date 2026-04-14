#include <QCoreApplication>
#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    // Правильное использование синглтона - через getInstance()
    MyTcpServer* server = MyTcpServer::getInstance();
    
    // Если нужно использовать Database:
    // Database* db = Database::getInstance();
    
    return a.exec();
}