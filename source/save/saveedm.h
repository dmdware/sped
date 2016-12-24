


#include "../platform.h"

#define EDMAP_VERSION		2.0f

#define TAG_EDMAP		{'D', 'M', 'D', 'S', 'P'}	//DMD sprite project

class EdMap;
class TexRef;
class Brush;

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b);
void SaveBrush(FILE* fp, int* texrefs, Brush* b);
void SaveEdMap(const char* fullpath, EdMap* map);
bool LoadEdMap(const char* fullpath, EdMap* map);
void FreeEdMap(EdMap* map);