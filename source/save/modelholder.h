

#ifndef MODELHOLDER_H
#define MODELHOLDER_H

#include "../render/model2.h"
#include "../platform.h"

class Surf;

class ModelHolder
{
public:
	int modeli;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix rotationmat;
	//VertexArray* frames;
	//int nframes;
	Model2 model;

	ModelHolder();
	ModelHolder(int model, Vec3f pos);
	~ModelHolder();
	ModelHolder(const ModelHolder& original);
	ModelHolder& operator=(const ModelHolder &original);

	void retransform();
	void regennormals();
	void destroy();

	Vec3f traceray(Vec3f line[], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f *retexc,
		Vec3f *nearnorm, bool exact=false);

	void addclipmesh(Surf *surf);
};

extern std::list<ModelHolder> g_modelholder;

void FreeModelHolders();
void DrawModelHolders();
void DrawModelHoldersDepth();

#endif
