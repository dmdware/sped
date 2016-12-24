#include "sprite.h"
#include "../utils.h"
#include "../texture.h"
#include "../gui/gui.h"
#include "../debug.h"

std::vector<SpriteToLoad> g_spriteload;
int g_lastLSp = -1;
Sprite g_sprite[SPRITES];

Sprite::Sprite()
{
	on = false;
	difftexi = 0;
	teamtexi = 0;
	pixels = NULL;
}

Sprite::~Sprite()
{
    free();
}

void Sprite::free()
{
    if(pixels)
    {
        delete pixels;
        pixels = NULL;
    }

	//Free textures?

	on = false;
}

void FreeSprites()
{
	for(int i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(!s->on)
			continue;

		s->free();
	}
}

bool Load1Sprite()
{
	if(g_lastLSp+1 < g_spriteload.size())
		SetStatus(g_spriteload[g_lastLSp+1].relative.c_str());

	CHECKGLERROR();

	if(g_lastLSp >= 0)
	{
		SpriteToLoad* s = &g_spriteload[g_lastLSp];
		LoadSprite(s->relative.c_str(), s->spindex, s->loadteam, s->loaddepth);
	}

	g_lastLSp ++;

	if(g_lastLSp >= g_spriteload.size())
	{
		return false;	// Done loading all
	}

	return true;	// Not finished loading
}

void QueueSprite(const char* relative, unsigned int* spindex, bool loadteam, bool loaddepth)
{
	SpriteToLoad stl;
	stl.relative = relative;
	stl.spindex = spindex;
	stl.loadteam = loadteam;
	stl.loaddepth = loaddepth;
	g_spriteload.push_back(stl);
}

int NewSprite()
{
	for(int i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(!s->on)
			return i;
	}

	return -1;
}

bool FindSprite(unsigned int &spriteidx, const char* relative)
{
	char corrected[SPE_MAX_PATH+1];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);
	char fullpath[SPE_MAX_PATH+1];
	FullPath(corrected, fullpath);

	for(int i=0; i<SPRITES; i++)
	{
		Sprite* s = &g_sprite[i];

		if(s->on && stricmp(s->fullpath.c_str(), fullpath) == 0)
		{
			//g_texindex = i;
			//texture = t->texname;
			spriteidx = i;
			return true;
		}
	}

	return false;
}

/*
TODO
Convert to C90
All upper case to lower case
CreateTex -> createtex
All class to struct
LoadedTex -> loadedtex_t or loadedtex
*/

void LoadSprite(const char* relative, unsigned int* spindex, bool loadteam, bool loaddepth)
{
	if(FindSprite(*spindex, relative))
		return;

	int i = NewSprite();

	if(i < 0)
		return;

	Sprite* s = &g_sprite[i];
	s->on = true;
	*spindex = i;

	char full[SPE_MAX_PATH+1];
	FullPath(relative, full);
	CorrectSlashes(full);
	s->fullpath = full;

	char reltxt[SPE_MAX_PATH+1];
	char reldiff[SPE_MAX_PATH+1];
	char relteam[SPE_MAX_PATH+1];
	char reldepth[SPE_MAX_PATH+1];
	sprintf(reltxt, "%s.txt", relative);
	sprintf(reldiff, "%s.png", relative);
	sprintf(relteam, "%s_team.png", relative);
	sprintf(reldepth, "%s_depth.png", relative);
	ParseSprite(reltxt, s);

	CreateTex(s->difftexi, reldiff, true, false);
#ifndef PLATFORM_MOBILE
	if(loadteam)
		CreateTex(s->teamtexi, relteam, true, false);
	if(loaddepth)
		CreateTex(s->depthtexi, reldepth, true, false);
#endif
	
	char pixfull[SPE_MAX_PATH+1];
	FullPath(reldiff, pixfull);
	s->pixels = LoadTexture(pixfull);

	if(!s->pixels)
		Log("Failed to load sprite %s\r\n", relative);
	else
		Log("%s\r\n", relative);

	
}

void ParseSprite(const char* relative, Sprite* s)
{
	char fullpath[SPE_MAX_PATH+1];
	FullPath(relative, fullpath);

	FILE* fp = fopen(fullpath, "r");
	if(!fp) return;

	float centerx;
	float centery;
	float width;
	float height;
	float clipszx, clipszy;
	float clipminx, clipminy, clipmaxx, clipmaxy;

	fscanf(fp, "%f %f", &centerx, &centery);
	fscanf(fp, "%f %f", &width, &height);
	fscanf(fp, "%f %f", &clipszx, &clipszy);
	fscanf(fp, "%f %f %f %f", &clipminx, &clipminy, &clipmaxx, &clipmaxy);
	
	s->offset[0] = -centerx;
	s->offset[1] = -centery;
	s->offset[2] = s->offset[0] + width;
	s->offset[3] = s->offset[1] + height;

	//s->crop[0] = clipminx / width;
	//s->crop[1] = clipminy / height;
	//s->crop[2] = clipmaxx / width;
	//s->crop[3] = clipmaxy / height;
	//s->crop[2] = (clipminx + clipszx) / width;
	//s->crop[3] = (clipminy + clipszy) / height;
	s->crop[0] = 0;
	s->crop[1] = 0;
	s->crop[2] = clipszx / width;
	s->crop[3] = clipszy / height;

	//s->cropoff[0] = clipminx - centerx;
	//s->cropoff[1] = clipminy - centery;
	//s->cropoff[2] = clipmaxx - centerx;
	//s->cropoff[3] = clipmaxy - centery;
	//s->cropoff[2] = clipminx + clipszx - centerx;
	//s->cropoff[3] = clipminy + clipszy - centery;
	s->cropoff[0] = -centerx;
	s->cropoff[1] = -centery;
	s->cropoff[2] = clipszx - centerx;
	s->cropoff[3] = clipszy - centery;

	fclose(fp);

#if 0
	char fullpath[SPE_MAX_PATH+1];

	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");

	if(g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING)
		sprintf(frame, "_fr%03d", g_renderframe);

	if(g_rendertype == RENDER_UNIT)
		sprintf(side, "_si%d", g_rendside);

	std::string incline = "";

	if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
	{
		if(g_currincline == INC_0000)	incline = "_inc0000";
		else if(g_currincline == INC_0001)	incline = "_inc0001";
		else if(g_currincline == INC_0010)	incline = "_inc0010";
		else if(g_currincline == INC_0011)	incline = "_inc0011";
		else if(g_currincline == INC_0100)	incline = "_inc0100";
		else if(g_currincline == INC_0101)	incline = "_inc0101";
		else if(g_currincline == INC_0110)	incline = "_inc0110";
		else if(g_currincline == INC_0111)	incline = "_inc0111";
		else if(g_currincline == INC_1000)	incline = "_inc1000";
		else if(g_currincline == INC_1001)	incline = "_inc1001";
		else if(g_currincline == INC_1010)	incline = "_inc1010";
		else if(g_currincline == INC_1011)	incline = "_inc1011";
		else if(g_currincline == INC_1100)	incline = "_inc1100";
		else if(g_currincline == INC_1101)	incline = "_inc1101";
		else if(g_currincline == INC_1110)	incline = "_inc1110";
	}

	std::string stage = "";

	if(rendstage == RENDSTAGE_TEAM)
		stage = "_team";

	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG(fullpath, &finalsprite);
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);

	sprintf(fullpath, "%s%s%s%s.txt", g_renderbasename, side, frame, incline.c_str());
	std::ofstream ofs(fullpath, std::ios_base::out);
	ofs<<finalcenter.x<<" "<<finalcenter.y);
	ofs<<finalimagew<<" "<<finalimageh);
	ofs<<finalclipsz.x<<" "<<finalclipsz.y);
	ofs<<finalclipmin.x<<" "<<finalclipmin.y<<" "<<finalclipmax.x<<" "<<finalclipmax.y;
#endif

#if 0
	char infopath[SPE_MAX_PATH+1];
	strcpy(infopath, texpath);
	StripExt(infopath);
	strcat(infopath, ".txt");

	std::ifstream infos(infopath);

	if(!infos)
		return;

	int centeroff[2];
	int imagesz[2];
	int clipsz[2];

	infos>>centeroff[0]>>centeroff[1];
	infos>>imagesz[0]>>imagesz[1];
	infos>>clipsz[0]>>clipsz[1];

	t->sprite.offset[0] = -centeroff[0];
	t->sprite.offset[1] = -centeroff[1];
	t->sprite.offset[2] = t->sprite.offset[0] + imagesz[0];
	t->sprite.offset[3] = t->sprite.offset[1] + imagesz[1];
#endif
}

bool PlayAnim(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first || frame > last+1)
    {
        frame = first;
        return false;
    }

    frame += rate;

    if(frame > last)
    {
        if(loop)
            frame = first;
		else
			frame = last;

        return true;
    }

    return false;
}

//Play animation backwards
bool PlayAnimB(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first-1 || frame > last)
    {
        frame = last;
        return false;
    }

    frame -= rate;

    if(frame < first)
    {
        if(loop)
            frame = last;
		else
			frame = first;

        return true;
    }

    return false;
}
