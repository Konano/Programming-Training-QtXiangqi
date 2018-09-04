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

    if (isServer) sendGame();
}

void MainWindow::readJSON(QByteArray byteArray)
{
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(byteArray, &jsonError);  // 转化为 JSON 文档
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (doucment.isObject()) {  // JSON 文档为对象
            QJsonObject object = doucment.object();  // 转化为对象
            QJsonValue value = object.value("Type");
            QString strName = value.toString();
            if (QString::compare(strName, QString("Game")) == 0) {
                value = object.value("Chess");
                QJsonArray array = value.toArray();
                rep(i, 16) {
                    value = array.at(i);
                    QJsonObject obj = value.toObject();
                    redChess[i]->setXY(8-obj.value("X").toInt(), 9-obj.value("Y").toInt());
                    redChess[i]->setAlive(obj.value("Alive").toBool());
                }
                rep(i, 16) {
                    value = array.at(i+16);
                    QJsonObject obj = value.toObject();
                    blackChess[i]->setXY(8-obj.value("X").toInt(), 9-obj.value("Y").toInt());
                    blackChess[i]->setAlive(obj.value("Alive").toBool());
                }
            }
            if (QString::compare(strName, QString("Step")) == 0) {
                bool color = object.value("Color").toBool();
                int ID = object.value("ID").toInt();
                int X = object.value("X").toInt();
                int Y = object.value("Y").toInt();
                // moveEnemyChess(color, ID, X, Y);
            }
        }
    }
}

void MainWindow::recvMessage()
{
    QString info;
    info = this->readWriteSocket->readAll();

    qDebug() << info;

    readJSON(info.toLatin1());
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

void MainWindow::sendJSON(QJsonObject json)
{
    // 构建 Json 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    QString strJson(byteArray);
    qDebug() << strJson;

    readWriteSocket->write(byteArray.data());
}

void MainWindow::sendGame()
{
    // 构建 Json 数组 - Version
    QJsonArray Chess;
    rep(i, 16) Chess.append(QJsonValue(redChess[i]->outputJSON()));
    rep(i, 16) Chess.append(QJsonValue(blackChess[i]->outputJSON()));

    // 构建 Json 对象
    QJsonObject json;
    json.insert("Type", "Game");
    json.insert("Chess", QJsonValue(Chess));

    sendJSON(json);
}
