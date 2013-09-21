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

const QPoint NULL_POINT(-10000, -10000);

typedef bool (*RectsCompareFunction)(const QRect& a, const QRect& b);
bool rectsCompareByArea(const QRect& a, const QRect& b);
bool rectsCompareByHeight(const QRect& a, const QRect& b);
bool rectsCompareByWidth(const QRect& a, const QRect& b);
bool rectsCompareByMaxSide(const QRect& a, const QRect& b);

typedef bool (*ContsComparator)(const QRect& a, const QRect& b);
bool contsComparatorMoveLeft(const QRect& a, const QRect& b);
bool contsComparatorMoveUp(const QRect& a, const QRect& b);
bool contsComparatorByArea(const QRect& a, const QRect& b);
bool contsComparatorByHeight(const QRect& a, const QRect& b);
bool contsComparatorByWidth(const QRect& a, const QRect& b);
bool contsComparatorByMaxSide(const QRect& a, const QRect& b);

struct ContainersComparator
{
    bool operator()(const QRect& a, const QRect& b);
    static Packing2D::ContsComparator comp;
};


double calculateEfficiency(const QVector < QRect >& rects, QRect container, bool smart);


typedef QVector < QPoint >(*PackingFunction)(QVector < QRect > &rects, QVector < QRect >& containers, QRect, RectsCompareFunction, ContsComparator);

void _bruteForcePacking(QVector < QRect >& rects, QVector < QRect >& containers, bool useMask, QVector < bool >& mask,
                        RectsCompareFunction rectsCmp, ContsComparator contsComp, bool shouldSort, QVector < QPoint >& res);

QVector < QPoint > bruteForcePacking(QVector < QRect >& rects, QVector < QRect >& containers, QRect mainContainer,
                                     RectsCompareFunction rectsCmp, ContsComparator contsComp);
QVector < QPoint > stupidGAPacking(QVector < QRect >& rects, QVector < QRect >& containers, QRect mainContainer,
                                   RectsCompareFunction rectsCmp, ContsComparator contsComp);

}

#endif // PACKINGALGORITHMS_H
