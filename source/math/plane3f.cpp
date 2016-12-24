#include "vec3f.h"
#include "plane3f.h"
#include "physics.h"
#include "math3d.h"
#include "../utils.h"

Plane3f::Plane3f()
{
}

Plane3f::Plane3f(float x, float y, float z, float d)
{
	m_normal.x = x;
	m_normal.y = y;
	m_normal.z = z;
	m_d = d;
}

Plane3f::~Plane3f()
{
}

bool Close(Plane3f a, Plane3f b)
{
	if(fabs(a.m_normal.x - b.m_normal.x) <= CLOSE_EPSILON && fabs(a.m_normal.y - b.m_normal.y) <= CLOSE_EPSILON && fabs(a.m_normal.z - b.m_normal.z) <= CLOSE_EPSILON && fabs(a.m_d - b.m_d) <= CLOSE_EPSILON)
		return true;

	if(fabs(-a.m_normal.x - b.m_normal.x) <= CLOSE_EPSILON && fabs(-a.m_normal.y - b.m_normal.y) <= CLOSE_EPSILON && fabs(-a.m_normal.z - b.m_normal.z) <= CLOSE_EPSILON && fabs(-a.m_d - b.m_d) <= CLOSE_EPSILON)
		return true;

	return false;
}

//#define PLANE_DEBUG

Vec3f PointOnPlane(Plane3f p)
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

float PlaneDistance(Vec3f normal, Vec3f point)
{
	float distance = 0; // This variable holds the distance from the plane to the origin

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// So, we come up with D = -(Ax + By + Cz)
	// Basically, the negated dot product of the normal of the plane and the point.
	distance = - (normal.x * point.x + normal.y * point.y + normal.z * point.z);

	return distance;
}

bool PointBehindPlane(Vec3f point, Plane3f plane)
{
	float result = point.x*plane.m_normal.x + point.y*plane.m_normal.y + point.z*plane.m_normal.z + plane.m_d;

	if(result <= 0)
		return true;

	return false;
}

bool PointOnOrBehindPlane(Vec3f point, Plane3f plane, float epsilon)
{
	float result = point.x*plane.m_normal.x + point.y*plane.m_normal.y + point.z*plane.m_normal.z + plane.m_d;

	if(result <= epsilon)
		return true;

	return false;
}

bool PointOnOrBehindPlane(Vec3f point, Vec3f normal, float dist, float epsilon)
{
	float result = point.x*normal.x + point.y*normal.y + point.z*normal.z + dist;

	if(result <= epsilon)
		return true;

	return false;
}

void RotatePlane(Plane3f& p, Vec3f about, float radians, Vec3f axis)
{
	Vec3f pop = PointOnPlane(p);
	pop = RotateAround(pop, about, radians, axis.x, axis.y, axis.z);
	p.m_normal = Rotate(p.m_normal, radians, axis.x, axis.y, axis.z);
	p.m_d = PlaneDistance(p.m_normal, pop);
}

// http://thejuniverse.org/PUBLIC/LinearAlgebra/LOLA/planes/std.html
void MakePlane(Vec3f* norm, float* d, Vec3f point, Vec3f setnorm)
{
	*norm = setnorm;
	*d = -Dot(setnorm, point);
}

// Parametric line
void ParamLine(Vec3f* line, Vec3f* change)
{
	(*change) = line[1] - line[0];
	//(*change) = Normalize(*change);

}

// line intersects plane?
bool LineInterPlane(const Vec3f* line, const Vec3f norm, const float d, Vec3f* inter)
{
	Vec3f change = line[1] - line[0];
	//ParamLine(line, &change);

	float denom = Dot(norm, change);

	if(fabs(denom) <= EPSILON)
		return false;

	float SegScalar = (d - Dot(norm, line[0])) / denom;

	//TODO: Check if SegScalar is [0.0, 1.0]?
	if(SegScalar < 0.0f)
		return false;

	*inter = change * SegScalar + line[0];

	return true;
}
