// FlowWidget.cpp

#include "FlowWidget.h"
#include "../arffHelper/ArffUtil.h"
#include "Unused.h"

#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

// PUBLIC SLOTS:

void FlowWidget::HandleNewTime(int time)
{
    SetCurrentTime(time);
}

// PUBLIC:

FlowWidget::FlowWidget(QWidget *parent) : QWidget(parent),
    m_pStream(NULL), m_pArff(NULL), 
    m_horVector(0), m_verVector(0), m_duration(0)
{
    SetCurrentTime(0);
    SetInterval(100000); // 100ms
}

FlowWidget::~FlowWidget()
{
    if (m_pStream!=NULL)
        fclose(m_pStream);
}

QSize FlowWidget::minimumSizeHint() const /*override*/
{
    return QSize(160,90);
}

void FlowWidget::SetCurrentTime(int currentTime)
{
    m_currentTime = currentTime;
    CalculateMinMaxGaze();

    // update area
    update();
}

void FlowWidget::SetDuration(qint64 duration)
{
    m_duration = duration;
}

void FlowWidget::SetMedia(QString pathToFile)
{
    ReadFlow(pathToFile.toStdString().c_str());
}

void FlowWidget::SetData(Arff &arff)
{
    m_pArff = &arff;
    int confInd;
    bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, m_xInd, m_yInd, confInd);
    UNUSED(confInd);
    UNUSED(res);
    int rows, columns;
    m_pArff->Size(rows, columns);


    if (rows<2)
        return;

    m_meanHorGazeVel = 0;
    m_meanVerGazeVel = 0;

    for (int i=0; i<rows-1; i++){
        m_meanHorGazeVel += abs((*m_pArff)[i][m_xInd]-(*m_pArff)[i+1][m_xInd]) / ((*m_pArff)[i+1][m_timeInd]-(*m_pArff)[i][m_timeInd]);
        m_meanVerGazeVel += abs((*m_pArff)[i][m_yInd]-(*m_pArff)[i+1][m_yInd]) / ((*m_pArff)[i+1][m_timeInd]-(*m_pArff)[i][m_timeInd]);
    }

    m_meanHorGazeVel /= (double)(rows-1);
    m_meanVerGazeVel /= (double)(rows-1);

}

void FlowWidget::SetInterval(qint64 interval)
{
    m_intervalDuration = interval;
}


// PROTECTED:

void FlowWidget::paintEvent(QPaintEvent *) /*override*/
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    PaintFlow(&painter);
    PaintGazeRect(&painter);
    PaintVelocity(&painter);
    PaintLegend(&painter);
}

void FlowWidget::resizeEvent(QResizeEvent *event) /*override*/
{
    QWidget::resizeEvent(event);
    UpdateRectangle();
}

// PRIVATE:

void FlowWidget::PaintFlow(QPainter *painter)
{
    if (CalculateFrameNum() != m_currentFrame){
        m_currentFrame = CalculateFrameNum();
        DrawImage();
    }

    painter->drawImage(m_targetRect, m_image, QRect(QPoint(0,0), m_image.size()));
}

void FlowWidget::PaintGazeRect(QPainter *painter)
{
    double width = (double)(m_maxGazeX-m_minGazeX)*(double)m_targetRect.width()/(double)m_width;
    double height = (double)(m_maxGazeY-m_minGazeY)*(double)m_targetRect.height()/(double)m_height;
    double startX = m_targetRect.left() + (double)(m_minGazeX)*(double)m_targetRect.width()/(double)m_width;
    double startY = m_targetRect.top() + (double)(m_minGazeY)*(double)m_targetRect.height()/(double)m_height;

    painter->setPen(QPen(Qt::black, 4.0*m_targetRect.width()/1000.0));
    painter->drawRect(startX, startY,width, height);
}

void FlowWidget::PaintVelocity(QPainter *painter)
{
    if (m_pStream==NULL)
        return;

    // update mean velocity
    CalculateMeanVelocity();

    QPointF center(m_targetRect.left()+m_targetRect.height()/3.5, m_targetRect.top()+m_targetRect.height()/3.5);
    double coorLength = m_targetRect.height()/5.0;

    // paint coordinates
    painter->setPen(QPen(Qt::black, 2.0*m_targetRect.width()/1000.0));
    painter->setBrush(Qt::black);
    QPointF points[2];

    // x axis
    points[0] = center - QPoint(coorLength,0);
    points[1] = center + QPoint(coorLength,0);
    PaintVector(points, painter);

    // y axis
    points[0] = center - QPoint(0,coorLength);
    points[1] = center + QPoint(0,coorLength);
    PaintVector(points, painter);

    // paint horizontal velocity component
    painter->setPen(QPen(Qt::blue, 2.5*m_targetRect.width()/1000.0));
    painter->setBrush(Qt::blue);
    points[0] = center;
    if (m_horVector > 0)
    {
        // check limits because threshold for max is 0.7
        if (m_horVector/m_normMaxU > 1)
            points[1] = center + QPoint(coorLength,0);
        else
            points[1] = center + QPoint(coorLength*m_horVector/m_normMaxU,0);

    }
    else
    {
        if (m_horVector/m_normMinU > 1)
            points[1] = center - QPoint(coorLength,0);
        else
            points[1] = center + QPoint(coorLength*(-1)*m_horVector/m_normMinU,0);
    }
    //PaintVector(points, painter);
    // Store for later
    QPointF horComp = points[1];

    // paint vertical velocity component
    painter->setPen(QPen(Qt::yellow, 2.5*m_targetRect.width()/1000.0));
    painter->setBrush(Qt::yellow);
    points[0] = center;
    if (m_verVector > 0)
    {
        if (m_verVector/m_normMaxV > 1)
            points[1] = center + QPoint(0, coorLength);
        else
            points[1] = center + QPoint(0, coorLength*m_verVector/m_normMaxV);
    }
    else
    {
        if (m_verVector/m_normMinV > 1)
            points[1] = center - QPoint(0, coorLength);
        else
            points[1] = center + QPoint(0, coorLength*(-1)*m_verVector/m_normMinV);
    }
    //PaintVector(points, painter);

    // paint velocity component
    painter->setPen(QPen(Qt::red, 3*m_targetRect.width()/1000.0));
    painter->setBrush(Qt::red);
    points[1] += horComp;
    points[1] -= center; // added twice
    PaintVector(points, painter);

    // paint gaze velocity
    double normHorVel = m_horGazeVel/(1.0*m_meanHorGazeVel);
    double normVerVel = m_verGazeVel/(1.0*m_meanVerGazeVel);

    // keep aspect ratio if both are big
    if (abs(normHorVel)>1 && abs(normVerVel)>1)
    {
        if (abs(normHorVel)>abs(normVerVel))
            normVerVel /= abs(normHorVel);
        else
            normHorVel /= abs(normVerVel);
    }
    if (abs(normHorVel)>1)
        normHorVel /= abs(normHorVel);
    if (abs(normVerVel)>1)
        normVerVel /= abs(normVerVel);

    points[1] = center + QPoint(normHorVel*coorLength,normVerVel*coorLength);
    painter->setPen(QPen(QColor(0,0,255,255), 2.5*m_targetRect.width()/1000.0));
    painter->setBrush(QBrush(QColor(0,0,255,255)));
    PaintVector(points, painter);


}

void FlowWidget::PaintVector(QPointF *points, QPainter *painter)
{
    QLineF line(points[0], points[1]);
    painter->drawLine(line);

    // Draw the arrows
    double angle = ::acos(line.dx() / line.length());
    double Pi = 3.14159265359;
    double arrowSize = (double)m_targetRect.height()/30.0;
    if (sqrt(pow(points[0].x()-points[1].x(),2) + pow(points[0].y()-points[1].y(),2)) < arrowSize)
        arrowSize = sqrt(pow(points[0].x()-points[1].x(),2) + pow(points[0].y()-points[1].y(),2));

    if (line.dy() >= 0)
        angle = 2*Pi - angle;

    QPointF destArrowP1 = points[1] + QPointF(sin(angle - Pi / 3) * arrowSize,
                                              cos(angle - Pi / 3) * arrowSize);
    QPointF destArrowP2 = points[1] + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                              cos(angle - Pi + Pi / 3) * arrowSize);

    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
}

void FlowWidget::PaintLegend(QPainter *painter)
{
    int legendHeight = m_targetRect.height()/30.0;
    int legendWidth = m_targetRect.width()/10;
    QFont font = painter->font();
    font.setPointSize(legendHeight/1.5);
    painter->setFont(font);

    painter->setBrush(QBrush(QColor(255,0,0,160)));
    painter->setPen(QPen(QColor(255,0,0,160)));
    painter->drawRect(QRect(m_targetRect.left(),m_targetRect.top(), legendWidth, legendHeight));
    painter->setPen(QPen(Qt::black));
    painter->drawText(QRect(m_targetRect.left(),m_targetRect.top(), legendWidth, legendHeight), 0, "video flow");


    painter->setBrush(QBrush(QColor(0,0,255,160)));
    painter->setPen(QPen(QColor(0,0,255,160)));
    painter->drawRect(QRect(m_targetRect.left(),m_targetRect.top()+legendHeight, legendWidth, legendHeight));
    painter->setPen(QPen(Qt::black));
    painter->drawText(QRect(m_targetRect.left(),m_targetRect.top()+legendHeight, legendWidth*1.5, legendHeight), 0, "gaze velocity");
}

void FlowWidget::ReadFlow(const char *filename, float normLevel /*=0.7*/)
{
    // close stream if open
    if (m_pStream != NULL)
        fclose(m_pStream);

    // open new file
    m_pStream = fopen(filename, "rb");
    // return if not opened
    if (m_pStream == NULL)
        return;

    // read data about flow
    size_t res;
    res = fread(&m_numOfFrames, sizeof(int), 1, m_pStream);
    res = fread(&m_width, sizeof(int), 1, m_pStream);
    res = fread(&m_height, sizeof(int), 1, m_pStream);
    res = fread(&m_storedWidth, sizeof(int), 1, m_pStream);
    res = fread(&m_storedHeight, sizeof(int), 1, m_pStream);

    // allocate QImage
    m_image = QImage(m_storedWidth, m_storedHeight, QImage::Format_RGB32);
    // initialize to fixed color
    unsigned char *pImageData = m_image.bits(); // BGRA format
    for (int i=0; i<m_storedWidth*m_storedHeight*4; i++)
        pImageData[i] = 220;

    // update rectangle
    UpdateRectangle();

    // read max min for each frame
    float minU, maxU, minV, maxV;
    for (int i=0; i<m_numOfFrames; i++)
    {
        res = fread(&minU, sizeof(float), 1, m_pStream);
        res = fread(&maxU, sizeof(float), 1, m_pStream);
        res = fread(&minV, sizeof(float), 1, m_pStream);
        res = fread(&maxV, sizeof(float), 1, m_pStream);

        m_vMinU.push_back(minU);
        m_vMaxU.push_back(maxU);
        m_vMinV.push_back(minV);
        m_vMaxV.push_back(maxV);

        fseek(m_pStream, 2*m_storedWidth*m_storedHeight*sizeof(short int), SEEK_CUR);
    }
    UNUSED(res);

    // reset seek to first frame
    fseek(m_pStream, 5*sizeof(int), SEEK_SET);

    m_currentFrame = 0;
    m_ncols = 0;

    // calculate normalization factor as 70% of min-max samples
    vector<float> tmpVec;

    // check limits
    normLevel = normLevel <= 0.0 ? 0.01: normLevel;
    normLevel = normLevel > 1.0? 1.0: normLevel;

    int threshold = normLevel*(m_vMinU.size()-1);
    tmpVec = m_vMinU;
    sort(tmpVec.begin(), tmpVec.end(), greater<float>()); // sort negative in descending order
    m_normMinU = tmpVec[threshold];
    tmpVec = m_vMaxU;
    sort(tmpVec.begin(), tmpVec.end());
    m_normMaxU = tmpVec[threshold];
    tmpVec = m_vMinV;
    sort(tmpVec.begin(), tmpVec.end(), greater<float>()); // sort negative in descending order
    m_normMinV = tmpVec[threshold];
    tmpVec = m_vMaxV;
    sort(tmpVec.begin(), tmpVec.end());
    m_normMaxV = tmpVec[threshold];

}

int FlowWidget::CalculateFrameNum()
{
    int frameNum = (int)((double)m_numOfFrames*(double)m_currentTime/(double)m_duration);

    if (frameNum < 0)
        return 0;

    if (frameNum > m_numOfFrames-1)
        return m_numOfFrames-1;

    return frameNum;
}

void FlowWidget::DrawImage()
{
    if (m_pStream==NULL)
        return;

    // move file pointer to correct position
    long int frameOffset = 5*sizeof(int) + m_currentFrame*(4*sizeof(float) + 2*m_storedWidth*m_storedHeight*sizeof(short int));
    fseek(m_pStream, frameOffset, SEEK_SET);
    // skip max-min information
    fseek(m_pStream, 4*sizeof(float), SEEK_CUR);

    short int u,v;
    float hor, ver;
    int rgb[3];
    int pointerImage = 0;
    unsigned char *pImageData = m_image.bits(); // BGRA format
    size_t res;
    for (int y=0; y<m_storedHeight; y++)
    {
        for (int x=0; x<m_storedWidth; x++)
        {
            res = fread(&u, sizeof(short int), 1, m_pStream);
            res = fread(&v, sizeof(short int), 1, m_pStream);

            // without normalization
            hor = (float)u*m_vMinU[m_currentFrame]/(float)numeric_limits<short int>::min();
            ver = (float)v*m_vMinV[m_currentFrame]/(float)numeric_limits<short int>::min();

            ComputeColor(hor, ver, rgb);
           
            pImageData[pointerImage+2] = rgb[0];
            pImageData[pointerImage+1] = rgb[1];
            pImageData[pointerImage+0] = rgb[2];

            pointerImage += 4;
        }
    }
    UNUSED(res);
}


// The code of the folowing 3 function was based on
// http://vision.middlebury.edu/flow/data/flow-code.zip/colorcode.cpp
void FlowWidget::SetCols(int r, int g, int b, int k)
{
    m_colorWheel[k][0] = r;
    m_colorWheel[k][1] = g;
    m_colorWheel[k][2] = b;
}

void FlowWidget::MakeColorWheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    m_ncols = RY + YG + GC + CB + BM + MR;
    if (m_ncols > MAXCOLS)
    exit(1);
    int i;
    int k = 0;
    for (i = 0; i < RY; i++) SetCols(255,      255*i/RY,     0,        k++);
    for (i = 0; i < YG; i++) SetCols(255-255*i/YG, 255,      0,        k++);
    for (i = 0; i < GC; i++) SetCols(0,        255,      255*i/GC,     k++);
    for (i = 0; i < CB; i++) SetCols(0,        255-255*i/CB, 255,      k++);
    for (i = 0; i < BM; i++) SetCols(255*i/BM,     0,        255,      k++);
    for (i = 0; i < MR; i++) SetCols(255,      0,        255-255*i/MR, k++);
}

void FlowWidget::ComputeColor(float u, float v, int *rgb)
{
    if (m_ncols == 0)
        MakeColorWheel();

    float rad = sqrt(u * u + v * v);
    float a = atan2(-v, -u) / M_PI;
    float fk = (a + 1.0) / 2.0 * (m_ncols-1);
    int k0 = (int)fk;
    int k1 = (k0 + 1) % m_ncols;
    float f = fk - k0;
    //f = 0; // uncomment to see original color wheel
    float col0, col;
    for (int b = 0; b < 3; b++) {
        col0 = m_colorWheel[k0][b] / 255.0;
        col = m_colorWheel[k1][b] / 255.0;
        col = (1 - f) * col0 + f * col;
        if (rad <= 1)
            col = 1 - rad * (1 - col); // increase saturation with radius
        else
            col *= .75; // out of range
        rgb[b] = (int)(255.0 * col);
    }
}

void FlowWidget::UpdateRectangle()
{
    QSize size = m_image.size();
    size.scale(this->size(), Qt::KeepAspectRatio);
    m_targetRect = QRect(QPoint(0,0), size);
    m_targetRect.moveCenter(this->rect().center());
}

void FlowWidget::CalculateMinMaxGaze()
{
    if (m_pArff == NULL)
        return;

    int rows, columns;
    m_pArff->Size(rows, columns);
    UNUSED(columns);
    if (rows == 0)
        return;

    m_minGazeX = numeric_limits<int>::max();
    m_minGazeY = numeric_limits<int>::max();
    m_maxGazeX = numeric_limits<int>::min();
    m_maxGazeY = numeric_limits<int>::min();

    m_horGazeVel = 0.0;
    m_verGazeVel = 0.0;
    int counter = 0;

    for (int i=0; i<rows-1; i++)
    {
        if ((*m_pArff)[i][m_timeInd]>m_currentTime-m_intervalDuration && (*m_pArff)[i][m_timeInd]<m_currentTime+m_intervalDuration)
        {
            if ((*m_pArff)[i][m_xInd] < m_minGazeX)
                m_minGazeX = (*m_pArff)[i][m_xInd];
            if ((*m_pArff)[i][m_xInd] > m_maxGazeX)
                m_maxGazeX = (*m_pArff)[i][m_xInd];

            if ((*m_pArff)[i][m_yInd] < m_minGazeY)
                m_minGazeY = (*m_pArff)[i][m_yInd];
            if ((*m_pArff)[i][m_yInd] > m_maxGazeY)
                m_maxGazeY = (*m_pArff)[i][m_yInd];

            // calculate velocity
            m_horGazeVel += ((*m_pArff)[i+1][m_xInd]-(*m_pArff)[i][m_xInd]) / ((*m_pArff)[i+1][m_timeInd]-(*m_pArff)[i][m_timeInd]);
            m_verGazeVel += ((*m_pArff)[i+1][m_yInd]-(*m_pArff)[i][m_yInd]) / ((*m_pArff)[i+1][m_timeInd]-(*m_pArff)[i][m_timeInd]);
            counter++;
        }
    }

    if (counter > 0)
    {
        m_horGazeVel /= (double)counter;
        m_verGazeVel /= (double)counter;
    }

    if (m_minGazeX < 0)
        m_minGazeX = 0;
    if (m_minGazeX >= m_width)
        m_minGazeX = m_width-1;

    if (m_maxGazeX < 0)
        m_maxGazeX = 0;
    if (m_maxGazeX >= m_width)
        m_maxGazeX = m_width-1;

    if (m_minGazeY < 0)
        m_minGazeY = 0;
    if (m_minGazeY >= m_height)
        m_minGazeY = m_height-1;

    if (m_maxGazeY < 0)
        m_maxGazeY = 0;
    if (m_maxGazeY >= m_height)
        m_maxGazeY = m_height-1;
}

void FlowWidget::CalculateMeanVelocity()
{
    if (m_pStream==NULL)
        return;

    long int frameOffset = 5*sizeof(int) + m_currentFrame*(4*sizeof(float) + 2*m_storedWidth*m_storedHeight*sizeof(short int));
    fseek(m_pStream, frameOffset, SEEK_SET);
    // skip max-min information
    fseek(m_pStream, 4*sizeof(float), SEEK_CUR);

    short int u,v;
    int rgb[3];
    int pointerImage = 0;
    unsigned char *pImageData = m_image.bits(); // BGRA format
    int yStart = m_minGazeY*(double)m_storedHeight/(double)m_height;
    int yEnd = m_minGazeY*(double)m_storedHeight/(double)m_height;
    int xStart = m_minGazeX*(double)m_storedWidth/(double)m_width;
    int xEnd = m_maxGazeX*(double)m_storedWidth/(double)m_width;

    m_horVector = 0;
    m_verVector = 0;

    // skip first lines of the image
    fseek(m_pStream, 2*m_storedWidth*yStart*sizeof(short int), SEEK_CUR);

    size_t res;
    for (int y=0; y<yEnd-yStart+1; y++){ //** check +1 for corner cases
        for (int x=0; x<m_storedWidth; x++){
            res = fread(&u, sizeof(short int), 1, m_pStream);
            res = fread(&v, sizeof(short int), 1, m_pStream);

            // process data
            if (x>=xStart && x<=xEnd)
            {
                if (u < 0)
                    m_horVector += (double)u*m_vMinU[m_currentFrame]/(double)numeric_limits<short int>::min();
                else
                    m_horVector += (double)u*m_vMaxU[m_currentFrame]/(double)numeric_limits<short int>::max();

                if (v < 0)
                    m_verVector += (double)v*m_vMinV[m_currentFrame]/(double)numeric_limits<short int>::min();
                else
                    m_verVector += (double)v*m_vMaxV[m_currentFrame]/(double)numeric_limits<short int>::max();
            }

            pImageData[pointerImage+2] = rgb[0];
            pImageData[pointerImage+1] = rgb[1];
            pImageData[pointerImage+0] = rgb[2];

            pointerImage += 4;
        }
    }
    UNUSED(res);

    m_horVector /= (yEnd-yStart+1)*(xEnd-xStart+1);
    m_verVector /= (yEnd-yStart+1)*(xEnd-xStart+1);
}
