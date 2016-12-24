

#include "line.h"


Line::Line()
{
}

Line::Line(Vec3f start, Vec3f end)
{
	m_vertex[0] = start;
	m_vertex[1] = end;
}