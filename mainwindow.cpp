#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fstream>
#include <QFileInfo>
#include <QGraphicsView>
#include <QPair>
#include "packingalgorithms.h"
#include <ctime>
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

    currScene = (QScene*)((QGraphicsView*)ui->tabWidgetMainScene->currentWidget())->scene();
    if (currScene && ui->tabWidgetMainScene->count() > 1)
        disconnect(currScene, SIGNAL(efficiencyChanged(double)), this, SLOT(onEfficiencyChanged(double)));
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
        ui->lineEditWidth->setText(QString::number(r.width() - 1));
        ui->lineEditHeight->setText(QString::number(r.height() - 1));
    }
}

void MainWindow::packSprites(Packing2D::PackingFunction algo)
{
    QVector < QListWidgetItem* > items = ui->spritesListWidget->items();
    QVector < QRect > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRect(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRect > conts = currScene->getFreeSpace();

    Packing2D::CompareFunction cmp = Packing2D::compareByArea;
    Packing2D::Comparator comp = Packing2D::comparatorByArea;
    if (ui->comboBoxSortBy->currentText() == "Height")
        cmp = Packing2D::compareByHeight, comp = Packing2D::comparatorByHeight;
    else if (ui->comboBoxSortBy->currentText() == "Width")
        cmp = Packing2D::compareByWidth, comp = Packing2D::comparatorByHeight;

    if (ui->radioButtonMoveLeft->isChecked())
        comp = Packing2D::comparatorMoveLeft;
    if (ui->radioButtonMoveUp->isChecked())
        comp = Packing2D::comparatorMoveUp;

    QVector < QPair < bool,QPoint > > res = algo(rects, conts, currScene->getAtlasBoundRect(), cmp, comp);

    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
        {
            currScene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                                 qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                                 res[i].second);
            delete items[i];
        }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
   /* if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::ControlModifier && ui->tabWidget->currentWidget() != focusWidget())
    {
        int index = ui->tabWidget->currentIndex() + 1;
        if (index == ui->tabWidget->count())
            index = 0;
        ui->tabWidget->setCurrentIndex(index);
    }
    else if (e->key() == Qt::Key_T && e->modifiers() == Qt::ControlModifier)
    {
        on_toolButtonAddTab_released();
    }
    else if (e->key() == Qt::Key_C && e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        on_toolButtonClearAtlas_released();
    }*/
}

void MainWindow::onEfficiencyChanged(double e)
{
    ui->labelEfficiency->setText("<b>Efficiency = " + QString::number(e, 'f', 2) + "%</b>");

    QPixmap pix(ui->labelEfficiency->width(), ui->labelEfficiency->width());
    QPainter painter(&pix);
    painter.fillRect(0, 0, pix.width(), pix.height(), Qt::white);
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
    currScene->save();
}

void MainWindow::on_toolButtonNewTab_released()
{
    scenes.push_back(new QScene);
    views.push_back(new QGraphicsView);

    views.back()->setScene(scenes.back());
    views.back()->centerOn(0, 0);
    scenes.back()->changeAtlasSize(1024, 1024);

    ui->tabWidgetMainScene->insertTab(ui->tabWidgetMainScene->count(), views.back(), "Atlas " + QString::number(ui->tabWidgetMainScene->count()));
    ui->tabWidgetMainScene->setCurrentIndex(ui->tabWidgetMainScene->count() - 1);
}

void MainWindow::on_pushButtonGenerateSprites_released()
{
    int Min = 16, Max = 200;
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
    packSprites(Packing2D::bruteForcePacking);
}

void MainWindow::on_pushButtonGA_released()
{
    packSprites(Packing2D::stupidGAPacking);
}

void MainWindow::on_pushButtonPackBestWay_released()
{
    time_t currTime = time(0);
    ui->progressBarPackingProgress->setEnabled(true);
    ui->progressBarPackingProgress->setValue(0);
    ui->progressBarPackingProgress->update();

    QVector < QListWidgetItem* > items = ui->spritesListWidget->items();
    QVector < QRect > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRect(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRect > conts = currScene->getFreeSpace();

    const int nCmp = 3;
    Packing2D::CompareFunction cmpFunction[nCmp] = { Packing2D::compareByArea, Packing2D::compareByHeight, Packing2D::compareByWidth };
    const int nFunctions = 1;
    Packing2D::PackingFunction algo[nFunctions] = { Packing2D::stupidGAPacking };
    const int nComps = 3;
    Packing2D::Comparator comp[nComps] = { Packing2D::comparatorByArea, Packing2D::comparatorByHeight, Packing2D::comparatorByWidth,
                                           /*Packing2D::comparatorMoveLeft, Packing2D::comparatorMoveUp*/ };
    const int nTimes = 1;

    ui->progressBarPackingProgress->setRange(0, nCmp * nFunctions * nComps * nTimes);
    ui->progressBarPackingProgress->update();

    QVector<QPair<bool, QPoint> > res;
    double bestEfficiency = -1.0;

    for (int t = 0; t < nTimes; t++)
        for (int cmp = 0; cmp < nCmp; cmp++)
            for (int c = 0; c < nComps; c++)
                for (int f = 0; f < nFunctions; f++)
                {
                    double e;
                    QVector<QPair<bool, QPoint> > tmp;
                    getResult(rects, conts, cmpFunction[cmp], comp[c], algo[f], tmp, e);
                    if (e > bestEfficiency)
                        bestEfficiency = e, res = tmp;
                    ui->progressBarPackingProgress->setValue(ui->progressBarPackingProgress->value() + 1);
                    ui->progressBarPackingProgress->update();
                    QCoreApplication::processEvents();
                }

    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
        {
            currScene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                                 qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                                 res[i].second);
            delete items[i];
        }

    ui->progressBarPackingProgress->setValue(0);
    ui->progressBarPackingProgress->setEnabled(false);
    ui->progressBarPackingProgress->update();

    qDebug("%i", time(0) - currTime);
}

void MainWindow::getResult(QVector<QRect> &rects, QVector<QRect> &conts, Packing2D::CompareFunction cmp,
                           Packing2D::Comparator comp, Packing2D::PackingFunction algo, QVector<QPair<bool, QPoint> > &res, double &efficiency)
{
    res = algo(rects, conts, currScene->getAtlasBoundRect(), cmp, comp);
    QVector < QRect > r;
    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
            r.push_back(QRect(res[i].second, rects[i].size()));
    efficiency = Packing2D::calculateEfficiency(r, currScene->getAtlasBoundRect(), false);
}

void MainWindow::on_actionAddFolder_triggered()
{

}
