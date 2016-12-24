

#include "../render/model2.h"
#include "../math/brush.h"
#include "entitytype.h"
#include "entity.h"
#include "../texture.h"
#include "tile.h"
#include "selection.h"

int themodel;
float g_maxelev = 10000;

#if 0
class Kilobyte
{
public:
	char bytes[1024];

	Kilobyte(){}
	~Kilobyte(){}
};
#endif

void InitOnce()
{
	for(int i=0; i<ENTITIES; i++)
	{
		g_entity[i] = NULL;
	}

	themodel = 0;

	Vec3f charMax = Vec3f(10.0f, 3.4f, 10.0f) * 2.57f;
	Vec3f charMin = Vec3f(-10.0f, -69.9f, -10.0f) * 2.57f;
	float charMaxStep = 20.0f;
	float charSpeed = 20.0f;
	float charJump = 1000.0f;
	float charCrouch = (charMax.y-charMin.y)/3.0f;
	float charAnimRate = 1.0f;

	//g_entityT[ENTITY_BATTLECOMPUTER] = EntityT(-1, "models/battlecomp/battlecomp.ms3d", Vec3f(2.57f,2.57f,2.57f), Vec3f(0,0,0), charMin, charMax, charMaxStep, charSpeed, charJump, charCrouch, charAnimRate, -1);

	MakeTiles();
}

unsigned int g_temptex;

void Queue()
{
	// 73 units to 188 cm (2.57 ratio)
	//QueueModel(&themodel, "models/battlecomp/battlecomp.ms3d", Vec3f(2.57f,2.57f,2.57f), Vec3f(0,188,0));

	QueueTexture(&g_circle, "gui/circle.png", true, true);

	unsigned int temp;
	CreateTex(g_temptex, "renders/apt00_fr000_team.png", true, false);
}
