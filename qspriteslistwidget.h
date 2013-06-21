#ifndef QSPRITESLISTWIDGET_H
#define QSPRITESLISTWIDGET_H

#include <QListWidget>
#include <QVector>
#include "qgraphicsspriteitem.h"

class QSpritesListWidget : public QListWidget
{
    Q_OBJECT
public:
    QSpritesListWidget(QWidget* parent = 0);

    void addSprite(QPixmap pixmap, QString id);
    QVector < QListWidgetItem* > getItems() const;

protected:
    void startDrag(Qt::DropActions supportedActions);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void contextMenuEvent(QContextMenuEvent *event);

private:
    int iconsSize;

public slots:
    void changeIconsSize(int size);
    void eraseActiveItem();
};

#endif // QSPRITESLISTWIDGET_H
