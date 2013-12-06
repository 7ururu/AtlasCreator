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

void QScene::addSprite(QImage img, QString id, QPointF pos)
{
    QGraphicsSpriteItem* sprite = new QGraphicsSpriteItem(img, id);
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

        QImage img;
        QString id;
        dataStream >> img >> id;

        addSprite(img, id, QPointF(event->scenePos().x() - img.width() / 2, event->scenePos().y() - img.height() / 2));

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

void QScene::savePixmap(const QString &path) const
{
    QImage resultImage(atlasBound->boundingRect().width(), atlasBound->boundingRect().height(), QImage::Format_ARGB32);

    resultImage.fill(0x00ffffff);

    int m = QGraphicsSpriteItem::getMargin();
    for (int k = 0; k < spriteItems.size(); k++)
    {
        QImage img = spriteItems[k]->getImage().convertToFormat(QImage::Format_ARGB32);
        for (int i = 0; i < img.width(); i++)
            for (int j = 0; j < img.height(); j++)
                resultImage.setPixel(spriteItems[k]->x() + m + i, spriteItems[k]->y() + m + j, img.pixel(i, j));
        for (int i = 0; i < m; i++)
            for (int j = 0; j < img.height(); j++)
            {
                resultImage.setPixel(spriteItems[k]->x() + i, spriteItems[k]->y() + m + j,
                                     img.pixel(0, j) & 0x00ffffff);
                resultImage.setPixel(spriteItems[k]->x() + i + m + img.width(), spriteItems[k]->y() + m + j,
                                     img.pixel(img.width() - 1, j) & 0x00ffffff);
            }
        for (int j = 0; j < m; j++)
            for (int i = 0; i < img.width(); i++)
            {
                resultImage.setPixel(spriteItems[k]->x() + m + i, spriteItems[k]->y() + j,
                                     img.pixel(i, 0) & 0x00ffffff);
                resultImage.setPixel(spriteItems[k]->x() + m + i, spriteItems[k]->y() + j + m + img.height(),
                                     img.pixel(i, img.height() - 1) & 0x00ffffff);
            }
        for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
            {
                resultImage.setPixel(spriteItems[k]->x() + i, spriteItems[k]->y() + j,
                                     img.pixel(0, 0) & 0x00ffffff);
                resultImage.setPixel(spriteItems[k]->x() + img.width() + m + i, spriteItems[k]->y() + j,
                                     img.pixel(img.width() - 1, 0) & 0x00ffffff);
                resultImage.setPixel(spriteItems[k]->x() + i, spriteItems[k]->y() + img.height() + m + j,
                                     img.pixel(0, img.height() - 1) & 0x00ffffff);
                resultImage.setPixel(spriteItems[k]->x() + img.width() + m + i, spriteItems[k]->y() + img.height() + m + j,
                                     img.pixel(img.width() - 1, img.height() - 1) & 0x00ffffff);
            }
    }

    resultImage.save(path);
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

void QScene::saveData(std::ofstream &out, const QString& atlasName) const
{
    QFile formatFile(QApplication::applicationDirPath() + "/config.txt");
    formatFile.open(QFile::ReadOnly);
    QString format(formatFile.readAll());

    for (int i = 0; i < spriteItems.size(); i++)
    {
        QString s = format;
        s.replace("%ID%", spriteItems[i]->getId());
        s.replace("%ATLAS_NAME%", atlasName);
        s.replace("%X%", QString::number(spriteItems[i]->x() + QGraphicsSpriteItem::getMargin()));
        s.replace("%Y%", QString::number(spriteItems[i]->y() + QGraphicsSpriteItem::getMargin()));
        s.replace("%WIDTH%", QString::number(spriteItems[i]->getPixmap().width()));
        s.replace("%HEIGHT%", QString::number(spriteItems[i]->getPixmap().height()));

        out << s.toStdString() << std::endl;

        /*if (spriteItems[i]->getId().toStdString().find("[") == std::string::npos)
        {
            out << "spriteFromString[\"" << spriteItems[i]->getId().toStdString() << "\"] = " <<
                   spriteItems[i]->getId().toStdString() << ";" << std::endl;
        }*/
    }

    out << std::endl << std::endl;
}

void QScene::saveExtraData(std::ofstream &out) const
{
    for (int i = 0; i < spriteItems.size(); i++)
    {
        if (spriteItems[i]->getId().toStdString().find("[") == std::string::npos)
        {
            out << spriteItems[i]->getId().toStdString() << "," << std::endl;
        }
    }
    out << std::endl << std::endl;
}
