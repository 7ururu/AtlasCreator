#ifndef QGRAPHICSSPRITEITEM_H
#define QGRAPHICSSPRITEITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPixmap>
#include <QString>

class QGraphicsSpriteItem : public QGraphicsItem
{
public:
    QGraphicsSpriteItem(const QPixmap& pix, QString id);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    static int getMargin();
    QPixmap    getPixmap() const;
    QString    getId() const;

    void setBoundingRectColor(QColor color);
    static void setMargin(int m);
    static void setSnapRadius(int radius);

    bool hasFocus() const;
    bool isOnMouse() const;

    void changeItemActivity(bool activity);
    void snap();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    static int margin;
    static int snapRadius;

    QPixmap        pixmap;
    QString        id;
    QColor         boundColor;
    bool           isPressed;
    bool           isActive;
};

#endif // QGRAPHICSSPRITEITEM_H
