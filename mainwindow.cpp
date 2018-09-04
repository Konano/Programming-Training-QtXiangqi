#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "serverdialog.h"
#include "clientdialog.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    listenSocket = new QTcpServer(this);
    readWriteSocket = new QTcpSocket(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionCreate_triggered()
{
    ServerDialog config(listenSocket, this);
    connect(&config, SIGNAL(connected(QTcpSocket*)), this, SLOT(acceptConnection(QTcpSocket*)));
    config.exec();
}

void MainWindow::on_actionConnect_triggered()
{
    ClientDialog config(readWriteSocket, this);
    connect(&config, SIGNAL(connected(QTcpSocket*)), this, SLOT(acceptConnection(QTcpSocket*)));
    config.exec();
}

void MainWindow::acceptConnection(QTcpSocket* readWriteSocket)
{
    this->readWriteSocket = readWriteSocket;
    connect(this->readWriteSocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
}

void MainWindow::recvMessage()
{
    QString info;
    info = this->readWriteSocket->readAll();
    qDebug() << info;
}
