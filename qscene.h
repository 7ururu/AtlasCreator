#ifndef QSCENE_H
#define QSCENE_H

#include <QGraphicsScene>
#include "qgraphicsspriteitem.h"

class QScene : public QGraphicsScene
{
    Q_OBJECT
public:
    QScene(QObject *parent = 0);
    ~QScene();

    void addSprite(QPixmap pixmap, QString id, QPointF pos);
    QGraphicsSpriteItem* getActiveItem();
    QVector < QGraphicsSpriteItem* > getItems() const;
    QVector < QRectF > getFreeSpace() const;
    void clear();
    QRectF getAtlasBoundRect() const;

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private:
    QGraphicsRectItem* atlasBound;

    QVector < QGraphicsSpriteItem* > sprites;

    void moveActiveSprite(int dx, int dy);

public slots:
    void findIntersections();
    void changeItemsActivity();
    void changeAtlasSize(int w, int h);
    void changeActiveSpritePosition(int dx, int dy);
    void save();
    void eraseActiveItem();
    void calculateEfficiency();

signals:
    void efficiencyChanged(double e);
};

#endif // QSCENE_H
