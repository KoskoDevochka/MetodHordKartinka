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
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

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
    void addDataToTable(const QString &data);
    void clearTable();

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
    QLabel *connectionStatusLabel;
    QWidget *adminLogWindow;
    QTextEdit *adminOutput;      // вывод админских ответов

    QTableWidget *dataTable;
    QWidget *tableContainer;
};

#endif // CLIENTWINDOW_H