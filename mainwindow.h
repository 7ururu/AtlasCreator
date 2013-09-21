#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QFileDialog>
#include <QVector>
#include <QKeyEvent>

#include "qgraphicsspriteitem.h"
#include "qscene.h"
#include "packingalgorithms.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QVector < QScene* > scenes;
    QVector < QGraphicsView* > views;

    QScene* currScene;

    void updateScene();
    void packSprites(Packing2D::PackingFunction algo);
    void packSpritesBestWay();
    void getResult(QVector<QRect> &rects, QVector<QRect> &conts, Packing2D::RectsCompareFunction rectsCmp, Packing2D::ContsComparator contsComp,
                   Packing2D::PackingFunction algo, QVector < QPoint >& res, double& efficiency);
    bool shouldContinuePacking();

protected:
    void keyReleaseEvent(QKeyEvent *e);

public slots:
    void onEfficiencyChanged(double e);
private slots:
    void on_actionAddSprites_triggered();
    void on_actionSaveAtlas_triggered();
    void on_actionDeleteActiveItem_triggered();
    void on_actionMoveItemUp_triggered();
    void on_actionMoveItemDown_triggered();
    void on_actionMoveItemLeft_triggered();
    void on_actionMoveItemRight_triggered();
    void on_actionAddFolder_triggered();
    void on_spinBoxMargin_valueChanged(int arg1);
    void on_spinBoxSnap_valueChanged(int arg1);
    void on_radioButton2048_toggled(bool checked);
    void on_radioButton1024_toggled(bool checked);
    void on_radioButton512_toggled(bool checked);
    void on_radioButtonCustom_toggled(bool checked);
    void on_pushButtonPackBestWay_released();
    void on_pushButtonBruteForce_released();
    void on_pushButtonGA_released();
    void on_toolButtonNewTab_released();
    void on_toolButtonClearList_released();
    void on_lineEditWidth_editingFinished();
    void on_lineEditHeight_editingFinished();
    void on_tabWidgetMainScene_currentChanged(int index);
    void on_tabWidgetMainScene_tabCloseRequested(int index);
    void on_pushButtonGenerateSprites_released();
    void on_toolButtonClearScene_released();
};

#endif // MAINWINDOW_H
