// PaintGaze.cpp

#include "PaintGaze.h"
#include "../arffHelper/ArffUtil.h"
#include "Unused.h"

#include <iostream>

#define INTERVAL_DURATION 100000 // 100ms

using namespace std;

// PUBLIC SLOTS:
void PaintGaze::HandleTime(int time, QObject *pSender)
{
    if (pSender == this)
        return;

    SetCurrentTime(time);
}

void PaintGaze::HandleToggleView()
{
    if (m_pArff && m_pSecArff)
    {
        swap(m_pArff, m_pSecArff);
        GetSetup();
    }
}

// PUBLIC:

PaintGaze::PaintGaze() : m_pArff(NULL), m_pSecArff(NULL) 
{
    SetCurrentTime(0);
    SetInterval(INTERVAL_DURATION); 
}

void PaintGaze::SetData(Arff &arff)
{
    m_pArff = &arff;
    GetSetup();
}

void PaintGaze::SetFovData(Arff &arff)
{
    m_pSecArff = &arff;
}

void PaintGaze::SetCurrentTime(long int currentTime)
{
    m_currentTime = currentTime;
    CalculateIntervals();
}

void PaintGaze::SetInterval(long int intervalDuration)
{
    m_intervalDuration = intervalDuration;
    CalculateIntervals();
}

void PaintGaze::Paint(QPainter *painter, QSize size)
{
    if (m_pArff == NULL)
        return;

    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows > 0)
    {
        painter->setPen(QPen(Qt::gray, 1.3*size.width()/1000.0));
        int circleSize = size.width()/60;
        int halfCircleSize = circleSize/2;
        int xPos, yPos; // x,y position on the drawing area
        double videoAspectRatio = (double)m_videoWidth/(double)m_videoHeight;
        double areaAspectRatio = (double)size.width()/(double)size.height();
        int width, height, xDisp, yDisp;
        if (videoAspectRatio > areaAspectRatio){
            width = size.width();
            xDisp = 0;
            height = width/videoAspectRatio;
            yDisp = (size.height()-height)/2;
        }
        else{
            height = size.height();
            yDisp = 0;
            width = height*videoAspectRatio;
            xDisp = (size.width()-width)/2;
        }



        bool changedColor = false;
        int position=-1;
        for(int i=m_endPoint; i>=m_startPoint; i--)
        {
            if ((*m_pArff)[i][m_timeInd]<m_currentTime && !changedColor){
                painter->setPen(QPen(Qt::red, 3.5*size.height()/1000.0));
                position = i;
                changedColor = true;
            }
            
            xPos = (int)(xDisp+((double)width/m_videoWidth)*(*m_pArff)[i][m_xInd]);
            yPos = (int)(yDisp+((double)height/m_videoHeight)*(*m_pArff)[i][m_yInd]);

            painter->drawEllipse(xPos-halfCircleSize, yPos-halfCircleSize, circleSize, circleSize);
                    
        }

        if (position != -1){
            painter->setPen(QPen(Qt::green, 3.5*size.height()/1000.0));
            xPos = (int)(xDisp+((double)width/m_videoWidth)*(*m_pArff)[position][m_xInd]);
            yPos = (int)(yDisp+((double)height/m_videoHeight)*(*m_pArff)[position][m_yInd]);

            painter->drawEllipse(xPos-halfCircleSize, yPos-halfCircleSize, circleSize, circleSize);
        }
    }
}

// PRIVATE:

void PaintGaze::CalculateIntervals()
{
    m_endPoint = 0;
    m_startPoint = 0;

    if (m_pArff == NULL)
        return;

    int rows, columns;
    m_pArff->Size(rows, columns);
    if (rows == 0)
        return;

    m_endPoint = ArffUtil::FindPosition(m_pArff, m_timeInd, m_currentTime + m_intervalDuration);
    m_startPoint = ArffUtil::FindPosition(m_pArff, m_timeInd, m_currentTime - m_intervalDuration);
}

void PaintGaze::GetSetup()
{
    int confInd;
    bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, m_xInd, m_yInd, confInd);
    UNUSED(res);
    m_videoWidth = m_pArff->WidthPx();
    m_videoHeight = m_pArff->HeightPx();

    CalculateIntervals();
}
