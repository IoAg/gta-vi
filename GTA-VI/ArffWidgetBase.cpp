// ArffWidgetBase.cpp

#include "ArffWidgetBase.h"
#include "../arffHelper/ArffUtil.h"
#include "Unused.h"

#include <QTime>
#include <cassert>
#include <iostream>

#define MIN_WINDOW_DUR 100000
#define MAX_WINDOW_DUR 90000000
#define LOW_ALPHA 150
#define DEFAULT_ALPHA 180
#define HIGH_ALPHA 200

using namespace std;
using namespace std::chrono;

// PUBLIC SLOTS:

void ArffWidgetBase::HandleWindowDur(int dur_us, QObject *pSender)
{
    if (pSender == this)
        return;

    blockSignals(true);

    SetWindowDuration(dur_us);

    blockSignals(false);
}

void ArffWidgetBase::HandleTime(int curTime_us, QObject *pSender)
{
    if (pSender == this)
        return;

    blockSignals(true);

    SetTime(curTime_us);

    blockSignals(false);
}

void ArffWidgetBase::HandleUpdate()
{
    Update();
}

void ArffWidgetBase::HandleSelectedEyeMovement(int eyeMovement)
{
    m_selectedEyeMovement = eyeMovement;
    if (m_IsPressedLeft)
    {
        ChangeInterval(m_selectedEyeMovement);
        emit SendUpdate();
    }
}

/*virtual*/ void ArffWidgetBase::HandleToggleView()
{
    if (m_pXYArff && m_pXYSecArff)
    {
        swap(m_pXYArff, m_pXYSecArff);
        swap(m_windowMaxVal, m_windowSecMaxVal);
            
        int confInd;
        bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, m_xInd, m_yInd, confInd);
        UNUSED(res);

        update();
    }
}


// PUBLIC:

ArffWidgetBase::ArffWidgetBase(QWidget *) : m_pArff(0), m_pXYArff(0), m_pXYSecArff(0), m_intervalAtt(-1), m_LeftPressPosition(-1), m_prevDragPosX(-1)
{
    InitializeLegend();
    m_windowDuration = 10000000; // 10s duration
    m_gridTickX = m_windowDuration/50; // 200ms tick
    m_gridTickY = 100; // 100 points of whatever it is(pixels, pixels/s, ...)
    m_windowMaxVal = 1.0;
    m_winStartTime = 0;
    m_pointerStart = 0;
    m_pointerEnd = 0;
    m_intervalLength = 0;
    m_attToPaint = 0;
    m_selectedEyeMovement = 0;
    m_IsPressedLeft = false;

    ClearEventVariables();
    setFocusPolicy(Qt::ClickFocus); // set focus policy by clicking

    setAttribute(Qt::WA_NoSystemBackground);
}

void ArffWidgetBase::SetGridInterval(int tickX, int tickY)
{
    m_gridTickX = tickX;
    m_gridTickY = tickY;
}

void ArffWidgetBase::SetWindowDuration(int duration)
{
    // return for very small or big durations
    if (duration < MIN_WINDOW_DUR || duration > MAX_WINDOW_DUR)
        return;
    m_windowDuration = duration;

    Update();
}

void ArffWidgetBase::SetTime(int currentTime)
{
    m_currentTime = currentTime;
    m_winStartTime = currentTime - m_windowDuration/2;

    Update();
}

void ArffWidgetBase::SetData(Arff &arff, int attToPaint, double maxValue)
{
    m_pArff = &arff;
    m_pXYArff = &arff;
    int confInd;
    bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, m_xInd, m_yInd, confInd);
    if (res)
        InitializeLegend(attToPaint);
    
    m_pointerStart = 0;
    m_pointerEnd = 0;
    m_intervalLength = 0;
    m_windowMaxVal = maxValue;
    if (maxValue < 0)
        m_windowMaxVal = geometry().height();
    m_attToPaint = attToPaint;

    SetTime(0);
}

void ArffWidgetBase::SetFovData(Arff &arff, double maxValue)
{
    m_pXYSecArff = &arff;
    m_windowSecMaxVal = maxValue;
}

void ArffWidgetBase::SetIntervalAtt(int intervalAtt)
{
    m_intervalAtt = intervalAtt;
}

QSize ArffWidgetBase::minimumSizeHint() const
{
    return QSize(160, 90);
}

void ArffWidgetBase::Update()
{
    m_gridTickX = m_windowDuration/50; 
    // reset counters only when data have been set
    m_pointerStart = 0;
    m_pointerEnd = 0;
    m_intervalLength = 1;
    m_DoubleLeftPressPos = -1;

    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows == 0)
        return;

    m_pointerEnd = ArffUtil::FindPosition(m_pArff, m_timeInd, m_winStartTime + m_windowDuration);
    m_pointerStart = ArffUtil::FindPosition(m_pArff, m_timeInd, m_winStartTime);
    m_intervalLength = m_pointerEnd - m_pointerStart + 1;

    // bring intervals up-to-date
    CalculateIntervals();

    update();
}

void ArffWidgetBase::MoveToStart()
{
    if (m_pArff == NULL)
        return;
    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows == 0)
        return;

    int startingTime = (*m_pArff)[0][m_timeInd];

    emit SendTime(startingTime);

    SetTime(startingTime);
}

// PROTECTED:
void ArffWidgetBase::paintEvent(QPaintEvent *)
{   
    int rows, columns;
    m_pArff->Size(rows, columns);
    // if no data points exist 
    if (rows == 0)
        return;
    
    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing, true);
    
    PaintBase(&painter);
    //PaintLine(&painter); 
}

void ArffWidgetBase::PaintBase(QPainter *painter)
{
    int rows, columns;
    m_pArff->Size(rows, columns);
    // if no data points exist 
    if (rows == 0)
        return;

    PaintAreas(painter);
    PaintGrid(painter); 
    PaintVideoTime(painter);
    PaintText(painter);
    PaintLine(painter);
    PaintLegend(painter);
}

void ArffWidgetBase::PaintAreas(QPainter *painter)
{
    QRect geom = geometry();
    int attValue;

    // draw all intervals
    for (int i=0; i<(int)m_vIntervals.size()-1; i++)
    {
        double start_width = (double)geom.width()*((*m_pArff)[m_pointerStart+m_vIntervals[i]][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        double end_width = (double)geom.width()*((*m_pArff)[m_pointerStart+m_vIntervals[i+1]-1][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        double length_width = end_width-start_width;

        attValue = (*m_pArff)[m_pointerStart+m_vIntervals[i]][m_attToPaint];
        attValue = attValue >= m_colorNum?0:attValue;
        // check if clicked interval
        if (m_IsPressedLeft && m_LeftPressPosition >= start_width && m_LeftPressPosition < end_width)
        {
            painter->setBrush(QBrush(QColor(m_vLegendColors[attValue][0],m_vLegendColors[attValue][1],m_vLegendColors[attValue][2],HIGH_ALPHA)));
        }
        else
        {
            painter->setBrush(QBrush(QColor(m_vLegendColors[attValue][0],m_vLegendColors[attValue][1],m_vLegendColors[attValue][2],LOW_ALPHA)));
        }

        // paint rectangle
        painter->drawRect(QRect(start_width,0, length_width, geom.height()));
    
    }
}

void ArffWidgetBase::PaintLine(QPainter *painter)
{
    UNUSED(*painter);
}

void ArffWidgetBase::PaintVideoTime(QPainter *painter)
{
    QRect geom = geometry();
    QPoint points[2];
    painter->setPen(QPen(Qt::black, 8.0*geom.height()/1000.0));

    double x = (double)geom.width()*(m_currentTime - m_winStartTime)/(double)m_windowDuration;
    points[0].setX((int)x);
    points[1].setX((int)x);

    // draw upper part
    points[0].setY(0);
    points[1].setY(geom.height()/8);
    painter->drawPolyline(points, 2);

    // draw lower part
    points[0].setY(geom.height());
    points[1].setY(geom.height()*0.875);
    painter->drawPolyline(points, 2);
}

void ArffWidgetBase::PaintGrid(QPainter *painter)
{
    QRect geom = geometry();
    long int rest;
    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows > 0)
        rest = (m_winStartTime-(int)(*m_pArff)[0][m_timeInd]) % m_gridTickX;
    else
        rest = 0;

    long int gridTime = m_gridTickX - rest;
    QPoint points[2];
    painter->setPen(QPen(Qt::darkGray, 2.0*geom.height()/1000.0, Qt::DashLine));
    while (gridTime < m_windowDuration)
    {
        // calculate points
        points[0].setX((double)geom.width()*(double)gridTime/(double)m_windowDuration);
        points[1].setX(points[0].x());

        points[0].setY(0);
        points[1].setY(geom.height());

        // draw line
        painter->drawPolyline(points, 2);
        
        // update time position
        gridTime += m_gridTickX;
    }
}

void ArffWidgetBase::PaintText(QPainter *painter)
{
    QRect geom = geometry();
    // paint window start time
    painter->setPen(QPen(Qt::black));
    int precision;
    if (m_winStartTime < 1000000)
        precision=1;
    else if (m_winStartTime < 10000000)
        precision = 2;
    else
        precision=3;
    painter->drawText(0,geom.height()-5, QString::number((double)m_winStartTime/(double)1000000, 'g', precision));

    // paint current time
    const QLocale & cLocale = QLocale::c();
    QString sTime = cLocale.toString(m_winStartTime+m_windowDuration/2);
    sTime.replace(cLocale.groupSeparator(), ",");
    painter->drawText(geom.width()/2+30,geom.height()-5, sTime + QString(" us"));

    // paint window end time
    if (m_winStartTime+m_windowDuration < 1000000)
        precision=1;
    else if (m_winStartTime+m_windowDuration < 10000000)
        precision = 2;
    else
        precision=3;
    painter->drawText(geom.width() - 35 - 5*precision,geom.height()-5, QString::number((double)(m_winStartTime+m_windowDuration)/(double)1000000, 'g', precision));
}

void ArffWidgetBase::PaintLegend(QPainter *painter)
{
    QRect geom = geometry();
    int width = geom.width();
    int height = geom.height();

    QWidget *pParent = nativeParentWidget();
    if (pParent)
    {
        QRect parGeom = pParent->geometry();
        width = parGeom.width();
        height = parGeom.height();
    }
    
    int legendHeight = height/60;
    int legendWidth = width/20;

    // scale font size
    QFont font = painter->font();
    font.setPointSize(legendHeight/1.5);
    painter->setFont(font);

    painter->setPen(QPen(Qt::black));
    for (int i=0; i<m_colorNum; i++)
    {
        // paint white background
        painter->setBrush(QBrush(QColor(255,255,255,255)));
        painter->drawRect(QRect(0,i*legendHeight, legendWidth, legendHeight));
        // paint attribute color
        painter->setBrush(QBrush(QColor(m_vLegendColors[i][0],m_vLegendColors[i][1],m_vLegendColors[i][2],DEFAULT_ALPHA)));
        painter->drawRect(QRect(0,i*legendHeight, legendWidth, legendHeight));

        // paint text
        QString label = QString::number(i) + QString(" ") + QString::fromStdString(m_vLegendCaption[i]);
        painter->drawText(QRect(0,i*legendHeight, legendWidth, legendHeight), 0, label);
    }
}

void ArffWidgetBase::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_IsPressedLeft = true;
        m_LeftPressPosition = event->x();
        //setCursor(Qt::SplitHCursor);

        update();

    }
    else if (event->button() == Qt::RightButton)
    {
        setCursor(Qt::ClosedHandCursor);
        m_RightPressPosition = event->x();
    }
    else // call default otherwise
        QWidget::mousePressEvent(event);
}

void ArffWidgetBase::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_DoubleLeftPressPos = event->x();
        InsertInterval();
    }
    else
        QWidget::mouseDoubleClickEvent(event);
}

void ArffWidgetBase::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::ArrowCursor);

        m_IsPressedLeft = false;
        m_prevDragPosX = -1;
        update(); // only repaint
    }
    else if (event->button() == Qt::RightButton)
    {
        setCursor(Qt::ArrowCursor);
        m_RightPressPosition = -1;
    }
    else // call default otherwise
        QWidget::mousePressEvent(event);
}

void ArffWidgetBase::mouseMoveEvent(QMouseEvent *event)
{
    if (m_IsPressedLeft)
    {
        if (m_attToPaint < 0 || m_attToPaint > (int)(*m_pArff)[m_pointerStart].size()-1)
            return;

        if (m_prevDragPosX < 0)
        {
            m_prevDragPosX = event->x();
            return;
        }

        // if any other element is assigned a different label
        // we update the intevals and keep the interval selection active

        // find selected interval limits
        QRect geom = geometry();
        double start_width = 0, end_width;
        int intPointer=-1;
        for (int i=0; i<(int)m_vIntervals.size()-1; i++)
        {
            end_width = (double)geom.width()*((*m_pArff)[m_pointerStart+m_vIntervals[i+1]-1][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);

            if (m_LeftPressPosition>start_width && m_LeftPressPosition<end_width)
            {
                intPointer = i;
                break;
            }

            start_width = end_width;
        }

        // not within interval, update left press position
        if (intPointer == -1)
        {
            m_LeftPressPosition = event->x();
            intPointer = m_vIntervals.size()-1;
            return;
        }

        double distLeft = event->x() - start_width;
        double distRight = end_width - event->x();
        bool changeLeft = false;
        double maxBorderDist = 0.05 * (double)geom.width();
        if (distLeft > maxBorderDist && distRight > maxBorderDist)
            return;
        else if (distLeft < distRight)
            changeLeft = true;

        setCursor(Qt::SplitHCursor);

        double dir = event->x() - m_prevDragPosX;
        bool moveLeft = false;
        if (dir < 0)
            moveLeft = true;

        if (changeLeft && moveLeft)
        {
            ExpandIntLeft(intPointer, event->x());
        }
        else if (changeLeft && !moveLeft)
        {
            ExpandIntRight(intPointer, event->x());
        }
        else if (!changeLeft && moveLeft)
        {
            intPointer++;
            ExpandIntLeft(intPointer, event->x());
        }
        else if (!changeLeft && !moveLeft)
        {
            intPointer++;
            ExpandIntRight(intPointer, event->x());
        }

        m_prevDragPosX = event->x();

        emit SendUpdate();
    }
    else if (m_RightPressPosition != -1)
    {
        QRect geom = geometry();
        double displacement = (double)(event->x()-m_RightPressPosition)/(double)(geom.width());
        displacement *= (double)m_windowDuration;

        m_RightPressPosition = event->x();

        int currentTime = m_currentTime - displacement;
        emit SendTime(currentTime);
        
        SetTime(currentTime);
    }
    else
        QWidget::mouseMoveEvent(event);
}

void ArffWidgetBase::ExpandIntLeft(int intPointer, double newLimit)
{
    if (intPointer < 0 || intPointer >= (int)m_vIntervals.size())
        return;

    int attValue = (*m_pArff)[m_pointerStart+m_vIntervals[intPointer]][m_attToPaint];
    QRect geom = geometry();

    int endPoint = m_vIntervals[intPointer];
    int startPoint = endPoint;
    while (startPoint > 0 && newLimit < (double)geom.width()*((*m_pArff)[m_pointerStart + startPoint][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration))
        startPoint--;

    int attValueLeft = (*m_pArff)[m_pointerStart+startPoint][m_attToPaint];
    if (attValueLeft == attValue)
        return;

    for (int i=startPoint; i<endPoint; i++)
        m_pArff->ChangeData(m_pointerStart + i, m_attToPaint, attValue);
}

void ArffWidgetBase::ExpandIntRight(int intPointer, double newLimit)
{
    if (intPointer < 0 || intPointer >= (int)m_vIntervals.size())
        return;

    int arffIndex = m_pointerStart+m_vIntervals[intPointer];
    int attValue = (*m_pArff)[arffIndex][m_attToPaint];
    QRect geom = geometry();

    attValue = (*m_pArff)[m_pointerStart+m_vIntervals[intPointer-1]][m_attToPaint];

    int startPoint = m_vIntervals[intPointer-1];
    int endPoint = startPoint;
    while (endPoint<m_intervalLength && newLimit>(double)geom.width()*((*m_pArff)[m_pointerStart+endPoint][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration))
        endPoint++;

    int attValueRight = (*m_pArff)[m_pointerStart+endPoint-1][m_attToPaint];
    if (attValue == attValueRight)
        return;

    for (int i=startPoint; i<endPoint; i++)
        m_pArff->ChangeData(m_pointerStart + i, m_attToPaint, attValue);
}

void ArffWidgetBase::wheelEvent(QWheelEvent *event)
{
    QRect geom = geometry();
    // find where the roll took place
    double pointerTime = m_winStartTime + ((double)event->x()/(double)geom.width())*(double)m_windowDuration;

    // set new window duration
    if (event->delta() < 0)
        SetWindowDuration(2*m_windowDuration);
    else
        SetWindowDuration(m_windowDuration/2);

    emit SendWindowDur(m_windowDuration);

    int winStartTime = pointerTime-((double)event->x()/(double)geom.width())*m_windowDuration;
    int currentTime = winStartTime + m_windowDuration/2;

    // set time for the new duration
    SetTime(currentTime);
    emit SendTime(currentTime);
}

void ArffWidgetBase::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();

    if (key == Qt::Key_Delete)
        DeleteInterval();
    else if (key == Qt::Key_Insert)
        InsertInterval();
    else
        QWidget::keyPressEvent(event);
}

void ArffWidgetBase::DeleteInterval()
{
    ChangeInterval(0);
}

void ArffWidgetBase::InsertInterval()
{
    if (m_selectedEyeMovement >= m_colorNum)
        return;
    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows == 0)
        return;
    
    QRect geom = geometry();
    double pressPos = 0;
    if (m_DoubleLeftPressPos >= 0)
        pressPos = m_DoubleLeftPressPos;
    else
        pressPos = m_LeftPressPosition;

    long int leftPressTime = (long int)(m_windowDuration*pressPos/(double)geom.width()) + m_winStartTime;
    if (leftPressTime < (*m_pArff)[0][m_timeInd] || leftPressTime > (*m_pArff)[rows-1][m_timeInd])
        return;

    int startIndex, endIndex;
    if (m_DoubleLeftPressPos >= 0)
    {
        if (m_intervalAtt < 0)
        {
            Update();
            return;
        }

        int index = (int) ArffUtil::FindPosition(m_pArff, m_timeInd, leftPressTime);
        double labelOtherAtt = (*m_pArff)[index][m_intervalAtt];
        double labelPaintAtt = (*m_pArff)[index][m_attToPaint];
        startIndex = index;
        endIndex = index;
        while (startIndex >= 0 && (*m_pArff)[startIndex][m_intervalAtt] == labelOtherAtt && (*m_pArff)[startIndex][m_attToPaint] == labelPaintAtt)
            startIndex--;
        startIndex++;
        while (endIndex < rows && (*m_pArff)[endIndex][m_intervalAtt] == labelOtherAtt && (*m_pArff)[endIndex][m_attToPaint] == labelPaintAtt)
            endIndex++;
        endIndex--;
    }
    else
    {
        int intDuration = 40000; // 40ms on each side
        startIndex = (int) ArffUtil::FindPosition(m_pArff, m_timeInd, leftPressTime - intDuration);
        endIndex = (int) ArffUtil::FindPosition(m_pArff, m_timeInd, leftPressTime + intDuration);
    }

    for (int i=startIndex; i<=endIndex; i++)
        m_pArff->ChangeData(i, m_attToPaint, m_selectedEyeMovement);

    emit SendUpdate();
}

void ArffWidgetBase::ChangeInterval(int newValue)
{
    if (m_selectedEyeMovement >= m_colorNum)
        return;

    QRect geom = geometry();
    int activeInterval=-1;

    // find the selected interval
    for (int i=0; i<(int)m_vIntervals.size()-1; i++)
    {
        double start_width = (double)geom.width()*((*m_pArff)[m_pointerStart+m_vIntervals[i]][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);
        double end_width = (double)geom.width()*((*m_pArff)[m_pointerStart+m_vIntervals[i+1]-1][m_timeInd] - (double)m_winStartTime)/(double)(m_windowDuration);

        if (m_LeftPressPosition >= start_width && m_LeftPressPosition < end_width)
        {
            activeInterval = i;
            break;
        }
    }

    // click between intervals, nothing to change
    if (activeInterval == -1)
        return;

    // set elements of selected interval to unassigned
    for (int i=m_vIntervals[activeInterval]; i<m_vIntervals[activeInterval+1]; i++)
        m_pArff->ChangeData(m_pointerStart+i, m_attToPaint, newValue);

    emit SendUpdate();
}

void ArffWidgetBase::CalculateIntervals()
{
    // iterate through data points and use asked attribute
    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows == 0 || m_attToPaint < 0 || m_attToPaint > columns-1)
        return;

    m_vIntervals.clear();
    m_vIntervals.push_back(0);

    int attValue = (*m_pArff)[m_pointerStart][m_attToPaint];

    for (int i=1; i<m_intervalLength; i++)
    {
        if (attValue != (*m_pArff)[m_pointerStart+i][m_attToPaint])
        {
            m_vIntervals.push_back(i); // add to intervals

            // re-assign values
            attValue = (*m_pArff)[m_pointerStart+i][m_attToPaint];
        }
    }

    // last interval that is within the window
    if (m_intervalLength > 1)
        m_vIntervals.push_back(m_intervalLength-1); // add interval length to intervals
}

void ArffWidgetBase::ClearEventVariables()
{
    m_LeftPressPosition = -1;
}

void ArffWidgetBase::InitializeLegend()
{
    // Currently up to 10 eye movement are possible
    m_vLegendColors.clear();
    m_vLegendColors.push_back(vector<int> {255,255,255,255});
    m_vLegendColors.push_back(vector<int> {255,0,0,0});
    m_vLegendColors.push_back(vector<int> {0,255,0,0});
    m_vLegendColors.push_back(vector<int> {0,0,255,0});
    m_vLegendColors.push_back(vector<int> {100,100,100,0});
    m_vLegendColors.push_back(vector<int> {0,180,180,0});
    m_vLegendColors.push_back(vector<int> {255,0,255,0});
    m_vLegendColors.push_back(vector<int> {10,150,70,0});
    m_vLegendColors.push_back(vector<int> {150,30,70,0});
    m_vLegendColors.push_back(vector<int> {150,70,150,0});
    m_vLegendColors.push_back(vector<int> {170,130,100,0});

    // Initialize default names
    m_vLegendCaption.clear();
    m_vLegendCaption.push_back("unassigned");
    m_vLegendCaption.push_back("fixation");
    m_vLegendCaption.push_back("saccade");
    m_vLegendCaption.push_back("SP");
    m_vLegendCaption.push_back("noise");
    m_vLegendCaption.push_back("");
    m_vLegendCaption.push_back("");
    m_vLegendCaption.push_back("");
    m_vLegendCaption.push_back("");
    m_vLegendCaption.push_back("");
    m_vLegendCaption.push_back("other");

    assert(m_vLegendColors.size() == m_vLegendCaption.size());

    m_colorNum = m_vLegendColors.size();
}

void ArffWidgetBase::InitializeLegend(int attInd)
{
    InitializeLegend();
    vector<string> legendCaption;

    if (m_pArff->GetAttMapping(attInd, legendCaption))
    {
        if (legendCaption.size() > m_vLegendCaption.size())
        {
            cout << "ERROR: cannot handle more than 9 colors for the provided input argument" << endl;
            exit(-1);
        }

        m_vLegendCaption = legendCaption;
        m_colorNum = m_vLegendCaption.size();
    }
    else
    {
        cout << "WARNING: using default legend colors" << endl;
    }
}
