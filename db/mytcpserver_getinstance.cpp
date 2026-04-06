// Добавьте эту функцию в mytcpserver.cpp после конструктора
MyTcpServer* MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return &instance;
}
