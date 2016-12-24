

#ifndef PARTICLE_H
#define PARTICLE_H


#include "../utils.h"
#include "../math/math3d.h"
#include "../math/vec3f.h"

class Particle;
class Billboard;

class ParticleT
{
public:
	int billbT;
	int delay;
	float decay;
	Vec3f minvelocity;
	Vec3f velvariation;
	Vec3f minacceleration;
	Vec3f accelvariation;
	float minsize;
	float sizevariation;
	void (*collision)(Particle* part, Billboard* billb, Vec3f trace, Vec3f normal);
};

enum PARTICLETYPE{EXHAUST, EXHAUSTBIG, FIREBALL, FIREBALL2, SMOKE, SMOKE2, DEBRIS, FLAME, PLUME, PARTICLE_TYPES};
extern ParticleT g_particleT[PARTICLE_TYPES];

class EmitterCounter
{
public:
	long last;

	EmitterCounter() { last = GetTicks(); }
	bool EmitNext(int delay)
	{
		if(GetTicks()-last > delay)
		{
			last = GetTicks();
			return true;
		}
		else
			return false;
	}
};

class Billboard;

class Particle
{
public:
	bool on;
	int type;
	float life;
	Vec3f vel;
	float dist;

	Particle() { on = false; }
	Particle(Vec3f p, Vec3f v) { on = true; vel = v; }

	void Update(Billboard* billb);
};

#define PARTICLES 256
extern Particle g_particle[PARTICLES];

class EmitterPlace
{
public:
	EmitterPlace() {}
	EmitterPlace(int t, Vec3f off) { type=t; offset=off; }
	Vec3f offset;
	int type;
};

void LoadParticles();
void Particles();
void EmitParticle(int type, Vec3f pos);
void UpdateParticles();

#endif
