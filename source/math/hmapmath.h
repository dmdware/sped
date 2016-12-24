#ifndef HMAPMATH_H
#define HMAPMATH_H

#include "../math/vec3f.h"

class Heightmap;

float Bilerp(Heightmap* hmap, float x, float z);
bool GetMapIntersection2(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool GetMapIntersection(Heightmap* hmap, Vec3f* vLine, Vec3f* vIntersection);
bool FastMapIntersect(Heightmap* hmap, Vec3f* line, Vec3f* intersection);
float Highest(int minx, int minz, int maxx, int maxz);
float Lowest(int minx, int minz, int maxx, int maxz);
bool TileUnclimable(float px, float pz);
bool AnyLandi(int tx, int tz);
bool AnyWateri(int tx, int tz);
bool AnyWater(int cmx, int cmz);
bool AtWater(int cmx, int cmz);
bool AtLand(int cmx, int cmz);
void MapFrust(Vec3f* interTopLeft,
	Vec3f* interTopRight,
	Vec3f* interBottomLeft,
	Vec3f* interBottomRight);

#endif
