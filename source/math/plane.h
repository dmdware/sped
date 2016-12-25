

#ifndef PLANE_H
#define PLANE_H

#include "vec3f.h"
#include "physics.h"
#include "plane3f.h"

void MakePlane(Vec3f* norm, float* d, Vec3f point, Vec3f setnorm);
void ParamLine(Vec3f* line, Vec3f* change);
bool LineInterPlane(const Vec3f* line, const Vec3f norm, const float d, Vec3f* inter);

#endif