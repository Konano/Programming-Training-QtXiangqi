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

    gameInit();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionCreate_triggered()
{
    isServer = true;
    ServerDialog config(listenSocket, this);
    connect(&config, SIGNAL(connected(QTcpSocket*)), this, SLOT(acceptConnection(QTcpSocket*)));
    config.exec();
}

void MainWindow::on_actionConnect_triggered()
{
    isServer = false;
    ClientDialog config(readWriteSocket, this);
    connect(&config, SIGNAL(connected(QTcpSocket*)), this, SLOT(acceptConnection(QTcpSocket*)));
    config.exec();
}

void MainWindow::acceptConnection(QTcpSocket* readWriteSocket)
{
    this->readWriteSocket = readWriteSocket;
    connect(this->readWriteSocket, SIGNAL(readyRead()), this, SLOT(recvMessage()));

    //if (isServer) SendGame();
}

void MainWindow::recvMessage()
{
    QString info;
    info = this->readWriteSocket->readAll();
    qDebug() << info;
}

void MainWindow::gameInit()
{
    bg = new QImage;
    bg->load(":/images/background.png");
    scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(*bg));
    ui->graphicsView->setScene(scene);

    rep(i, 16) redChess[i] = new Chess(scene, true, i, isServer, true, this);
    rep(i, 16) blackChess[i] = new Chess(scene, false, i, 1-isServer, true, this);

    ui->graphicsView->show();
    update();
}
