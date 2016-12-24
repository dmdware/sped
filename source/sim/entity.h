

#include "../math/vec3f.h"
#include "../math/camera.h"

#define BODY_LOWER		0
#define BODY_UPPER		1

class Entity
{
public:
    float frame[2];
	int type;
	int controller;
	Camera camera;
	float amount;
	float clip;
	int state;
	int cluster;
	float dist;
	bool nolightvol;
	int script;

	Entity()
	{
		controller = -1;
		frame[BODY_LOWER] = 0;
		frame[BODY_UPPER] = 0;
		amount = -1;
		clip = -1;
	}

	Vec3f traceray(Vec3f line[]);
	bool collision(Vec3f scaleDown, Vec3f center);
};

#define ENTITIES	1024
extern Entity* g_entity[ENTITIES];

void DestroyEntities();
bool PlaceEntity(int type, int controller, float amount, float clip, Vec3f pos, float yaw, int* ID);
void DrawEntities();