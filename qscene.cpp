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
    setBackgroundBrush(QBrush(QColor(96, 96, 96), Qt::Dense2Pattern));

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
    for (int i = 0; i < spriteItems.size(); i++)
        spriteItems[i]->changeItemActivity(spriteItems[i]->hasFocus());
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

void QScene::save()
{
    /*static QString fileName;
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
        resultFile << sprites[i]->getId().toStdString() << " = Sprite(" << atlasName.toStdString() << ", CIwFVec2(" <<
                      sprites[i]->x() << ", " <<  sprites[i]->y() << "), CIwFVec2(" <<
                      sprites[i]->getPixmap().width() << ", " << sprites[i]->getPixmap().height() << "));" << std::endl;
        if (sprites[i]->getId().toStdString().find("[") == std::string::npos)
        {
            resultFile << "spriteFromString[\"" << sprites[i]->getId().toStdString() << "\"] = " <<
                          sprites[i]->getId().toStdString() << ";" << std::endl;
        }
    }
    resultFile << std::endl << std::endl;
    for (int i = 0; i < sprites.size(); i++)
        if (sprites[i]->getId().toStdString().find("[") == std::string::npos)
        {
            resultFile << sprites[i]->getId().toStdString() << "," << std::endl;
        }

    resultImage.save(fileName);
    resultFile.close();*/
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
