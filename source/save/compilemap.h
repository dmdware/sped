

#include "../platform.h"
#include "edmap.h"
#include "../math/triangle.h"

class CutBrushSide
{
public:
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	Plane3f m_tceq[2];
	std::list<Triangle> m_frag;
};

class CutBrush
{
public:
	std::list<CutBrushSide> m_side;
};

extern float g_defrenderpitch;
extern float g_defrenderyaw;
extern int g_1tilewidth;
extern int g_renderframe;

void ToCutSide(CutBrushSide* cuts, BrushSide* eds);
void CompileMap(const char* full, EdMap* map);
void ResetView(bool checkupscale);
