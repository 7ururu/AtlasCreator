#ifndef PACKINGALGORITHMS_H
#define PACKINGALGORITHMS_H

#include <QRect>
#include <QVector>
#include <QPair>
#include <QMap>
#include <algorithm>
#include <vector>
#include <queue>
#include <cmath>

namespace Packing2D
{

typedef bool (*CompareFunction)(const QPair < QRect,int >& a, const QPair < QRect,int >& b);
bool compareByArea(const QPair < QRect,int >& a, const QPair < QRect,int >& b);
bool compareByHeight(const QPair < QRect,int >& a, const QPair < QRect,int >& b);
bool compareByWidth(const QPair < QRect,int >& a, const QPair < QRect,int >& b);

typedef bool (*Comparator)(const QRect& a, const QRect& b);
bool comparatorMoveLeft(const QRect& a, const QRect& b);
bool comparatorMoveUp(const QRect& a, const QRect& b);
bool comparatorByArea(const QRect& a, const QRect& b);
bool comparatorByHeight(const QRect& a, const QRect& b);
bool comparatorByWidth(const QRect& a, const QRect& b);

struct ComparatorType
{
    bool operator()(const QRect& a, const QRect& b);
    static Packing2D::Comparator comp;
};


double calculateEfficiency(const QVector < QRect >& rects, QRect container, bool smart);


typedef QVector < QPair < bool,QPoint > >(*PackingFunction)(QVector < QRect > &rects, QVector < QRect >& containers, QRect, CompareFunction, Comparator);

QVector < QPair < bool,QPoint > > _bruteForcePacking(QVector<QRect> &rects, QVector<QRect> &containers, bool useMask,
                                                      QVector < bool >& mask, CompareFunction cmp, Comparator comp);
QVector < QPair < bool,QPoint > > bruteForcePacking(QVector < QRect > &rects, QVector < QRect >& containers, QRect mainContainer,
                                                     CompareFunction cmp, Comparator comp);
QVector < QPair < bool,QPoint > > stupidGAPacking(QVector < QRect > &rects, QVector < QRect >& containers, QRect mainContainer,
                                                   CompareFunction cmp, Comparator comp);

}

#endif // PACKINGALGORITHMS_H
