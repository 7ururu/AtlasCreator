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
    
private slots:
    void on_radioButton2048_toggled(bool checked);
    void on_radioButton1024_toggled(bool checked);
    void on_radioButton512_toggled(bool checked);
    void on_radioButtonCustom_toggled(bool checked);

    void on_lineEditWidth_editingFinished();
    void on_lineEditHeight_editingFinished();

    void on_spinBoxMargin_valueChanged(int arg1);
    void on_spinBoxSnap_valueChanged(int arg1);

    void on_actionAdd_sprites_triggered();

    void on_actionMove_item_up_triggered();
    void on_actionMove_item_down_triggered();
    void on_actionMove_item_left_triggered();
    void on_actionMove_item_right_triggered();

    void on_actionDelete_active_item_triggered();

    void on_tabWidget_currentChanged(int index);

    void on_tabWidget_tabCloseRequested(int index);

    void on_actionSave_atlas_triggered();

    void on_toolButtonAddTab_released();

    void on_pushButtonBruteForce_released();

    void on_pushButton_2_released();

    void on_toolButtonClearAtlas_released();

    void on_pushButtonGA_released();

    void on_toolButtonClearList_released();

    void on_pushButtonAnnealing_released();

    void on_pushButtonPackBest_released();

private:
    Ui::MainWindow *ui;

    QVector < QScene* > scenes;
    QVector < QGraphicsView* > views;

    QScene* scene;

    void updateScene();
    void packSprites(Packing2D::PackingFunction algo);
    void getResult(QVector<QRectF> &rects, QVector<QRectF> &conts, Packing2D::CompareFunction cmp, Packing2D::Comparator comp,
                   Packing2D::PackingFunction algo, QVector < QPair < bool,QPointF > >& res, double& efficiency);

protected:
    void keyReleaseEvent(QKeyEvent *e);

public slots:
    void efficiencyChanged(double e);
};

#endif // MAINWINDOW_H
