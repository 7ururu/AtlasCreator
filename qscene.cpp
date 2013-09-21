#include "qscene.h"
#include <QMimeData>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <fstream>
#include <QMap>
#include "packingalgorithms.h"
#include <QApplication>

QScene::QScene(QObject *parent) : QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(QColor(196, 196, 196), Qt::Dense2Pattern));

    atlasBound = addRect(0, 0, 1024, 1024);

    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(onItemsChange()));
}

QScene::~QScene()
{
    for (int i = 0; i < spriteItems.size(); i++)
        delete spriteItems[i];
}

void QScene::addSprite(QPixmap pixmap, QString id, QPointF pos)
{
    QGraphicsSpriteItem* sprite = new QGraphicsSpriteItem(pixmap, id);
    sprite->setPos(pos);
    addItem(sprite);
    spriteItems.push_back(sprite);
    sprite->snap();
}

QGraphicsSpriteItem *QScene::getActiveItem() const
{
    for (int i = 0; i < spriteItems.size(); i++)
        if (spriteItems[i]->hasFocus())
            return spriteItems[i];
    return 0;
}


QVector<QRect> QScene::getFreeSpace() const
{
    QVector < QRect > res;
    res.push_back(atlasBound->boundingRect().toRect());

    return res;
}

QRect QScene::getAtlasBoundRect() const
{
    return atlasBound->boundingRect().toRect();
}

QVector<QGraphicsSpriteItem *> QScene::items() const
{
    return spriteItems;
}

void QScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
        event->accept();
    else
        event->ignore();
}

void QScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
    {
        event->accept();
        event->setDropAction(Qt::MoveAction);
    }
    else
        event->ignore();
}

void QScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("/sprite"))
    {
        QByteArray spriteData = event->mimeData()->data("/sprite");
        QDataStream dataStream(&spriteData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QString id;
        dataStream >> pixmap >> id;

        addSprite(pixmap, id, QPointF(event->scenePos().x() - pixmap.width() / 2, event->scenePos().y() - pixmap.height() / 2));

        for (int i = 0; i < spriteItems.size(); i++)
            spriteItems[i]->changeItemActivity(i == (spriteItems.size() - 1));

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
        event->ignore();
}

void QScene::updateIntersections()
{
    for (int i = 0; i < spriteItems.size(); i++)
    {
        bool flag = false;
        for (int j = 0; j < spriteItems.size(); j++)
            if (i != j && spriteItems[i]->collidesWithItem(spriteItems[j]))
            {
                flag = true;
                break;
            }
        if (flag || !atlasBound->contains(spriteItems[i]->pos()) ||
                    !atlasBound->contains(spriteItems[i]->pos() + spriteItems[i]->boundingRect().bottomRight()))
        {
            spriteItems[i]->setBoundingRectColor(Qt::red);
        }
        else
            spriteItems[i]->setBoundingRectColor(Qt::green);
    }
    update();
}

void QScene::updateItemsActivity()
{
    bool isSomeoneOnMouse = false;
    for (int i = 0; i < spriteItems.size(); i++)
        isSomeoneOnMouse |= spriteItems[i]->isOnMouse();
    if (isSomeoneOnMouse)
    {
        for (int i = 0; i < spriteItems.size(); i++)
            spriteItems[i]->changeItemActivity(spriteItems[i]->isOnMouse());
    }
    update();
}

void QScene::changeAtlasSize(int w, int h)
{
    atlasBound->setRect(0, 0, w, h);
    setSceneRect(0, 0, w + 1, h + 1);
}

void QScene::changeActiveSpritePosition(int dx, int dy)
{
    for (int i = 0; i < spriteItems.size(); i++)
        if (spriteItems[i]->hasFocus())
        {
            spriteItems[i]->setPos(spriteItems[i]->x() + dx, spriteItems[i]->y() + dy);
            spriteItems[i]->snap();
        }
}

void QScene::save(const QString &path) const
{
    QFile formatFile(QApplication::applicationDirPath() + "/config.txt");
    formatFile.open(QFile::ReadOnly);
    QString format(formatFile.readAll());
    QString atlasName = QFileInfo(path).baseName();

    QPixmap resultImage(atlasBound->boundingRect().width(), atlasBound->boundingRect().height());
    std::ofstream resultFile((QFileInfo(path).absolutePath() + QString("/" + atlasName + ".txt")).toStdString().c_str());

    resultImage.fill(QColor(255, 255, 255, 0));
    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    for (int i = 0; i < spriteItems.size(); i++)
    {
        painter.drawPixmap(spriteItems[i]->x(), spriteItems[i]->y(), spriteItems[i]->getPixmap());

        QString s = format;
        s.replace("%ID%", spriteItems[i]->getId());
        s.replace("%ATLAS_NAME%", atlasName);
        s.replace("%X%", QString::number(spriteItems[i]->x()));
        s.replace("%Y%", QString::number(spriteItems[i]->y()));
        s.replace("%WIDTH%", QString::number(spriteItems[i]->getPixmap().width()));
        s.replace("%HEIGHT%", QString::number(spriteItems[i]->getPixmap().height()));

        resultFile << s.toStdString() << std::endl;
    }

    resultImage.save(path);
    resultFile.close();
}

void QScene::eraseActiveItem()
{
    for (int i = 0; i < spriteItems.size(); i++)
        if (spriteItems[i]->hasFocus())
        {
            removeItem(spriteItems[i]);
            delete spriteItems[i];
            spriteItems.erase(&spriteItems[i]);
            break;
        }
    update();
}

void QScene::clear()
{
    for (int i = 0; i < spriteItems.size(); i++)
    {
        removeItem(spriteItems[i]);
        delete spriteItems[i];
    }
    spriteItems.clear();
}

void QScene::calculateEfficiency()
{
    QVector < QRect > rects;

    for (int i = 0; i < spriteItems.size(); i++)
        rects.push_back(atlasBound->boundingRect().intersected(QRectF(spriteItems[i]->pos(), spriteItems[i]->boundingRect().size())).toRect());

    emit efficiencyChanged(Packing2D::calculateEfficiency(rects, atlasBound->boundingRect().toRect(), true));
}

void QScene::onItemsChange()
{
    updateIntersections();
    updateItemsActivity();
    calculateEfficiency();
}
