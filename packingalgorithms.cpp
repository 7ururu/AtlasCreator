#include "packingalgorithms.h"
#include <cmath>

Packing2D::Comparator Packing2D::ComparatorType::comp = 0;

bool Packing2D::compareByArea(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b)
{
    return a.first.width() * a.first.height() > b.first.width() * b.first.height();
}

bool Packing2D::compareByHeight(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b)
{
    return a.first.height() > b.first.height();
}

bool Packing2D::compareByWidth(const QPair < QRectF,int >& a, const QPair < QRectF,int >& b)
{
    return a.first.width() > b.first.width();
}


bool Packing2D::comparatorMoveLeft(const QRectF& a, const QRectF& b)
{
    if (a.topLeft().x() != b.topLeft().x())
        return a.topLeft().x() > b.topLeft().x();
    return a.topLeft().y() > b.topLeft().y();
}

bool Packing2D::comparatorMoveUp(const QRectF& a, const QRectF& b)
{
    if (a.topLeft().y() != b.topLeft().y())
        return a.topLeft().y() > b.topLeft().y();
    return a.topLeft().x() > b.topLeft().x();
}

bool Packing2D::comparatorByArea(const QRectF& a, const QRectF& b)
{
    return a.width() * a.height() > b.width() * b.height();
}

bool Packing2D::comparatorByHeight(const QRectF& a, const QRectF& b)
{
    return a.height() > b.height();
}

bool Packing2D::comparatorByWidth(const QRectF& a, const QRectF& b)
{
    return a.width() > b.width();
}

double Packing2D::calculateEfficiency(const QVector<QRectF> &rects, QRectF container, bool smart)
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
    QMap < double, QVector < QPair < QPair < double,bool >,bool > > > events;
    for (int i = 0; i < rects.size(); i++)
    {
        events[rects[i].top()].push_back(qMakePair(qMakePair(rects[i].left(), false), false));
        events[rects[i].top()].push_back(qMakePair(qMakePair(rects[i].right(), true), false));

        events[rects[i].bottom()].push_back(qMakePair(qMakePair(rects[i].left(), false), true));
        events[rects[i].bottom()].push_back(qMakePair(qMakePair(rects[i].right(), true), true));
    }

    QVector < QPair < double,bool > > curr;
    double lastY = -1.0;
    for (QMap < double, QVector < QPair < QPair < double,bool >,bool > > > :: iterator it = events.begin(); it != events.end(); it++)
    {
        qSort(curr);

        int opened = 0;
        double L;
        double length = 0.0;
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
        QVector < QPair < QPair < double,bool >,bool > >& v = it.value();

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


QVector < QPair < bool,QPointF > > Packing2D::_bruteForcePacking(QVector<QRectF> &rects, QVector<QRectF> &containers, bool useMask,
                                                                 QVector < bool >& mask, CompareFunction cmp, Comparator comp)
{
    QVector < QPair < bool,QPointF > > res(rects.size());
    for (int i = 0; i < res.size(); i++)
        res[i].first = false;

    QVector < QPair < QRectF,int > > r;
    for (int i = 0; i < rects.size(); i++)
        r.push_back(qMakePair(rects[i], i));

    std::sort(r.begin(), r.end(), cmp);

    Packing2D::ComparatorType::comp = comp;
    std::priority_queue < QRectF,std::vector < QRectF >,Packing2D::ComparatorType > conts;

    for (int i = 0; i < containers.size(); i++)
        conts.push(containers[i]);

    QVector < int > added;

    int step = 0;
    while (!conts.empty())
    {
        QRectF cont = conts.top();
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
                        QRectF(res[r[added[j]].second].second, r[added[j]].first.size()).intersects(
                        QRectF(cont.x() + m * dx[k], cont.y() + m * dy[k], cont.width() - m * dx[k], cont.height() - m * dy[k])))
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

        for (int i = 0; i < r.size(); i++)
            if (!res[r[i].second].first && r[i].first.width() <= cont.width() && r[i].first.height() <= cont.height())
            {
                res[r[i].second].first = true;
                res[r[i].second].second = cont.topLeft();
                added.push_back(i);

                QRectF f1 = QRectF(cont.topLeft() + QPointF(0, r[i].first.height()), QSizeF(cont.width(), cont.height() - r[i].first.height())),
                       f2 = QRectF(cont.topLeft() + QPointF(r[i].first.width(), 0), QSizeF(cont.width() - r[i].first.width(), r[i].first.height()));
                QRectF s1 = QRectF(cont.topLeft() + QPointF(0, r[i].first.height()), QSizeF(r[i].first.width(), cont.height() - r[i].first.height())),
                       s2 = QRectF(cont.topLeft() + QPointF(r[i].first.width(), 0), QSizeF(cont.width() - r[i].first.width(), cont.height()));

                QRectF f = f1, s = s1;
                if (comp(f2, f1))      f = f2;
                if (comp(s2, s1))      s = s2;

                if ((useMask && mask[step]) ||
                    (!useMask && comp(f, s)))
                    conts.push(f1), conts.push(f2);
                else
                    conts.push(s1), conts.push(s2);

                step++;
                break;
            }
    }
    return res;
}

QVector < QPair < bool,QPointF > > Packing2D::bruteForcePacking(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                                CompareFunction cmp, Comparator comp)
{
    QVector < bool > mask;
    return _bruteForcePacking(rects, containers, false, mask, cmp, comp);
}

QVector < QPair < bool,QPointF > > Packing2D::stupidGAPacking(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                              CompareFunction cmp, Comparator comp)
{
    const int N = 100;
    const int NGENERATIONS = 100;

    QVector < bool > gen[2 * N];
    double e[N * 2];

    for (int i = 0; i < 2 * N; i++)
    {
        gen[i].resize(rects.size());
        for (int j = 0; j < gen[i].size(); j++)
            gen[i][j] = rand() % 2;
    }

    QVector < QPair < bool,QPointF > > res;
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

            QVector < QPair < bool,QPointF > > tmp = _bruteForcePacking(rects, containers, true, gen[n], cmp, comp);
            QVector < QRectF > r;
            for (int j = 0; j < rects.size(); j++)
                if (tmp[j].first)
                    r.push_back(QRectF(tmp[j].second, rects[j].size()));

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

QVector < QPair < bool,QPointF > > Packing2D::stupidAnnealing(QVector < QRectF > &rects, QVector < QRectF >& containers, QRectF mainContainer,
                                                              CompareFunction cmp, Comparator comp)
{
    QVector < QPair < bool,QPointF > > res;
    double bestEfficiency = -1.0;

    double temperature = 10000.0;
    double coolingRate = 0.002;

    QVector < bool > current(rects.size()),
                     neighboor(rects.size());
    for (int i = 0; i < current.size(); i++)
        current[i] = rand() % 2;

    struct Propability
    {
        double calc(double curr, double neig, double temperature)
        {
            if (curr < neig)
                return 1.0;
            return std::exp((curr - neig) / temperature);
        }
    };
    Propability propability;

    while (temperature > 1.0)
    {
        for (int i = 0; i < neighboor.size(); i++)
        {
            neighboor[i] = current[i];
            if (rand() % 2)
                neighboor[i] ^= 1;
        }

        QVector < QPair < bool,QPointF > > currF = Packing2D::_bruteForcePacking(rects, containers, true, current, cmp, comp),
                                           neigF = Packing2D::_bruteForcePacking(rects, containers, true, neighboor, cmp, comp);
        QVector < QRectF > currR, neigR;
        for (int j = 0; j < rects.size(); j++)
        {
            if (currF[j].first)
                currR.push_back(QRectF(currF[j].second, rects[j].size()));
            if (neigF[j].first)
                neigR.push_back(QRectF(neigF[j].second, rects[j].size()));
        }

        double currE = Packing2D::calculateEfficiency(currR, mainContainer, false),
               neigE = Packing2D::calculateEfficiency(neigR, mainContainer, false);

        if (currE > bestEfficiency)
            bestEfficiency = currE, res = currF;
        if (neigE > bestEfficiency)
            bestEfficiency = neigE, res = neigF;

        if (propability.calc(currE, neigE, temperature) >= (double)rand() / 32767)
            current = neighboor;

        temperature *= 1.0 - coolingRate;
    }

    return res;
}

bool Packing2D::ComparatorType::operator ()(const QRectF &a, const QRectF &b)
{
    return comp(a, b);
}
