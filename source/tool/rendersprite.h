

#ifndef RENDERSPRITE_H
#define RENDERSPRITE_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "../math/camera.h"

extern char g_renderpath[SPE_MAX_PATH+1];
extern char g_renderbasename[SPE_MAX_PATH+1];
extern int g_rendertype;
extern int g_renderframes;
extern float g_transpkey[3];
extern int g_nrendsides;
extern bool g_rendtopo;

extern bool g_doframes;
extern bool g_dosides;
extern bool g_doinclines;
extern bool g_dorots;
extern bool g_usepalette;
extern int g_savebitdepth;

extern int g_origwidth;
extern int g_origheight;
extern bool g_warned;

extern Vec2i g_spritecenter;
extern Vec2i g_clipmin;
extern Vec2i g_clipmax;
extern float g_transpkey[3];
extern bool g_doframes;
extern bool g_dosides;
extern bool g_doinclines;
extern bool g_dorots;
extern bool g_usepalette;
extern int g_savebitdepth;
extern Vec3f g_origlightpos;
extern Camera g_origcam;
extern int g_rendside;
extern int g_rendpitch, g_rendyaw, g_rendroll;
extern int g_nrendsides;
extern bool g_antialias;
extern bool g_fit2pow;
extern bool g_exportdepth;
extern bool g_exportteam;
extern bool g_hidetexerr;

extern int g_deswidth;
extern int g_desheight;
extern unsigned int g_rendertex[4];
extern unsigned int g_renderrb[4];
extern unsigned int g_renderfb[4];

#define RENDER_BUILDING		0
#define RENDER_UNIT			1
#define RENDER_TERRTILE		2
#define RENDER_ROAD			3
#define RENDER_UNSPEC		4

#define RENDSTAGE_COLOR		0
#define RENDSTAGE_TEAM		1
#define RENDSTAGE_DEPTH		2

extern bool g_antialias;
extern bool g_fit2pow;
extern bool g_exportdepth;
extern bool g_exportteam;
extern bool g_hidetexerr;

//should be a power of two,
//otherwise there's some space-saving optimizations i could've made
//in SaveRender();... checking if downsample size fits in a smaller image.
//edit: MUST be a power of two. until i fix it.
#define ANTIALIAS_UPSCALE	4

//Arbitrary max texture size, needed for determining upscale vertex screen coordinates.
//If it's too small for the upscale sprite, the coordinates won't fit and the sprite won't be rendered at those points.
#define MAX_TEXTURE		4096


void AdjustFrame(bool resetview=true);
void PrepareRender(const char* fullpath, int rendtype);
void UpdateRender();
void RotateView();
void EndRender();
void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height);
void SaveRender(int rendstage);

#endif
