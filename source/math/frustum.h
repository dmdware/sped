



#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "../platform.h"
#include "math3d.h"

class Frustum 
{
public:
	void CalculateFrustum(const float* proj, const float* modl);
	bool PointInFrustum(float x, float y, float z);
	bool SphereInFrustum(float x, float y, float z, float radius);
	bool CubeInFrustum(float x, float y, float z, float size);
	bool BoxInFrustum(float x, float y, float z, float width, float height, float sizeZ);

private:
	float m_Frustum[6][4];	// This holds the A B C and D values for each side of our frustum.
};

extern Frustum g_frustum;

#endif
