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

    void addSprite(QImage img, QString id, QPointF pos);
    QGraphicsSpriteItem* getActiveItem() const;
    QVector< QRect > getFreeSpace() const;
    QRect getAtlasBoundRect() const;

    QVector < QGraphicsSpriteItem* > items() const;

    void changeAtlasSize(int w, int h);
    void changeActiveSpritePosition(int dx, int dy);
    void savePixmap(const QString& path) const;
    void saveData(std::ofstream& out, const QString &atlasName) const;
    void saveExtraData(std::ofstream& out) const;
    void eraseActiveItem();
    void clear();

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);

private:
    QVector < QGraphicsSpriteItem* > spriteItems;
    QGraphicsRectItem* atlasBound;

    void updateIntersections();
    void updateItemsActivity();
    void calculateEfficiency();

private slots:
    void onItemsChange();

signals:
    void efficiencyChanged(double e);
};

#endif // QSCENE_H
