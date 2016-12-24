



#ifndef MATH3D_H
#define MATH3D_H

#include "../platform.h"
#include "vec3f.h"

#define DEGTORAD(a)		(M_PI * a / 180)
#define RADTODEG(a)		(180 * a / M_PI)

#ifndef PI_ON_180
#	define PI_ON_180	(M_PI/180.0)
#endif

#define CAMERA_SPEED	5.0f

class Plane;
class Vec3f;
class Camera;
class Matrix;
class Triangle;
class Vec4f;

Vec3f VMin(float minf, Vec3f v);
float Magnitude(Vec3f vNormal);
float Magnitude(Vec2f vNormal);
float Magnitude2(Vec3f vNormal);
float Magnitude3(Vec3f vNormal);
Vec3f Normalize(Vec3f vNormal);
Vec3f Cross(Vec3f vVector1, Vec3f vVector2);
float Dot(Vec3f vVector1, Vec3f vVector2);
Vec3f Vector(Vec3f vPoint1, Vec3f vPoint2);
Vec3f Normal(Vec3f vTriangle[]);
Vec3f Normal2(Vec3f vTriangle[]);
bool IntersectedPlane(Vec3f vPoly[], Vec3f vLine[], Vec3f &vNormal, float &originDistance);
bool WithinYaw(Camera* c, Vec3f p, float angle);
float DYaw(Camera* c, Vec3f p);
double AngleBetweenVectors(Vec3f Vector1, Vec3f Vector2);
Vec3f IntersectionPoint(Vec3f vNormal, Vec3f vLine[], double distance);
bool Intersection(Vec3f l0, Vec3f l, Plane p, Vec3f& inter);
Vec3f OnNear(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec);
Vec3f OnNearPersp(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov, float mind);
Vec3f ScreenPerspRay(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov);
Vec3f Rotate(Vec3f v, float rad, float x, float y, float z);
Vec3f RotateAround(Vec3f v, Vec3f around, float rad, float x, float y, float z);
float Clipf(float n, float lower, float upper);
int Clipi(int n, int lower, int upper);
float GetYaw(float dx, float dz);
Matrix LookAt(float eyex, float eyey, float eyez,
               float centerx, float centery, float centerz,
               float upx, float upy, float upz);
#if 0
Matrix LookAtOrtho(float eyex, float eyey, float eyez,
               float centerx, float centery, float centerz,
               float upx, float upy, float upz);
#endif
Matrix PerspProj(float fov, float aspect, float znear, float zfar);
Matrix OrthoProj(float l, float r, float t, float b, float n, float f);
Vec4f ScreenPos(Matrix* mvp, Vec3f vec, float width, float height, bool persp);
float Snap(float base, float value);
float SnapNearest(float base, float value);

#endif
