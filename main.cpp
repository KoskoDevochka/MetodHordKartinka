#include <QCoreApplication>
#include "mytcpserver.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Инициализация БД (синглтон создастся автоматически)
    Database* db = Database::getInstance();
    Q_UNUSED(db); // чтобы не было warning о неиспользуемой переменной
    
    // Правильное использование синглтона - через getInstance()
    MyTcpServer* server = MyTcpServer::getInstance();
    Q_UNUSED(server);
    
    // Если нужно использовать Database:
    // Database* db = Database::getInstance();
    
    return a.exec();
}
