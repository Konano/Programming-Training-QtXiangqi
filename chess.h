#ifndef CHESS_H
#define CHESS_H

#include <QObject>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QJsonObject>

#include <QDebug>

#define imageSize 50

class Chess : public QObject
{
    Q_OBJECT
public:
    explicit Chess(QGraphicsScene *scene, bool color, int type, bool isPlayer, bool alive, QObject *parent);
    ~Chess();
    QJsonObject outputJSON();
    void setXY(int x, int y);
    void setAlive(bool alive);


signals:

public slots:

private:
	QGraphicsScene *scene;
	QGraphicsItem *pic;

    int x, y;
    // 棋盘位置
    int posx, posy;
    // 画板中心坐标
    int type;
    // 0 帅 1-5 兵 6-7 炮 8-9 车 10-11 马 12-13 象 14-15 士
    bool color;
    // 0 Black 1 Red
    bool isPlayer, alive;

protected:
    //void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // CHESS_H
