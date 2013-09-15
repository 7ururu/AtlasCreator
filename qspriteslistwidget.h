#ifndef QSPRITESLISTWIDGET_H
#define QSPRITESLISTWIDGET_H

#include <QListWidget>
#include <QVector>

class QSpritesListWidget : public QListWidget
{
    Q_OBJECT
public:
    QSpritesListWidget(QWidget* parent = 0);

    void addItem(QPixmap pixmap, QString id);
    QVector < QListWidgetItem* > items() const;

protected:
    void startDrag(Qt::DropActions supportedActions);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void contextMenuEvent(QContextMenuEvent *event);

private:
    static const int ICONS_SIZE = 16;
    static const int ICONS_INDENT = 4;

private slots:
    void eraseActiveItems();
};

#endif // QSPRITESLISTWIDGET_H
