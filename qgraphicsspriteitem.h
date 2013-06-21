#ifndef QGRAPHICSSPRITEITEM_H
#define QGRAPHICSSPRITEITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsScene>
#include <QString>

class QGraphicsSpriteItem : public QGraphicsItem
{
public:
    QGraphicsSpriteItem(const QPixmap& pix, QString id, QGraphicsItem* atlasBound);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setBoundingRectColor(QColor color);

    static void setMargin(int m);
    static int getMargin();
    static void setSnapRadius(int radius);

    QPixmap getPixmap() const;

    bool isActiveItem() const;
    void changeItemActivity(bool activity);
    bool isPressed() const;
    void snapIt();
    QString getId() const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QPixmap pix;
    QColor boundColor;
    static int margin;
    static int snapRadius;
    bool isMoving;
    QGraphicsItem* atlasBound;
    bool itemActivity;
    QString ID;
};

#endif // QGRAPHICSSPRITEITEM_H
