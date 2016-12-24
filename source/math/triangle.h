
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec3f.h"
#include "vec2f.h"

class Vec3f;

class Triangle
{
public:
   Vec3f m_vertex[3];
};

class Triangle2 : public Triangle
{
public:
	Vec2f m_texcoord[3];
};

#endif