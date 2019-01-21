// EquirectangularToFovVideo.cu

#include "EquirectangularToFovVideo.h"
#include "Util.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

#define PI 3.14159265

using namespace std;

// PUBLIC:

EquirectangularToFovVideo::EquirectangularToFovVideo(Arff* pArff) : EquirectangularToFovBase(pArff), eq_d{nullptr}, fov_d{nullptr}, m_pInfo{new ConvertionInfo()}
{
}

/*virtual*/ EquirectangularToFovVideo::~EquirectangularToFovVideo()
{
    if (eq_d != nullptr)
        cudaFree(eq_d);
    if (fov_d != nullptr)
        cudaFree(fov_d);
    delete m_pInfo;
}

// The following functions are the same as the ones provided from the base class
// but specialized to run on the device (GPU)

__device__ void EquirectangularToSpherical_d(unsigned int xEq, unsigned int yEq, unsigned int widthPx, unsigned int heightPx, double *horRads, double *verRads)
{
    *horRads = (xEq * 2.0 * PI) / widthPx;
    *verRads = (yEq * PI) / heightPx;
}

__device__ void SphericalToEquirectangular_d(double horRads, double verRads, unsigned int widthPx, unsigned int heightPx, unsigned int *xEq, unsigned int *yEq)
{
    int x = (int)((horRads / (2.0 * PI)) * widthPx + 0.5); // round double to closer int
    int y = (int)((verRads / PI) * heightPx + 0.5);

    // make sure returned values are within the video
    if (x < 0)
        x = widthPx + x - 1;
    else if (x >= (int)widthPx)
        x -= widthPx;
    *xEq = (unsigned int) x;

    if (y < 0)
        y = heightPx + y - 1;

    if (y >= (int)heightPx)
        y = 2 * heightPx - y - 1;

    *yEq = (unsigned int) y;
}

__device__ void SphericalToCartesian_d(double horRads, double verRads, Vec3 *cart)
{
    cart->x = sin(verRads)*cos(horRads);
    cart->y = cos(verRads);
    cart->z = sin(verRads)*sin(horRads);
}

__device__ void CartesianToSpherical_d(Vec3 cart, double *horRads, double *verRads)
{
    *horRads = atan2(cart.z, cart.x);
    *verRads = acos(cart.y);
}

__device__ Vec3 RotatePoint_d(Matrix33 rot, Vec3 v)
{
    //Vec3 res(0,0,0);
    Vec3 res = v; // Avoid __device__ constructor by using copy constructor

    res.x = rot.mat[0][0]*v.x + rot.mat[0][1]*v.y + rot.mat[0][2]*v.z;
    res.y = rot.mat[1][0]*v.x + rot.mat[1][1]*v.y + rot.mat[1][2]*v.z;
    res.z = rot.mat[2][0]*v.x + rot.mat[2][1]*v.y + rot.mat[2][2]*v.z;

    return res;
}

__global__ void GPUCalculation(const uchar *eq_d, uchar *fov_d, ConvertionInfo *info)
{
    double fovWidth_rads = (info->fovWidth_deg * PI / 180);
    double fovHeight_rads = (info->fovHeight_deg * PI / 180);

    double horRads, verRads;
    double vidHorRads, vidVerRads;
    unsigned int xEq, yEq;

    int idx = blockIdx.x*blockDim.x + threadIdx.x;
    int idy = blockIdx.y*blockDim.y + threadIdx.y;

    int x = idx;
    int y = idy;

    if (x >= info->fovWidth_px || y >= info->fovHeight_px)
        return;

    horRads =  x * fovWidth_rads / info->fovWidth_px - fovWidth_rads / 2.0;
    verRads = y * fovHeight_rads / info->fovHeight_px - fovHeight_rads / 2.0;
    // make it point towards center of equirectangular projection
    horRads += PI;
    verRads += PI/2;
    //Vec3 pixelVec(0,0,0);
    Vec3 pixelVec = info->tmpVec; // Avoid __device__ constructor by using copy constructor
    SphericalToCartesian_d(horRads, verRads, &pixelVec);

    Vec3 vidPixelVec = RotatePoint_d(info->rot, pixelVec);
    CartesianToSpherical_d(vidPixelVec, &vidHorRads, &vidVerRads);
    SphericalToEquirectangular_d(vidHorRads, vidVerRads, info->eqWidth_px, info->eqHeight_px, &xEq, &yEq);

	int posEq = yEq*info->eqWidth_px*4 + xEq*4;
    int posFov = y*info->fovWidth_px*4 + x*4;
    *(fov_d + posFov) = *(eq_d + posEq);
    *(fov_d + posFov + 1) = *(eq_d + posEq + 1);
    *(fov_d + posFov + 2) = *(eq_d + posEq + 2);
    *(fov_d + posFov + 3) = *(eq_d + posEq + 3);
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

    const uchar *eqImageBits = eqImage->bits();
    uchar *fovImageBits = fovImage->bits();

    GenerateSampling(fovImage);

    // Set up GPU for calculation
    ConvertionInfo *info = new ConvertionInfo();
    ConvertionInfo *info_d;
    m_pInfo->rot = rot;
    m_pInfo->fovWidth_deg = m_fovWidthDeg;
    m_pInfo->fovHeight_deg = m_fovHeightDeg;
    m_pInfo->fovWidth_px = fovImage->width();
    m_pInfo->fovHeight_px = fovImage->height();
    m_pInfo->eqWidth_px = eqImage->width();
    m_pInfo->eqHeight_px = eqImage->height();

    if (eq_d == nullptr)
        cudaMalloc((void**)&eq_d, eqImage->byteCount());
    if (fov_d == nullptr)
        cudaMalloc((void**)&fov_d, fovImage->byteCount());
    cudaMalloc((void**)&info_d, sizeof(ConvertionInfo)); 
    cudaMemcpy(eq_d, eqImageBits, eqImage->byteCount(), cudaMemcpyHostToDevice);
    cudaMemcpy(fov_d, fovImageBits, fovImage->byteCount(), cudaMemcpyHostToDevice);
    cudaMemcpy(info_d, m_pInfo, sizeof(ConvertionInfo), cudaMemcpyHostToDevice);

    int device;
    cudaGetDevice(&device);
    cudaDeviceProp devProp;
    cudaGetDeviceProperties(&devProp, device);
    int maxThreads = devProp.maxThreadsPerBlock;
    int maxThreadsDim = floor(sqrt(maxThreads));

    dim3 dimBlock(maxThreadsDim, maxThreadsDim);
    dim3 dimGrid(fovImage->width()/dimBlock.x + 1, fovImage->height()/dimBlock.y + 1);

    GPUCalculation<<<dimGrid,dimBlock>>>(eq_d, fov_d, info_d);

    cudaMemcpy(fovImageBits, fov_d, fovImage->byteCount(), cudaMemcpyDeviceToHost);
    cudaFree(info_d);

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
