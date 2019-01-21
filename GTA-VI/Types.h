// Types.h

#ifndef __TYPES_H__
#define __TYPES_H__

#include <ostream>

using namespace std;

struct Matrix33
{
    double mat[3][3] = {};

    friend ostream& operator<<(ostream& os, Matrix33 const & m) 
    {
        return os << m.mat[0][0] << ", " << m.mat[0][1] << ", " << m.mat[0][2] << '\n'
                  << m.mat[1][0] << ", " << m.mat[1][1] << ", " << m.mat[1][2] << '\n'
                  << m.mat[2][0] << ", " << m.mat[2][1] << ", " << m.mat[2][2];
    }
};

struct Vec3
{
    double x=0;
    double y=0;
    double z=0;
  
    Vec3(double ix, double iy, double iz): x(ix), y(iy), z(iz) {}

    friend ostream& operator<<(ostream& os, Vec3 const & v) 
    {
        return os << v.x << ", " << v.y << ", " << v.z;
    }
};

#endif /*__TYPES_H__*/
