// EquirectangularToFov.h
// equirectangular to field of view conversion

#ifndef __EQUIRECTANGULARTOFOVBASE_H__
#define __EQUIRECTANGULARTOFOVBASE_H__

#include "../arffHelper/Arff.h"
#include "Types.h"

#include <vector>

using namespace std;

class EquirectangularToFovBase
{
    //Q_OBJECT

public:
    EquirectangularToFovBase(Arff* pArff);
    virtual ~EquirectangularToFovBase() = 0;

protected:
    void EquirectangularToSpherical(unsigned int xEq, unsigned int yEq, unsigned int widthPx, unsigned int heightPx, double *horRads, double *verRads);
    ///< Converts the provided equirectangular coordinates to angles i.e. spherical representation.

    void SphericalToEquirectangular(double horRads, double verRads, unsigned int widthPx, unsigned int heightPx, unsigned int *xEq, unsigned int *yEq);
    ///< Converts spherical coordinates to pixels in the equirectangular representation.

    void SphericalToCartesian(double horRads, double verRads, Vec3 *cart);
    ///< Converts a unit vector from spherical to cartesian coordinates.
    ///< \p horRads is the angle in the between X axis and the vector in the XZ plane
    ///< \p verRads is teh angle between the vector and the Y axis.

    void CartesianToSpherical(Vec3 cart, double *horRads, double *verRads);
    ///< Converts a cartesian vector to unit spherical vector.

    void GetHeadPos(long int time, double *x, double *y, double *tilt);
    ///< Retrieve the head position.

    void GetHeadPos(unsigned int ind, double *x, double *y, double *tilt);

    void GetEyePos(unsigned int ind, double *x, double *y);

    Matrix33 HeadToVideoRotation(Vec3 head, double headTiltRads, Vec3 video);

    Matrix33 YZXrotation(Vec3 vec, double tiltRads);

    Arff        *m_pArff;
    int         m_horHeadInd;
    int         m_verHeadInd;
    int         m_tiltHeadInd;
    int         m_timeInd;
    int         m_xInd;
    int         m_yInd;
    int         m_confInd;
    int         m_fovWidthPx;
    int         m_fovHeightPx;
    double      m_fovWidthDeg;
    double      m_fovHeightDeg;

    vector<double> m_vHorSampling;
    vector<double> m_vVerSampling;
};


#endif /*__EQUIRECTANGULARTOFOV_H__*/
