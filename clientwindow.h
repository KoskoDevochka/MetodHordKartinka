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
    QWidget *adminLogWindow;
};

#endif
