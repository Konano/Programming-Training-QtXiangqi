#ifndef CHESS_H
#define CHESS_H

#include <QObject>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QJsonObject>

#include <QDebug>

#define imageSize 50

class PIC : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
signals:
    void mousePressed();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        emit mousePressed();
        QGraphicsItem::mousePressEvent(event);
    }
};


class Chess : public QObject
{
    Q_OBJECT
public:
    explicit Chess(QGraphicsScene *scene, bool color, int id, QObject *parent);
    ~Chess();
    QJsonObject outputJSON();
    void setXY(int x, int y);
    void setAlive(bool alive);
    void myChess();

    int getX();
    int getY();
    int getID();
    int getType();
    bool isAlive();

    bool crossRiver = false;

signals:
    void mousePressed(int);

private slots:
    void pic_mousePress();

private:
	QGraphicsScene *scene;
	PIC *pic;

    int x, y;
    // 棋盘位置
    int posx, posy;
    // 画板中心坐标
    int id;
    // 0 帅 1-5 兵 6-7 炮 8-9 车 10-11 马 12-13 象 14-15 士
    bool color;
    // 0 Black 1 Red
    bool isPlayer, alive;
};


class Selete : public QObject
{
    Q_OBJECT
public:
    explicit Selete(QGraphicsScene *scene, int x, int y, QObject *parent = 0);
    void setVisible(bool);
    void setSensitive(bool);

signals:
    void mousePressed(int, int);

private slots:
    void pic_mousePress();

private:
    QGraphicsScene *scene;
    PIC *pic;

    int x, y;
    // 棋盘位置
    int posx, posy;
    // 画板中心坐标
    bool sensitive = false;
    bool visible = false;
};

#endif // CHESS_H
