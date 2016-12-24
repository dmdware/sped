

#include "sortb.h"
#include "../math/brush.h"
#include "../sim/map.h"
#include "../save/edmap.h"
#include "../math/camera.h"
#include "../math/math3d.h"
#include "../platform.h"

static Vec3f sorteye;
static Vec3f sortview;

bool EdBCompare(const Brush& a, const Brush& b)
{
	//if(a.age == b.age)
	//	return a.name < b.name;

	//return a.age > b.age;

	float fara = 0;
	float farb = 0;

	for(int i=0; i<a.m_nsharedv; i++)
	{
		Vec3f* sharedv = &a.m_sharedv[i];

		float thisfar = Dot(sortview, *sharedv - sorteye);

		if(thisfar > fara || i == 0)
			fara = thisfar;
	}

	for(int i=0; i<b.m_nsharedv; i++)
	{
		Vec3f* sharedv = &b.m_sharedv[i];

		float thisfar = Dot(sortview, *sharedv - sorteye);

		if(thisfar > farb || i == 0)
			farb = thisfar;
	}

	return fara > farb;
}

void SortEdB(EdMap* map, Vec3f focus, Vec3f eye)
{
	sorteye = eye;
	sortview = Normalize(focus - eye);

	map->m_brush.sort(EdBCompare);
}