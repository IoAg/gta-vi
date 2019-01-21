// ArffWidgetCoord.cpp

#include "ArffWidgetCoord.h"
#include "../arffHelper/ArffUtil.h"
#include "Unused.h"

#include <QTime>
#include <cassert>

using namespace std;

// PUBLIC:

ArffWidgetCoord::ArffWidgetCoord(QWidget *parent) : ArffWidgetBase(parent), m_plotAttInd(0)
{
}


// PROTECTED:
void ArffWidgetCoord::PaintLine(QPainter *painter)
{
    QPoint points[m_intervalLength];
    QRect geom = geometry();

    painter->setPen(QPen(Qt::black, 3.0*geom.height()/1000.0));
    painter->translate(0, geom.height());
    painter->scale(1,-1);
    painter->setRenderHint(QPainter::Antialiasing, true);
    for (int i=0; i<m_intervalLength; i++)
    {
        double x = (double)geom.width()*((*m_pXYArff)[m_pointerStart+i][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        points[i].setX((int)x);
        points[i].setY((double)(*m_pXYArff)[m_pointerStart+i][m_plotAttInd]*(double)geom.height()/m_windowMaxVal);
        if (i>0)
            painter->drawLine(points[i], points[i-1]);
    }

    painter->translate(0, geom.height());
    painter->scale(1,-1);
    painter->setRenderHint(QPainter::Antialiasing, false);
}

// CLASS ArffWidgetCoordX

ArffWidgetCoordX::ArffWidgetCoordX(QWidget *parent) : ArffWidgetCoord(parent)
{
}

void ArffWidgetCoordX::SetData(Arff &arff, int attToPaint, double maxValue)
{
    ArffWidgetCoord::SetData(arff, attToPaint, maxValue);
    m_plotAttInd = m_xInd;
}

void ArffWidgetCoordX::SetFovData(Arff &arff, double maxValue)
{
    ArffWidgetCoord::SetFovData(arff, maxValue);
    m_plotAttInd = m_xInd;
}

// CLASS ArffWidgetCoordY

ArffWidgetCoordY::ArffWidgetCoordY(QWidget *parent) : ArffWidgetCoord(parent)
{
}

void ArffWidgetCoordY::SetData(Arff &arff, int attToPaint, double maxValue)
{
    ArffWidgetCoord::SetData(arff, attToPaint, maxValue);
    m_plotAttInd = m_yInd;
}

void ArffWidgetCoordY::SetFovData(Arff &arff, double maxValue)
{
    ArffWidgetCoord::SetFovData(arff, maxValue);
    m_plotAttInd = m_yInd;
}
