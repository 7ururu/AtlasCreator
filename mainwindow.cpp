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
    scene = 0;
    srand(time(0));

    QGraphicsSpriteItem::setMargin(1);

    on_toolButtonAddTab_released();

    connect(ui->iconsSizeSlider, SIGNAL(valueChanged(int)), ui->spritesListWidget, SLOT(changeIconsSize(int)));

    ui->progressBar->setVisible(false);

    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_radioButton2048_toggled(bool checked)
{
    if (checked)
        scene->changeAtlasSize(2048, 2048);
}

void MainWindow::on_radioButton1024_toggled(bool checked)
{
    if (checked)
        scene->changeAtlasSize(1024, 1024);
}

void MainWindow::on_radioButton512_toggled(bool checked)
{
    if (checked)
        scene->changeAtlasSize(512, 512);
}

void MainWindow::on_radioButtonCustom_toggled(bool checked)
{
    if (checked)
    {
        ui->stackedWidget->setCurrentWidget(ui->page1);
        scene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
    }
    else
        ui->stackedWidget->setCurrentWidget(ui->page0);
}

void MainWindow::on_lineEditWidth_editingFinished()
{
    scene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
}

void MainWindow::on_lineEditHeight_editingFinished()
{
    scene->changeAtlasSize(ui->lineEditWidth->text().toInt(), ui->lineEditHeight->text().toInt());
}

void MainWindow::on_spinBoxMargin_valueChanged(int arg1)
{
    QGraphicsSpriteItem::setMargin(arg1);
}

void MainWindow::on_spinBoxSnap_valueChanged(int arg1)
{
    QGraphicsSpriteItem::setSnapRadius(arg1);
}

void MainWindow::on_actionAdd_sprites_triggered()
{
    static QStringList fileNames;
    fileNames = QFileDialog::getOpenFileNames(this, tr("Add images"),
                                              fileNames.empty() ? "" :QFileInfo(fileNames.front()).absolutePath(),
                                              tr("Image Files (*.png *.jpg *.bmp)"));
    for (int i = 0; i < fileNames.size(); i++)
    {
        QString id = QFileInfo(fileNames[i]).baseName();
        QPixmap pixmap = QPixmap(fileNames[i]);
        ui->spritesListWidget->addSprite(pixmap, id);
    }
}

void MainWindow::on_actionMove_item_up_triggered()
{
    scene->changeActiveSpritePosition(0, -1);
}

void MainWindow::on_actionMove_item_down_triggered()
{
    scene->changeActiveSpritePosition(0, 1);
}

void MainWindow::on_actionMove_item_left_triggered()
{
    scene->changeActiveSpritePosition(-1, 0);
}

void MainWindow::on_actionMove_item_right_triggered()
{
    scene->changeActiveSpritePosition(1, 0);
}

void MainWindow::on_actionDelete_active_item_triggered()
{
    QGraphicsSpriteItem* active = scene->getActiveItem();
    if (!active)
        return;
    ui->spritesListWidget->addSprite(active->getPixmap(), active->getId());
    scene->eraseActiveItem();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    updateScene();
}

void MainWindow::updateScene()
{
    if (ui->tabWidget->count() == 0)
        return;

    if (scene && ui->tabWidget->count() > 1)
        disconnect(scene, SIGNAL(efficiencyChanged(double)), this, SLOT(efficiencyChanged(double)));
    scene = (QScene*)((QGraphicsView*)ui->tabWidget->currentWidget())->scene();
    connect(scene, SIGNAL(efficiencyChanged(double)), this, SLOT(efficiencyChanged(double)));

    QRectF r = ((QGraphicsView*)ui->tabWidget->currentWidget())->sceneRect();

    if (r.width() == 2049 && r.height() == 2049)
        ui->radioButton2048->setChecked(true);
    else if (r.width() == 1025 && r.height() == 1025)
        ui->radioButton1024->setChecked(true);
    else if (r.width() == 513 && r.height() == 513)
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
    QVector < QListWidgetItem* > items = ui->spritesListWidget->getItems();
    QVector < QRectF > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRectF(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRectF > conts = scene->getFreeSpace();

    Packing2D::CompareFunction cmp = Packing2D::compareByArea;
    Packing2D::Comparator comp = Packing2D::comparatorByArea;
    if (ui->comboBoxSorting->currentText() == "Height")
        cmp = Packing2D::compareByHeight, comp = Packing2D::comparatorByHeight;
    else if (ui->comboBoxSorting->currentText() == "Width")
        cmp = Packing2D::compareByWidth, comp = Packing2D::comparatorByHeight;

    if (ui->radioButtonMoveLeft->isChecked())
        comp = Packing2D::comparatorMoveLeft;
    if (ui->radioButtonMoveUp->isChecked())
        comp = Packing2D::comparatorMoveUp;

    QVector < QPair < bool,QPointF > > res = algo(rects, conts, scene->getAtlasBoundRect(), cmp, comp);

    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
        {
            scene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                             qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                             res[i].second);
            delete items[i];
        }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::ControlModifier && ui->tabWidget->currentWidget() != focusWidget())
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
    }
}

void MainWindow::efficiencyChanged(double e)
{
    ui->labelEfficiency->setText("<b>Efficiency = " + QString::number(e, 'f', 2) + "%</b>");

    QPixmap pix(128, 128);
    QPainter painter(&pix);
    painter.fillRect(0, 0, pix.width(), pix.height(), Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter);
    ui->labelMiniMap->setPixmap(pix);
}


void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    QVector < QGraphicsSpriteItem* > items = scenes[index]->getItems();
    for (int i = 0; i < items.size(); i++)
        ui->spritesListWidget->addSprite(items[i]->getPixmap(), items[i]->getId());

    ui->tabWidget->removeTab(index);
    delete scenes[index];   scenes.remove(index, 1);
    delete views[index];    views.remove(index, 1);

    if (ui->tabWidget->count() == 0)
        on_toolButtonAddTab_released();

    updateScene();
}

void MainWindow::on_actionSave_atlas_triggered()
{
    scene->save();
}

void MainWindow::on_toolButtonAddTab_released()
{
    scenes.push_back(new QScene);
    views.push_back(new QGraphicsView);

    views.back()->setScene(scenes.back());
    views.back()->centerOn(0, 0);
    scenes.back()->changeAtlasSize(1024, 1024);

    ui->tabWidget->insertTab(ui->tabWidget->count(), views.back(), "Atlas " + QString::number(ui->tabWidget->count()));
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}

void MainWindow::on_pushButton_2_released()
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

void MainWindow::on_toolButtonClearAtlas_released()
{
    QVector < QGraphicsSpriteItem* > items = scene->getItems();
    for (int i = 0; i < items.size(); i++)
        ui->spritesListWidget->addSprite(items[i]->getPixmap(), items[i]->getId());
    scene->clear();
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

void MainWindow::on_pushButtonAnnealing_released()
{
    packSprites(Packing2D::stupidAnnealing);
}

void MainWindow::on_pushButtonPackBest_released()
{
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    ui->progressBar->update();

    QVector < QListWidgetItem* > items = ui->spritesListWidget->getItems();
    QVector < QRectF > rects;
    for (int i = 0; i < items.size(); i++)
    {
        QPixmap pixmap = qVariantValue<QPixmap>(items[i]->data(Qt::UserRole));

        rects.push_back(QRectF(0, 0, pixmap.width() + QGraphicsSpriteItem::getMargin() * 2,
                                     pixmap.height() + QGraphicsSpriteItem::getMargin() * 2));
    }

    QVector < QRectF > conts = scene->getFreeSpace();

    const int nCmp = 3;
    Packing2D::CompareFunction cmpFunction[nCmp] = { Packing2D::compareByArea, Packing2D::compareByHeight, Packing2D::compareByWidth };
    const int nFunctions = 2;
    Packing2D::PackingFunction algo[nFunctions] = { Packing2D::bruteForcePacking, Packing2D::stupidGAPacking};//, Packing2D::stupidAnnealing };
    const int nComps = 5;
    Packing2D::Comparator comp[nComps] = { Packing2D::comparatorByArea, Packing2D::comparatorByHeight, Packing2D::comparatorByWidth,
                                           Packing2D::comparatorMoveLeft, Packing2D::comparatorMoveUp };
    const int nTimes = 2;

    ui->progressBar->setRange(0, nCmp * nFunctions * nComps * nTimes);
    ui->progressBar->update();

    QVector<QPair<bool, QPointF> > res;
    double bestEfficiency = -1.0;

    for (int t = 0; t < nTimes; t++)
        for (int cmp = 0; cmp < nCmp; cmp++)
            for (int c = 0; c < nComps; c++)
                for (int f = 0; f < nFunctions; f++)
                {
                    double e;
                    QVector<QPair<bool, QPointF> > tmp;
                    getResult(rects, conts, cmpFunction[cmp], comp[c], algo[f], tmp, e);
                    if (e > bestEfficiency)
                        bestEfficiency = e, res = tmp;
                    ui->progressBar->setValue(ui->progressBar->value() + 1);
                    ui->progressBar->update();
                    QCoreApplication::processEvents();
                }

    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
        {
            scene->addSprite(qVariantValue<QPixmap>(items[i]->data(Qt::UserRole)),
                             qVariantValue<QString>(items[i]->data(Qt::UserRole + 1)),
                             res[i].second);
            delete items[i];
        }

    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(false);
    ui->progressBar->update();
}

void MainWindow::getResult(QVector < QRectF >& rects, QVector < QRectF >& conts, Packing2D::CompareFunction cmp,
                           Packing2D::Comparator comp, Packing2D::PackingFunction algo, QVector<QPair<bool, QPointF> > &res, double &efficiency)
{
    res = algo(rects, conts, scene->getAtlasBoundRect(), cmp, comp);
    QVector < QRectF > r;
    for (int i = 0; i < res.size(); i++)
        if (res[i].first)
            r.push_back(QRectF(res[i].second, rects[i].size()));
    efficiency = Packing2D::calculateEfficiency(r, scene->getAtlasBoundRect(), false);
}
