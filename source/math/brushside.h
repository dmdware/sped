


#ifndef BRUSHSIDE_H
#define BRUSHSIDE_H

#include "vec3f.h"
#include "plane.h"
#include "polygon.h"
#include "triangle.h"
#include "../render/model2.h"
#include "../render/vertexarray.h"

//tex coord equation - not used
class TexCEq
{
	float m_rot;	//degrees
	float m_scale[2];	//default 0.1, world to tex coordinates
	float m_offset[2];	//in world distances
};

class CutBrushSide;

class BrushSide
{
public:
	Plane m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	unsigned int m_ownerm;	//team colour map
	int m_ntris;
	Triangle2* m_tris;
	Plane m_tceq[2];	//tex coord uv equations
	Polyg m_outline;
	int* m_vindices;	//indices into parent brush's shared vertex array; only stores unique vertices as defined by polygon outline
	Vec3f m_centroid;

	BrushSide(const BrushSide& original);
	BrushSide& operator=(const BrushSide &original);
	BrushSide();
	BrushSide(Vec3f normal, Vec3f point);
	~BrushSide();
	void makeva();
	void vafromcut(CutBrushSide* cutside);
	void usedifftex();
	void usespectex();
	void usenormtex();
	void useteamtex();
	void gentexeq();	//fit texture to face
	void fittex();
	void remaptex();
};

Vec3f PlaneCrossAxis(Vec3f normal);

#endif
