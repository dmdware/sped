


#ifndef MAP_H
#define MAP_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/brush.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../math/brush.h"

class Map
{
public:
	int m_nbrush;
	Brush* m_brush;
	std::list<int> m_transpbrush;
	std::list<int> m_opaquebrush;
	std::list<int> m_skybrush;

	Map();
	~Map();
	void destroy();
};

extern Map g_map;

void DrawMap(Map* map);
void DrawMap2(Map* map);

#endif
