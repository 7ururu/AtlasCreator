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

QScene::QScene(QObject *parent) : QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(QColor(224, 224, 224), Qt::Dense2Pattern));

    atlasBound = addRect(0, 0, 1024, 1024);

    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(findIntersections()));
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(changeItemsActivity()));

    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(calculateEfficiency()));
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(calculateEfficiency()));
}

QScene::~QScene()
{
    for (int i = 0; i < sprites.size(); i++)
        delete sprites[i];
}

void QScene::addSprite(QPixmap pixmap, QString id, QPointF pos)
{
    QGraphicsSpriteItem* sprite = new QGraphicsSpriteItem(pixmap, id, atlasBound);
    sprite->setPos(pos);

    sprites.push_back(sprite);
    addItem(sprite);
}

QGraphicsSpriteItem *QScene::getActiveItem()
{
    for (int i = 0; i < sprites.size(); i++)
        if (sprites[i]->isActiveItem())
            return sprites[i];
    return 0;
}

QVector<QGraphicsSpriteItem *> QScene::getItems() const
{
    return sprites;
}

QVector<QRectF> QScene::getFreeSpace() const
{
    QVector < QRectF > res;
    res.push_back(atlasBound->boundingRect());
    /*for (int i = 0; i < sprites.size(); i++)
    {
        QVector < QRectF > next;
        for (int j = 0; j < res.size(); j++)
            if (res[j].intersects(QRectF(sprites[i]->pos(), sprites[i]->boundingRect().size())))
            {

            }
    }*/
    return res;
}

void QScene::clear()
{
    for (int i = 0; i < sprites.size(); i++)
    {
        removeItem(sprites[i]);
        delete sprites[i];
    }
    sprites.clear();
    update();
}

QRectF QScene::getAtlasBoundRect() const
{
    return atlasBound->boundingRect();
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

        QGraphicsSpriteItem* sprite = new QGraphicsSpriteItem(pixmap, id, atlasBound);
        sprites.push_back(sprite);
        addItem(sprite);
        sprite->setPos(event->scenePos().x() - pixmap.width() / 2, event->scenePos().y() - pixmap.height() / 2);
        sprite->snapIt();

        for (int i = 0; i < sprites.size(); i++)
            sprites[i]->changeItemActivity(i == (sprites.size() - 1));

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
        event->ignore();
}

void QScene::findIntersections()
{
    for (int i = 0; i < sprites.size(); i++)
    {
        bool flag = false;
        for (int j = 0; j < sprites.size(); j++)
            if (i != j && sprites[i]->collidesWithItem(sprites[j]))
            {
                flag = true;
                break;
            }
        if (flag || !atlasBound->contains(sprites[i]->pos()) ||
                    !atlasBound->contains(sprites[i]->pos() + sprites[i]->boundingRect().bottomRight()))
            sprites[i]->setBoundingRectColor(Qt::red);
        else
            sprites[i]->setBoundingRectColor(Qt::green);
    }
    update();
}

void QScene::changeItemsActivity()
{
    int i;
    for (i = 0; i < sprites.size(); i++)
        if (sprites[i]->isPressed())
            break;
    if (i == sprites.size())
        return;
    for (int j = 0; j < sprites.size(); j++)
        sprites[j]->changeItemActivity(i == j);
    update();
}

void QScene::changeAtlasSize(int w, int h)
{
    atlasBound->setRect(0, 0, w, h);
    setSceneRect(0, 0, w + 1, h + 1);
}

void QScene::changeActiveSpritePosition(int dx, int dy)
{
    for (int i = 0; i < sprites.size(); i++)
        if (sprites[i]->isActiveItem())
        {
            sprites[i]->setPos(sprites[i]->x() + dx, sprites[i]->y() + dy);
            sprites[i]->snapIt();
        }
}

void QScene::save()
{
    static QString fileName;
    fileName = QFileDialog::getSaveFileName(0, "Choose file to save", fileName, tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.size() == 0)
        return;
    QString atlasName = QFileInfo(fileName).baseName();

    QPixmap resultImage(atlasBound->boundingRect().width(), atlasBound->boundingRect().height());
    std::ofstream resultFile((QFileInfo(fileName).absolutePath() + QString("/" + atlasName + ".txt")).toStdString().c_str());

    resultImage.fill(QColor(255, 255, 255, 0));
    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    for (int i = 0; i < sprites.size(); i++)
    {
        painter.drawPixmap(sprites[i]->x(), sprites[i]->y(), sprites[i]->getPixmap());
        resultFile << sprites[i]->getId().toStdString() << " = Sprite(" << atlasName.toStdString() << ", CIwSVec2(" <<
                      sprites[i]->x() << ", " <<  sprites[i]->y() << "), CIwSVec2(" <<
                      sprites[i]->getPixmap().width() << ", " << sprites[i]->getPixmap().height() << "));" << std::endl;
    }
    resultImage.save(fileName);
    resultFile.close();
}

void QScene::eraseActiveItem()
{
    for (int i = 0; i < sprites.size(); i++)
        if (sprites[i]->isActiveItem())
        {
            removeItem(sprites[i]);
            delete sprites[i];
            sprites.erase(&sprites[i]);
            break;
        }
    update();
}

void QScene::calculateEfficiency()
{
    QVector < QRectF > rects;

    for (int i = 0; i < sprites.size(); i++)
        rects.push_back(atlasBound->boundingRect().intersected(QRectF(sprites[i]->pos(), sprites[i]->boundingRect().size())));

    emit efficiencyChanged(Packing2D::calculateEfficiency(rects, atlasBound->boundingRect(), true));
}
