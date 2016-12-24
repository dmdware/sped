#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../texture.h"
#include "vertexarray.h"
#include "../sim/tile.h"

#define TILE_SIZE			g_tilesize
#define TILE_Y_SCALE		20000.0f
#define TILE_Y_POWER		1.7f
#define TILE_Y_AFTERPOW		(TILE_Y_SCALE/pow(TILE_Y_SCALE/2.0f, TILE_Y_POWER))

#define TILE_Y_SCALE_2		(TILE_Y_SCALE / 2000.0f)
#define ELEV_SANDONLYMAXY	(10 * TILE_Y_SCALE_2)
#define ELEV_SANDGRASSMAXY	(100 * TILE_Y_SCALE_2)
#define ELEV_GRASSONLYMAXY	(600 * TILE_Y_SCALE_2)
#define ELEV_GRASSROCKMAXY	(990 * TILE_Y_SCALE_2)
#define MAPMINZ				(0)
#define MAPMAXZ				(m_widthy*TILE_SIZE)

//#define MAX_MAP				256
//#define MAX_MAP				128
#define MAX_MAP				64

class Vec2f;
class Vec3f;
class Shader;
class Matrix;
class Plane3f;

#define TILE_SAND				0
#define TILE_GRASS				1
#define TILE_ROCK				2
#define TILE_ROCK_NORM			3
#define TILE_CRACKEDROCK		4
#define TILE_CRACKEDROCK_NORM	5
#define TILE_PRERENDER			6
#define TILE_TYPES				7

extern Vec2i g_mapview[2];

//sim/tile.h
//#define TEX_DIFF		0
//#define TEX_SPEC		1
//#define TEX_NORM		2
//#define TEX_TYPES		3

extern unsigned int g_rimtexs[TEX_TYPES];

class Heightmap
{
public:
	/*
	Number of tiles, not heightpoints/corners.
	Number of height points/corners is +1.
	*/
	int m_widthx;
	int m_widthy;

	float *m_heightpoints;
	Vec3f *m_drawverts;
	Vec3f *m_collverts;
	Vec2f *m_texcoords0;
	Vec3f *m_normals;
	bool *m_triconfig;
	Plane3f *m_tridivider;
	float m_tilescale;
	VertexArray m_rimva;

	Heightmap()
	{
		m_widthx = 0;
		m_widthy = 0;
	}

	void allocate(int wx, int wy);
	void remesh(float tilescale=1.0f);

	inline float getheight(int tx, int tz)
	{
		return m_heightpoints[ (tz)*(m_widthx+1) + tx ];
	}

	float accheight(int x, int z);
	float accheight2(int x, int z);
	void changeheight(int x, int z, float change);
	void setheight(int x, int z, float height);
	void destroy();
	int &getcountry(int x, int z);
	Vec3f *getdrawtileverts(int x, int z);
	Vec3f *getcolltileverts(int x, int z);
	Vec3f getnormal(int x, int z);
	void hidetile(int x, int z);
	void unhidetile(int x, int z);

	~Heightmap()
	{
		destroy();
	}
};

#endif
