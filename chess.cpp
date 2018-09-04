#include "chess.h"
#include <QImage>

const int dx[16] = {0,-4,-2,0,2,4,-3,3,-4,4,-3,3,-2,2,-1,1};
const int dy[16] = {4,1,1,1,1,1,2,2,4,4,4,4,4,4,4,4};

const int cx[9] = {56,110,166,222,276,331,386,441,494};
const int cy[10] = {54,108,163,218,273,328,383,439,493,547};

const int fileID[16] = {0,1,1,1,1,1,6,6,8,8,10,10,12,12,14,14};

Chess::Chess(QGraphicsScene *scene, bool color, int type, bool isPlayer, bool alive, QObject *parent) :
    scene(scene), color(color), type(type), isPlayer(isPlayer), alive(alive), QObject(parent)
{
    int tmp = (isPlayer ? 1 : -1);
	x = 4 + tmp*dx[type];
    y = (isPlayer ? 5 : 4) + tmp*dy[type];
	posx = cx[x];
	posy = cy[y];

	qDebug() << posx << posy << x << y << isPlayer;

    QString filePath(tr(":/images/") + tr(color ? "r-" : "b-") + QString::number(fileID[type]) + tr(".png"));

    QImage image;
    image.load(filePath);
    pic = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	pic->setPos(posx-imageSize/2, posy-imageSize/2);
    scene->addItem(pic);
}

Chess::~Chess()
{
    delete pic;
}

