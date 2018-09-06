#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>

#include <QMediaPlayer>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <vector>

#include "chess.h"

#define rep(i, r) for(int i=0; i<r; i++)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void acceptConnection(QTcpSocket*);
    void on_actionCreate_triggered();
    void on_actionConnect_triggered();
    void on_actionImport_triggered();
    void on_actionSave_triggered();
    void recvMessage();
    void timeSlot();
    void chessPress(int);
    void selectPress(int, int);

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpServer *listenSocket;
    QTcpSocket *readWriteSocket;
    QImage *bg;
    QGraphicsScene *scene;
    QTimer *msTimer;
    QMediaPlayer *qtaudioPlayer;

    bool isServer = true;
    Chess *redChess[16], *blackChess[16];
    // 0 帅 1-5 兵 6-7 炮 8-9 车 10-11 马 12-13 象 14-15 士

    void gameInit();
    void GAMEOVER(bool win, int type);

    void sendJSON(QJsonObject json);
    void readJSON(QByteArray byteArray);

    void sendGame();
    void sendStep(int id, int x, int y);
    void sendOver(int type);

    bool isYourTurn = false;
    void YourTurn();
    void NotYourTurn();

    int countdown = 60;
    void startCountdown(int st_time);
    void setTime(int countdown);

    bool gameStart = false;

    Chess *pressedChess = NULL;
    void holdChess();
    void releaseChess();

    void killChess(int x, int y);
    void moveChess(Chess *c, int x, int y);
    int posChess(int x, int y);

    bool checkPos(int x, int y);

    Select *selectMap[9][10];
    void select(int x, int y, bool sensetive);
    void selectClear();

    bool canAttack(Chess *now, int aim);
    bool Check();

    void chessImport(QString str, Chess **arr, int st);
    QString chessSave(Chess **arr, int st, int ed);
};

#endif // MAINWINDOW_H


