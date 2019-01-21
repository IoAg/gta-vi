// Util.cpp

#include "Util.h"
#include <cmath>
#include <cstdlib>

#define PI 3.14159265

using namespace std;

Vec3 operator/(Vec3 v, double scalar)
{
    return Vec3(v.x/scalar, v.y/scalar, v.z/scalar);
}

Vec3 operator+(Vec3 v1, Vec3 v2)
{
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

bool operator==(Vec3 v1, Vec3 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

double Magnitude(Vec3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vec3 Normalize(Vec3 v)
{
    return v/Magnitude(v);
}

Vec3 Perpendicular(Vec3 v)
{
    int range = 100;
    v = Normalize(v);
    if (v.x == 0)
        return Vec3(1,0,0);
    if (v.y == 0)
        return Vec3(0,1,0);
    if (v.z == 0)
        return Vec3(0,0,1);

    Vec3 res(0,0,0);
    res.x = (double)(rand() % (2*range) - range)/range;
    res.y = (double)(rand() % (2*range) - range)/range;
    res.z = -(v.x*res.x + v.y*res.y)/v.z;

    return res;
}

double DotProd(Vec3 v1, Vec3 v2)
{
    double res = 0.0;
    res += v1.x*v2.x;
    res += v1.y*v2.y;
    res += v1.z*v2.z;

    return res;
}

Vec3 CrossProd(Vec3 v1, Vec3 v2)
{
    Vec3 res{0, 0, 0};

    res.x = v1.y*v2.z - v2.y*v1.z;
    res.y = -v1.x*v2.z + v2.x*v1.z;
    res.z = v1.x*v2.y - v2.x*v1.y;

    return res;
}

Matrix33 operator*(Matrix33 left, Matrix33 right)
{
    Matrix33 res;
    res.mat[0][0] = left.mat[0][0]*right.mat[0][0] + left.mat[0][1]*right.mat[1][0] + left.mat[0][2]*right.mat[2][0];
    res.mat[0][1] = left.mat[0][0]*right.mat[0][1] + left.mat[0][1]*right.mat[1][1] + left.mat[0][2]*right.mat[2][1];
    res.mat[0][2] = left.mat[0][0]*right.mat[0][2] + left.mat[0][1]*right.mat[1][2] + left.mat[0][2]*right.mat[2][2];

    res.mat[1][0] = left.mat[1][0]*right.mat[0][0] + left.mat[1][1]*right.mat[1][0] + left.mat[1][2]*right.mat[2][0];
    res.mat[1][1] = left.mat[1][0]*right.mat[0][1] + left.mat[1][1]*right.mat[1][1] + left.mat[1][2]*right.mat[2][1];
    res.mat[1][2] = left.mat[1][0]*right.mat[0][2] + left.mat[1][1]*right.mat[1][2] + left.mat[1][2]*right.mat[2][2];

    res.mat[2][0] = left.mat[2][0]*right.mat[0][0] + left.mat[2][1]*right.mat[1][0] + left.mat[2][2]*right.mat[2][0];
    res.mat[2][1] = left.mat[2][0]*right.mat[0][1] + left.mat[2][1]*right.mat[1][1] + left.mat[2][2]*right.mat[2][1];
    res.mat[2][2] = left.mat[2][0]*right.mat[0][2] + left.mat[2][1]*right.mat[1][2] + left.mat[2][2]*right.mat[2][2];

    return res;
}

Matrix33 Transpose(Matrix33 in)
{
    Matrix33 res;
    res.mat[0][0] = in.mat[0][0];
    res.mat[0][1] = in.mat[1][0];
    res.mat[0][2] = in.mat[2][0];

    res.mat[1][0] = in.mat[0][1];
    res.mat[1][1] = in.mat[1][1];
    res.mat[1][2] = in.mat[2][1];

    res.mat[2][0] = in.mat[0][2];
    res.mat[2][1] = in.mat[1][2];
    res.mat[2][2] = in.mat[2][2];

    return res;
}

Matrix33 Rodrigues(Vec3 v, double angle)
{
    Matrix33 rot;
    rot.mat[0][0] = (1-cos(angle))*v.x*v.x + cos(angle);
    rot.mat[0][1] = (1-cos(angle))*v.x*v.y - sin(angle)*v.z;
    rot.mat[0][2] = (1-cos(angle))*v.x*v.z + sin(angle)*v.y;
    rot.mat[1][0] = (1-cos(angle))*v.y*v.x + sin(angle)*v.z;
    rot.mat[1][1] = (1-cos(angle))*v.y*v.y + cos(angle);
    rot.mat[1][2] = (1-cos(angle))*v.y*v.z - sin(angle)*v.x;
    rot.mat[2][0] = (1-cos(angle))*v.z*v.x - sin(angle)*v.y;
    rot.mat[2][1] = (1-cos(angle))*v.z*v.y + sin(angle)*v.x;
    rot.mat[2][2] = (1-cos(angle))*v.z*v.z + cos(angle);

    return rot;
}

Matrix33 RotationVecVec(Vec3 v1, Vec3 v2)
{
    // This function finds the rotation from one vector to the other. The process
    // is the following:
    // a. Normalize vectors
    // b. Middle direction
    // c. Normalize middle vector
    // d. Use middle vector in Rodrigues' formula with rotation of pi

    v1 = Normalize(v1);
    v2 = Normalize(v2);

    Vec3 midVec(0,0,0);
    if (v1+v2 == Vec3(0,0,0)) // vectors are opposite
        midVec = Perpendicular(v1);
    else
        midVec = v1+v2;

    midVec = Normalize(midVec);

    // for pi rotation cos(pi) = -1, sin(pi) = 0
    Matrix33 rot = Rodrigues(midVec, PI);
    //rot.mat[0][0] = 2*midVec.x*midVec.x - 1;
    //rot.mat[0][1] = 2*midVec.x*midVec.y;
    //rot.mat[0][2] = 2*midVec.x*midVec.z;
    //rot.mat[1][0] = 2*midVec.y*midVec.x;
    //rot.mat[1][1] = 2*midVec.y*midVec.y - 1;
    //rot.mat[1][2] = 2*midVec.y*midVec.z;
    //rot.mat[2][0] = 2*midVec.z*midVec.x;
    //rot.mat[2][1] = 2*midVec.z*midVec.y;
    //rot.mat[2][2] = 2*midVec.z*midVec.z - 1;

    return rot;
}

Vec3 RotatePoint(Matrix33 rot, Vec3 v)
{
    Vec3 res(0,0,0);

    res.x = rot.mat[0][0]*v.x + rot.mat[0][1]*v.y + rot.mat[0][2]*v.z;
    res.y = rot.mat[1][0]*v.x + rot.mat[1][1]*v.y + rot.mat[1][2]*v.z;
    res.z = rot.mat[2][0]*v.x + rot.mat[2][1]*v.y + rot.mat[2][2]*v.z;

    return res;
}
