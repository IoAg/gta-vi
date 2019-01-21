// EquirectangularToFovGaze.cpp

#include "EquirectangularToFovGaze.h"
#include "Util.h"

#define PI 3.14159265

using namespace std;

EquirectangularToFovGaze::EquirectangularToFovGaze(Arff *pArff) : 
    EquirectangularToFovBase(pArff)
{
}

/*virtual*/ EquirectangularToFovGaze::~EquirectangularToFovGaze()
{
}

unique_ptr<Arff> EquirectangularToFovGaze::Convert()
{
    unique_ptr<Arff> convertedArff(new Arff());
    convertedArff->SetWidthPx(m_fovWidthPx);
    convertedArff->SetHeightPx(m_fovHeightPx);
    convertedArff->AddColumn("time", "integer");
    convertedArff->AddColumn("x", "numeric");
    convertedArff->AddColumn("y", "numeric");
    convertedArff->AddColumn("confidence", "numeric");
    int rows, columns;
    m_pArff->Size(rows, columns);

    //DataPoint iniPoint, convPoint;
    for (unsigned int ind=0; ind < (unsigned int)rows; ind++)
    {
        double xHead, yHead, tiltHead;
        GetHeadPos(ind, &xHead, &yHead, &tiltHead);
        double horHeadRads, verHeadRads;
        EquirectangularToSpherical(xHead, yHead, m_pArff->WidthPx(), m_pArff->HeightPx(), &horHeadRads, &verHeadRads);
        Vec3 headVec(0,0,0);
        SphericalToCartesian(horHeadRads, verHeadRads, &headVec);

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

        Vec3 rotGazeVec = RotatePoint(rot, GazeVec);
        double rotHorRads, rotVerRads;
        CartesianToSpherical(rotGazeVec, &rotHorRads, &rotVerRads);

        double xFov, yFov;
        SphericalToFov(rotHorRads, rotVerRads, &xFov, &yFov);
        vector<double> row;
        row.push_back((*m_pArff)[ind][m_timeInd]);
        row.push_back(xFov);
        row.push_back(yFov);
        row.push_back((*m_pArff)[ind][m_confInd]);
        convertedArff->AddRow(row);
    }

    return move(convertedArff);
}

// PRIVATE:
void EquirectangularToFovGaze::SphericalToFov(double horRads, double verRads, double *x, double *y)
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
