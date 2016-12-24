







#include "gui.h"
#include "../render/shader.h"
#include "../texture.h"
#include "font.h"
#include "../math/math3d.h"
#include "../platform.h"
#include "../window.h"
#include "draw2d.h"
#include "../render/heightmap.h"
#include "../app/appmain.h"
#include "cursor.h"
#include "../debug.h"

GUI g_gui;

unsigned int g_zoomtex = -1;
bool g_zoomdrawframe = false;

void GUI::draw()
{

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
	CHECKGLERROR();

#if 0
	DrawImage(g_texture[0].texname, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

	Widget::draw();
	CHECKGLERROR();
	Widget::drawover();

#if 0
	//not engine
	if(g_mode == APPMODE_PLAY &&
		g_keys[SDL_SCANCODE_TAB])
	{
		Lobby_DrawPyL();
		Lobby_DrawState();
	}
#endif

#if 0
	DrawImage(g_depth, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

#if 0
	if(g_depth != -1)
		DrawImage(g_depth, 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 0
	if(g_mode == APPMODE_PLAY)
		DrawImage(g_tiletexs[IN_PRERENDER], 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 1
	unsigned int spi = g_cursor[g_curst];
	Sprite* sp = &g_sprite[spi];

	float crop[] = {0,0,(float)g_width-1,(float)g_height-1}; 
	DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3], 0,0,1,1, crop);
#elif 1
	static int si = -100;
	si++;
	if(si >= 0)
	{
		BlType* bt = &g_bltype[BL_CHEMPL];
		Sprite* sp = &g_sprite[bt->sprite[0]];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#elif 0
	static int si = -100;
	si++;
	if(si >= 0)
	{
		InType* tt = &g_intype[0];
		Sprite* sp = &g_sprite[tt->sprite];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#else
	UType* ut = &g_utype[UNIT_LABOURER];
	static int si = -100;
	si++;
	if(si >= 0)
	{
		//si = si%(DIRS*ut->nframes);
		si = si%(ut->nframes);
		unsigned int spi = ut->sprite[si/ut->nframes][si%ut->nframes];
		//unsigned int spi = ut->sprite[0][si%ut->nframes];

		Sprite* sp = &g_sprite[spi];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#endif

#ifdef PLATFORM_MOBILE
	if(g_zoomdrawframe)
	{
		glFlush();
		glFinish();
		
		Vec2i zoombox[2];
		
		zoombox[0].x = g_mouse.x - ZOOMBOX;
		zoombox[0].y = g_mouse.y - ZOOMBOX;
		zoombox[1].x = g_mouse.x + ZOOMBOX;
		zoombox[1].y = g_mouse.y + ZOOMBOX;
		
		zoombox[0].x = imax(0, zoombox[0].x);
		zoombox[0].y = imax(0, zoombox[0].y);
		zoombox[1].x = imin(g_width-1, zoombox[1].x);
		zoombox[1].y = imin(g_height-1, zoombox[1].y);
		
#if 0
		LoadedTex zoompix;
		zoompix.sizex = zoombox[1].x - zoombox[0].x;
		zoompix.sizey = zoombox[1].y - zoombox[0].y;
		zoompix.channels = 3;
		zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 3 * zoompix.sizex * zoompix.sizey );
		
		glReadPixels(zoombox[0].x, zoombox[0].y, zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
		//CreateTex(&zoompix, &zoomtex, true, false);
		
		glBindTexture(GL_TEXTURE_2D, g_zoomtex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
#endif
		
		//DrawImage(g_zoomtex, g_mouse.x - ZOOMBOX*2, g_mouse.y - ZOOMBOX*2, g_mouse.x + ZOOMBOX*2, g_mouse.y + ZOOMBOX*2, 0,1,1,0, crop);
		
		//zoompix.destroy();
		//glDeleteTextures(1, &zoomtex);
		
#if 0
		LoadedTex zoompix2;
		Resample(&zoompix, &zoompix2, Vec2i(zoompix.sizex,zoompix.sizey)*2);
		
		Vec2i raster;
		raster.x = imax(0, g_mouse.x - ZOOMBOX);
		raster.y = imax(0, g_mouse.y - ZOOMBOX);
		
		glRasterPos(raster.x, raster.y);
		
		glDrawPixels(zoompix2.sizex, zoompix2.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix2.data);
#endif
	}
#endif
	
	CHECKGLERROR();

	EndS();
	CHECKGLERROR();

	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	//glEnable(GL_DEPTH_TEST);
	//DrawSelector();
	//DrawMarquee();

	CHECKGLERROR();
	EndS();
	CHECKGLERROR();

	glEnable(GL_DEPTH_TEST);
}

void GUI::inev(InEv* ie)
{
	Widget::inev(ie);

	if(!ie->intercepted)
	{
		//if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT) Log("mouse up l");

		if(ie->type == INEV_MOUSEMOVE && mousemovefunc) mousemovefunc(ie);
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && lbuttondownfunc) lbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && lbuttonupfunc) lbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_MIDDLE && mbuttondownfunc) mbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_MIDDLE && mbuttonupfunc) mbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_RIGHT && rbuttondownfunc) rbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_RIGHT && rbuttonupfunc) rbuttonupfunc();
		else if(ie->type == INEV_MOUSEWHEEL && mousewheelfunc) mousewheelfunc(ie->amount);
		else if(ie->type == INEV_KEYDOWN && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();
		else if(ie->type == INEV_KEYUP && keyupfunc[ie->scancode]) keyupfunc[ie->scancode]();
		//cse fix
		else if(ie->type == INEV_COPY && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();
		else if(ie->type == INEV_PASTE && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();

		if(ie->type != INEV_MOUSEMOVE && anykeydownfunc) anykeydownfunc(-1);
	}
}

void GUI::reframe()
{
	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-1;
	m_pos[3] = g_height-1;

	Widget::reframe();
}

void SetStatus(const char* status, bool logthis)
{
	if(logthis)
	{
		Log(status);
		
	}

#if 0
	Log(status);
	
#endif
	/*
	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
	upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';*/

	GUI* gui = &g_gui;

	//gui->get("load")->get("status", WIDGET_TEXT)->m_text = upper;
	ViewLayer* loadingview = (ViewLayer*)gui->get("loading");

	if(!loadingview)
		return;

	Widget* statustext = loadingview->get("status");

	if(!statustext)
		return;

	statustext->m_text = RichText(UStr(status));
}

bool MousePosition()
{
	Vec2i old = g_mouse;
	SDL_GetMouseState(&g_mouse.x, &g_mouse.y);
	
#ifdef PLATFORM_MOBILE
	g_mouse.x >>= 1;
	g_mouse.y >>= 1;
#endif

	if(g_mouse.x == old.x && g_mouse.y == old.y)
		return false;

	return true;
}

void CenterMouse()
{
	g_mouse.x = g_width/2;
	g_mouse.y = g_height/2;
	
#ifndef PLATFORM_MOBILE
	SDL_WarpMouseInWindow(g_window, g_mouse.x, g_mouse.y);
#else
	//for mobile, half of real screen is full "drawing" screen
	SDL_WarpMouseInWindow(g_window, g_mouse.x*2, g_mouse.y*2);
#endif
}

void Ortho(int width, int height, float r, float g, float b, float a)
{
	CHECKGLERROR();
	UseS(SHADER_ORTHO);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->m_slot[SSLOT_WIDTH], (float)width);
	glUniform1f(s->m_slot[SSLOT_HEIGHT], (float)height);
	glUniform4f(s->m_slot[SSLOT_COLOR], r, g, b, a);
	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].slot[SSLOT_NORMAL]);
	g_currw = width;
	g_currh = height;
	CHECKGLERROR();
}
