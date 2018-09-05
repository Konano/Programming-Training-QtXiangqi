#include "chess.h"
#include <QImage>

const int dx[16] = {0,-4,-2,0,2,4,-3,3,-4,4,-3,3,-2,2,-1,1};
const int dy[16] = {4,1,1,1,1,1,2,2,4,4,4,4,4,4,4,4};

const int cx[9] = {56,110,166,222,276,331,386,441,494};
const int cy[10] = {54,108,163,218,273,328,383,439,493,547};

const int type[16] = {0,1,1,1,1,1,6,6,8,8,10,10,12,12,14,14};

Chess::Chess(QGraphicsScene *scene, bool color, int id, QObject *parent) :
    scene(scene), color(color), id(id), isPlayer(false), alive(true), QObject(parent)
{
    int tmp = (color ? 1 : -1);
    x = 4 + tmp*dx[id];
    y = (color ? 5 : 4) + tmp*dy[id];
	posx = cx[x];
	posy = cy[y];

	//qDebug() << posx << posy << x << y << isPlayer;

    QString filePath(tr(":/images/") + tr(color ? "r-" : "b-") + QString::number(type[id]) + tr(".png"));
    QImage image;
    image.load(filePath);
    pic = new PIC();
    pic->setPixmap(QPixmap::fromImage(image));
	pic->setPos(posx-imageSize/2, posy-imageSize/2);
    pic->setVisible(alive);
    connect(pic, SIGNAL(mousePressed()), this, SLOT(pic_mousePress()));
    scene->addItem(pic);
}

int Chess::getX() { return x; }
int Chess::getY() { return y; }
int Chess::getID() { return id; }
int Chess::getType() { return type[id]; }
bool Chess::isAlive() { return alive; }

Chess::~Chess()
{
    scene->removeItem(pic);
    delete pic;
}

void Chess::setXY(int x, int y)
{
    this->x = x;
    this->y = y;
    posx = cx[x];
    posy = cy[y];
    pic->setPos(posx-imageSize/2, posy-imageSize/2);
}

void Chess::setAlive(bool alive)
{
    this->alive = alive;
    pic->setVisible(alive);
}

void Chess::myChess()
{
    isPlayer = true;
}

void Chess::pic_mousePress()
{
    if (isPlayer && alive) emit mousePressed(id);
}

QJsonObject Chess::outputJSON()
{
    QJsonObject pageObject;
    pageObject.insert("Alive", alive);
    pageObject.insert("X", x);
    pageObject.insert("Y", y);

    return pageObject;
}

Selete::Selete(QGraphicsScene *scene, int x, int y, QObject *parent) :
    scene(scene), x(x), y(y), QObject(parent)
{
    posx = cx[x];
    posy = cy[y];

    QString filePath(":/images/select.png");
    QImage image;
    image.load(filePath);
    pic = new PIC();
    pic->setPixmap(QPixmap::fromImage(image));
    pic->setPos(posx-imageSize/2, posy-imageSize/2);
    pic->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    connect(pic, SIGNAL(mousePressed()), this, SLOT(pic_mousePress()));
}

void Selete::setVisible(bool visible)
{
    if (this->visible == false && visible == true) scene->addItem(pic);
    if (this->visible == true && visible == false) scene->removeItem(pic);
    this->visible = visible;
}

void Selete::setSensitive(bool sensitive)
{
    this->sensitive = sensitive;
}

void Selete::pic_mousePress()
{
    if (visible && sensitive) emit mousePressed(x, y);
}
