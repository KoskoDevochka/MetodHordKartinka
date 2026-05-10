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
#include <QHeaderView>
#include <QMessageBox>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTextCursor>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), isAuthorized(false), adminLogWindow(nullptr), adminOutput(nullptr)
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
        bool isAdmin = adminCheckbox->isChecked(); // только для UI, сервер сам проверит

        if (username.isEmpty() || password.isEmpty()) {
            statusAuthLabel->setText("Заполните все поля");
            return;
        }

        socket->write(QString("login %1 %2\r\n").arg(username, password).toUtf8());
        socket->flush();

        if (socket->waitForReadyRead(5000)) {
            QByteArray response = socket->readAll();
            QString respStr = QString::fromUtf8(response);

            if (respStr.contains("SUCCESS")) {
                isAuthorized = true;

                // Если пользователь хочет админский интерфейс (галочка), показываем окно
                // Но сервер разрешит команды только если в БД is_admin=1
                if (isAdmin) {
                    showAdminLogWindow();
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

    // РЕГИСТРАЦИЯ
    connect(registerBtn, &QPushButton::clicked, [this]() {
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text().trimmed();

        if (username.isEmpty() || password.isEmpty()) {
            statusAuthLabel->setText("Заполните все поля");
            return;
        }

        socket->write(QString("register %1 %2\r\n").arg(username, password).toUtf8());
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
    resize(900, 700);

    QWidget *central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ВЕРХНЯЯ ПАНЕЛЬ С МЕТКАМИ СТАТУСА
    QHBoxLayout *statusLayout = new QHBoxLayout();

    statusLabel = new QLabel("● Авторизован");
    statusLabel->setStyleSheet("color: green; font-weight: bold;");
    statusLayout->addWidget(statusLabel);

    statusLayout->addStretch();

    connectionStatusLabel = new QLabel(" Подключено к серверу");
    connectionStatusLabel->setStyleSheet("color: green;");
    statusLayout->addWidget(connectionStatusLabel);

    mainLayout->addLayout(statusLayout);

    //  ВЫБОР ФУНКЦИИ
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

    //  ВХОДНЫЕ ДАННЫЕ
    QGroupBox *inputBox = new QGroupBox("Входные данные");
    QVBoxLayout *inputLayout = new QVBoxLayout;
    inputArea = new QTextEdit;
    inputArea->setPlaceholderText("Введите текст для обработки...\nДля метода хорд: a b epsilon (через пробел)\nДля стеганографии: путь_к_картинке|сообщение");
    inputLayout->addWidget(inputArea);
    inputBox->setLayout(inputLayout);
    mainLayout->addWidget(inputBox);

    //  КНОПКИ
    QHBoxLayout *btnLayout = new QHBoxLayout();
    sendBtn = new QPushButton("Отправить на сервер");
    sendBtn->setStyleSheet("background-color: lightblue; height: 30px;");
    disconnectBtn = new QPushButton("Отключиться от сервера");
    disconnectBtn->setStyleSheet("background-color: orange; height: 30px;");
    btnLayout->addWidget(sendBtn);
    btnLayout->addWidget(disconnectBtn);
    mainLayout->addLayout(btnLayout);

    // ТАБЛИЦА ДАННЫХ (2 колонки)
    QLabel *tableTitle = new QLabel(" Данные от сервера (Таблица)");
    tableTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(tableTitle);

    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(2);
    QStringList headers;
    headers << "Функция" << "Ответ сервера";
    dataTable->setHorizontalHeaderLabels(headers);
    dataTable->horizontalHeader()->setStretchLastSection(true);
    dataTable->setAlternatingRowColors(true);
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable->setMinimumHeight(200);
    dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(dataTable);

    QGroupBox *logBox = new QGroupBox("Лог сообщений (текстовый)");
    QVBoxLayout *logLayout = new QVBoxLayout;
    logArea = new QTextEdit;
    logArea->setReadOnly(true);
    logLayout->addWidget(logArea);
    logBox->setLayout(logLayout);
    mainLayout->addWidget(logBox);

    // СИГНАЛЫ
    connect(sendBtn, &QPushButton::clicked, this, &ClientWindow::sendRequest);
    connect(disconnectBtn, &QPushButton::clicked, this, &ClientWindow::disconnectFromServer);
    connect(socket, &QTcpSocket::readyRead, this, &ClientWindow::onReadyRead);

    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        if (connectionStatusLabel) {
            connectionStatusLabel->setText(" Отключено от сервера");
            connectionStatusLabel->setStyleSheet("color: red;");
        }
    });

    connect(socket, &QTcpSocket::connected, this, [this]() {
        if (connectionStatusLabel) {
            connectionStatusLabel->setText(" Подключено к серверу");
            connectionStatusLabel->setStyleSheet("color: green;");
        }
    });

    logArea->append("Авторизация успешна!");
}

void ClientWindow::addDataToTable(const QString &data)
{
    if (!dataTable) return;

    int row = dataTable->rowCount();
    dataTable->insertRow(row);

    QString funcName = functionCombo->currentText();
    dataTable->setItem(row, 0, new QTableWidgetItem(funcName));
    dataTable->setItem(row, 1, new QTableWidgetItem(data.left(200)));

    dataTable->resizeColumnsToContents();
    dataTable->scrollToBottom();
}

void ClientWindow::clearTable()
{
    if (dataTable) {
        dataTable->setRowCount(0);
    }
}

//АДМИНСКОЕ ОКНО С ДУБЛИРОВАНИЕМ ОТВЕТОВ
void ClientWindow::showAdminLogWindow()
{
    if (adminLogWindow) {
        adminLogWindow->close();
        delete adminLogWindow;
        adminLogWindow = nullptr;
        adminOutput = nullptr;
    }

    adminLogWindow = new QWidget();
    adminLogWindow->setWindowTitle("Панель администратора");
    adminLogWindow->resize(700, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(adminLogWindow);

    // Кнопки управления
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnUsers = new QPushButton("Список пользователей");
    QPushButton *btnLogs = new QPushButton("Показать логи");
    QPushButton *btnClearLogs = new QPushButton("Очистить логи");
    QPushButton *btnDeleteUser = new QPushButton("Удалить пользователя");
    QLineEdit *delUserEdit = new QLineEdit;
    delUserEdit->setPlaceholderText("Имя пользователя для удаления");
    btnLayout->addWidget(btnUsers);
    btnLayout->addWidget(btnLogs);
    btnLayout->addWidget(btnClearLogs);
    btnLayout->addWidget(delUserEdit);
    btnLayout->addWidget(btnDeleteUser);
    mainLayout->addLayout(btnLayout);

    // Область вывода информации
    adminOutput = new QTextEdit;
    adminOutput->setReadOnly(true);
    mainLayout->addWidget(adminOutput);

    // Функция отправки команды (без ожидания, только выводим команду)
    auto sendAdminCommand = [this](const QString& cmd, const QString& extra = QString()) {
        QString fullCmd = cmd;
        if (!extra.isEmpty()) fullCmd += " " + extra;
        socket->write((fullCmd + "\r\n").toUtf8());
        socket->flush();
        if (adminOutput) {
            adminOutput->append("> " + fullCmd);
            adminOutput->append("(ожидание ответа...)");
        }
    };

    connect(btnUsers, &QPushButton::clicked, [=]() {
        sendAdminCommand("admin_list_users");
    });
    connect(btnLogs, &QPushButton::clicked, [=]() {
        sendAdminCommand("admin_list_logs");
    });
    connect(btnClearLogs, &QPushButton::clicked, [=]() {
        sendAdminCommand("admin_clear_logs");
    });
    connect(btnDeleteUser, &QPushButton::clicked, [=, &delUserEdit]() {
        QString user = delUserEdit->text().trimmed();
        if (!user.isEmpty()) {
            sendAdminCommand("admin_delete_user", user);
            delUserEdit->clear();
        } else if (adminOutput) {
            adminOutput->append("Ошибка: введите имя пользователя");
        }
    });

    adminLogWindow->show();
}

void ClientWindow::sendRequest()
{
    if (!socket->isOpen() || !isAuthorized) {
        QMessageBox::warning(this, "Ошибка", "Нет подключения к серверу");
        return;
    }

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
    QString response = QString::fromUtf8(data);

    // Всегда показывать в основном логе
    logArea->append(QString("Ответ сервера:\n%1\n%2").arg(response).arg(QString(50, '-')));

    // Добавить в таблицу (для основных функций)
    addDataToTable(response.trimmed());

    // Дублируем ответ в админское окно, если оно открыто
    if (adminOutput && !adminOutput->isHidden()) {

        QTextCursor cursor(adminOutput->document());
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString lastLine = cursor.selectedText();
        if (lastLine == "(ожидание ответа...)") {
            cursor.removeSelectedText();
            cursor.deletePreviousChar();
        }
        adminOutput->append(response);
        adminOutput->append("---------------------------------");
    }
}

void ClientWindow::disconnectFromServer()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected(1000);
    }

    if (adminLogWindow) {
        adminLogWindow->close();
        delete adminLogWindow;
        adminLogWindow = nullptr;
        adminOutput = nullptr;
    }

    this->close();

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
        adminOutput = nullptr;
    }

    event->accept();
}