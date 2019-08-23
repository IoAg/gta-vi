// EquirectangularToFovSpeed.h
//
// Calculation of eye within head, head and head+eye speeds. These are absolute
// values in degrees per second

#ifndef __EQUIRECTANGULARTOFOVSPEED_H__
#define __EQUIRECTANGULARTOFOVSPEED_H__

#include "EquirectangularToFovBase.h"

using namespace std;

class EquirectangularToFovSpeed : public EquirectangularToFovBase
{
public:
    EquirectangularToFovSpeed(Arff *pArff, double integrationPeriod=1);
    ///< \p integrationPeriod is the duration that is used for the calculation of the 
    ///< step that is used between samples for speed calculation. It acts
    ///< as simple filtering.

    virtual ~EquirectangularToFovSpeed();

    void AddSpeed();
    ///< Adds three new attributes to ARFF representing speed in deg/s for eye+head,
    ///< eye within head (fov) and head only.

    vector<double> GetHeadSpeed();

    vector<double> GetEyeFovSpeed();

    vector<double> GetHeadPlusEyeSpeed();


private:
    void GetSpeed(const vector<Vec3> &inVec, const int attInd);

    vector<double> GetSpeed(const vector<Vec3> &inVec);

    void FillVectors();

    void SphericalToFov(double horRads, double verRads, double *x, double *y);

    vector<Vec3>    m_vEyeFov;
    vector<Vec3>    m_vHead;
    vector<Vec3>    m_vHeadEye;
    vector<double>  m_vXfov;
    vector<double>  m_vYfov;

    int             m_step;
};

#endif /*__EQUIRECTANGULARTOFOVSPEED_H__*/
