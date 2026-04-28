#include "clientwindow.h"
#include "mytcpserver.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QCheckBox>
#include <QTimer>
#include <QCloseEvent>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), isAuthorized(false), adminLogWindow(nullptr)
{
    showAuthDialog();
}

ClientWindow::~ClientWindow() {}

void ClientWindow::showAuthDialog()
{
    QDialog *authDialog = new QDialog(this);
    authDialog->setWindowTitle("Авторизация");
    authDialog->setFixedSize(300, 220);
    authDialog->setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(authDialog);

    layout->addWidget(new QLabel("Логин:"));
    usernameEdit = new QLineEdit;
    layout->addWidget(usernameEdit);

    layout->addWidget(new QLabel("Пароль:"));
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QCheckBox *adminCheckbox = new QCheckBox("Войти как администратор");
    layout->addWidget(adminCheckbox);

    layout->addSpacing(10);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *loginBtn = new QPushButton("Войти");
    QPushButton *registerBtn = new QPushButton("Зарегистрироваться");
    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(registerBtn);
    layout->addLayout(btnLayout);

    statusAuthLabel = new QLabel("");
    statusAuthLabel->setStyleSheet("color: red;");
    layout->addWidget(statusAuthLabel);

    // Если сокет ещё не подключён, подключаем
    if (socket->state() != QTcpSocket::ConnectedState) {
        socket->connectToHost("127.0.0.1", 33333);
        if (!socket->waitForConnected(3000)) {
            statusAuthLabel->setText("Ошибка: сервер не запущен");
        }
    }

    // ЛОГИН
    connect(loginBtn, &QPushButton::clicked, [this, authDialog, adminCheckbox]() {
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text().trimmed();
        bool isAdmin = adminCheckbox->isChecked();

        if (username.isEmpty() || password.isEmpty()) {
            statusAuthLabel->setText("Заполните все поля");
            return;
        }

        socket->write(QString("login %1 %2\r\n").arg(username).arg(password).toUtf8());
        socket->flush();

        if (socket->waitForReadyRead(5000)) {
            QByteArray response = socket->readAll();
            QString respStr = QString::fromUtf8(response);

            if (respStr.contains("SUCCESS")) {
                isAuthorized = true;

                if (isAdmin) {
                    showAdminLogWindow();  //открываем окно с логами
                }

                authDialog->close();
                showMainWindow();
                return;
            } else {
                statusAuthLabel->setText("Неверный логин или пароль.\nЕсли нет аккаунта — зарегистрируйтесь.");
            }
        } else {
            statusAuthLabel->setText("Нет ответа от сервера");
        }
    });

    connect(registerBtn, &QPushButton::clicked, [this]() {
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text().trimmed();

        if (username.isEmpty() || password.isEmpty()) {
            statusAuthLabel->setText("Заполните все поля");
            return;
        }

        socket->write(QString("register %1 %2\r\n").arg(username).arg(password).toUtf8());
        socket->flush();

        if (socket->waitForReadyRead(5000)) {
            QByteArray response = socket->readAll();
            QString respStr = QString::fromUtf8(response);

            if (respStr.contains("SUCCESS")) {
                statusAuthLabel->setStyleSheet("color: green;");
                statusAuthLabel->setText("Регистрация успешна! Теперь войдите.");
                usernameEdit->clear();
                passwordEdit->clear();
            } else {
                statusAuthLabel->setStyleSheet("color: red;");
                statusAuthLabel->setText("Ошибка: пользователь уже существует");
            }
        } else {
            statusAuthLabel->setText("Ошибка: нет ответа от сервера");
        }
    });

    authDialog->exec();

    if (!isAuthorized) {
        QApplication::quit();
    }
}

void ClientWindow::showMainWindow()
{
    setWindowTitle("Клиент MetodHordKartinka");
    resize(700, 600);

    QWidget *central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    statusLabel = new QLabel("● Авторизован");
    statusLabel->setStyleSheet("color: green;");
    mainLayout->addWidget(statusLabel);

    QGroupBox *funcBox = new QGroupBox("Выберите функцию");
    QVBoxLayout *funcLayout = new QVBoxLayout;
    functionCombo = new QComboBox;
    functionCombo->addItem("Шифр Виженера");
    functionCombo->addItem("SHA-384");
    functionCombo->addItem("Метод хорд");
    functionCombo->addItem("Стеганография");
    funcLayout->addWidget(functionCombo);
    funcBox->setLayout(funcLayout);
    mainLayout->addWidget(funcBox);

    QGroupBox *inputBox = new QGroupBox("Входные данные");
    QVBoxLayout *inputLayout = new QVBoxLayout;
    inputArea = new QTextEdit;
    inputArea->setPlaceholderText("Введите текст для обработки...");
    inputLayout->addWidget(inputArea);
    inputBox->setLayout(inputLayout);
    mainLayout->addWidget(inputBox);

    sendBtn = new QPushButton("Отправить на сервер");
    sendBtn->setStyleSheet("background-color: lightblue; height: 30px;");
    mainLayout->addWidget(sendBtn);

    disconnectBtn = new QPushButton("Отключиться от сервера");
    disconnectBtn->setStyleSheet("background-color: orange; height: 30px;");
    mainLayout->addWidget(disconnectBtn);

    QGroupBox *logBox = new QGroupBox("Лог сообщений");
    QVBoxLayout *logLayout = new QVBoxLayout;
    logArea = new QTextEdit;
    logArea->setReadOnly(true);
    logLayout->addWidget(logArea);
    logBox->setLayout(logLayout);
    mainLayout->addWidget(logBox);

    connect(sendBtn, &QPushButton::clicked, this, &ClientWindow::sendRequest);
    connect(disconnectBtn, &QPushButton::clicked, this, &ClientWindow::disconnectFromServer);
    connect(socket, &QTcpSocket::readyRead, this, &ClientWindow::onReadyRead);

    logArea->append("Авторизация успешна!");
}

void ClientWindow::showAdminLogWindow()
{
    if (adminLogWindow) {
        adminLogWindow->close();
        delete adminLogWindow;
    }

    adminLogWindow = new QWidget();
    adminLogWindow->setWindowTitle("Логи сервера (Админ)");
    adminLogWindow->resize(600, 400);

    QTextEdit *logAreaAdmin = new QTextEdit(adminLogWindow);
    logAreaAdmin->setReadOnly(true);
    logAreaAdmin->append("Логи сервера будут появляться здесь");
    logAreaAdmin->append("Администратор вошёл в систему");

    QVBoxLayout *layout = new QVBoxLayout(adminLogWindow);
    layout->addWidget(logAreaAdmin);

    adminLogWindow->show();

    MyTcpServer* server = MyTcpServer::getInstance();
    connect(server, &MyTcpServer::logMessage, logAreaAdmin, &QTextEdit::append);
}

void ClientWindow::sendRequest()
{
    int funcId = functionCombo->currentIndex();
    QString inputData = inputArea->toPlainText().trimmed();

    if (inputData.isEmpty()) {
        logArea->append("Введите данные для обработки");
        return;
    }

    QString message = QString("%1|%2\r\n").arg(funcId).arg(inputData);
    socket->write(message.toUtf8());
    socket->flush();

    logArea->append(QString("Отправлено: %1").arg(inputData.left(100)));
    inputArea->clear();
}

void ClientWindow::onReadyRead()
{
    QByteArray data = socket->readAll();
    logArea->append(QString("Ответ сервера:\n%1\n%2").arg(QString::fromUtf8(data)).arg(QString(50, '-')));
}

void ClientWindow::disconnectFromServer()
{
    // Отключаем сокет
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }

    // Закрываем окно логов админа
    if (adminLogWindow) {
        adminLogWindow->close();
        delete adminLogWindow;
        adminLogWindow = nullptr;
    }

    // Закрываем текущее окно клиента
    this->close();

    // Создаём новое окно клиента
    ClientWindow *newClient = new ClientWindow();
    newClient->show();
}

void ClientWindow::closeEvent(QCloseEvent *event)
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }

    if (adminLogWindow) {
        adminLogWindow->close();
        delete adminLogWindow;
        adminLogWindow = nullptr;
    }

    event->accept();
}
