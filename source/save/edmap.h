

#ifndef EDMAP_H
#define EDMAP_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/brush.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../sim/door.h"
#include "modelholder.h"

class EdMap
{
public:
	std::list<Brush> m_brush;
};

extern EdMap g_edmap;
extern std::list<Brush*> g_selB;
extern Brush* g_sel1b;	//drag selected brush
extern int g_dragV;	//drag vertex of selected brush
extern int g_dragS;	//drag side of selected brush
extern bool g_dragW;
extern int g_dragD;
extern int g_dragM;	//drag model holder
extern std::list<ModelHolder*> g_selM;
extern ModelHolder* g_sel1m;	//drag selected model (model being dragged or manipulated currently)

#define DRAG_DOOR_POINT		1
#define DRAG_DOOR_AXIS		2

void DrawEdMap(EdMap* map, bool showsky);
void DrawEdMapDepth(EdMap* map, bool showsky);

#endif
