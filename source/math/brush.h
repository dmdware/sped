

#ifndef BRUSH_H
#define BRUSH_H

#include "vec3f.h"
#include "plane.h"
#include "polygon.h"
#include "triangle.h"
#include "brushside.h"
#include "../sim/door.h"

#define STOREY_HEIGHT	250.0f //20.0f

class Texture;
class Surf;

class Brush
{
public:
	int m_nsides;
	BrushSide* m_sides;
	int m_nsharedv;
	Vec3f* m_sharedv;	//shared vertices array
	int m_texture;	//used to determine brush attributes
	EdDoor* m_door;
	bool m_broken;
	
	Brush& operator=(const Brush& original);
	Brush(const Brush& original);
	Brush();
	~Brush();
	void add(BrushSide b);
	void setsides(int nsides, BrushSide* sides);
	void getsides(int* nsides, BrushSide** sides);
	void removeside(int i);
	void collapse();
	void remaptex();
	Vec3f traceray(Vec3f line[], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f *retexc,
		Vec3f* nearnorm);
	bool addclipmesh(Surf *surf);
	void prunev(bool* invalidv);
	void moveto(Vec3f newp);
};

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height);
void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height);
void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const Vec3f vmin, const Vec3f vmax);
bool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes);

#endif