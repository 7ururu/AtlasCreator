#include "packingalgorithms.h"
#include <cmath>
#include <set>

Packing2D::ContsComparator Packing2D::ContainersComparator::comp = 0;

bool Packing2D::rectsCompareByArea(const QRect& a, const QRect& b)
{
    return a.width() * a.height() > b.width() * b.height();
}

bool Packing2D::rectsCompareByHeight(const QRect& a, const QRect& b)
{
    return a.height() > b.height();
}

bool Packing2D::rectsCompareByWidth(const QRect& a, const QRect& b)
{
    return a.width() > b.width();
}

bool Packing2D::rectsCompareByMaxSide(const QRect& a, const QRect& b)
{
    return std::max(a.width(), a.height()) > std::max(b.width(), b.height());
}

bool Packing2D::contsComparatorMoveLeft(const QRect& a, const QRect& b)
{
    if (a.topLeft().x() != b.topLeft().x())
        return a.topLeft().x() > b.topLeft().x();
    return a.topLeft().y() > b.topLeft().y();
}

bool Packing2D::contsComparatorMoveUp(const QRect& a, const QRect& b)
{
    if (a.topLeft().y() != b.topLeft().y())
        return a.topLeft().y() > b.topLeft().y();
    return a.topLeft().x() > b.topLeft().x();
}

bool Packing2D::contsComparatorByArea(const QRect& a, const QRect& b)
{
    return a.width() * a.height() > b.width() * b.height();
}

bool Packing2D::contsComparatorByHeight(const QRect& a, const QRect& b)
{
    return a.height() > b.height();
}

bool Packing2D::contsComparatorByWidth(const QRect& a, const QRect& b)
{
    return a.width() > b.width();
}

bool Packing2D::contsComparatorByMaxSide(const QRect& a, const QRect& b)
{
    return std::max(a.width(), a.height()) > std::max(b.width(), b.height());
}

bool Packing2D::ContainersComparator::operator()(const QRect &a, const QRect &b)
{
    return comp(a, b);
}

double Packing2D::calculateEfficiency(const QVector<QRect> &rects, QRect container, bool smart)
{
    double containerArea = container.width() * container.height();
    double itemsArea     = 0.0;

    if (!smart)
    {
        for (int i = 0; i < rects.size(); i++)
            itemsArea += rects[i].width() * rects[i].height();
        return itemsArea / containerArea * 100;
    }

    //sweap line to find items area
    const bool START = false,
               END   = true;
    QMap < int, QVector < QPair < QPair < int,bool >,bool > > > events;
    for (int i = 0; i < rects.size(); i++)
    {
        events[rects[i].y()].push_back(qMakePair(qMakePair(rects[i].x(), START), START));
        events[rects[i].y()].push_back(qMakePair(qMakePair(rects[i].x() + rects[i].width(), END), START));

        events[rects[i].y() + rects[i].height()].push_back(qMakePair(qMakePair(rects[i].x(), START), END));
        events[rects[i].y() + rects[i].height()].push_back(qMakePair(qMakePair(rects[i].x() + rects[i].width(), END), END));
    }

    QVector < QPair < int,bool > > curr;
    int lastY = 0;
    for (QMap < int, QVector < QPair < QPair < int,bool >,bool > > > :: iterator it = events.begin(); it != events.end(); it++)
    {
        qSort(curr);

        int openedRects = 0;
        int firstStart = 0;
        int totalLength = 0;
        for (int i = 0; i < curr.size(); i++)
        {
            if (curr[i].second == START)
            {
                if (openedRects == 0)
                    firstStart = curr[i].first;
                openedRects++;
            }
            else if (curr[i].second == END)
            {
                openedRects--;
                if (openedRects == 0)
                    totalLength += curr[i].first - firstStart;
            }
        }

        if (it != events.begin())
            itemsArea += totalLength * (it.key() - lastY);
        lastY = it.key();

        QVector < QPair < QPair < int,bool >,bool > >& v = it.value();

        for (int i = 0; i < v.size(); i++)
            if (v[i].second == START)
                curr.push_back(v[i].first);
            else if (v[i].second == END)
            {
                for (int j = 0; j < curr.size(); j++)
                    if (curr[j] == v[i].first)
                    {
                        curr.erase(&curr[j]);
                        break;
                    }
            }
    }

    return itemsArea / containerArea * 100;
}


void Packing2D::_bruteForcePacking(QVector < QRect >& rects, QVector < QRect >& containers, bool useMask, QVector < bool >& mask,
                                   Packing2D::RectsCompareFunction rectsCmp, Packing2D::ContsComparator contsComp, bool shouldSort,
                                   QVector < QPoint >& res)
{
    if (shouldSort)
        std::sort(rects.begin(), rects.end(), rectsCmp);

    for (int i = 0; i < rects.size(); i++)
        res[i] = Packing2D::NULL_POINT;

    Packing2D::ContainersComparator::comp = contsComp;
    std::priority_queue < QRect,std::vector < QRect >,Packing2D::ContainersComparator > conts;
    for (int i = 0; i < containers.size(); i++)
        conts.push(containers[i]);

    std::set < int > indexes;
    for (int i = 0; i < rects.size(); i++)
        indexes.insert(i);
    QVector < int > added;

    while (!conts.empty())
    {
        QRect cont = conts.top();
        conts.pop();

        int move = -1;
        if (contsComp == Packing2D::contsComparatorMoveLeft)  move = 0;
        if (contsComp == Packing2D::contsComparatorMoveUp)    move = 1;
        if (move != -1)
        {
            int dx[] = { -1, 0 };
            int dy[] = { 0, -1 };

            int L = 0, R = 64;
            while (L < R)
            {
                int m = (L + R + 1) / 2;
                bool good = cont.x() + m * dx[move] >= 0 && cont.y() + m * dy[move] >= 0;
                for (int j = 0; j < added.size(); j++)
                {
                    if (QRect(res[added[j]], rects[added[j]].size()).intersects(
                        QRect(cont.x() + m * dx[move], cont.y() + m * dy[move], cont.width() - m * dx[move], cont.height() - m * dy[move])))
                    {
                        good = false;
                        break;
                    }
                }
                if (!good)
                    R = m - 1;
                else
                    L = m;
            }
            cont.setX(cont.x() + L * dx[move]);
            cont.setY(cont.y() + L * dy[move]);
        }

        for (std::set < int > :: iterator it = indexes.begin(); it != indexes.end(); it++)
        {
            int i = *it;
            if (rects[i].width() <= cont.width() && rects[i].height() <= cont.height())
            {
                res[i] = cont.topLeft();
                added.push_back(i);

                QRect f1 = QRect(cont.topLeft() + QPoint(0, rects[i].height()), QSize(cont.width(), cont.height() - rects[i].height())),
                      f2 = QRect(cont.topLeft() + QPoint(rects[i].width(), 0), QSize(cont.width() - rects[i].width(), rects[i].height()));
                QRect s1 = QRect(cont.topLeft() + QPoint(0, rects[i].height()), QSize(rects[i].width(), cont.height() - rects[i].height())),
                      s2 = QRect(cont.topLeft() + QPoint(rects[i].width(), 0), QSize(cont.width() - rects[i].width(), cont.height()));

                QRect f = f1, s = s1;
                if (contsComp(f2, f1))      f = f2;
                if (contsComp(s2, s1))      s = s2;

                if ((useMask && mask[i]) || (!useMask && contsComp(f, s)))
                    conts.push(f1), conts.push(f2);
                else
                    conts.push(s1), conts.push(s2);

                indexes.erase(it);
                break;
            }
        }
    }
}

QVector < QPoint > Packing2D::bruteForcePacking(QVector < QRect >& rects, QVector < QRect >& containers, QRect mainContainer,
                                                Packing2D::RectsCompareFunction rectsCmp, Packing2D::ContsComparator contsComp)
{
    QVector < QPoint > result(rects.size());
    QVector < bool > mask;
    _bruteForcePacking(rects, containers, false, mask, rectsCmp, contsComp, true, result);
    return result;
}

QVector < QPoint > Packing2D::stupidGAPacking(QVector < QRect >& rects, QVector < QRect >& containers, QRect mainContainer,
                                              Packing2D::RectsCompareFunction rectsCmp, Packing2D::ContsComparator contsComp)
{
    const int NGENS = 30;
    const int NGENERATIONS = 30;

    QVector < bool > gen[2 * NGENS];
    double e[NGENS * 2];

    for (int i = 0; i < 2 * NGENS; i++)
    {
        gen[i].resize(rects.size());
        for (int j = 0; j < gen[i].size(); j++)
            gen[i][j] = rand() % 2;
    }

    std::sort(rects.begin(), rects.end(), rectsCmp);

    QVector < QPoint > result;
    QVector < QPoint > tmpResult(rects.size());
    QVector < QRect > effRects(rects.size());
    double bestEfficiency = -1.0;

    for (int k = 0; k < NGENERATIONS; k++)
    {
        int currGen = NGENS;
        for (int i = 0; i < NGENS; i++)
        {
            int j = rand() % NGENS;
            int p = rand() % rects.size();
            for (int t = 0; t < gen[currGen].size(); t++)
            {
                gen[currGen][t] = t <= p ? gen[i][t] : gen[j][t];
                if (rand() % (NGENS * 2) == NGENS - 1)
                    gen[currGen][t] ^= 1;
            }

            _bruteForcePacking(rects, containers, true, gen[currGen], rectsCmp, contsComp, false, tmpResult);
            for (int j = 0; j < rects.size(); j++)
                if (tmpResult[j] == Packing2D::NULL_POINT)
                    effRects[j] = QRect(0, 0, 0, 0);
                else
                    effRects[j] = QRect(tmpResult[j], rects[j].size());

            e[currGen] = calculateEfficiency(effRects, mainContainer, false);

            if (e[currGen] > bestEfficiency)
            {
                bestEfficiency = e[currGen];
                result = tmpResult;
            }

            currGen++;
        }

        for (int i = 0; i < 2 * NGENS; i++)
        {
            int t = i;
            for (int j = i + 1; j < 2 * NGENS; j++)
                if (e[t] < e[j])
                    t = j;
            std::swap(e[t], e[i]);
            std::swap(gen[t], gen[i]);
        }
    }

    return result;
}
