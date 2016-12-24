#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/polygon.h"
#include "../math/math3d.h"
#include "hmapmath.h"
#include "../render/water.h"
#include "../utils.h"
#include "../window.h"
#include "../math/camera.h"

float Bilerp(Heightmap* hmap, float x, float z)
{
	x /= (float)TILE_SIZE;
	z /= (float)TILE_SIZE;

	int x1 = (int)(x);
	int x2 = x1 + 1;

	int z1 = (int)(z);
	int z2 = z1 + 1;

	float xdenom = (float)(x2-x1);
	float x2fac = (float)(x2-x)/xdenom;
	float x1fac = (float)(x-x1)/xdenom;

	float hR1 = hmap->getheight(x1,z1)*x2fac + hmap->getheight(x2,z1)*x1fac;
	float hR2 = hmap->getheight(x1,z2)*x2fac + hmap->getheight(x2,z2)*x1fac;

	float zdenom = (float)(z2-z1);

	return hR1*(z2-z)/zdenom + hR2*(z-z1)/zdenom;
}

#if 0
bool FastMapIntersect(Heightmap* hmap, Vec3f* line, Vec3f* intersection)
{
	Vec3f ray = line[1] - line[0];
	Vec3f tilestart = line[0] / TILE_SIZE;
	Vec3f tileray = ray / TILE_SIZE;

	int len = (int)Magnitude(tileray);

	for(int i=0; i<len; i++)
	{
		Vec3f pos = tilestart + tileray * (float)i / (float)len;
		int tx = (int)pos.x;
		int tz = (int)pos.z;

		if(tx < 0)
			continue;

		if(tz < 0)
			continue;

		if(tx >= hmap->m_widthx)
			continue;

		if(tz >= hmap->m_widthy)
			continue;

		if(TileIntersect(hmap, line, tx, tz, intersection))
			return true;
	}

	return false;
}
#endif
