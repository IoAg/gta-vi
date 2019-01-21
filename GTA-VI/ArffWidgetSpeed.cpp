// ArffWidgetSpeed.cpp

#include "ArffWidgetSpeed.h"
#include "EquirectangularToFovSpeed.cpp"
#include "GazeSpeed.h"

#include <algorithm>
#include <iostream>

#define RELATION360 "GAZE_360"
#define MAX_SPEED 200.0
#define STEP 5

using namespace std;

// PUBLIC SLOTS:

void ArffWidgetSpeed::HandleToggleView()
{
    ArffWidgetBase::HandleToggleView();
    
    if (m_vSpeed.size() != m_vSecondSpeed.size() || !m_FovDisplayed)
        return;

    if (m_pSpeed == &m_vSpeed)
        m_pSpeed = &m_vSecondSpeed;
    else
        m_pSpeed = &m_vSpeed;

    update();
}

// PUBLIC:
ArffWidgetSpeed::ArffWidgetSpeed(QWidget *parent) : ArffWidgetBase(parent), m_pSpeed(&m_vSpeed), m_FovDisplayed(false)
{
}

// PROTECTED:
void ArffWidgetSpeed::SetData(Arff &arff, int attToPaint, double maxValue)
{
    ArffWidgetBase::SetData(arff, attToPaint, maxValue);

    if (IsArff360())
    {
        EquirectangularToFovSpeed eqToSpeed(m_pArff, STEP);
        // Always get head+eye speed because the provided ARFF is converted to 
        // hold the data to display in x,y
        m_vSpeed = eqToSpeed.GetHeadPlusEyeSpeed(); 
        NormalizeSpeed(m_vSpeed);
        m_vSecondSpeed = eqToSpeed.GetEyeFovSpeed();
        NormalizeSpeed(m_vSecondSpeed);
        m_vHeadSpeed = eqToSpeed.GetHeadSpeed();
        NormalizeSpeed(m_vHeadSpeed);
    }
    else
    {
        GazeSpeed gazeSpeed(m_pArff, STEP);
        m_vSpeed = gazeSpeed.GetSpeed();
        NormalizeSpeed(m_vSpeed);
    }
}

void ArffWidgetSpeed::SetFovData(Arff &arff, double maxValue)
{
    ArffWidgetBase::SetFovData(arff, maxValue);
    DisplayFov();
}

void ArffWidgetSpeed::DisplayFov()
{
    m_FovDisplayed = true;
}

void ArffWidgetSpeed::DisplayHead()
{
    EquirectangularToFovSpeed eqToSpeed(m_pArff, STEP);
    m_vSpeed = eqToSpeed.GetHeadSpeed();
    NormalizeSpeed(m_vSpeed);
    m_vHeadSpeed.clear();
}

// PROTECTED:

void ArffWidgetSpeed::PaintLine(QPainter *painter)
{
    QRect geom = geometry();

    painter->setPen(QPen(Qt::black, 4.0*geom.height()/1000.0));
    PaintSpeed(*m_pSpeed, painter);
    
    painter->setPen(QPen(Qt::darkRed, 4.0*geom.height()/1000.0));
    PaintSpeed(m_vHeadSpeed, painter);

    PaintSpeedLines(painter);
}

// PRIVATE:

void ArffWidgetSpeed::PaintSpeed(const vector<double> &vSpeed, QPainter *painter)
{
    if (vSpeed.empty())
        return;

    QRect geom = geometry();
    QPoint points[m_intervalLength];
    
    for (int i=0; i<m_intervalLength; i++)
    {
        //double x = (double)geom.width()*((*m_pEqArff)[m_pointerStart+i][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        double x = (double)geom.width()*((*m_pArff)[m_pointerStart+i][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        points[i].setX((int)x);
        points[i].setY((double)vSpeed[m_pointerStart+i]*(double)geom.height()/m_windowMaxVal);
    }
    
    painter->translate(0, geom.height());
    painter->scale(1,-1); 
    painter->drawPolyline(points,m_intervalLength);
    painter->translate(0, geom.height());
    painter->scale(1,-1); 
}

void ArffWidgetSpeed::PaintSpeedLines(QPainter *painter)
{
    vector<double> percentage {0.025, 0.05, 0.1, 0.2, 0.4, 0.7, 1};

    QRect geom = geometry();
	double boxHeight = 0.02*geom.height();
    QPoint points[2];
    for (auto perc:percentage)
    {
        // draw speed line
        painter->setPen(QPen(Qt::darkCyan, 2.0*geom.height()/1000.0));
        points[0].setX(0);
        points[1].setX(geom.width());
    
        points[0].setY((double)geom.height() * (1 - TransformPercentage(perc)));
        points[1].setY(points[0].y());

        painter->drawPolyline(points, 2);

        // draw text
        points[0].setX(geom.width() - 6 * boxHeight);
        points[0].setY((double)geom.height() * (1 - TransformPercentage(perc)));
        points[1].setX(geom.width());
        points[1].setY((double)geom.height() * (1 - TransformPercentage(perc)) + boxHeight);
        QRect rect(points[0], points[1]);
        painter->setBrush(QBrush(QColor(255,255,255,255)));
        painter->drawRect(rect);

        QString label = QString::number(int(perc*MAX_SPEED)) + QString(" deg/sec");
		// scale font size
		QFont font = painter->font();
		font.setPointSize(boxHeight/1.5);
		painter->setFont(font);
		painter->setPen(QPen(Qt::black));

        painter->drawText(rect, Qt::AlignCenter, label);

    }
}

bool ArffWidgetSpeed::IsArff360()
{
    string relation = m_pArff->GetRelation();
    transform(relation.begin(), relation.end(), relation.begin(), ::toupper);

    return relation == RELATION360;
}

void ArffWidgetSpeed::NormalizeSpeed(vector<double> &vSpeed)
{
    for (double &value:vSpeed)
        value = value>MAX_SPEED?MAX_SPEED:value;
    m_windowMaxVal = MAX_SPEED;
    
    for (double &value:vSpeed)
        value = TransformPercentage(value / MAX_SPEED) * MAX_SPEED;
}

double ArffWidgetSpeed::TransformPercentage(double perc)
{
    // Use exponential decay to spread percentages at the lower end. The smaller
    // the exponential multiplier the bigger the spread in the lower values
    double expMult = -2.5;
    double maxValMult = exp(expMult);
    double newPerc = (1 - exp(expMult*perc)) / (1 - maxValMult);

    return newPerc;
}
