

#ifndef ORDRAW_H
#define ORDRAW_H


#include "../platform.h"
#include "../tool/rendertopo.h"

void UpdateOrient();
void ViewTopo(const char* fullpath);

class Or
{
public:
	unsigned int difftexi;
	unsigned int postexi[3];
	unsigned int jumptexi;
	float maxrad;
};

//a single orientability map object, with frames, or inclines, etc.
class OrList
{
public:
	Or *ors;
	
	bool on;
	bool inclines;
	bool sides;
	int32_t nsides;
	bool rotations;
	bool frames;
	int32_t nframes;

	std::string fullpath;

	int nors;

	void free()
	{
		on = false;
		delete [] ors;
		ors = NULL;
		nors = 0;
	}

	OrList()
	{
		ors = NULL;
		on = false;
		nors = 0;
	}
	~OrList()
	{
		free();
	}
};

extern OrList g_orlist;

#endif