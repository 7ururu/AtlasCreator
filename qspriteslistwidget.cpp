#include "qspriteslistwidget.h"
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>
#include "qgraphicsspriteitem.h"

QSpritesListWidget::QSpritesListWidget(QWidget *parent) : QListWidget(parent)
{
    iconsSize = 64;

    setDragEnabled(true);
    setViewMode(QListView::IconMode);
    setIconSize(QSize(iconsSize, iconsSize));
    setSpacing(8);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setContextMenuPolicy(Qt::DefaultContextMenu);

    this->setStyleSheet("QListView {background-color: rgba(224, 224, 224, 255);}");
}

void QSpritesListWidget::addSprite(QPixmap pixmap, QString id)
{
    QListWidgetItem *sprite = new QListWidgetItem(this);
    sprite->setIcon(QIcon(pixmap));
    sprite->setData(Qt::UserRole, QVariant(pixmap));
    sprite->setData(Qt::UserRole + 1, QVariant(id));
    sprite->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

QVector<QListWidgetItem *> QSpritesListWidget::getItems() const
{
    bool selected = !selectedItems().empty();
    QVector < QListWidgetItem* > res;
    for (int i = 0; i < count(); i++)
    {
        QListWidgetItem* tmp = item(i);
        if (selected && !tmp->isSelected())
            continue;
        res.push_back(tmp);
    }

    return res;
}

void QSpritesListWidget::startDrag(Qt::DropActions supportedActions)
{
    QListWidgetItem *item = currentItem();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QPixmap pixmap = qVariantValue<QPixmap>(item->data(Qt::UserRole));
    QString id = qVariantValue<QString>(item->data(Qt::UserRole + 1));

    dataStream << pixmap << id;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("/sprite", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));
    drag->setPixmap(pixmap);

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
        delete takeItem(row(item));
}

void QSpritesListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
        event->accept();
    else
        event->ignore();
}

void QSpritesListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
        event->ignore();
}

void QSpritesListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
    {
        QByteArray itemData = event->mimeData()->data("/sprite");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPixmap pixmap;
        QString id;
        dataStream >> pixmap >> id;

        addSprite(pixmap, id);

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
        event->ignore();
}

void QSpritesListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (selectedItems().empty())
        return;
    QMenu menu(this);
    QAction* action = menu.addAction("delete");
    connect(action, SIGNAL(triggered()), this, SLOT(eraseActiveItem()));
    menu.exec(event->globalPos());
}

void QSpritesListWidget::changeIconsSize(int size)
{
    iconsSize = size;
    setIconSize(QSize(iconsSize, iconsSize));
}

void QSpritesListWidget::eraseActiveItem()
{
    QList < QListWidgetItem* > items = selectedItems();
    for (int i = 0; i < items.size(); i++)
        delete takeItem(row(items[i]));
}
