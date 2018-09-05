#include "clientdialog.h"
#include "ui_clientdialog.h"

#include <QIntValidator>
#include <QRegExpValidator>
#include <QLabel>
#include <QPushButton>

ClientDialog::ClientDialog(QTcpSocket *readWriteSocket, QWidget *parent) :
    readWriteSocket(readWriteSocket), QDialog(parent),
    ui(new Ui::ClientDialog)
{
    ui->setupUi(this);
    ui->lineEdit_Post->setValidator(new QIntValidator(0, 65535, this));
    ui->lineEdit_IP->setValidator(new QRegExpValidator(QRegExp("((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d))))"), this));

    connect(readWriteSocket, SIGNAL(connected()), this, SLOT(finish()));
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

void ClientDialog::connectHost()
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
    connect(dialog, SIGNAL(rejected()), readWriteSocket, SLOT(disConnection()));
    // 新建提示框结束

    readWriteSocket->connectToHost(QHostAddress(ui->lineEdit_IP->text()), QString(ui->lineEdit_Post->text()).toInt());
    dialog->show();
}

void ClientDialog::finish()
{
    emit connected(readWriteSocket);
	accept();
}
