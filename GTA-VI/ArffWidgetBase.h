// WindowArffBase.h

#ifndef __ARFFWIDGETBASE_H__
#define __ARFFWIDGETBASE_H__

#include <QtWidgets>
#include <vector>

#include "../arffHelper/Arff.h"

using namespace std;

class ArffWidgetBase : public QWidget
{
    Q_OBJECT
signals:
    void SendWindowDur(int dur_us);

    void SendTime(int curTime_us);
    ///< Sends the current time.

    void SendUpdate();
    ///< Singal to upadte connected widgets including sending object.

public slots:
    void HandleWindowDur(int dur_us, QObject *pSender);

    void HandleTime(int curTime_us, QObject *pSender);

    void HandleUpdate();

    void HandleSelectedEyeMovement(int eyeMovement);

    virtual void HandleToggleView();

public:
    ArffWidgetBase(QWidget *parent=0);

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    ///< Returns the required mimimum size for the widget in order
    ///< to render correctly. If no layout is present returns invalid size.

    void SetGridInterval(int tickX, int tickY);
    ///< Sets the tick of x,y axes. Otherwise uses default.

    void SetWindowDuration(int duration);
    ///< Sets the duration of the window in us.

    void SetTime(int currentTime);
    ///< Sets the current time of display. Initially 0.

    virtual void SetData(Arff &arff, int attToPaint, double maxValue=-1.0);
    ///< Sets the ARFF data and the index of the attribute to paint. \p maxValue 
    ///< is the normlization on the plotted data.

    virtual void SetFovData(Arff &arff, double maxValue);
    ///< Sets Field Of View data created from te equirectangular data. This 
    ///< function is called after data has been set.

    void SetIntervalAtt(int intervalAtt);
    ///< Sets an attribute that is going to be used as guide for interval
    ///< insertion in the painted attribute. This primary useful for secondary
    ///< labels.

    void Update();
    ///< Updates all displayed information and repaints the area.

    void MoveToStart();
    ///< Moves the time to the time of the first sample point and emits signal in order
    ///< to sync all other widgets. If by the time of the call no data are present
    ///< it does nothing.

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    void PaintBase(QPainter *painter);

    void PaintAreas(QPainter *painter);
    ///< Paints areas on the background with color.

    void PaintVideoTime(QPainter *painter);
    ///< Paints the position of the video on window.

    virtual void PaintLine(QPainter *painter);
    ///< Paints the line on the canvas.

    void PaintGrid(QPainter *painter);
    ///< Paints grid lines on the canvas.

    void PaintText(QPainter *painter);
    ///< Paints axes limits on the canvas.

    void PaintLegend(QPainter *painter);
    ///< Paints legend for colors and values.

    void mousePressEvent(QMouseEvent *event);
    ///< Handles the press of mouse buttons.

    void mouseDoubleClickEvent(QMouseEvent *event);
    ///< Handles left mouse double click. It inserts an interval based on the
    ///< on the interval and paint attributes. It works with SetIntervalAtt(int)

    void mouseReleaseEvent(QMouseEvent *event);
    ///< Handles the realese of mouse buttons.

    void mouseMoveEvent(QMouseEvent *event);
    ///< Mouse tracking is on when a button is clicked.

    void wheelEvent(QWheelEvent *event);
    ///< Tracks mouse wheel events.

    void keyPressEvent(QKeyEvent *event);
    ///< Handles keyboard key presses.

    void ExpandIntLeft(int intPointer, double newLimit);

    void ExpandIntRight(int intPointer, double newLimit);

    void DeleteInterval(void);
    ///< Sets the selected interval to unassigned.

    void InsertInterval(void);
    ///< Insert new interval to the selected interval at the click point.
    ///< The new interval has duration of 80ms.

    void ChangeInterval(int newValue);
    ///< If the mouse is clicked and a number is typed then the interval
    ///< is changed to the new eye movement.

    void CalculateIntervals(void);
    ///< Calculates the intervals for the given time window and
    ///< sets the interval pointer to the interval of the left click position
    ///< if available.

    void ClearEventVariables(void);
    ///< Clears the state of all local variables.

    void InitializeLegend();

    void InitializeLegend(int attInd);

    int                         m_gridTickY; // tick on y axis
    int                         m_gridTickX; // tick on x axis
    int                         m_windowDuration; // duration of window in us
    int                         m_windowMaxVal; // max value to display
    int                         m_windowSecMaxVal; // max value to display for secondary data
    int                         m_winStartTime; // start time of the window
    int                         m_currentTime;
    Arff                        *m_pArff; // pointer to Arff
    Arff                        *m_pXYArff; // pointer to Arff only for valid XY
    int                         m_timeInd;
    int                         m_xInd;
    int                         m_yInd;
    Arff                        *m_pXYSecArff; // Arff pointer for secondary data for valid XY

    int                         m_pointerStart; // starting DataPoint for current time
    int                         m_pointerEnd; // end point for end of window
    int                         m_intervalLength; // amount of samples in between
    int                         m_attToPaint; // pointer of the attribute to paint as background
    int                         m_intervalAtt;

    // event specific variables
    bool                        m_IsPressedLeft; // indicates if the left button is pressed
    double                      m_LeftPressPosition; // position of the left press relative to widget
    double                      m_DoubleLeftPressPos; 
    int                         m_RightPressPosition; // position of the left press relative to widget
    vector<int>                 m_vIntervals; // each element represents the start of interval

    vector<string>              m_vLegendCaption;
    vector<vector<int>>         m_vLegendColors;
    int                         m_colorNum;

    int                         m_selectedEyeMovement; // when numbers are pressed a specific eye movement is selected

    double                      m_prevDragPosX;
};

#endif /*__ARFFWIDGETBASE_H__*/
