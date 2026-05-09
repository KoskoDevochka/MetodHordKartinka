#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDialog>
#include <QTableWidget>  // НОВОЕ: для таблицы
#include <QVBoxLayout>   // НОВОЕ: для layout
#include <QHBoxLayout>   // НОВОЕ: для layout

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void sendRequest();
    void onReadyRead();
    void disconnectFromServer();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void showAuthDialog();
    void showMainWindow();
    void showAdminLogWindow();
    void addDataToTable(const QString &data);  // НОВОЕ: добавляет данные в таблицу
    void clearTable();                          // НОВОЕ: очищает таблицу

    QTcpSocket *socket;
    bool isAuthorized;

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLabel *statusAuthLabel;

    QTextEdit *logArea;
    QTextEdit *inputArea;
    QPushButton *sendBtn;
    QPushButton *disconnectBtn;
    QComboBox *functionCombo;
    QLabel *statusLabel;
    QLabel *connectionStatusLabel;  // НОВОЕ: статус подключения (зелёный/красный)
    QWidget *adminLogWindow;

    // НОВЫЕ ВИДЖЕТЫ ДЛЯ ТАБЛИЦЫ
    QTableWidget *dataTable;         // ТАБЛИЦА для отображения данных
    QWidget *tableContainer;         // Контейнер для таблицы
};

#endif
