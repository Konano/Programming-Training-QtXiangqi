#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

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
    void recvMessage();
    void sendJSON(QJsonObject json);
    void readJSON(QByteArray byteArray);

private:
    Ui::MainWindow *ui;
    QTcpServer *listenSocket;
    QTcpSocket *readWriteSocket;
    QImage *bg;
    QGraphicsScene *scene;

    bool isServer = true;
    Chess *redChess[16], *blackChess[16];
    // 0 帅 1-5 兵 6-7 炮 8-9 车 10-11 马 12-13 象 14-15 士

    void gameInit();
    void sendGame();
};

#endif // MAINWINDOW_H


