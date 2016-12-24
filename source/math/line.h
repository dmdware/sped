

#ifndef LINE_H
#define LINE_H

#include "vec3f.h"

class Line
{
public:
	Vec3f m_vertex[2];

	Line();
	Line(Vec3f start, Vec3f end);
};

#endif