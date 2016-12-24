

#ifndef POLYGON_H
#define POLYGON_H

#include "../platform.h"
#include "vec3f.h"

class Polyg	//name shortened due to naming conflict on Windows
{
public:
	std::list<Vec3f> m_edv;	//used for constructing the polygon on-the-fly
	Vec3f* m_drawoutva;		//used for drawing outline

	Polyg();
	~Polyg();
	Polyg(const Polyg& original);
	Polyg& operator=(const Polyg& original);
	void makeva();
	void freeva();
};

bool InsidePolygon(Vec3f vIntersection, Vec3f Poly[], long verticeCount);
bool IntersectedPolygon(Vec3f vPoly[], Vec3f vLine[], int verticeCount, Vec3f* vIntersection=NULL);

#endif
