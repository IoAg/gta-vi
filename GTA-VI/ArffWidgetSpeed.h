// ArffWidgetSpeed.h

#include "ArffWidgetBase.h"

class ArffWidgetSpeed : public ArffWidgetBase
{
public slots:
    void HandleToggleView();

public:
    ArffWidgetSpeed(double filterWindow);

    virtual void SetData(Arff &arff, int attToPaint, double maxValue=-1.0);

    virtual void SetFovData(Arff &arff, double maxValue=-1.0);

    void DisplayFov();

    void DisplayHead();

protected:
    virtual void PaintLine(QPainter *painter);

private:
    vector<double> m_vSpeed;
    vector<double> m_vSecondSpeed;
    vector<double> m_vHeadSpeed;
    vector<double> *m_pSpeed; 
    bool           m_FovDisplayed;
    double         m_speedWindow; // window duration from which we compute the step for speed calculation

    void PaintSpeed(const vector<double>& vSpeed, QPainter *painter);
    // Paints speed from the provided vector to the painter.

    void PaintSpeedLines(QPainter *painter);
    // Paints the speed line. Speed is calculated differently depending on the type of
    // of stimulus (normal or 360 video).

    bool IsArff360();
    // Checks the relation of the ARFF and returns true if the recording was in
    // 360 degrees equirectangular.

    void NormalizeSpeed(vector<double> &vSpeed);
    // Removes extreme speed values and converts speed to a non linear representation.

    double TransformPercentage(double perc);
    // Expands the range for lower speed values to make them more visible.
};
