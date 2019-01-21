// EquirectangularToFovGaze.h
// Conversion of gaze from equirectangular coordinates to FOV coordinates

#ifndef __EQUIRECTANGULARTOFOVGAZE_H__
#define __EQUIRECTANGULARTOFOVGAZE_H__

#include "EquirectangularToFovBase.h"

#include <memory>

using namespace std;

class EquirectangularToFovGaze : public EquirectangularToFovBase
{
public:
    EquirectangularToFovGaze(Arff *pArff);
    virtual ~EquirectangularToFovGaze();
    
    unique_ptr<Arff> Convert();
    ///< Cnverts the gaze of the input ARFF object to the 
    ///< field of view and stores it to a new ARFF object. A pointer to this
    ///< object is returned.

private:
    void SphericalToFov(double horRads, double verRads, double *x, double *y);
};

#endif /*__EQUIRECTANGULARTOFOVGAZE_H__*/
