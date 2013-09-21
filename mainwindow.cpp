#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fstream>
#include <QFileInfo>
#include <QGraphicsView>
#include <QPair>
#include "packingalgorithms.h"
#include <ctime>
#include <QDir>
#include "qgraphicsspriteitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    currScene = 0;
    srand(time(0));

    QGraphicsSpriteItem::setMargin(1);

    on_toolButtonNewTab_released();
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_radioButton2048_toggled(bool checked)
{
    if (checked)
        currScene->changeAtlasSize(2048, 2048);
}

void MainWindow::on_radioButton1024_toggled(bool checked)
{
    if (checked)
        currScene->changeAtlasSize(1024, 1024);
}

void MainWindow::on_radioButton512_toggled(bool checked)
{
    if (checked)
        currScene->changeAtlasSize(512, 512);
}

void MainWindow::on_radioButtonCustom_toggled(bool checked)
{
    if (checked)
        currScene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
    ui->lineEditWidth->setEnabled(checked);
    ui->lineEditHeight->setEnabled(checked);
}

void MainWindow::on_lineEditWidth_editingFinished()
{
    currScene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
}

void MainWindow::on_lineEditHeight_editingFinished()
{
    currScene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
}

void MainWindow::on_spinBoxMargin_valueChanged(int arg1)
{
    QGraphicsSpriteItem::setMargin(arg1);
}

void MainWindow::on_spinBoxSnap_valueChanged(int arg1)
{
    QGraphicsSpriteItem::setSnapRadius(arg1);
}

void MainWindow::on_actionAddSprites_triggered()
{
    static QStringList fileNames;
    QStringList temp = QFileDialog::getOpenFileNames(this, tr("Add images"),
                                                     fileNames.empty() ? "" :QFileInfo(fileNames.front()).absolutePath(),
                                                     tr("Image Files (*.png *.jpg *.bmp)"));
    if (temp.empty())
        return;

    fileNames = temp;
    for (int i = 0; i < fileNames.size(); i++)
    {
        QString id = QFileInfo(fileNames[i]).baseName();
        QPixmap pixmap = QPixmap(fileNames[i]);
        ui->spritesListWidget->addItem(pixmap, id);
    }
}

void MainWindow::on_actionMoveItemUp_triggered()
{
    currScene->changeActiveSpritePosition(0, -1);
}

void MainWindow::on_actionMoveItemDown_triggered()
{
    currScene->changeActiveSpritePosition(0, 1);
}

void MainWindow::on_actionMoveItemLeft_triggered()
{
    currScene->changeActiveSpritePosition(-1, 0);
}

void MainWindow::on_actionMoveItemRight_triggered()
{
    currScene->changeActiveSpritePosition(1, 0);
}

void MainWindow::on_actionDeleteActiveItem_triggered()
{
    QGraphicsSpriteItem* active = currScene->getActiveItem();
    if (!active)
        return;
    ui->spritesListWidget->addItem(active->getPixmap(), active->getId());
    currScene->eraseActiveItem();
}

void MainWindow::on_tabWidgetMainScene_currentChanged(int index)
{
    updateScene();
}

void MainWindow::updateScene()
{
    if (ui->tabWidgetMainScene->count() == 0)
        return;

    if (currScene && ui->tabWidgetMainScene->count() > 1)
        disconnect(currScene, SIGNAL(efficiencyChanged(double)), this, SLOT(onEfficiencyChanged(double)));
    currScene = (QScene*)((QGraphicsView*)ui->tabWidgetMainScene->currentWidget())->scene();
    connect(currScene, SIGNAL(efficiencyChanged(double)), this, SLOT(onEfficiencyChanged(double)));

    QRectF r = ((QGraphicsView*)ui->tabWidgetMainScene->currentWidget())->sceneRect();
    r.setWidth(r.width() - 1);
    r.setHeight(r.height() - 1);

    if (r.width() == 2048 && r.height() == 2048)
        ui->radioButton2048->setChecked(true);
    else if (r.width() == 1024 && r.height() == 1024)
        ui->radioButton1024->setChecked(true);
    else if (r.width() == 512 && r.height() == 512)
        ui->radioButton512->setChecked(true);
    else
    {
        ui->radioButtonCustom->setChecked(true);
        ui->lineEditWidth->setText(QString::number(r.width()));
        ui->lineEditHeight->setText(QString::number(r.height()));
    }

    ui->tabWidgetMainScene->currentWidget()->setFocus();
}

void MainWindow::packSprites(Packing2D::PackingFunction algo)
{
    QVector < QListWidgetItem* > items = ui->spritesListWidget->items();
    if (items.empty())
        return;
    QVector < QRect > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRect(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRect > conts = currScene->getFreeSpace();

    Packing2D::RectsCompareFunction rectsCmp = Packing2D::rectsCompareByArea;
    Packing2D::ContsComparator contsComp = Packing2D::contsComparatorByArea;
    if (ui->comboBoxSortBy->currentText() == "Height")
        rectsCmp = Packing2D::rectsCompareByHeight, contsComp = Packing2D::contsComparatorByHeight;
    else if (ui->comboBoxSortBy->currentText() == "Width")
        rectsCmp = Packing2D::rectsCompareByWidth, contsComp = Packing2D::contsComparatorByHeight;
    else if (ui->comboBoxSortBy->currentText() == "Max side")
        rectsCmp = Packing2D::rectsCompareByMaxSide, contsComp = Packing2D::contsComparatorByMaxSide;

    if (ui->radioButtonMoveLeft->isChecked())
        contsComp = Packing2D::contsComparatorMoveLeft;
    if (ui->radioButtonMoveUp->isChecked())
        contsComp = Packing2D::contsComparatorMoveUp;

    QVector < QPoint > res = algo(rects, conts, currScene->getAtlasBoundRect(), rectsCmp, contsComp);

    for (int i = 0; i < rects.size(); i++)
    {
        for (int j = i; j < rects.size(); j++)
        {
            QPixmap pixmap = qVariantValue<QPixmap>(items[j]->data(Qt::UserRole));
            if (rects[i].size() == pixmap.size() + QSize(QGraphicsSpriteItem::getMargin() * 2, QGraphicsSpriteItem::getMargin() * 2))
            {
                qSwap(items[i], items[j]);
                break;
            }
        }
    }
    for (int i = 0; i < rects.size(); i++)
        if (res[i] != Packing2D::NULL_POINT)
        {
            currScene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                                 qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                                 res[i]);
            delete items[i];
        }
}

void MainWindow::packSpritesBestWay()
{
    QVector < QListWidgetItem* > items = ui->spritesListWidget->items();
    if (items.empty())
        return;

    time_t currTime = time(0);
    ui->progressBarPackingProgress->setEnabled(true);
    ui->progressBarPackingProgress->setValue(0);
    ui->progressBarPackingProgress->update();

    QVector < QRect > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRect(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRect > conts = currScene->getFreeSpace();

    const int nCmp = 4;
    Packing2D::RectsCompareFunction cmpFunction[nCmp] = { Packing2D::rectsCompareByArea,
                                                          Packing2D::rectsCompareByMaxSide,
                                                          Packing2D::rectsCompareByHeight,
                                                          Packing2D::rectsCompareByWidth };
    const int nFunctions = 1;
    Packing2D::PackingFunction algo[nFunctions] = { Packing2D::stupidGAPacking };
    const int nComps = 6;
    Packing2D::ContsComparator comp[nComps] = { Packing2D::contsComparatorByArea,
                                                Packing2D::contsComparatorByMaxSide,
                                                Packing2D::contsComparatorByHeight,
                                                Packing2D::contsComparatorByWidth,
                                                Packing2D::contsComparatorMoveLeft,
                                                Packing2D::contsComparatorMoveUp};
    const int nTimes = 1;

    ui->progressBarPackingProgress->setRange(0, nCmp * nFunctions * nComps * nTimes);
    ui->progressBarPackingProgress->update();

    QVector < QPoint > resPoints;
    QVector < QRect > resRects;
    QVector< QPoint > tmp;
    double bestEfficiency = -1.0;

    for (int t = 0; t < nTimes; t++)
        for (int cmp = 0; cmp < nCmp; cmp++)
            for (int c = 0; c < nComps; c++)
                for (int f = 0; f < nFunctions; f++)
                {
                    double e;
                    getResult(rects, conts, cmpFunction[cmp], comp[c], algo[f], tmp, e);
                    if (e > bestEfficiency)
                        bestEfficiency = e, resPoints = tmp, resRects = rects;
                    ui->progressBarPackingProgress->setValue(ui->progressBarPackingProgress->value() + 1);
                    ui->progressBarPackingProgress->update();
                    QCoreApplication::processEvents();
                }

    qDebug("%f", bestEfficiency);

    for (int i = 0; i < rects.size(); i++)
    {
        for (int j = i; j < rects.size(); j++)
        {
            QPixmap pixmap = qVariantValue<QPixmap>(items[j]->data(Qt::UserRole));
            if (resRects[i].size() == pixmap.size() + QSize(QGraphicsSpriteItem::getMargin() * 2, QGraphicsSpriteItem::getMargin() * 2))
            {
                qSwap(items[i], items[j]);
                break;
            }
        }
    }
    for (int i = 0; i < rects.size(); i++)
        if (resPoints[i] != Packing2D::NULL_POINT)
        {
            currScene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                                 qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                                 resPoints[i]);
            delete items[i];
        }

    ui->progressBarPackingProgress->setValue(0);
    ui->progressBarPackingProgress->setEnabled(false);
    ui->progressBarPackingProgress->update();

    qDebug("%i", time(0) - currTime);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::ControlModifier && ui->tabWidgetMainScene->currentWidget() != focusWidget())
    {
        int index = ui->tabWidgetMainScene->currentIndex() + 1;
        if (index == ui->tabWidgetMainScene->count())
            index = 0;
        ui->tabWidgetMainScene->setCurrentIndex(index);
    }
    else if (e->key() == Qt::Key_T && e->modifiers() == Qt::ControlModifier)
    {
        on_toolButtonNewTab_released();
    }
    else if (e->key() == Qt::Key_C && e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        on_toolButtonClearScene_released();
    }
}

void MainWindow::onEfficiencyChanged(double e)
{
    ui->labelEfficiency->setText("<b>Efficiency = " + QString::number(e, 'f', 3) + "%</b>");

    QPixmap pix(128, 128);
    QPainter painter(&pix);
    painter.fillRect(0, 0, pix.width(), pix.height(), Qt::gray);
    painter.setRenderHint(QPainter::Antialiasing);
    currScene->render(&painter);
    ui->labelMiniMap->setPixmap(pix);
}

void MainWindow::on_tabWidgetMainScene_tabCloseRequested(int index)
{
    QVector < QGraphicsSpriteItem* > items = scenes[index]->items();
    for (int i = 0; i < items.size(); i++)
        ui->spritesListWidget->addItem(items[i]->getPixmap(), items[i]->getId());

    ui->tabWidgetMainScene->removeTab(index);
    delete scenes[index];   scenes.remove(index, 1);
    delete views[index];    views.remove(index, 1);

    if (ui->tabWidgetMainScene->count() == 0)
        on_toolButtonNewTab_released();

    updateScene();
}

void MainWindow::on_actionSaveAtlas_triggered()
{
    static QString fileName;
    QString temp = QFileDialog::getSaveFileName(this, tr("Save atlas"), QFileInfo(fileName).absolutePath());
    if (temp.isEmpty())
        return;
    fileName = temp;
    currScene->save(fileName);
}

void MainWindow::on_toolButtonNewTab_released()
{
    scenes.push_back(new QScene);
    views.push_back(new QGraphicsView);

    views.back()->setScene(scenes.back());
    views.back()->centerOn(0, 0);

    if (scenes.size() > 1)
        scenes.back()->changeAtlasSize(currScene->getAtlasBoundRect().width(), currScene->getAtlasBoundRect().height());
    else
        scenes.back()->changeAtlasSize(1024, 1024);

    ui->tabWidgetMainScene->insertTab(ui->tabWidgetMainScene->count(), views.back(), "Atlas " + QString::number(ui->tabWidgetMainScene->count()));
    ui->tabWidgetMainScene->setCurrentIndex(ui->tabWidgetMainScene->count() - 1);
}

void MainWindow::on_pushButtonGenerateSprites_released()
{
    const int Min = 16,
              Max = 200;
    for (int i = 0; i < 100; i++)
    {
        QPixmap pix(rand() % (Max - Min + 1) + Min, rand() % (Max - Min + 1) + Min);
        QPainter painter(&pix);
        painter.fillRect(0, 0, pix.width(), pix.height(), QBrush(QColor(rand()%256,rand()%256,rand()%256)));
        pix.save("pix" + QString::number(i) + ".png");
    }
}

void MainWindow::on_toolButtonClearList_released()
{
    ui->spritesListWidget->clear();
}

void MainWindow::on_pushButtonBruteForce_released()
{
    do packSprites(Packing2D::bruteForcePacking); while (shouldContinuePacking());
}

void MainWindow::on_pushButtonGA_released()
{
    do packSprites(Packing2D::stupidGAPacking); while (shouldContinuePacking());
}

void MainWindow::on_pushButtonPackBestWay_released()
{
    do packSpritesBestWay(); while (shouldContinuePacking());
}

void MainWindow::getResult(QVector<QRect> &rects, QVector<QRect> &conts, Packing2D::RectsCompareFunction rectsCmp, Packing2D::ContsComparator contsComp,
                           Packing2D::PackingFunction algo, QVector < QPoint >& res, double& efficiency)
{
    res = algo(rects, conts, currScene->getAtlasBoundRect(), rectsCmp, contsComp);
    QVector < QRect > r;
    for (int i = 0; i < rects.size(); i++)
        if (res[i] != Packing2D::NULL_POINT)
            r.push_back(QRect(res[i], rects[i].size()));
    efficiency = Packing2D::calculateEfficiency(r, currScene->getAtlasBoundRect(), false);
}

bool MainWindow::shouldContinuePacking()
{
    static int prevItemsSize = 0;
    if (ui->checkBoxApplyToAll->isChecked() && ui->spritesListWidget->items().size() != prevItemsSize &&
        ui->spritesListWidget->items().size() > 0)
    {
        prevItemsSize = ui->spritesListWidget->items().size();
        on_toolButtonNewTab_released();
        return true;
    }
    else
    {
        prevItemsSize = ui->spritesListWidget->items().size();
        return false;
    }
}

void MainWindow::on_actionAddFolder_triggered()
{
    static QString folderName;
    QString temp = QFileDialog::getExistingDirectory(this, tr("Add folder"), QFileInfo(folderName).absolutePath());
    if (temp.isEmpty())
        return;
    folderName = temp;
    QVector < QString > folders;
    folders.push_back(folderName);
    while (!folders.isEmpty())
    {
        qDebug(folders.back().toStdString().c_str());
        QFileInfoList files = QDir(folders.back()).entryInfoList();
        folders.pop_back();
        for (int i = 0; i < files.size(); i++)
            if (files[i].isFile())
            {
                QFileInfo file = files[i];
                if (file.suffix() == "png" || file.suffix() == "jpg" || file.suffix() == "bmp")
                {
                    QString id = file.baseName();
                    QPixmap pixmap = QPixmap(file.filePath());
                    ui->spritesListWidget->addItem(pixmap, id);
                }
            }
            else if (files[i].isDir() && files[i].fileName() != ".." && files[i].fileName() != ".")
            {
                QString folder = files[i].absoluteFilePath();
                folder.replace("/", "\\");
                folders.push_back(folder);
            }

    }
}

void MainWindow::on_toolButtonClearScene_released()
{
    QVector < QGraphicsSpriteItem* > items = currScene->items();
    for (int i = 0; i < items.size(); i++)
        ui->spritesListWidget->addItem(items[i]->getPixmap(), items[i]->getId());
    currScene->clear();
}
