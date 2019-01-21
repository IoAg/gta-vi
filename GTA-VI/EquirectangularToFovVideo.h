// EquirectangularToFovVideo.h
// equirectangular to field of view conversion for videos/images

#ifndef __EQUIRECTANGULARTOFOVVIDEO_H__
#define __EQUIRECTANGULARTOFOVVIDEO_H__

#include "EquirectangularToFovBase.h"

#include <QImage>

using namespace std;

#ifdef USE_CUDA
struct ConvertionInfo
{
    double fovWidth_deg;
    double fovHeight_deg;
    int fovWidth_px;
    int fovHeight_px;
    int eqWidth_px;
    int eqHeight_px;

    Matrix33 rot;
    Vec3 tmpVec;

    ConvertionInfo(): tmpVec{Vec3(0,0,0)} {}
};
#endif

class EquirectangularToFovVideo : public EquirectangularToFovBase
{
public:
    EquirectangularToFovVideo(Arff* pArff);
    virtual ~EquirectangularToFovVideo();

    bool Convert(const QImage *eqImage, long int time, QImage *fovImage);

    double GetAspectRatio();
    ///< Return the aspect ratio of the FOV. Aspect ratio = width_fov/height_fov

private:
    void GenerateSampling(const QImage *image);
    ///< This function generates the sampling on the sphere for the given image size

    void Generate1DSampling(double fovRads, vector<double> *samples);
    ///< For the fiven field of view (rads) populates the provided vector with
    ///< the correct sampling angles. The samples are placed of the periphery 
    ///< of the circle. This means that every sample represents the same angle

    vector<double> m_vHorSampling;
    vector<double> m_vVerSampling;

#ifdef USE_CUDA
    uchar *eq_d;
    uchar *fov_d;
    ConvertionInfo *m_pInfo;
#endif
};

#endif /*__EQUIRECTANGULARTOFOVVIDEO_H__*/
