// EquirectangularToFovSpeed.cpp

#include "EquirectangularToFovSpeed.h"
#include "Util.h"
#include "../arffHelper/ArffUtil.h"

#include <cassert>
#include <cmath>

#define USECS_TO_SECS 1000000.0
#define PI 3.14159265
#define DELTA 0.00001

using namespace std;

EquirectangularToFovSpeed::EquirectangularToFovSpeed(Arff *pArff, double integrationPeriod) :
    EquirectangularToFovBase(pArff)
{
    double samplingPeriod = ArffUtil::GetSamplingPeriod(m_pArff);
    m_step = ceil(integrationPeriod / samplingPeriod);
    m_step = m_step<1 ? 1 : m_step;
    
    FillVectors();
}

/*virtual*/ EquirectangularToFovSpeed::~EquirectangularToFovSpeed()
{
}

void EquirectangularToFovSpeed::AddSpeed()
{
    int rows, columns;
    m_pArff->Size(rows, columns);
    // add x,y within FOV
    m_pArff->AddColumn("x_fov", "numeric");
    for (int r=0; r<rows; r++)
        (*m_pArff)[r][columns] = m_vXfov[r];
    columns++;

    m_pArff->AddColumn("y_fov", "numeric");
    for (int r=0; r<rows; r++)
        (*m_pArff)[r][columns] = m_vYfov[r];
    columns++;

    // add speed attributes
    string eyeFovAttName = "eye_fov_speed_" + to_string(m_step);
    m_pArff->AddColumn(eyeFovAttName, "numeric");
    GetSpeed(m_vEyeFov, columns);
    columns++;

    string headAttName = "head_speed_" + to_string(m_step);
    m_pArff->AddColumn(headAttName, "numeric");
    GetSpeed(m_vHead, columns);
    columns++;

    string headEyeAttName = "head_plus_eye_speed_" + to_string(m_step);
    m_pArff->AddColumn(headEyeAttName, "numeric");
    GetSpeed(m_vHeadEye, columns);
}

vector<double> EquirectangularToFovSpeed::GetHeadSpeed()
{
    return GetSpeed(m_vHead);
}

vector<double> EquirectangularToFovSpeed::GetEyeFovSpeed()
{
    return GetSpeed(m_vEyeFov);
}

vector<double> EquirectangularToFovSpeed::GetHeadPlusEyeSpeed()
{
    return GetSpeed(m_vHeadEye);
}

// PRIVATE:

void EquirectangularToFovSpeed::GetSpeed(const vector<Vec3> &inVec, const int attInd)
{
    vector<double> speed = GetSpeed(inVec);

    for (unsigned int ind=0; ind<speed.size(); ind++)
        (*m_pArff)[ind][attInd] = speed[ind];
}

vector<double> EquirectangularToFovSpeed::GetSpeed(const vector<Vec3> &inVec)
{
    int rows, columns;
    m_pArff->Size(rows, columns);

    assert(inVec.size() == (unsigned int) rows);
    vector<double> res;
    res.resize(rows);

    for (int ind=m_step; ind<rows; ind++)
    {
        double dotProd = DotProd(inVec[ind], inVec[ind-m_step]);
        // even though vectors are nomalized rounding errors can push the dot product slightly above 1
        dotProd = dotProd > 1.0 && dotProd < 1.0 + DELTA? 1.0: dotProd;
        double rads = acos(dotProd);
        double degs = rads * 180 / PI;
        double time = (double)((*m_pArff)[ind][m_timeInd] - (*m_pArff)[ind-m_step][m_timeInd]) / USECS_TO_SECS;
        double degsSec = degs/time;
        assert(degsSec==degsSec); // check for NaN values

        //res[ind] = degsSec;
        res[ind - m_step/2] = degsSec;
    }

    return res;
}

void EquirectangularToFovSpeed::FillVectors()
{
    m_vHead.clear();
    m_vHeadEye.clear();
    m_vEyeFov.clear();

    int rows, columns;
    m_pArff->Size(rows, columns);
    for (unsigned int ind=0; ind < (unsigned int) rows; ind++)
    {
        double xHead, yHead, tiltHead;
        GetHeadPos(ind, &xHead, &yHead, &tiltHead);
        double horHeadRads, verHeadRads;
        EquirectangularToSpherical(xHead, yHead, m_pArff->WidthPx(), m_pArff->HeightPx(), &horHeadRads, &verHeadRads);
        Vec3 headVec(0,0,0);
        SphericalToCartesian(horHeadRads, verHeadRads, &headVec);
        m_vHead.push_back(headVec);

		Vec3 vidVec(-1,0,0); // middle of the video is the center
        double tiltHeadRads = tiltHead * PI / 180;
        Matrix33 rot = HeadToVideoRotation(headVec, tiltHeadRads, vidVec);
        rot = Transpose(rot);
        
		// Convert gaze to FOV coordinates
        double xGaze, yGaze;
        GetEyePos(ind, &xGaze, &yGaze);
        double horGazeRads, verGazeRads;
        EquirectangularToSpherical(xGaze, yGaze, m_pArff->WidthPx(), m_pArff->HeightPx(), &horGazeRads, &verGazeRads);
        Vec3 GazeVec(0,0,0);
        SphericalToCartesian(horGazeRads, verGazeRads, &GazeVec);
		m_vHeadEye.push_back(GazeVec);

        Vec3 GazeWithinHeadVec = RotatePoint(rot, GazeVec);
		m_vEyeFov.push_back(GazeWithinHeadVec);
		
		double rotHorRads, rotVerRads;
        CartesianToSpherical(GazeWithinHeadVec, &rotHorRads, &rotVerRads);

        double xFov, yFov;
        SphericalToFov(rotHorRads, rotVerRads, &xFov, &yFov);
        m_vXfov.push_back(xFov);
        m_vYfov.push_back(yFov);
    }
}

// PRIVATE:
void EquirectangularToFovSpeed::SphericalToFov(double horRads, double verRads, double *x, double *y)
{
    // Change them to [0, 2*pi) and [0, pi) range
    horRads = horRads < 0? 2*PI + horRads: horRads;
    verRads = verRads < 0? PI + verRads: verRads;
    // convert them from the middle of the video to the start of the coordinates
    horRads -= PI;
    verRads -= PI / 2.0;

    double fovHorRads = m_fovWidthDeg * PI / 180.0;
    double fovVerRads = m_fovHeightDeg * PI / 180.0;

    *x = m_fovWidthPx * (horRads + fovHorRads / 2.0) / fovHorRads;
    *y = m_fovHeightPx * (verRads + fovVerRads / 2.0) / fovVerRads;
}
