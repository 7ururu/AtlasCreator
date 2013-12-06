#include "qspriteslistwidget.h"
#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>

QSpritesListWidget::QSpritesListWidget(QWidget *parent) : QListWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setViewMode(QListView::IconMode);
    setIconSize(QSize(ICONS_SIZE, ICONS_SIZE));
    setSpacing(ICONS_INDENT);

    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void QSpritesListWidget::addItem(QImage img, QString id)
{
    QListWidgetItem *sprite = new QListWidgetItem(this);
    sprite->setIcon(QIcon(QPixmap::fromImage(img)));
    sprite->setData(Qt::UserRole, QVariant(img));
    sprite->setData(Qt::UserRole + 1, QVariant(id));
    sprite->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

void QSpritesListWidget::startDrag(Qt::DropActions supportedActions)
{
    QListWidgetItem *item = currentItem();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    QImage img = qVariantValue < QImage >(item->data(Qt::UserRole));
    QString id = qVariantValue < QString >(item->data(Qt::UserRole + 1));

    dataStream << img << id;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("/sprite", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(img.width() / 2, img.height() / 2));
    drag->setPixmap(QPixmap::fromImage(img));

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
        delete item;
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
        QImage img;
        QString id;
        dataStream >> img >> id;

        addItem(img, id);

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
    connect(action, SIGNAL(triggered()), this, SLOT(eraseActiveItems()));
    menu.exec(event->globalPos());
}

void QSpritesListWidget::eraseActiveItems()
{
    QList < QListWidgetItem* > items = selectedItems();
    for (int i = 0; i < items.size(); i++)
        delete items[i];
}

QVector<QListWidgetItem *> QSpritesListWidget::items() const
{
    QVector < QListWidgetItem* > result;
    for (int i = 0; i < count(); i++)
        result.push_back(item(i));
    return result;
}

