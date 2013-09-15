#include "qgraphicsspriteitem.h"
#include "qscene.h"

int QGraphicsSpriteItem::snapRadius = 0;
int QGraphicsSpriteItem::margin = 0;

QGraphicsSpriteItem::QGraphicsSpriteItem(const QPixmap& pix, QString id) :
    pixmap(pix), id(id)
{
    setFlag(ItemIsMovable);
    boundColor = Qt::green;
    isPressed = false;
    isActive = false;
}

QRectF QGraphicsSpriteItem::boundingRect() const
{
    return QRectF(0, 0, pixmap.width() + margin * 2, pixmap.height() + margin * 2);
}

void QGraphicsSpriteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPos((int)x(), (int)y());
    painter->drawPixmap(QRect(margin, margin, pixmap.width(), pixmap.height()), pixmap);
    painter->setPen(QPen(boundColor));
    if (hasFocus())
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
    return pixmap;
}

bool QGraphicsSpriteItem::hasFocus() const
{
    return isActive;
}

void QGraphicsSpriteItem::changeItemActivity(bool activity)
{
    isActive = activity;
}

bool QGraphicsSpriteItem::isOnMouse() const
{
    return isPressed;
}

void QGraphicsSpriteItem::snap()
{
    QList < QGraphicsItem* > items = ((QScene*)scene())->items();
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
}

QString QGraphicsSpriteItem::getId() const
{
    return id;
}

void QGraphicsSpriteItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (!isPressed)
        return;
    snap();
}

void QGraphicsSpriteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    isPressed = true;
    QGraphicsItem::mousePressEvent(event);
}

void QGraphicsSpriteItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    isPressed = false;
    QGraphicsItem::mouseReleaseEvent(event);
}
