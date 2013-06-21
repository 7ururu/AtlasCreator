#ifndef PACKINGALGORITHMS_H
#define PACKINGALGORITHMS_H

#include <QRectF>
#include <QVector>
#include <QPair>
#include <QMap>
#include <algorithm>
#include <vector>
#include <queue>
#include <cmath>

namespace Packing2D
{

typedef bool (*CompareFunction)(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b);
bool compareByArea(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b);
bool compareByHeight(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b);
bool compareByWidth(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b);

typedef bool (*Comparator)(const QRectF& a, const QRectF& b);
bool comparatorMoveLeft(const QRectF& a, const QRectF& b);
bool comparatorMoveUp(const QRectF& a, const QRectF& b);
bool comparatorByArea(const QRectF& a, const QRectF& b);
bool comparatorByHeight(const QRectF& a, const QRectF& b);
bool comparatorByWidth(const QRectF& a, const QRectF& b);

struct ComparatorType
{
    bool operator()(const QRectF& a, const QRectF& b);
    static Packing2D::Comparator comp;
};


double calculateEfficiency(const QVector < QRectF >& rects, QRectF container, bool smart);


typedef QVector < QPair < bool,QPointF > >(*PackingFunction)(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF, CompareFunction, Comparator);

QVector < QPair < bool,QPointF > > _bruteForcePacking(QVector<QRectF> &rects, QVector<QRectF> &containers, bool useMask,
                                                      QVector < bool >& mask, CompareFunction cmp, Comparator comp);
QVector < QPair < bool,QPointF > > bruteForcePacking(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                     CompareFunction cmp, Comparator comp);
QVector < QPair < bool,QPointF > > stupidGAPacking(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                   CompareFunction cmp, Comparator comp);
QVector < QPair < bool,QPointF > > stupidAnnealing(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                   CompareFunction cmp, Comparator comp);

}

#endif // PACKINGALGORITHMS_H
