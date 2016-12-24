

#ifndef PLANE_H
#define PLANE_H

#include "vec3f.h"
#include "physics.h"

class Plane
{
public:
	Vec3f m_normal;
	float m_d;
	
	bool operator==(const Plane p) const
	{
		//if(fabs(m_normal.x - p.m_normal.x) <= EPSILON && fabs(m_normal.y - p.m_normal.y) <= EPSILON && fabs(m_normal.z - p.m_normal.z) <= EPSILON && fabs(m_d - p.m_d) <= EPSILON)
		//	return true;
		
		if(m_normal.x == p.m_normal.x && m_normal.y == p.m_normal.y && m_normal.z == p.m_normal.z && m_d == p.m_d)
			return true;

		return false;
	}

	Plane();
	Plane(float x, float y, float z, float d);
	~Plane();
};

bool Close(Plane a, Plane b);
Vec3f PointOnPlane(Plane p);
float PlaneDistance(Vec3f normal, Vec3f point);
bool PointOnOrBehindPlane(Vec3f point, Plane plane, float epsilon=CLOSE_EPSILON);
bool PointOnOrBehindPlane(Vec3f point, Vec3f normal, float dist, float epsilon=CLOSE_EPSILON);
void RotatePlane(Plane& p, Vec3f about, float radians, Vec3f axis);
void MakePlane(Vec3f* norm, float* d, Vec3f point, Vec3f setnorm);
void ParamLine(Vec3f* line, Vec3f* change);
bool LineInterPlane(const Vec3f* line, const Vec3f norm, const float d, Vec3f* inter);

#endif