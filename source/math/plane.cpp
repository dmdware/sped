

#include "vec3f.h"
#include "plane.h"
#include "physics.h"
#include "math3d.h"
#include "../utils.h"

Plane::Plane()
{
}

Plane::Plane(float x, float y, float z, float d)
{
	m_normal.x = x;
	m_normal.y = y;
	m_normal.z = z;
	m_d = d;
}

Plane::~Plane()
{
}

bool Close(Plane a, Plane b)
{
	if(fabs(a.m_normal.x - b.m_normal.x) <= CLOSE_EPSILON && fabs(a.m_normal.y - b.m_normal.y) <= CLOSE_EPSILON && fabs(a.m_normal.z - b.m_normal.z) <= CLOSE_EPSILON && fabs(a.m_d - b.m_d) <= CLOSE_EPSILON)
		return true;

	if(fabs(-a.m_normal.x - b.m_normal.x) <= CLOSE_EPSILON && fabs(-a.m_normal.y - b.m_normal.y) <= CLOSE_EPSILON && fabs(-a.m_normal.z - b.m_normal.z) <= CLOSE_EPSILON && fabs(-a.m_d - b.m_d) <= CLOSE_EPSILON)
		return true;

	return false;
}

//#define PLANE_DEBUG

Vec3f PointOnPlane(Plane p)
{
	//Ax + By + Cz + D = 0
	//x = -D/A	if(A != 0)
	//y = -D/B	if(B != 0)
	//z = -D/C	if(C != 0)

#ifdef PLANE_DEBUG
	Log("point on plane ("<<p.m_normal.x<<","<<p.m_normal.y<<","<<p.m_normal.z<<"),"<<p.m_d<<std::endl;
	
#endif

	int greatest = -1;
	float greatestd = 0;

	if(greatest < 0 || fabs(p.m_normal.x) > greatestd)
	{
		greatest = 0;
		greatestd = fabs(p.m_normal.x);
	}
	if(greatest < 0 || fabs(p.m_normal.y) > greatestd)
	{
		greatest = 1;
		greatestd = fabs(p.m_normal.y);
	}
	if(greatest < 0 || fabs(p.m_normal.x) > greatestd)
	{
		greatest = 2;
		greatestd = fabs(p.m_normal.z);
	}

	if(fabs(p.m_normal.x) > EPSILON)
	//if(greatest == 0)
		return Vec3f(- p.m_d / p.m_normal.x, 0, 0);

	if(fabs(p.m_normal.y) > EPSILON)
	//if(greatest == 1)
		return Vec3f(0, - p.m_d / p.m_normal.y, 0);

	if(fabs(p.m_normal.z) > EPSILON)
	//if(greatest == 2)
		return Vec3f(0, 0, - p.m_d / p.m_normal.z);

	return Vec3f(0, 0, 0);
}

bool PointOnOrBehindPlane(Vec3f point, Plane plane, float epsilon)
{
	float result = point.x*plane.m_normal.x + point.y*plane.m_normal.y + point.z*plane.m_normal.z + plane.m_d;

	if(result <= epsilon)
		return true;

	return false;
}

void RotatePlane(Plane& p, Vec3f about, float radians, Vec3f axis)
{
	Vec3f pop = PointOnPlane(p);
	pop = RotateAround(pop, about, radians, axis.x, axis.y, axis.z);
	p.m_normal = Rotate(p.m_normal, radians, axis.x, axis.y, axis.z);
	p.m_d = PlaneDistance(p.m_normal, pop);
}
