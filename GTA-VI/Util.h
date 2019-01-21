// Util.h

#ifndef __UTIL_H__
#define __UTIL_H__

#include "Types.h"

Vec3 operator/(Vec3 v, double scalar);
Vec3 operator+(Vec3 v1, Vec3 v2);
bool operator==(Vec3 v1, Vec3 v2);

double Magnitude(Vec3 v);
Vec3 Normalize(Vec3 v);
Vec3 Perpendicular(Vec3 v);
///< returns a perpendicular vector (out of infinite) to the provided one

Vec3 CrossProd(Vec3 v1, Vec3 v2);

double DotProd(Vec3 v1, Vec3 v2);

Matrix33 operator*(Matrix33 left, Matrix33 right);

Matrix33 Transpose(Matrix33 in);

Matrix33 Rodrigues(Vec3 v, double angle);
///< Rodrigues formula for rotation around vector 

Matrix33 RotationVecVec(Vec3 v1, Vec3 v2);
///< Returns the rotation matrix in order to rotate one vector to the other

Vec3 RotatePoint(Matrix33 rot, Vec3 v);

#endif /*__UTIL_H__*/
