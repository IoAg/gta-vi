/// PaintGaze.h

#ifndef __PAINTGAZE_H__
#define __PAINTGAZE_H__

#include <QtWidgets>
#include <vector>

#include "../arffHelper/Arff.h"

using namespace std;

class PaintGaze : public QObject
{
    Q_OBJECT
public slots:
    void HandleTime(int time, QObject *pSender);
    ///< Handles new position of video.

    void HandleToggleView();
    ///< Toggles the diplayed gaze between the primary and secondary ARFF files

public:
    PaintGaze();

    void SetData(Arff &arff);
    ///< Sets the data to read that contain time,x,y information.

    void SetFovData(Arff &arff);
    ///< Sets the data to Field Of View ARFF converted from equirectangualr file.

    void SetCurrentTime(long int currentTime);
    ///< Sets the current time.

    void SetInterval(long int intervalDuration);
    ///< Sets the interval duration.

    void Paint(QPainter *painter, QSize size);
    ///< Paints the gaze points with painter and normalizes 
    ///< to the provided size.

private:
    void CalculateIntervals();
    ///< Calculates the intervals for before and after the current time.

    void GetSetup();
    ///< It populates the variables needed to correclt display the gaze

    Arff                        *m_pArff; // pointer to ARFF data container
    Arff                        *m_pSecArff; // pointer to sendary ARFF
    int                         m_timeInd;
    int                         m_xInd;
    int                         m_yInd;
    long int                    m_currentTime; // current time in us
    long int                    m_intervalDuration; // duration of intervals for before and after
    int                         m_startPoint; // point of m_vpDataPoins for the interval in the past
    int                         m_endPoint; // point in the future
    double                      m_videoWidth; // width of the video
    double                      m_videoHeight; // height of the video

};


#endif /*__PAINTGAZE_H__*/
