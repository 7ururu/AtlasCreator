#include "qgraphicsspriteitem.h"
#include <QGraphicsSceneMouseEvent>

int QGraphicsSpriteItem::snapRadius = 0;
int QGraphicsSpriteItem::margin = 0;

QGraphicsSpriteItem::QGraphicsSpriteItem(const QPixmap& pix, QString id, QGraphicsItem *atlasBound) :
    pix(pix), ID(id), atlasBound(atlasBound)
{
    setFlag(ItemIsMovable);
    boundColor = Qt::green;
    isMoving = false;
    itemActivity = false;
}

QRectF QGraphicsSpriteItem::boundingRect() const
{
    return QRectF(0, 0, pix.width() + margin * 2, pix.height() + margin * 2);
}

void QGraphicsSpriteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPos((int)pos().x(), (int)pos().y());
    painter->drawPixmap(QRect(margin, margin, pix.width(), pix.height()), pix);
    painter->setPen(QPen(boundColor));
    if (itemActivity)
        painter->fillRect(boundingRect(), QBrush(QColor(0, 0, 0, 64), Qt::Dense2Pattern));
    painter->drawRect(boundingRect());
}

void QGraphicsSpriteItem::setBoundingRectColor(QColor color)
{
    boundColor = color;
}

void QGraphicsSpriteItem::setMargin(int m)
{
    margin = m;
}

int QGraphicsSpriteItem::getMargin()
{
    return margin;
}

void QGraphicsSpriteItem::setSnapRadius(int radius)
{
    snapRadius = radius;
}

QPixmap QGraphicsSpriteItem::getPixmap() const
{
    return pix;
}

bool QGraphicsSpriteItem::isActiveItem() const
{
    return itemActivity;
}

void QGraphicsSpriteItem::changeItemActivity(bool activity)
{
    itemActivity = activity;
}

bool QGraphicsSpriteItem::isPressed() const
{
    return isMoving;
}

void QGraphicsSpriteItem::snapIt()
{
    QList < QGraphicsItem* > items = scene()->items();
    QGraphicsItem* hor = 0,
                 * ver = 0;
    int horDist = 1e9, horType = 0,
        verDist = 1e9, verType = 0;

    for (int i = 0; i < items.size(); i++)
    {
        if (items[i] == this)
            continue;

        if ((x() + snapRadius >= items[i]->x() && x() <= items[i]->x() + items[i]->boundingRect().width() + snapRadius) ||
            (x() + boundingRect().width() + snapRadius >= items[i]->x() && x() + boundingRect().width() <= items[i]->x() + items[i]->boundingRect().width() + snapRadius))
        {
            if (qAbs(y() - items[i]->y()) <= horDist)
                hor = items[i], horDist = qAbs(y() - items[i]->y()), horType = 0;
            if (qAbs(y() + boundingRect().height() - items[i]->y()) <= horDist)
                hor = items[i], horDist = qAbs(y() + boundingRect().height() - items[i]->y()), horType = 1;
            if (qAbs(y() + boundingRect().height() - items[i]->y() - items[i]->boundingRect().height()) <= horDist)
                hor = items[i], horDist = qAbs(y() + boundingRect().height() - items[i]->y() - items[i]->boundingRect().height()), horType = 2;
            if (qAbs(y() - items[i]->y() - items[i]->boundingRect().height()) <= horDist)
                hor = items[i], horDist = qAbs(y() - items[i]->y() - items[i]->boundingRect().height()), horType = 3;
        }

        if ((y() + snapRadius >= items[i]->y() && y() <= items[i]->y() + items[i]->boundingRect().height() + snapRadius) ||
            (y() + boundingRect().height() + snapRadius >= items[i]->y() && y() + boundingRect().height() <= items[i]->y() + items[i]->boundingRect().height() + snapRadius))
        {
            if (qAbs(x() - items[i]->x()) <= verDist)
                ver = items[i], verDist = qAbs(x() - items[i]->x()), verType = 0;
            if (qAbs(x() + boundingRect().width() - items[i]->x()) <= verDist)
                ver = items[i], verDist = qAbs(x() + boundingRect().width() - items[i]->x()), verType = 1;
            if (qAbs(x() + boundingRect().width() - items[i]->x() - items[i]->boundingRect().width()) <= verDist)
                ver = items[i], verDist = qAbs(x() + boundingRect().width() - items[i]->x() - items[i]->boundingRect().width()), verType = 2;
            if (qAbs(x() - items[i]->x() - items[i]->boundingRect().width()) <= verDist)
                ver = items[i], verDist = qAbs(x() - items[i]->x() - items[i]->boundingRect().width()), verType = 3;
        }
    }
    if (hor && horDist <= snapRadius)
    {
        if (horType == 0)
            setPos(x(), hor->y());
        if (horType == 1)
            setPos(x(), hor->y() - boundingRect().height());
        if (horType == 2)
            setPos(x(), hor->y() + hor->boundingRect().height() - boundingRect().height());
        if (horType == 3)
            setPos(x(), hor->y() + hor->boundingRect().height());
    }

    if (ver && verDist <= snapRadius)
    {
        if (verType == 0)
            setPos(ver->x(), y());
        if (verType == 1)
            setPos(ver->x() - boundingRect().width(), y());
        if (verType == 2)
            setPos(ver->x() + ver->boundingRect().width() - boundingRect().width(), y());
        if (verType == 3)
            setPos(ver->x() + ver->boundingRect().width(), y());
    }

    /*if (x() < atlasBound->x())
        setX(atlasBound->x());
    if (x() + boundingRect().width() > atlasBound->x() + atlasBound->boundingRect().width())
        setX(atlasBound->x() + atlasBound->boundingRect().width() - boundingRect().width());

    if (y() < atlasBound->y())
        setY(atlasBound->y());
    if (y() + boundingRect().height() > atlasBound->y() + atlasBound->boundingRect().height())
        setY(atlasBound->y() + atlasBound->boundingRect().height() - boundingRect().height());*/
}

QString QGraphicsSpriteItem::getId() const
{
    return ID;
}

void QGraphicsSpriteItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (!isMoving)
        return;
    snapIt();
}

void QGraphicsSpriteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    isMoving = true;
    QGraphicsItem::mousePressEvent(event);
}

void QGraphicsSpriteItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    isMoving = false;
    QGraphicsItem::mouseReleaseEvent(event);
}
