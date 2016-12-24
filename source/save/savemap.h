

#include "../platform.h"

#define TAG_MAP		{'D', 'M', 'D', 'M', 'C'}
#define MAP_VERSION		1.0f

class Map;
class CutBrush;
class Brush;

void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes);
void SaveMap(const char* fullpath, std::list<Brush>& brushes);
bool LoadMap(const char* fullpath, Map* map);
