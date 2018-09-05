#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "serverdialog.h"
#include "clientdialog.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include <QDebug>

#define timeLimit 60

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    listenSocket = new QTcpServer(this);
    readWriteSocket = new QTcpSocket(this);
    msTimer = new QTimer(this);
    connect(msTimer, SIGNAL(timeout()), this, SLOT(timeSlot()));

    gameInit();

    // YourTurn();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionCreate_triggered()
{
    // killChess(4, 9);
    // GAMEOVER(true, 1);
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

    if (isServer) {
        sendGame();
        if (isYourTurn)
            YourTurn();
        else
            NotYourTurn();
    }
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
                bool whoFirst = object.value("FirstPlayer").toBool();
                if (whoFirst == isServer)
                    YourTurn();
                else
                    NotYourTurn();
            }
            if (QString::compare(strName, QString("Step")) == 0) {
                int id = object.value("ID").toInt();
                int X = object.value("X").toInt();
                int Y = object.value("Y").toInt();
                seleteClear();
                moveChess((isServer?blackChess:redChess)[id], 8-X, 9-Y);
                YourTurn();
            }
            if (QString::compare(strName, QString("Over")) == 0) {
                GAMEOVER(true, object.value("Value").toInt());
            }
        }
    }
}

void MainWindow::GAMEOVER(bool win, int type)
{
    qDebug() << "GAMEOVER" << win;

    qtaudioPlayer = new QMediaPlayer;
    qtaudioPlayer->setMedia(QUrl(win ? "qrc:/videos/win.mp3" : "qrc:/videos/fail.mp3"));
    qtaudioPlayer->play();

    if (win == false) sendOver(type);

    QString text;
    switch (type) {
        case 1: text += tr("游戏结束\n"); break;
        case 2: text += (win!=isServer?tr("红方"):tr("黑方")) + tr("超时\n"); break;
        case 3: text += (win!=isServer?tr("红方"):tr("黑方")) + tr("认输\n"); break;
    }
    text += tr("获胜方：") + (win==isServer?tr("红方"):tr("黑方"));

    QDialog *dialog = new QDialog(this);
    QLabel *label = new QLabel(dialog);
    label->setText(text);
    QPushButton *btn = new QPushButton(dialog);
    btn->setText("OK");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(btn);
    if (win) {
        QString filename(":/images/win.png");
        QImage img;
        if (!img.load(filename)) qDebug("Fail");
        QLabel *label2 = new QLabel(dialog);
        label2->setPixmap(QPixmap::fromImage(img));
        label2->resize(img.width(), img.height());
        QHBoxLayout *layout2 = new QHBoxLayout();
        layout2->addWidget(label2);
        layout2->addLayout(layout);
        dialog->setLayout(layout2);
    } else dialog->setLayout(layout);
    connect(btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(close()));
    dialog->exec();
}

bool MainWindow::canAttack(Chess *now, int aim)
{
    if (now->isAlive() == false) return false;

    int x = now->getX();
    int y = now->getY();

    switch (now->getType())
    {
        case 1: {
            if (y != 0 && y != 9 && posChess(x, y+aim) == aim) return true;
            if (now->crossRiver) {
                if (x != 0 && posChess(x+1, y) == aim) return true;
                if (x != 8 && posChess(x-1, y) == aim) return true;
            }
            break;
        }
        case 6: {
            for(int i=x-1; i>=0; i--) if (posChess(i, y) == 0) {
            } else {
                for(i--; i>=0; i--) if (posChess(i, y) == aim) {
                    return true;
                } break;
            }
            for(int i=x+1; i<=8; i++) if (posChess(i, y) == 0) {
            } else {
                for(i++; i<=8; i++) if (posChess(i, y) == aim) {
                    return true;
                } break;
            }
            for(int i=y-1; i>=0; i--) if (posChess(x, i) == 0) {
            } else {
                for(i--; i>=0; i--) if (posChess(x, i) == aim) {
                    return true;
                } break;
            }
            for(int i=y+1; i<=9; i++) if (posChess(x, i) == 0) {
            } else {
                for(i++; i<=9; i++) if (posChess(x, i) == aim) {
                    return true;
                } break;
            }
            break;
        }
        case 8: {
            for(int i=x-1; i>=0; i--) if (posChess(i, y) == 0) {
            } else {
                if (posChess(i, y) == aim)
                    return true;
                break;
            }
            for(int i=x+1; i<=8; i++) if (posChess(i, y) == 0) {
            } else {
                if (posChess(i, y) == aim)
                    return true;
                break;
            }
            for(int i=y-1; i>=0; i--) if (posChess(x, i) == 0) {
            } else {
                if (posChess(x, i) == aim)
                    return true;
                break;
            }
            for(int i=y+1; i<=9; i++) if (posChess(x, i) == 0) {
            } else {
                if (posChess(x, i) == aim)
                    return true;
                break;
            }
            break;
        }
        case 10: {
            if (x-1 >= 0 && posChess(x-1, y) == 0) {
                if (checkPos(x-2, y-1) && posChess(x-2, y-1) == aim)
                    return true;
                if (checkPos(x-2, y+1) && posChess(x-2, y+1) == aim)
                    return true;
            }
            if (x+1 <= 8 && posChess(x+1, y) == 0) {
                if (checkPos(x+2, y-1) && posChess(x+2, y-1) == aim)
                    return true;
                if (checkPos(x+2, y+1) && posChess(x+2, y+1) == aim)
                    return true;
            }
            if (y-1 >= 0 && posChess(x, y-1) == 0) {
                if (checkPos(x-1, y-2) && posChess(x-1, y-2) == aim)
                    return true;
                if (checkPos(x+1, y-2) && posChess(x+1, y-2) == aim)
                    return true;
            }
            if (y+1 <= 9 && posChess(x, y+1) == 0) {
                if (checkPos(x-1, y+2) && posChess(x-1, y+2) == aim)
                    return true;
                if (checkPos(x+1, y+2) && posChess(x+1, y+2) == aim)
                    return true;
            }
            break;
        }
        case 12: {
            if (((y >= 5) == (y-2 >= 5)) && checkPos(x-2, y-2) && posChess(x-1, y-1) == 0 && posChess(x-2, y-2) == aim)
                return true;
            if (((y >= 5) == (y-2 >= 5)) && checkPos(x+2, y-2) && posChess(x+1, y-1) == 0 && posChess(x+2, y-2) == aim)
                return true;
            if (checkPos(x-2, y+2) && posChess(x-1, y+1) == 0 && posChess(x-2, y+2) == aim)
                return true;
            if (checkPos(x+2, y+2) && posChess(x+1, y+1) == 0 && posChess(x+2, y+2) == aim)
                return true;
            break;
        }
        case 14: {
            if (x-1 >= 3 && y != 7 && y != 0 && posChess(x-1, y-1) == aim)
                return true;
            if (x+1 <= 5 && y != 7 && y != 0 && posChess(x+1, y-1) == aim)
                return true;
            if (x-1 >= 3 && y != 9 && y != 2 && posChess(x-1, y+1) == aim)
                return true;
            if (x+1 <= 5 && y != 9 && y != 2 && posChess(x+1, y+1) == aim)
                return true;
            break;
        }
        case 0: {
            if (x-1 >= 3 && posChess(x-1, y) == aim)
                return true;
            if (x+1 <= 5 && posChess(x+1, y) == aim)
                return true;
            if (y != 7 && y != 0 && posChess(x, y-1) == aim)
                return true;
            if (y != 9 && y != 2 && posChess(x, y+1) == aim)
                return true;
            for(int i=y-1; i>=0; i--) {
                if (posChess(x, i) == aim) return true;
                if (posChess(x, i) != 0) break;
            }
            for(int i=y+1; i<=9; i++) {
                if (posChess(x, i) == aim) return true;
                if (posChess(x, i) != 0) break;
            }
            break;
        }
    }
    return false;
}

bool MainWindow::Check()
{
    rep(i, 16) if (canAttack((isServer?redChess:blackChess)[i], -1)) { qDebug() << "Y" << i; return true;}
    rep(i, 16) if (canAttack((isServer?blackChess:redChess)[i], 1)) { qDebug() << "N" << i; return true;}
    return false;
}

void MainWindow::YourTurn()
{
    qDebug() << "YourTurn";
    rep(i, 16) (isServer?redChess:blackChess)[i]->myChess();
    if ((isServer?redChess:blackChess)[0]->isAlive() == false) GAMEOVER(false, 1);
    gameStart = true;
    isYourTurn = true;
    startCountdown(timeLimit);
    if (Check())
    {
        qtaudioPlayer = new QMediaPlayer;
        qtaudioPlayer->setMedia(QUrl("qrc:/videos/check.mp3"));
        qtaudioPlayer->play();
    }
}

void MainWindow::NotYourTurn()
{
    gameStart = true;
    isYourTurn = false;
    startCountdown(timeLimit);
    if (Check())
    {
        qtaudioPlayer = new QMediaPlayer;
        qtaudioPlayer->setMedia(QUrl("qrc:/videos/check.mp3"));
        qtaudioPlayer->play();
    }
}

void MainWindow::startCountdown(int st_time)
{
    setTime(st_time);
    msTimer->start(1000);
}

void MainWindow::timeSlot()
{
    setTime(countdown-1);
    if (countdown == 0 && isYourTurn)
        GAMEOVER(false, 2);
    else
        msTimer->start(1000);
}

void MainWindow::killChess(int x, int y)
{
    int id = posChess(x, y);
    qDebug() << "killChess" << x << y << id;
    if (id == 0) return;
    bool isMyChess = (id > 0);
    id = (id>0 ? id : -id) - 1;
    ((isMyChess==isServer)?redChess:blackChess)[id]->setAlive(false);
    qtaudioPlayer = new QMediaPlayer;
    qtaudioPlayer->setMedia(QUrl("qrc:/videos/kill.mp3"));
    qtaudioPlayer->play();
}

void MainWindow::moveChess(Chess *c, int x, int y)
{
    killChess(x, y);
    selete(c->getX(), c->getY(), false);
    selete(x, y, false);
    if (c->getType() == 1 && c->getY()+y == 9)
        c->crossRiver = true; // qDebug() << "CrossRiver";
    c->setXY(x, y);
}

int MainWindow::posChess(int x, int y)
{
    // qDebug() << x << y << isServer << redChess[2]->getX() << redChess[2]->getX();
    rep(i, 16) if (redChess[i]->isAlive() &&  redChess[i]->getX() == x && redChess[i]->getY() == y)
        return (i+1)*(isServer?1:-1);
    rep(i, 16) if (blackChess[i]->isAlive() &&  blackChess[i]->getX() == x && blackChess[i]->getY() == y)
        return (i+1)*(isServer?-1:1);
    return 0;
}

bool MainWindow::checkPos(int x, int y)
{
    return (x >= 0 && x < 9 && y >= 0 && y < 10);
}

void MainWindow::selete(int x, int y, bool sensitive)
{
    // qDebug() << x << y;
    seleteMap[x][y]->setVisible(true);
    seleteMap[x][y]->setSensitive(sensitive);
}

void MainWindow::seleteClear()
{
    rep(i, 9) rep(j, 10) seleteMap[i][j]->setVisible(false);
}

void MainWindow::seletePress(int x, int y)
{
    Chess *clicked = pressedChess;
    releaseChess();
    moveChess(clicked, x, y);
    sendStep(clicked->getID(), x, y);
    NotYourTurn();
}

void MainWindow::chessPress(int type)
{
    qDebug() << "chessPress" << gameStart << isYourTurn << pressedChess;
    if (gameStart && isYourTurn) {
        if (pressedChess) {
            if (pressedChess == (isServer?redChess:blackChess)[type]) {
                releaseChess();
                return;
            }
            else
                releaseChess();
        }
        pressedChess = (isServer?redChess:blackChess)[type];
        holdChess();
    }
}

void MainWindow::holdChess()
{
    seleteClear();

    int x = pressedChess->getX();
    int y = pressedChess->getY();

    selete(x, y, false);

    qDebug() << x << y << pressedChess->getType();

    switch (pressedChess->getType())
    {
        case 1: {
            if (y != 0 && posChess(x, y-1) <= 0) selete(x, y-1, true);
            if (pressedChess->crossRiver) {
                if (x != 0 && posChess(x-1, y) <= 0) selete(x-1, y, true);
                if (x != 8 && posChess(x+1, y) <= 0) selete(x+1, y, true);
            }
            break;
        }
        case 6: {
            for(int i=x-1; i>=0; i--) if (posChess(i, y) == 0) {
                selete(i, y, true);
            } else {
                for(i--; i>=0; i--) if (posChess(i, y) < 0) {
                    selete(i, y, true); break;
                } break;
            }
            for(int i=x+1; i<=8; i++) if (posChess(i, y) == 0) {
                selete(i, y, true);
            } else {
                for(i++; i<=8; i++) if (posChess(i, y) < 0) {
                    selete(i, y, true); break;
                } break;
            }
            for(int i=y-1; i>=0; i--) if (posChess(x, i) == 0) {
                selete(x, i, true);
            } else {
                for(i--; i>=0; i--) if (posChess(x, i) < 0) {
                    selete(x, i, true); break;
                } break;
            }
            for(int i=y+1; i<=9; i++) if (posChess(x, i) == 0) {
                selete(x, i, true);
            } else {
                for(i++; i<=9; i++) if (posChess(x, i) < 0) {
                    selete(x, i, true); break;
                } break;
            }
            break;
        }
        case 8: {
            for(int i=x-1; i>=0; i--) if (posChess(i, y) == 0) {
                selete(i, y, true);
            } else {
                if (posChess(i, y) < 0)
                    selete(i, y, true);
                break;
            }
            for(int i=x+1; i<=8; i++) if (posChess(i, y) == 0) {
                selete(i, y, true);
            } else {
                if (posChess(i, y) < 0)
                    selete(i, y, true);
                break;
            }
            for(int i=y-1; i>=0; i--) if (posChess(x, i) == 0) {
                selete(x, i, true);
            } else {
                if (posChess(x, i) < 0)
                    selete(x, i, true);
                break;
            }
            for(int i=y+1; i<=9; i++) if (posChess(x, i) == 0) {
                selete(x, i, true);
            } else {
                if (posChess(x, i) < 0)
                    selete(x, i, true);
                break;
            }
            break;
        }
        case 10: {
            if (x-1 >= 0 && posChess(x-1, y) == 0) {
                if (checkPos(x-2, y-1) && posChess(x-2, y-1) <= 0)
                    selete(x-2, y-1, true);
                if (checkPos(x-2, y+1) && posChess(x-2, y+1) <= 0)
                    selete(x-2, y+1, true);
            }
            if (x+1 <= 8 && posChess(x+1, y) == 0) {
                if (checkPos(x+2, y-1) && posChess(x+2, y-1) <= 0)
                    selete(x+2, y-1, true);
                if (checkPos(x+2, y+1) && posChess(x+2, y+1) <= 0)
                    selete(x+2, y+1, true);
            }
            if (y-1 >= 0 && posChess(x, y-1) == 0) {
                if (checkPos(x-1, y-2) && posChess(x-1, y-2) <= 0)
                    selete(x-1, y-2, true);
                if (checkPos(x+1, y-2) && posChess(x+1, y-2) <= 0)
                    selete(x+1, y-2, true);
            }
            if (y+1 <= 9 && posChess(x, y+1) == 0) {
                if (checkPos(x-1, y+2) && posChess(x-1, y+2) <= 0)
                    selete(x-1, y+2, true);
                if (checkPos(x+1, y+2) && posChess(x+1, y+2) <= 0)
                    selete(x+1, y+2, true);
            }
            break;
        }
        case 12: {
            if (y-2 >= 5 && checkPos(x-2, y-2) && posChess(x-1, y-1) == 0 && posChess(x-2, y-2) <= 0)
                selete(x-2, y-2, true);
            if (y-2 >= 5 && checkPos(x+2, y-2) && posChess(x+1, y-1) == 0 && posChess(x+2, y-2) <= 0)
                selete(x+2, y-2, true);
            if (checkPos(x-2, y+2) && posChess(x-1, y+1) == 0 && posChess(x-2, y+2) <= 0)
                selete(x-2, y+2, true);
            if (checkPos(x+2, y+2) && posChess(x+1, y+1) == 0 && posChess(x+2, y+2) <= 0)
                selete(x+2, y+2, true);
            break;
        }
        case 14: {
            if (x-1 >= 3 && y-1 >= 7 && posChess(x-1, y-1) <= 0)
                selete(x-1, y-1, true);
            if (x+1 <= 5 && y-1 >= 7 && posChess(x+1, y-1) <= 0)
                selete(x+1, y-1, true);
            if (x-1 >= 3 && y+1 <= 9 && posChess(x-1, y+1) <= 0)
                selete(x-1, y+1, true);
            if (x+1 <= 5 && y+1 <= 9 && posChess(x+1, y+1) <= 0)
                selete(x+1, y+1, true);
            break;
        }
        case 0: {
            if (x-1 >= 3 && posChess(x-1, y) <= 0)
                selete(x-1, y, true);
            if (x+1 <= 5 && posChess(x+1, y) <= 0)
                selete(x+1, y, true);
            if (y-1 >= 7 && posChess(x, y-1) <= 0)
                selete(x, y-1, true);
            if (y+1 <= 9 && posChess(x, y+1) <= 0)
                selete(x, y+1, true);
            for(int i=y-1; i>=0; i--) {
                if (posChess(x, i) == -1) selete(x, i, true);
                if (posChess(x, i) != 0) break;
            }
            break;
        }
    }
}

void MainWindow::releaseChess()
{
    seleteClear();
    pressedChess = NULL;
}

void MainWindow::setTime(int countdown)
{
    this->countdown = countdown;
    ui->lcdNumber->display(countdown);
    update();
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
    isYourTurn = true;

    bg = new QImage;
    bg->load(":/images/background.png");
    scene = new QGraphicsScene;
    scene->addPixmap(QPixmap::fromImage(*bg));
    ui->graphicsView->setScene(scene);

    rep(i, 16) {
        redChess[i] = new Chess(scene, true, i, this);
        blackChess[i] = new Chess(scene, false, i, this);

        connect(redChess[i], SIGNAL(mousePressed(int)), this, SLOT(chessPress(int)));
        connect(blackChess[i], SIGNAL(mousePressed(int)), this, SLOT(chessPress(int)));
    }

    rep(i, 9) rep(j, 10) {
        seleteMap[i][j] = new Selete(scene, i, j, this);
        connect(seleteMap[i][j], SIGNAL(mousePressed(int,int)), this, SLOT(seletePress(int,int)));
    }

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
    json.insert("FirstPlayer", isYourTurn);

    sendJSON(json);
}

void MainWindow::sendStep(int id, int x, int y)
{
    // 构建 Json 对象
    QJsonObject json;
    json.insert("Type", "Step");
    json.insert("ID", id);
    json.insert("X", x);
    json.insert("Y", y);

    sendJSON(json);
}

void MainWindow::sendOver(int type)
{
    QJsonObject json;
    json.insert("Type", "Over");
    json.insert("Value", type);

    sendJSON(json);
}

void MainWindow::on_pushButton_clicked()
{
    if (gameStart == false) return;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "", tr("确定认输？"), QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
        GAMEOVER(false, 3);
}

void MainWindow::on_actionImport_triggered()
{
    if (gameStart) return;
    QString path = QFileDialog::getOpenFileName(this,tr("Open..."),".");
    qDebug() << path;
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    qDebug() << "RED";

    qDebug() << file.readLine();
    rep(i, 16) redChess[i]->setAlive(false);
    chessImport(file.readLine(), redChess, 0);
    chessImport(file.readLine(), redChess, 14);
    chessImport(file.readLine(), redChess, 12);
    chessImport(file.readLine(), redChess, 10);
    chessImport(file.readLine(), redChess, 8);
    chessImport(file.readLine(), redChess, 6);
    chessImport(file.readLine(), redChess, 1);
    for(int i=1; i<=5; i++) if (redChess[i]->isAlive())
        redChess[i]->crossRiver = (redChess[i]->getY() < 5);

    qDebug() << "BLACK";

    qDebug() << file.readLine();
    rep(i, 16) blackChess[i]->setAlive(false);
    chessImport(file.readLine(), blackChess, 0);
    chessImport(file.readLine(), blackChess, 14);
    chessImport(file.readLine(), blackChess, 12);
    chessImport(file.readLine(), blackChess, 10);
    chessImport(file.readLine(), blackChess, 8);
    chessImport(file.readLine(), blackChess, 6);
    chessImport(file.readLine(), blackChess, 1);
    for(int i=1; i<=5; i++) if (blackChess[i]->isAlive())
        blackChess[i]->crossRiver = (blackChess[i]->getY() > 4);

    file.close();
}

void MainWindow::chessImport(QString str, Chess **arr, int st)
{
    qDebug() << str.mid(0,str.length()-1);

    QStringList strlist = str.split(' ');
    int total = strlist.size();
    for(int i=1; i<total; i++)
    {
        str = strlist[i];
        int length = str.length()-1;
        int m = str.indexOf(',');

        arr[st]->setXY(str.mid(1,m-1).toInt(), 9-str.mid(m+1,length-1-(m+1)).toInt());
        arr[st]->setAlive(true);
        st++;
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString path = QFileDialog::getSaveFileName(this,tr("Save..."),".");
    qDebug() << path;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream cout(&file);

    cout << "red" << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 0, 0) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 14, 15) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 12, 13) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 10, 11) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 8, 9) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 6, 7) << endl;
    cout << chessSave(isServer==isYourTurn?redChess:blackChess, 1, 5) << endl;

    cout << "black" << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 0, 0) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 14, 15) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 12, 13) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 10, 11) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 8, 9) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 6, 7) << endl;
    cout << chessSave(isServer==isYourTurn?blackChess:redChess, 1, 5) << endl;

    file.close();
}

QString MainWindow::chessSave(Chess **arr, int st, int ed)
{
    QString tmp;
    int total = 0;
    for(int i=st; i<=ed; i++) if (arr[i]->isAlive()) {
        total++;
        tmp += tr(" <") +
               QString::number(arr[i]->getX()) + tr(",") +
               QString::number(9-arr[i]->getY()) + tr(">");
    }
    return QString::number(total) + tmp;
}
