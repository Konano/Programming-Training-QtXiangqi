#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <QDialog>
#include <QtNetwork>

namespace Ui {
class ServerDialog;
}

class ServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDialog(QTcpServer *listenSocket, QWidget *parent = 0);
    ~ServerDialog();

signals:
    void connected(QTcpSocket*);

private slots:
    void initServer();
    void acceptConnection();
    void disConnection();

private:
    Ui::ServerDialog *ui;
    QDialog *dialog;
    QTcpServer *listenSocket;
};

#endif // SERVERDIALOG_H
