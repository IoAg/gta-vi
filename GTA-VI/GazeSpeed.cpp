// GazeSpeed.cpp

#include "GazeSpeed.h"
#include "../arffHelper/ArffUtil.h"

#include <cmath>
#include <iostream>

#define USECS_TO_SECS 1000000.0
#define PI 3.14159265

const char *c_widthName = "width_mm";
const char *c_heightName = "height_mm";
const char *c_distanceName = "distance_mm";

using namespace std;

GazeSpeed::GazeSpeed(Arff *pArff, int step)
{
    m_pArff = pArff;
    m_step = step;

    FillVectors();
}

vector<double> GazeSpeed::GetSpeed()
{
    vector<double> speed;
    speed.resize(m_vXdeg.size());
    
    for (int ind=m_step; ind<(int)speed.size(); ind++)
    {
        double xDisp = pow(m_vXdeg[ind] - m_vXdeg[ind-m_step], 2);
        double yDisp = pow(m_vYdeg[ind] - m_vYdeg[ind-m_step], 2);

        double disp = sqrt(xDisp + yDisp);
        double time = ((*m_pArff)[ind][m_timeInd] - (*m_pArff)[ind-m_step][m_timeInd]) / USECS_TO_SECS;

        speed[ind - m_step/2] = disp / time;
    }

    return speed;
}

void GazeSpeed::FillVectors()
{
    double width_px = m_pArff->WidthPx();
    double height_px = m_pArff->HeightPx();

    string value;
	if(!m_pArff->GetMetadata(c_widthName, value))
    {
        cout << "ERROR: METADATA " << c_widthName << " is missing" << endl;
        exit(-1);
    }
    double width_mm = stod(value);

	if(!m_pArff->GetMetadata(c_heightName, value))
    {
        cout << "ERROR: METADATA " << c_heightName << " is missing" << endl;
        exit(-1);
    }
    double height_mm = stod(value);

	if(!m_pArff->GetMetadata(c_distanceName, value))
    {
        cout << "ERROR: METADATA " << c_distanceName << " is missing" << endl;
        exit(-1);
    }
    double distance_mm = stod(value);

    double thetaWidth = 2 * atan(width_mm/(2*distance_mm)) * 180/PI;
    double ppdx = width_px/thetaWidth;

    double thetaHeight = 2 * atan(height_mm/(2*distance_mm)) * 180/PI;
    double ppdy = height_px/thetaHeight;

    int xInd, yInd, confInd;
	bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, xInd, yInd, confInd);
    if (!res)
    {
        cout << "ERROR: could not find time or x or y or confidence in the provided ARFF file." << endl;
        exit(-1);
    }

    int rows, columns;
    m_pArff->Size(rows, columns);
    m_vXdeg.resize(rows);
    m_vYdeg.resize(rows);

    for (int ind=0; ind<rows; ind++)
    {
        m_vXdeg[ind] = (*m_pArff)[ind][xInd]/ppdx;
        m_vYdeg[ind] = (*m_pArff)[ind][yInd]/ppdy;
    }
}
