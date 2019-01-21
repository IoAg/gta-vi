// ArffWidgetCoord.h

#ifndef __PAINTWIDGET_H__
#define __PAINTWIDGET_H__

#include "ArffWidgetBase.h"

class ArffWidgetCoord : public ArffWidgetBase
{
public:
    ArffWidgetCoord(QWidget *parent=0);

protected:
    virtual void PaintLine(QPainter *painter);
    ///< Paints a coordinate line on the canvas.

    int m_plotAttInd;
};

class ArffWidgetCoordX : public ArffWidgetCoord
{
public:
    ArffWidgetCoordX(QWidget *parent=0);

    virtual void SetData(Arff &arff, int attToPaint, double maxValue=-1.0);

    virtual void SetFovData(Arff &arff, double maxValue);
};

class ArffWidgetCoordY : public ArffWidgetCoord
{
public:
    ArffWidgetCoordY(QWidget *parent=0);

    virtual void SetData(Arff &arff, int attToPaint, double maxValue=-1.0);

    virtual void SetFovData(Arff &arff, double maxValue);
};
#endif /*__PAINTWIDGET_H__*/
