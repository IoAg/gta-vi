// EquirectangularToFovVideo.cpp

#include "EquirectangularToFovVideo.h"
#include "Types.h"
#include "Util.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#define PI 3.14159265

using namespace std;

// PUBLIC:

EquirectangularToFovVideo::EquirectangularToFovVideo(Arff* pArff) : EquirectangularToFovBase(pArff)
{
}

/*virtual*/ EquirectangularToFovVideo::~EquirectangularToFovVideo()
{
}

bool EquirectangularToFovVideo::Convert(const QImage *eqImage, long int time, QImage *fovImage)
{
    double xEqHead, yEqHead, tiltHead; 
    GetHeadPos(time, &xEqHead, &yEqHead, &tiltHead);

    double horHeadRads, verHeadRads;
    EquirectangularToSpherical(xEqHead, yEqHead, m_pArff->WidthPx(), m_pArff->HeightPx(), &horHeadRads, &verHeadRads);

    Vec3 headVec(0,0,0);
    SphericalToCartesian(horHeadRads, verHeadRads, &headVec);
    
    Vec3 vidVec(-1,0,0); // pointing to the middle of equirectangular projection
    double headTiltRads = tiltHead * PI / 180;

    Matrix33 rot = HeadToVideoRotation(headVec, headTiltRads, vidVec);

    double horRads, verRads;
    double vidHorRads, vidVerRads;
    unsigned int xEq, yEq;

    const uchar *eqImageBits = eqImage->bits();
    uchar *fovImageBits = fovImage->bits();

    GenerateSampling(fovImage);
    for (int y=0; y<fovImage->height(); y++)
    {
        for (int x=0; x<fovImage->width(); x++)
        {
            horRads = m_vHorSampling[x];
            verRads = m_vVerSampling[y];
            // make it point towards center of equirectangular projection
            horRads += PI;
            verRads += PI/2;
            Vec3 pixelVec(0,0,0);
            SphericalToCartesian(horRads, verRads, &pixelVec);

            Vec3 vidPixelVec = RotatePoint(rot, pixelVec);
            CartesianToSpherical(vidPixelVec, &vidHorRads, &vidVerRads);
            SphericalToEquirectangular(vidHorRads, vidVerRads, eqImage->width(), eqImage->height(), &xEq, &yEq);

            //fovImage->setPixel(x, y, eqImage->pixel(xEq, yEq));
			int posEq = yEq*eqImage->bytesPerLine() + xEq*4;
            int posFov = y*fovImage->bytesPerLine() + x*4;
            *(fovImageBits + posFov) = *(eqImageBits + posEq);
            *(fovImageBits + posFov + 1) = *(eqImageBits + posEq + 1);
            *(fovImageBits + posFov + 2) = *(eqImageBits + posEq + 2);
            *(fovImageBits + posFov + 3) = *(eqImageBits + posEq + 3);
        }
    }

    // *** Placeholder
    return true;
}

double EquirectangularToFovVideo::GetAspectRatio()
{
    return (double)m_fovWidthPx/m_fovHeightPx;
}

// PRIVATE:

void EquirectangularToFovVideo::GenerateSampling(const QImage *image)
{
    if ((int)m_vHorSampling.size() == image->width() && (int)m_vVerSampling.size() == image->height())
        return;

    m_vHorSampling.resize(image->width());
    m_vVerSampling.resize(image->height());

    double fovWidthRads = (m_fovWidthDeg * PI / 180);
    double fovHeightRads = (m_fovHeightDeg * PI / 180);

    Generate1DSampling(fovWidthRads, &m_vHorSampling);
    Generate1DSampling(fovHeightRads, &m_vVerSampling);
}

void EquirectangularToFovVideo::Generate1DSampling(double fovRads, vector<double> *samples)
{
    for (size_t i=0; i<samples->size(); i++)
        (*samples)[i] = i * fovRads / samples->size() - fovRads / 2.0;
}
