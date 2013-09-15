#include "packingalgorithms.h"
#include <cmath>
#include <set>

Packing2D::Comparator Packing2D::ComparatorType::comp = 0;

bool Packing2D::compareByArea(const QPair < QRect,int >& a, const QPair < QRect,int >& b)
{
    return a.first.width() * a.first.height() > b.first.width() * b.first.height();
}

bool Packing2D::compareByHeight(const QPair < QRect,int >& a, const QPair < QRect,int >& b)
{
    return a.first.height() > b.first.height();
}

bool Packing2D::compareByWidth(const QPair < QRect,int >& a, const QPair < QRect,int >& b)
{
    return a.first.width() > b.first.width();
}


bool Packing2D::comparatorMoveLeft(const QRect& a, const QRect& b)
{
    if (a.topLeft().x() != b.topLeft().x())
        return a.topLeft().x() > b.topLeft().x();
    return a.topLeft().y() > b.topLeft().y();
}

bool Packing2D::comparatorMoveUp(const QRect& a, const QRect& b)
{
    if (a.topLeft().y() != b.topLeft().y())
        return a.topLeft().y() > b.topLeft().y();
    return a.topLeft().x() > b.topLeft().x();
}

bool Packing2D::comparatorByArea(const QRect& a, const QRect& b)
{
    return a.width() * a.height() > b.width() * b.height();
}

bool Packing2D::comparatorByHeight(const QRect& a, const QRect& b)
{
    return a.height() > b.height();
}

bool Packing2D::comparatorByWidth(const QRect& a, const QRect& b)
{
    return a.width() > b.width();
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

    //false - segment start,
    //true  - segment end
    QMap < int, QVector < QPair < QPair < int,bool >,bool > > > events;
    for (int i = 0; i < rects.size(); i++)
    {
        events[rects[i].top()].push_back(qMakePair(qMakePair(rects[i].left(), false), false));
        events[rects[i].top()].push_back(qMakePair(qMakePair(rects[i].right(), true), false));

        events[rects[i].bottom()].push_back(qMakePair(qMakePair(rects[i].left(), false), true));
        events[rects[i].bottom()].push_back(qMakePair(qMakePair(rects[i].right(), true), true));
    }

    QVector < QPair < int,bool > > curr;
    double lastY = -1.0;
    for (QMap < int, QVector < QPair < QPair < int,bool >,bool > > > :: iterator it = events.begin(); it != events.end(); it++)
    {
        qSort(curr);

        int opened = 0;
        int L;
        int length = 0;
        for (int i = 0; i < curr.size(); i++)
        {
            int j = i;
            while (j < curr.size() && curr[i].first == curr[j].first)
            {
                if (!curr[j].second)
                {
                    if (!opened)
                        L = curr[j].first;
                    opened++;
                }
                else
                    opened--;
                j++;
            }
            if (!opened)
                length += curr[i].first - L;
            i = j - 1;
        }

        if (lastY != -1.0)
            itemsArea += length * (it.key() - lastY);

        lastY = it.key();
        QVector < QPair < QPair < int,bool >,bool > >& v = it.value();

        for (int i = 0; i < v.size(); i++)
            if (!v[i].second)
                curr.push_back(v[i].first);
            else
            {
                for (int j = 0; j < curr.size(); j++)
                    if (curr[j] == v[i].first)
                    {
                        curr.remove(j, 1);
                        break;
                    }
            }
    }

    return itemsArea / containerArea * 100;
}


QVector < QPair < bool,QPoint > > Packing2D::_bruteForcePacking(QVector<QRect> &rects, QVector<QRect> &containers, bool useMask,
                                                                 QVector < bool >& mask, CompareFunction cmp, Comparator comp)
{
    QVector < QPair < bool,QPoint > > res(rects.size());
    for (int i = 0; i < res.size(); i++)
        res[i].first = false;

    QVector < QPair < QRect,int > > r;
    for (int i = 0; i < rects.size(); i++)
        r.push_back(qMakePair(rects[i], i));

    std::sort(r.begin(), r.end(), cmp);

    Packing2D::ComparatorType::comp = comp;
    std::priority_queue < QRect,std::vector < QRect >,Packing2D::ComparatorType > conts;

    for (int i = 0; i < containers.size(); i++)
        conts.push(containers[i]);

    std::set < int > indexes;
    for (int i = 0; i < rects.size(); i++)
        indexes.insert(i);

    QVector < int > added;

    int step = 0;
    while (!conts.empty())
    {
        QRect cont = conts.top();
        conts.pop();

        int move = -1;
        if (comp == Packing2D::comparatorMoveLeft)  move = 0;
        if (comp == Packing2D::comparatorMoveUp)    move = 1;
        if (move != -1)
        {
            int dx[] = { -1, 0 };
            int dy[] = { 0, -1 };
            int k = move;
            int L = 0, R = 128;
            while (L < R)
            {
                int m = (L + R + 1) / 2;
                bool good = cont.x() + m * dx[k] >= 0 && cont.y() + m * dy[k] >= 0;
                for (int j = 0; j < added.size(); j++)
                {
                    if (res[r[added[j]].second].first &&
                        QRect(res[r[added[j]].second].second, r[added[j]].first.size()).intersects(
                        QRect(cont.x() + m * dx[k], cont.y() + m * dy[k], cont.width() - m * dx[k], cont.height() - m * dy[k])))
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
            cont.setX(cont.x() + L * dx[k]);
            cont.setY(cont.y() + L * dy[k]);
        }

        //for (int i = 0; i < r.size(); i++)
        for (std::set < int > :: iterator it = indexes.begin(); it != indexes.end(); it++)
        {
            int i = *it;
            if (!res[r[i].second].first && r[i].first.width() <= cont.width() && r[i].first.height() <= cont.height())
            {
                res[r[i].second].first = true;
                res[r[i].second].second = cont.topLeft();
                added.push_back(i);

                QRect f1 = QRect(cont.topLeft() + QPoint(0, r[i].first.height()), QSize(cont.width(), cont.height() - r[i].first.height())),
                       f2 = QRect(cont.topLeft() + QPoint(r[i].first.width(), 0), QSize(cont.width() - r[i].first.width(), r[i].first.height()));
                QRect s1 = QRect(cont.topLeft() + QPoint(0, r[i].first.height()), QSize(r[i].first.width(), cont.height() - r[i].first.height())),
                       s2 = QRect(cont.topLeft() + QPoint(r[i].first.width(), 0), QSize(cont.width() - r[i].first.width(), cont.height()));

                QRect f = f1, s = s1;
                if (comp(f2, f1))      f = f2;
                if (comp(s2, s1))      s = s2;

                if ((useMask && mask[step]) ||
                    (!useMask && comp(f, s)))
                    conts.push(f1), conts.push(f2);
                else
                    conts.push(s1), conts.push(s2);

                step++;
                indexes.erase(it);
                break;
            }
        }
    }
    return res;
}

QVector < QPair < bool,QPoint > > Packing2D::bruteForcePacking(QVector < QRect > &rects, QVector < QRect >& containers, QRect mainContainer,
                                                                CompareFunction cmp, Comparator comp)
{
    QVector < bool > mask;
    return _bruteForcePacking(rects, containers, false, mask, cmp, comp);
}

QVector < QPair < bool,QPoint > > Packing2D::stupidGAPacking(QVector < QRect > &rects, QVector < QRect >& containers, QRect mainContainer,
                                                              CompareFunction cmp, Comparator comp)
{
    const int N = 50;
    const int NGENERATIONS = 50;

    QVector < bool > gen[2 * N];
    double e[N * 2];

    for (int i = 0; i < 2 * N; i++)
    {
        gen[i].resize(rects.size());
        for (int j = 0; j < gen[i].size(); j++)
            gen[i][j] = rand() % 2;
    }

    QVector < QPair < bool,QPoint > > res;
    double bestEfficiency = -1.0;

    for (int k = 0; k < NGENERATIONS; k++)
    {
        int n = N;
        for (int i = 0; i < N; i++)
        {
            int j = rand() % N;
            int p = rand() % rects.size();
            for (int t = 0; t < gen[n].size(); t++)
                gen[n][t] = t <= p ? gen[i][t] : gen[j][t];

            if ((rand() % N) % (N / 5) == 0)
                gen[n][rand() % gen[n].size()] ^= 1;

            QVector < QPair < bool,QPoint > > tmp = _bruteForcePacking(rects, containers, true, gen[n], cmp, comp);
            QVector < QRect > r;
            for (int j = 0; j < rects.size(); j++)
                if (tmp[j].first)
                    r.push_back(QRect(tmp[j].second, rects[j].size()));

            e[n] = calculateEfficiency(r, mainContainer, false);

            if (e[n] > bestEfficiency)
            {
                bestEfficiency = e[n];
                res = tmp;
            }

            n++;
        }

        for (int i = 0; i < 2 * N; i++)
        {
            int t = i;
            for (int j = i + 1; j < 2 * N; j++)
                if (e[t] < e[j])
                    t = j;
            std::swap(e[t], e[i]);
            std::swap(gen[t], gen[i]);
        }
    }

    return res;
}

bool Packing2D::ComparatorType::operator ()(const QRect &a, const QRect &b)
{
    return comp(a, b);
}
