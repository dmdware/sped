

#ifndef RENDERTOPO_H
#define RENDERTOPO_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/plane3f.h"

extern int g_rendlon;
extern int g_rendlat;

class Texture;
class Tet;
class SurfPt;
class Surf;
class LoadedTex;

void PrepareTopo(const char* fullpath, int rendtype);
void OrRender(int rendstage, Vec3f offset);
void GenTexEq(Tet *tet, Vec3f tri[3], Vec2f txc[3]);

void DrawClip();
void SepPt(Tet *tet,
		   SurfPt *sp,
		   std::list<Tet*>* checkremove);
bool UniqueTet(Tet *a, Tet *b);
bool Test(Surf *surf);
Vec3f OrNorm(SurfPt *fatt[3], float *edgeposx, float *edgeposy, bool *edgesamex, bool *edgesamey);
bool EdgeFront(Surf *surf, SurfPt* p1, SurfPt *p2, Tet *not);
void TrueNextPt(Vec2f p1orc, 
				  float edgeposx, bool edgesamex,
				  float edgeposy, bool edgesamey,
				  Vec2f startorc, Vec2f* trueorc);
void ClassifyEdge(Vec2f p1orc,
				  Vec2f p2orc,
				  float *edgeposx,
				  bool *edgeplaced,
				  float *edgeposy);
void UpdEdges(Surf *surf, SurfPt *p1, SurfPt *p2, float edgeposx, float edgeposy,
			  Tet* not);
void UpdEdges2(Surf *surf, SurfPt *p1, SurfPt *p2, float* edgeposx, float* edgeposy,
			  Tet* updtet, bool *edgeplaced);

void CheckTet(Tet *tet, const char* file, int line);

void GenTexC(Vec2f &retexc,
				Vec3f ir,
				Vec2f *texc,
				Vec3f *tri);


bool TraceRay2(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV);

bool TraceRay4(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV);

void Balance4(Surf *surf);


bool SplitEdges2(Surf *surf, Surf *fullsurf, Vec2f *vmin, Vec2f *vmax, float minlen);

void GetToEmerge(Surf *surf,
				 Tet *etet,
				 Vec3f *emline,
				 SurfPt **esp);


void Emerge(Surf *surf,
			SurfPt *esp,
			Vec3f emline[2]);

void CheckEmerged(Surf *surf, Tet** halfemerged);


void GetCen(Surf *surf,
			SurfPt *sp,
			Vec3f *cen);


bool MapGlobe2(Surf *surf);
bool MapGlobe3(Surf *surf);

void Balance3(Surf *surf, int bestupdown);
void RemDupTet(std::list<Tet*>* tets);

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
	Vec3f wrappos;
	Vec3f pressure;
	bool emerged;
	int ring;
	int file;

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
		ring = -1;
		file = -1;
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
	bool gone;
	//int level;
	SurfPt* neib[4];
	//Plane3f texceq[2];
	Vec3f texcpos[3];
	Vec2f texc[3];
	Texture *tex;//, *stex, *ntex;
	//Vec3f norm;
	bool hidden;
	bool approved;
	//float ormaparea, drawarea;
	float edgeorarea[3], edgedrawarea[3];
	bool placed;
	//bool edgeoutx[3], edgeouty[3];
	//bool edgeposx[3];	//does x increase going from first to second point on the edge?
	//bool edgeposy[3];	//does y increase going from first to second point on the edge?
	float edgeposx[3];	//x offset going from first to second point on the edge
	float edgeposy[3];	//y offset going from first to second point on the edge
	bool edgeplaced[3];	//has this edge had offset set?
	//bool edgesamex[3];	//does x stay same from first to second point on the edge?
	///bool edgesamey[3];	//does y stay same from first to second point on the edge?

	int ring;
	float file;
	int extdir;

	Tet()
	{
		ring = -1;
		file = -1;
		extdir = -1;
		gone = false;
		edgeposx[0]=edgeposx[1]=edgeposx[2]=edgeposy[0]=edgeposy[1]=edgeposy[2]=0;
		edgeplaced[0]=edgeplaced[1]=edgeplaced[2]=false;
		////edgeoutx[0]=false;
		////edgeoutx[1]=false;
		////edgeoutx[2]=false;
		//edgeoutx[3]=false;
		/////edgeouty[0]=false;
		/////edgeouty[1]=false;
		////edgeouty[2]=false;
		//edgeouty[3]=false;
	}
};

class Surf
{
public:
	//std::list<SurfPt> pts;
	std::list<SurfPt*> pts2;
	//std::list<Tet> tets;
	std::list<Tet*> tets2;
};


#define CLOSEPOSF	0.1f

#endif