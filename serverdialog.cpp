#include "serverdialog.h"
#include "ui_serverdialog.h"

#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

ServerDialog::ServerDialog(QTcpServer *listenSocket, QWidget *parent) :
    listenSocket(listenSocket), QDialog(parent),
    ui(new Ui::ServerDialog)
{
    ui->setupUi(this);
    ui->lineEdit_Post->setValidator(new QIntValidator(0, 65535, this));

    connect(this->listenSocket, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}

ServerDialog::~ServerDialog()
{
    delete ui;
}

void ServerDialog::initServer()
{
    // 新建提示框
    dialog = new QDialog(this);

    QLabel *label = new QLabel(dialog);
    label->setText("Wait for connect...");

    QPushButton *btn = new QPushButton(dialog);
    btn->setText("Cancel");

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(label);
    layout->addWidget(btn);
    dialog->setLayout(layout);

    connect(btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(disConnection()));
    // 新建提示框结束

    listenSocket->listen(QHostAddress::Any, QString(ui->lineEdit_Post->text()).toInt());
    dialog->show();
}

void ServerDialog::acceptConnection()
{
    emit connected(listenSocket->nextPendingConnection());
    accept();
}

void ServerDialog::disConnection()
{
    listenSocket->close();
    qDebug("disConnection");
}
