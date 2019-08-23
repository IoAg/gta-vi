// GazeSpeed.h
//
// This class calculates the speed for the provided ARFF file

#ifndef __GAZESPEED_H__
#define __GAZESPEED_H__

#include "../arffHelper/Arff.h"

#include <vector>

class GazeSpeed
{
public:
    GazeSpeed(Arff *pArff, double integrationPeriod=1);

    vector<double> GetSpeed();

private:
    void FillVectors();

    Arff *m_pArff;

    vector<double>  m_vXdeg;
    vector<double>  m_vYdeg;

    int m_step;
    int m_timeInd;
};
#endif /*__GAZESPEED_H__*/
