#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QtNetwork>

namespace Ui {
class ClientDialog;
}

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientDialog(QTcpSocket *readWriteSocket, QWidget *parent = 0);
    ~ClientDialog();

signals:
	connected(QTcpSocket*);

private slots:
	void connectHost();
	void finish();

private:
    Ui::ClientDialog *ui;
    QDialog *dialog;
    QTcpSocket *readWriteSocket;
};

#endif // CLIENTDIALOG_H
