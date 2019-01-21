// FlowWidget.h

#ifndef __FLOWWIDGET_H__
#define __FLOWWIDGET_H__

#include <QtWidgets>
#include <cstdio>

#include "../arffHelper/Arff.h"

#define MAXCOLS 60

class FlowWidget : public QWidget
{
    Q_OBJECT

public slots:
    void HandleNewTime(int time);
    ///< handles signal of time change from video or by scrolling the area

public:
    FlowWidget(QWidget *parent=0);

    ~FlowWidget(void);

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    void SetCurrentTime(int currentTime);
    ///< sets the time of the video in us

    void SetDuration(qint64 duration);
    ///< sets the duration of the video in us

    void SetMedia(QString pathToFile);
    ///< points to the flow file

    void SetData(Arff &arff);
    ///< sets gaze data

    void SetInterval(qint64 interval);
    ///< sets the time interval of window in which samples are counted

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    ///< called on update(), repaint(). Paints flow and bounding box with 
    ///< mean velocity vector in it

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    ///< handles painting box resize event

private:
    void PaintFlow(QPainter *painter);
    ///< draw image and paints on canvas

    void PaintGazeRect(QPainter *painter);
    ///< draws the rectangle that contains the gaze samples

    void PaintVelocity(QPainter *painter);
    ///< paints the coordinates system and the velocity vectors

    void PaintVector(QPointF *points, QPainter *painter);
    ///< paints the given vector, with the already set up painter

    void PaintLegend(QPainter *painter);
    ///< paints legend for colored vectors

    void ReadFlow(const char *filename, float normLevel=0.7);
    ///< read data from the flow file

    int CalculateFrameNum(void);
    ///< calculates the frame number of the current time

    void DrawImage(void);
    ///< draws the current frame to the image

    void SetCols(int r, int g, int b, int k);
    ///< Creates a color wheel. Based on http://vision.middlebury.edu/flow/data/flow-code.zip/colorco    de.cpp

    void MakeColorWheel(void);
    ///< Created color wheel. For more info see \ref SetCols

    void ComputeColor(float u, float v, int *rgb);
    ///< Computes color for the given u,v components. For more info see \ref SetCols

    void UpdateRectangle(void);
    ///< updates painting rectangle

    void CalculateMinMaxGaze(void);
    ///< calculates min max gaze coordinates for the interval
    ///< and the mean velocity of gaze samples

    void CalculateMeanVelocity(void);

    FILE                        *m_pStream; // flow stream
    Arff                        *m_pArff; // pointer to Arff

    int                         m_timeInd;
    int                         m_xInd;
    int                         m_yInd;

    int                         m_numOfFrames;
    int                         m_width;
    int                         m_height;
    int                         m_storedWidth;
    int                         m_storedHeight;
    vector<float>               m_vMinU;
    vector<float>               m_vMaxU;
    vector<float>               m_vMinV;
    vector<float>               m_vMaxV;
    float                       m_normMinU; // normalization factor based on percentage of minU
    float                       m_normMaxU;
    float                       m_normMinV;
    float                       m_normMaxV;

    int                         m_minGazeX; // max min gaze coordinates for current time within interval
    int                         m_maxGazeX;
    int                         m_minGazeY;
    int                         m_maxGazeY;

    double                      m_horVector; // flow vectors for rectangle
    double                      m_verVector;

    double                      m_horGazeVel; // gaze velocity for interval
    double                      m_verGazeVel;
    double                      m_meanHorGazeVel;
    double                      m_meanVerGazeVel;

    int                         m_colorWheel[MAXCOLS][3];
    int                         m_ncols;

    int                         m_currentFrame; // frame already drawn on QImage
    qint64                      m_currentTime; // in us
    qint64                      m_intervalDuration; // duration of intervals for before and after gaze sample
    QImage                      m_image; // image holdinh flow in RGB
    QRect                       m_targetRect; // rect to paint
    qint64                      m_duration; // duration of the video

};

#endif /*__FLOWWIDGET_H__*/
