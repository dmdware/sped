

#ifndef RENDERTOPO_H
#define RENDERTOPO_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/plane3f.h"

extern int g_rendlon;
extern int g_rendlat;

class Texture;
class Tet;
class SurfPt;
class Surf;

void PrepareTopo(const char* fullpath, int rendtype);
void OrRender(int rendstage, Vec3f offset);
void GenTexEq(Tet *tet, Vec3f tri[3], Vec2f txc[3]);

void SepPt(Tet *tet,
		   SurfPt *sp,
		   std::list<Tet*>* checkremove);
bool UniqueTet(Tet *a, Tet *b);
bool Test(Surf *surf);

class SurfPt
{
public:
	bool gone;
	Vec3f pos;
	//Vec2f texc;
	//Vec3f norm;
	//Texture *tex, *stex, *ntex;
	//std::vector<SurfPt*> neib;
	//std::vector<Vec3f> off;
	//std::list<SurfPt*> neibs;
	//int gen;	//rim generation layer
	std::list<Tet*> holder;
	Vec2f orc;	//orientability map texture coord
	bool placed;

	bool hasneib(SurfPt *neib)
	{
		//for(std::list<SurfPt*>::iterator nit=neibs.begin();
		//	nit!=neibs.end();
		//	++nit)
		//	if(*nit == neib)
		//		return true;
		return false;
	}

	/*
	connect three mutual neibs loop
	with single new pt in center
	tri break   calc surf area   and 
	line break   
	connect all to new pt that share that edge as tris (connect to third point of the tri with that edge)
        .
	  /   \
	. -----.
	  \   /
	    .

		V

        .
	  / | \
	. --.--.
	  \ | /
	    .
	*/

	SurfPt()
	{
		//neib[0] = NULL;
		//neib[1] = NULL;
		//neib[2] = NULL;
		//neib[3] = NULL;
		//tex = NULL;
		//stex = NULL;
		//ntex = NULL;
		placed = false;
		gone = false;
	}
};

class Tet
{
public:
	//int level;
	SurfPt* neib[4];
	Plane3f texceq[2];
	Texture *tex;//, *stex, *ntex;
	//Vec3f norm;
	bool hidden;
	bool approved;
	//float ormaparea, drawarea;
	float edgeorarea[3], edgedrawarea[3];
	bool placed;
};

class Surf
{
public:
	//std::list<SurfPt> pts;
	std::list<SurfPt*> pts2;
	//std::list<Tet> tets;
	std::list<Tet*> tets2;
};

#endif