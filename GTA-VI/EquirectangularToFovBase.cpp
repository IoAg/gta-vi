// EquirectangularToFovBase.cpp

#include "EquirectangularToFovBase.h"
#include "Types.h"
#include "Util.h"
#include "../arffHelper/ArffUtil.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <cassert>

#define PI 3.14159265
#define EPSILON 0.001


using namespace std;

const char *c_horHeadAttName = "x_head";
const char *c_verHeadAttName = "y_head";
const char *c_tiltHeadAttName = "angle_deg_head";

const char *c_fovWidthDegName = "fov_width_deg";
const char *c_fovHeightDegName = "fov_height_deg";
const char *c_fovWidthPxName = "fov_width_px";
const char *c_fovHeightPxName = "fov_height_px";

// PUBLIC:

EquirectangularToFovBase::EquirectangularToFovBase(Arff* pArff)
{
    if (pArff == NULL)
    {
        cout << "ERROR: you provided NULL pointer. Could not initialize." << endl;
        exit(-1);
    }
    m_pArff = pArff;
    bool res = ArffUtil::GetTXYCindex(m_pArff, m_timeInd, m_xInd, m_yInd, m_confInd);
    if (!res)
    {
        cout << "ERROR: could not find time or x or y or confidence in the provided ARFF file." << endl;
        exit(-1);
    }

    // get experiment values
    res = true;
    res &= m_pArff->GetAttIndex(c_horHeadAttName, m_horHeadInd);
    res &= m_pArff->GetAttIndex(c_verHeadAttName, m_verHeadInd);
    res &= m_pArff->GetAttIndex(c_tiltHeadAttName, m_tiltHeadInd);

    if (!res)
    {
        cout << "ERROR: could not find " << c_horHeadAttName << " or " << c_verHeadAttName <<  " or " << c_tiltHeadAttName << " in the provided ARFF file." << endl;
        exit(-1);
    }

    string value;
    if(!m_pArff->GetMetadata(c_fovWidthDegName, value))
    {
        cout << "ERROR: METADATA " << c_fovWidthDegName << " is missing" << endl;
        exit(-1);
    }
    m_fovWidthDeg = stod(value);
    if (!m_pArff->GetMetadata(c_fovHeightDegName, value))
    {
        cout << "ERROR: METADATA " << c_fovHeightDegName << " is missing" << endl;
        exit(-1);
    }
    m_fovHeightDeg = stod(value);
    if (!m_pArff->GetMetadata(c_fovWidthPxName, value))
    {
        cout << "ERROR: METADATA " << c_fovWidthPxName << " is missing" << endl;
        exit(-1);
    }
    m_fovWidthPx = stoi(value);
    if (!m_pArff->GetMetadata(c_fovHeightPxName, value))
    {
        cout << "ERROR: METADATA " << c_fovHeightPxName << " is missing" << endl;
        exit(-1);
    }
    m_fovHeightPx = stoi(value);
}

/*virtual*/ EquirectangularToFovBase::~EquirectangularToFovBase()
{
}

// PROTECTED:
void EquirectangularToFovBase::EquirectangularToSpherical(unsigned int xEq, unsigned int yEq, unsigned int widthPx, unsigned int heightPx, double *horRads, double *verRads)
{
    *horRads = (xEq * 2.0 * PI) / widthPx;
    *verRads = (yEq * PI) / heightPx;
}

void EquirectangularToFovBase::SphericalToEquirectangular(double horRads, double verRads, unsigned int widthPx, unsigned int heightPx, unsigned int *xEq, unsigned int *yEq)
{
    int x = (int)((horRads / (2.0 * PI)) * widthPx + 0.5); // round double to closer int
    int y = (int)((verRads / PI) * heightPx + 0.5);

    // make sure returned values are within the video
    if (y < 0)
    {
        y = -y;
        x += widthPx / 2;
    }

    if (y >= (int)heightPx)
    {
        y = 2 * heightPx - y - 1;
        x += widthPx / 2;
    }
    *yEq = (unsigned int) y;

    if (x < 0)
        x = widthPx + x - 1;
    else if (x >= (int)widthPx)
        x -= widthPx;

    *xEq = (unsigned int) x;
}

void EquirectangularToFovBase::SphericalToCartesian(double horRads, double verRads, Vec3 *cart)
{
    cart->x = sin(verRads)*cos(horRads);
    cart->y = cos(verRads);
    cart->z = sin(verRads)*sin(horRads);
}

void EquirectangularToFovBase::CartesianToSpherical(Vec3 cart, double *horRads, double *verRads)
{
    *horRads = atan2(cart.z, cart.x);
    *verRads = acos(cart.y);
}

void EquirectangularToFovBase::GetHeadPos(long int time, double *x, double *y, double *tilt)
{
    unsigned int ind = ArffUtil::FindPosition(m_pArff, m_timeInd, time);
    GetHeadPos(ind, x, y, tilt);
}

void EquirectangularToFovBase::GetHeadPos(unsigned int ind, double *x, double *y, double *tilt)
{
    *x = (*m_pArff)[ind][m_horHeadInd];
    *y = (*m_pArff)[ind][m_verHeadInd];
    *tilt = (*m_pArff)[ind][m_tiltHeadInd];
}

void EquirectangularToFovBase::GetEyePos(unsigned int ind, double *x, double *y)
{
    *x = (*m_pArff)[ind][m_xInd];
    *y = (*m_pArff)[ind][m_yInd];
}

Matrix33 EquirectangularToFovBase::HeadToVideoRotation(Vec3 head, double headTiltRads, Vec3 video)
{
    Matrix33 headToRef = YZXrotation(head, -headTiltRads);
    Matrix33 videoToRef = YZXrotation(video, 0);

    Matrix33 ret = videoToRef*Transpose(headToRef);
    return ret;
}

Matrix33 EquirectangularToFovBase::YZXrotation(Vec3 vec, double tiltRads)
{
    // we follow the Trait-Bryan angles with Y-Z'-X" intrinsic rotations
    double theta = asin(vec.y);

    double psi = 0;
    if (abs(theta) < PI/2.0 - 0.01)
        psi = atan2(vec.z, vec.x);

    /*
    Matrix33 Yrot;
    Yrot.mat[0][0] = cos(psi);
    Yrot.mat[0][1] = 0;
    Yrot.mat[0][2] = sin(psi);
    Yrot.mat[1][0] = 0;
    Yrot.mat[1][1] = 1;
    Yrot.mat[1][2] = 0;
    Yrot.mat[2][0] = -sin(psi);
    Yrot.mat[2][1] = 0;
    Yrot.mat[2][2] = cos(psi);

    Matrix33 Zrot;
    Zrot.mat[0][0] = cos(theta);
    Zrot.mat[0][1] = -sin(theta);
    Zrot.mat[0][2] = 0;
    Zrot.mat[1][0] = sin(theta);
    Zrot.mat[1][1] = cos(theta);
    Zrot.mat[1][2] = 0;
    Zrot.mat[2][0] = 0;
    Zrot.mat[2][1] = 0;
    Zrot.mat[2][2] = 1;

    Matrix33 Xrot;
    Xrot.mat[0][0] = 1;
    Xrot.mat[0][1] = 0;
    Xrot.mat[0][2] = 0;
    Xrot.mat[1][0] = 0;
    Xrot.mat[1][1] = cos(tiltRads);
    Xrot.mat[1][2] = -sin(tiltRads);
    Xrot.mat[2][0] = 0;
    Xrot.mat[2][1] = sin(tiltRads);
    Xrot.mat[2][2] = cos(tiltRads);

    Matrix33 ret = Xrot*Zrot*Yrot;
    */

    Matrix33 ret;
    ret.mat[0][0] = cos(theta)*cos(psi);
    ret.mat[0][1] = -sin(theta);
    ret.mat[0][2] = cos(theta)*sin(psi);
    ret.mat[1][0] = cos(tiltRads)*sin(theta)*cos(psi) + sin(tiltRads)*sin(psi);
    ret.mat[1][1] = cos(tiltRads)*cos(theta);
    ret.mat[1][2] = cos(tiltRads)*sin(theta)*sin(psi) - sin(tiltRads)*cos(psi);
    ret.mat[2][0] = sin(tiltRads)*sin(theta)*cos(psi) - cos(tiltRads)*sin(psi);
    ret.mat[2][1] = sin(tiltRads)*cos(theta);
    ret.mat[2][2] = sin(tiltRads)*sin(theta)*sin(psi) + cos(tiltRads)*cos(psi);
    return ret;

}
