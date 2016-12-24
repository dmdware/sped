

#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "../platform.h"
#include "../math/math3d.h"
#include "../math/vec3f.h"

using namespace std;

class BillboardT
{
public:
	bool on;
    char name[32];
    unsigned int tex;

	BillboardT()
	{
		on = false;
	}
};

#define BILLBOARD_TYPES			64
extern BillboardT g_billbT[BILLBOARD_TYPES];

class Billboard
{
public:
    bool on;
    int type;
    float size;
    Vec3f pos;
    float dist;
	int particle;
    
    Billboard()
    {
        on = false;
		particle = -1;
    }
};

#define BILLBOARDS  512
extern Billboard g_billb[BILLBOARDS];

extern unsigned int g_muzzle[4];

void Effects();
int NewBillboard();
int NewBillboard(char* tex);
int IdentifyBillboard(const char* name);
void SortBillboards();
void DrawBillboards();
void PlaceBillboard(const char* n, Vec3f pos, float size, int particle=-1);
void PlaceBillboard(int type, Vec3f pos, float size, int particle=-1);

#endif