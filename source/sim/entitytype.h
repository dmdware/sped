

#include "../platform.h"
#include "../math/vec3f.h"
#include "../sound.h"
#include "../phys/collider.h"
#include "../render/model2.h"

class EntityT
{
public:
	int collider;
	int model;
	Vec3f vMin, vMax;
	float maxStep;
	float speed;
	float jump;
	float crouch;
	float animRate;
	int item;
	Vec3f centerOff;
	//std::vector<Sound> openSound;
	//std::vector<Sound> closeSound;

	EntityT();
	EntityT(int collider, const char* modelFile, Vec3f modelScale, Vec3f modelOffset, Vec3f vMin, Vec3f vMax, float maxStep, float speed, float jump, float crouch, float animRate, int item);
};

#define ENTITY_TYPES	3
extern EntityT g_entityT[ENTITY_TYPES];
#define ENTITY_BATTLECOMPUTER		0
#define ENTITY_STATESOLDIER			1
#define ENTITY_OFFICETABLE1			2
