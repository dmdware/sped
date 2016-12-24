

#include "../platform.h"
#include "../save/savesprite.h"
#include "rendersprite.h"
#include "rendertopo.h"
#include "compilebl.h"
#include "../app/appmain.h"
#include "../gui/gui.h"
#include "../save/compilemap.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../sim/tile.h"
#include "../math/camera.h"
#include "../render/shadow.h"
#include "../math/vec4f.h"
#include "../render/screenshot.h"
#include "../render/sortb.h"
#include "../app/seviewport.h"
#include "../debug.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"

char g_renderpath[SPE_MAX_PATH+1];
char g_renderbasename[SPE_MAX_PATH+1];
int g_rendertype = -1;
int g_renderframes;
int g_origwidth;
int g_origheight;
int g_deswidth;
int g_desheight;
Vec2i g_spritecenter;
Vec2i g_clipmin;
Vec2i g_clipmax;
float g_transpkey[3] = {255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f};
bool g_doframes = false;
bool g_dosides = false;
bool g_doinclines = false;
bool g_dorots = false;
bool g_usepalette = false;
int g_savebitdepth = 8;

Vec3f g_origlightpos;
Camera g_origcam;
int g_rendside;
int g_rendpitch, g_rendyaw, g_rendroll;
int g_nrendsides = 8;
bool g_antialias = true;
bool g_fit2pow = false;
bool g_exportdepth = false;
bool g_exportteam = false;
bool g_hidetexerr = false;

bool g_warned = false;

unsigned int g_rendertex[4];
unsigned int g_renderdepthtex[4];
unsigned int g_renderrb[4];
unsigned int g_renderfb[4];
bool g_renderbs = false;

void MakeFBO(int sample, int rendstage)
{
	if(g_renderbs)
		return;

	if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
	{
		g_renderbs = true;
#if 0   //OpenGL 3.0 way
#if 1
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		CHECKGLERROR();
#if 0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		CHECKGLERROR();
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		CHECKGLERROR();

		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		CHECKGLERROR();
#else
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//-------------------------
		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		//Attach 2D texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		//-------------------------
		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//-------------------------
#endif
#else   //OpenGL 1.4 way

        glGenTextures(1, &g_rendertex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_rendertex[sample]);
        glTexImage2D(GL_TEXTURE_2D, 0, rendstage == RENDSTAGE_COLOR ? GL_RGBA8 : GL_RGBA8, g_deswidth, g_desheight, 0, rendstage == RENDSTAGE_COLOR ? GL_RGBA : GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Log("gge1 "<<glGetError()<<std::endl;

        glGenTextures(1, &g_renderdepthtex[sample]);
        glBindTexture(GL_TEXTURE_2D, g_renderdepthtex[sample]);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //Log("gge2 "<<glGetError()<<std::endl;
        //glDrawBuffer(GL_NONE); // No color buffer is drawn
        //glReadBuffer(GL_NONE);

        glGenFramebuffers(1, &g_renderfb[sample]);
        glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[sample]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_rendertex[sample], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderdepthtex[sample], 0);

        //Log("gge3 "<<glGetError()<<std::endl;

        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT_EXT};
        glDrawBuffers(1, DrawBuffers);

        //Log("gge4 "<<glGetError()<<std::endl;

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            ErrMess("Error", "Couldn't create framebuffer for render.");
            return;
        }
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef DEBUG
//		Log(__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
//		Log(__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
#endif
	}
}

void DelFBO(int sample)
{
#if 0
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CHECKGLERROR();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	CHECKGLERROR();
	glDeleteFramebuffers(1, &g_renderfb);
	CHECKGLERROR();
	glDeleteRenderbuffers(1, &g_renderrb);
	CHECKGLERROR();
	glDeleteTextures(1, &g_rendertex);
	CHECKGLERROR();
#else
	//Delete resources
	glDeleteTextures(1, &g_rendertex[sample]);
	glDeleteTextures(1, &g_renderdepthtex[sample]);
	//glDeleteRenderbuffers(1, &g_renderrb);
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffers(1, &g_renderfb[sample]);
#ifdef DEBUG
	CHECKGLERROR();
#endif
#endif

	g_renderbs = false;
}

bool CallResize(int w, int h)
{
	//return false;	//don't resize, has bad consequences when resizing with ANTIALIAS_UPSCALE=4

	int maxsz[] = {0,0};
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxsz);

	if((w >= maxsz[0]*0.7f || h >= maxsz[0]*0.7f) && !g_warned)
	{
		g_warned = true;
		char msg[1024];
		sprintf(msg, "The required texture size of %dx%d exceeds or approaches your system's supported maximum of %d. You might not be able to finish the render. Reduce 1_tile_pixel_width in config.ini.", w, h, maxsz[0]);
		WarnMess("Warning", msg);
	}

	Log("resz %d,%d", w, h);

#if 1
	//w=2048;
	//h=2048;
#if 0
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)w;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)h;
#else
    //SDL_SetWindowSize(g_window, w, h);
	Resize(w, h);
#endif

	int startx = 0;
	int starty = 0;

#if 0
	if(g_fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//startx = CW_USEDEFAULT;
		//starty = CW_USEDEFAULT
		startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selres.width/2;
		starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selres.height/2;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
#endif

#if 0
	//Log("rf", g_renderframe<<" desired: "<<w<<","<<h<<", g_wh: "<<g_width<<","<<g_height<<" wr.rl: "<<(WindowRect.right-WindowRect.left)<<","<<(WindowRect.bottom-WindowRect.top)<<std::endl;
	
#endif

#if 0
	if(WindowRect.right-WindowRect.left == g_width
		&& WindowRect.bottom-WindowRect.top == g_height)
#endif
	{
		if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
		{
			g_width = w;
			g_height = h;
		}

		return false;
	}

#if 0
	SetWindowPos(g_hWnd,0,0,0,WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif

	if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
	{
		g_width = w;
		g_height = h;
	}

	return true;
#else

	return true;
#endif
}

bool FitFocus(Vec2i vmin, Vec2i vmax)
{
#if 0
	int xextent = max(abs(vmin.x-g_width/2), abs(vmax.x-g_width/2));
	int yextent = max(abs(vmin.y-g_height/2), abs(vmax.y-g_height/2));
#else
	int xextent = imax(abs(vmin.x-g_width/2), abs(vmax.x-g_width/2));
	int yextent = imax(abs(vmin.y-g_height/2), abs(vmax.y-g_height/2));
#endif

	//int width2 = Max2Pow(xextent*2);
	//int height2 = Max2Pow(yextent*2);

	int width = xextent*2;
	int height = yextent*2;

	Log("ftf xye %d,%d", xextent, yextent);

	//g_deswidth = width;
	//g_desheight = height;

	//g_spritecenter.x = xextent;
	//g_spritecenter.y = yextent;

	// size must be multiple of 32 or else glReadPixels will crash !!!!!
	Vec2i compatsz;
	//compatsz.x = Max2Pow32(g_width);
	//compatsz.y = Max2Pow32(g_height);
	if(g_fit2pow)
	{
		compatsz.x = Max2Pow32(width);
		compatsz.y = Max2Pow32(height);
	}
	else
	{
		compatsz.x = (width);
		compatsz.y = (height);
	}

	g_deswidth = compatsz.x;
	g_desheight = compatsz.y;

	Log("ftf des %d,%d", g_deswidth, g_desheight);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" o des wh "<<g_deswidth<<","<<g_desheight<<"  xyextn:"<<xextent<<","<<yextent<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<") gwh:"<<g_width<<","<<g_height<<" "<<std::endl;
#endif

	//Sleep(10);

	return CallResize(compatsz.x, compatsz.y);

#if 0
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_projtype = PROJ_ORTHO;
	g_cam.position(0, 0, 1000.0f, 0, 0, 0, 0, 1, 0);
	g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
	g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);

	g_zoom = 1;

	Vec3f topleft(-g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomleft(-g_tilesize/2, 0, g_tilesize/2);
	Vec3f topright(g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomright(g_tilesize/2, 0, g_tilesize/2);

	int width;
	int height;

	if(g_mode == RENDERING)
	{
		width = g_width;
		height = g_height;
	}
	//if(g_mode == EDITOR)
	else
	{
		View* edview = g_gui.get("editor");
		Widget* viewportsframe = edview->get("viewports frame", WIDGET_FRAME);
		Widget* toprightviewport = viewportsframe->get("top right viewport", WIDGET_VIEWPORT);
		width = toprightviewport->m_pos[2] - toprightviewport->m_pos[0];
		height = toprightviewport->m_pos[3] - toprightviewport->m_pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	//VpWrap* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_cam.m_view;
	Vec3f viewvec = g_cam.m_view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->m_offset;
	Vec3f posvec = g_cam.m_pos;
	//Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.m_view + t->m_offset;
		//viewvec = posvec + Normalize(g_cam.m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
    //Vec3f upvec = t->m_up;
    Vec3f upvec = g_cam.m_up;
	//Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

	persp = false;

	Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	float minx = min(topleft4.x, min(topright4.x, min(bottomleft4.x, bottomright4.x)));
	float maxx = max(topleft4.x, max(topright4.x, max(bottomleft4.x, bottomright4.x)));
	//float miny = min(topleft4.y, min(topright4.y, min(bottomleft4.y, bottomright4.y)));
	//float maxy = max(topleft4.y, max(topright4.y, max(bottomleft4.y, bottomright4.y)));

	float xrange = (float)maxx - (float)minx;

	if(xrange <= 0.0f)
		xrange = g_1tilewidth;

	float zoomscale = (float)g_1tilewidth / xrange;

	g_zoom *= zoomscale;

	//Log("zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<std::endl;

	g_mode = PREREND_ADJFRAME;
#endif
}

void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height)
{
	//int width = g_width;
	//int height = g_height;

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

#ifdef DEBUG
	{

		Log("rf"<<g_renderframe<<" params:"<<aspect<<","<<width<<","<<height<<","<<g_zoom<<std::endl;
		Log("rf"<<g_renderframe<<" pmat0:"<<projection.m_matrix[0]<<","<<projection.m_matrix[1]<<","<<projection.m_matrix[2]<<","<<projection.m_matrix[3]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat1:"<<projection.m_matrix[4]<<","<<projection.m_matrix[5]<<","<<projection.m_matrix[6]<<","<<projection.m_matrix[7]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat2:"<<projection.m_matrix[8]<<","<<projection.m_matrix[9]<<","<<projection.m_matrix[10]<<","<<projection.m_matrix[11]<<std::endl;
		Log("rf"<<g_renderframe<<" pmat3:"<<projection.m_matrix[12]<<","<<projection.m_matrix[13]<<","<<projection.m_matrix[14]<<","<<projection.m_matrix[15]<<std::endl;
	}
#endif

	//VpWrap* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_cam.m_view;
	////Vec3f viewvec = g_cam.m_view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;

	/////Vec3f dir = Normalize( g_cam.m_view - g_cam.m_pos );
	/////Vec3f posvec = g_cam.m_view - dir * 100000.0f / g_zoom;

	//Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.m_view + t->m_offset;
		//viewvec = posvec + Normalize(g_cam.m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
    //Vec3f upvec = t->m_up;
    //////Vec3f upvec = g_cam.m_up;
    //Vec3f upvec = g_cam.up2();
	//Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	////Vec3f focusvec = viewvec;

    /////Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	/////Matrix mvpmat;
	/////mvpmat.set(projection.m_matrix);
	/////mvpmat.postmult(viewmat);


//	float aspect = fabsf((float)g_width / (float)g_height);
//	Matrix projection;

	VpWrap* v = &g_viewport[3];
	VpType* t = &g_vptype[v->m_type];

//	bool persp = false;

#if 0
	if(g_projtype == PROJ_PERSP && v->m_type == VIEWPORT_ANGLE45O)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else if(g_projtype == PROJ_ORTHO || v->m_type != VIEWPORT_ANGLE45O)
#endif
	{
	//	projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	Vec3f offset = Vec3f(0,0,0);
	//Vec3f viewvec = g_focus;	//g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view;
	Vec3f focusvec = v->focus() + offset;
	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos() + offset;

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	posvec = g_cam.m_view + t->m_offset;

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = g_cam.m_up;
	Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

	Matrix modelview;
	Matrix modelmat;
	float translation[] = {0, 0, 0};
	modelview.setTranslation(translation);
	modelmat.setTranslation(translation);
	modelview.postmult(viewmat);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult2(viewmat);

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" vmat0:"<<viewmat.m_matrix[0]<<","<<viewmat.m_matrix[1]<<","<<viewmat.m_matrix[2]<<","<<viewmat.m_matrix[3]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat1:"<<viewmat.m_matrix[4]<<","<<viewmat.m_matrix[5]<<","<<viewmat.m_matrix[6]<<","<<viewmat.m_matrix[7]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat2:"<<viewmat.m_matrix[8]<<","<<viewmat.m_matrix[9]<<","<<viewmat.m_matrix[10]<<","<<viewmat.m_matrix[11]<<std::endl;
		Log("rf"<<g_renderframe<<" vmat3:"<<viewmat.m_matrix[12]<<","<<viewmat.m_matrix[13]<<","<<viewmat.m_matrix[14]<<","<<viewmat.m_matrix[15]<<std::endl;
	}
#endif

	persp = false;

	//Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	//Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	//Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	//Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	bool showsky = false;

	bool setmm[] = {false, false, false, false};

	Heightmap hm;

	if((g_mode == PREREND_ADJFRAME ||
		g_mode == RENDERING) &&
		g_doinclines)
	{
		float heights[4];
		//As said about "g_cornerinc":
		//corners in order of digits displayed on name, not in clock-wise corner order
		//So we have to reverse using (3-x).
		//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
		//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
		//Edit: or no...
		heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
		heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
		//important, notice "g_cornerinc" uses clock-wise ordering of corners
		heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
		heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

		//Heightmap hm;
		hm.allocate(1, 1);
		//x,z, y
		//going round the corners clockwise
		hm.setheight(0, 0, heights[0]);
		hm.setheight(1, 0, heights[1]);
		hm.setheight(1, 1, heights[2]);
		hm.setheight(0, 1, heights[3]);
		hm.remesh();
	}

#if 1
	for(std::list<Brush>::iterator b=g_edmap.m_brush.begin(); b!=g_edmap.m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		//for(int i=0; i<1; i++)
		{
			BrushSide* side = &b->m_sides[i];
			VertexArray* va = &side->m_drawva;

			for(int j=0; j<va->numverts; j++)
			{
				Vec3f v = va->vertices[j];

				if((g_mode == PREREND_ADJFRAME ||
					g_mode == RENDERING) &&
					g_doinclines)
				{
					//TransformedPos[i].y += Bilerp(&hm,
					//	g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
					//	g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
					v.y += hm.accheight(
						g_tilesize/2.0f + v.x,
						g_tilesize/2.0f + v.z);
				}

				Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
				Log("rf"<<g_renderframe<<" v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
#endif

#if 0
				if(floorf(v4.x) < vmin->x)
					vmin->x = floorf(v4.x);
				if(floorf(v4.y) < vmin->y)
					vmin->y = floorf(v4.y);
				if(ceilf(v4.x) > vmax->x)
					vmax->x = ceilf(v4.x);
				if(ceilf(v4.y) > vmax->y)
					vmax->y = ceilf(v4.y);
#endif

				if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
				{
					vmin->x = floor(v4.x+0.5f);
					setmm[0] = true;
				}
				if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
				{
					vmin->y = floor(v4.y+0.5f);
					setmm[1] = true;
				}
				if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
				{
					vmax->x = floor(v4.x+0.5f);
					setmm[2] = true;
				}
				if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
				{
					vmax->y = floor(v4.y+0.5f);
					setmm[3] = true;
				}
			}
		}

		//break;
	}
#endif

#ifdef DEBUG
	//for(int my=0; my<4; my++)
	{
		Log("rf"<<g_renderframe<<" mat0:"<<mvpmat.m_matrix[0]<<","<<mvpmat.m_matrix[1]<<","<<mvpmat.m_matrix[2]<<","<<mvpmat.m_matrix[3]<<std::endl;
		Log("rf"<<g_renderframe<<" mat1:"<<mvpmat.m_matrix[4]<<","<<mvpmat.m_matrix[5]<<","<<mvpmat.m_matrix[6]<<","<<mvpmat.m_matrix[7]<<std::endl;
		Log("rf"<<g_renderframe<<" mat2:"<<mvpmat.m_matrix[8]<<","<<mvpmat.m_matrix[9]<<","<<mvpmat.m_matrix[10]<<","<<mvpmat.m_matrix[11]<<std::endl;
		Log("rf"<<g_renderframe<<" mat3:"<<mvpmat.m_matrix[12]<<","<<mvpmat.m_matrix[13]<<","<<mvpmat.m_matrix[14]<<","<<mvpmat.m_matrix[15]<<std::endl;
	}
#endif

#if 1
	for(std::list<ModelHolder>::iterator iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		//Model2* m = &g_model2[h->modeli];
		Model2* m = &h->model;
		Model2* origm = &g_model2[h->modeli];

		//m->usetex();
		//DrawVA(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);

#if 0	//TODO
		VertexArray* va = &h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i] + h->translation;
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
//#ifdef 1
			Log("rf"<<g_renderframe<<" mdl v:"<<v.x<<","<<v.y<<","<<v.z<<std::endl;
			Log("rf"<<g_renderframe<<" mdl v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
#endif

#if 0
			if(floorf(v4.x) < vmin->x)
				vmin->x = floorf(v4.x);
			if(floorf(v4.y) < vmin->y)
				vmin->y = floorf(v4.y);
			if(ceilf(v4.x) > vmax->x)
				vmax->x = ceilf(v4.x);
			if(ceilf(v4.y) > vmax->y)
				vmax->y = ceilf(v4.y);
#endif

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = true;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = true;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = true;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = true;
			}
		}
#else
		//perform frame transformation on-the-fly
		std::vector<Matrix> BoneTransforms;

		if(m->m_pScene->mNumAnimations > 0)
		{
			float TicksPerSecond = (float)(m->m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m->m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
			float frames = TicksPerSecond * (float)m->m_pScene->mAnimations[0]->mDuration;
			//int frame = g_renderframe % (int)frames;
			int frame = g_renderframe;
			float percentage = (float)frame / frames;
			float RunningTime = percentage * (float)m->m_pScene->mAnimations[0]->mDuration;

			m->BoneTransform(RunningTime, BoneTransforms);
		}
		else
		{
			BoneTransforms.resize( m->m_BoneInfo.size() );

			for(int i=0; i<m->m_BoneInfo.size(); i++)
			{
				BoneTransforms[i].InitIdentity();
			}
		}

		std::vector<Vec3f> TransformedPos;
		std::vector<Vec3f> TransformedNorm;
		TransformedPos.resize(m->Positions.size());
		TransformedNorm.resize(m->Normals.size());

		for(uint i=0; i<m->Positions.size(); i++)
		{
			Matrix Transform(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
			Transform.InitIdentity();

			//bool influenced = false;

			for(int bi=0; bi<NUM_BONES_PER_VERTEX; bi++)
			{
				if(m->Bones[i].IDs[bi] < 0)
					continue;

				//if(m->Bones[i].IDs[bi] >= m->m_BoneInfo.size())
				//	continue;

				//if(m->Bones[i].IDs[bi] >= BoneTransforms.size())
				//	continue;

				if(m->Bones[i].Weights[bi] == 0.0f)
					continue;

				//influenced = true;

				//Transform += m_BoneInfo[ Bones[i].IDs[bi] ].FinalTransformation * Bones[i].Weights[bi];

				if(bi == 0)
					Transform = BoneTransforms[ m->Bones[i].IDs[bi] ] * m->Bones[i].Weights[bi];
				else
					Transform += BoneTransforms[ m->Bones[i].IDs[bi] ] * m->Bones[i].Weights[bi];
			}

			//if(!influenced)
			//	Transform.InitIdentity();

			Vec4f Transformed = Vec4f(origm->Positions[i], 1.0f);
			Transformed.transform(Transform);
			Transformed.transform(h->rotationmat);
			//Transformed = Transformed * h->scale;
			TransformedPos[i]    = Vec3f(Transformed.x, Transformed.y, Transformed.z) * h->scale;
			TransformedNorm[i] = origm->Normals[i];
			TransformedNorm[i].transform(Transform);
			TransformedNorm[i].transform(h->rotationmat);
		}

		//If we're including inclines, adjust vertex heights
		if((g_mode == RENDERING || g_mode == PREREND_ADJFRAME) &&
			g_doinclines)
		{
			float heights[4];
			//As said about "g_cornerinc":
			//corners in order of digits displayed on name, not in clock-wise corner order
			//So we have to reverse using (3-x).
			//[0] corresponds to x000 where x is the digit. However this is the LAST corner (west corner).
			//[1] corresponds to 0x00 where x is the digit. However this is the 3rd corner (south corner).
			//Edit: or no...
			heights[0] = g_cornerinc[g_currincline][0] * TILE_RISE;
			heights[1] = g_cornerinc[g_currincline][1] * TILE_RISE;
			//important, notice "g_cornerinc" uses clock-wise ordering of corners
			heights[2] = g_cornerinc[g_currincline][2] * TILE_RISE;
			heights[3] = g_cornerinc[g_currincline][3] * TILE_RISE;

			Heightmap hm;
			hm.allocate(1, 1);
			//x,z, y
			//going round the corners clockwise
			hm.setheight(0, 0, heights[0]);
			hm.setheight(1, 0, heights[1]);
			hm.setheight(1, 1, heights[2]);
			hm.setheight(0, 1, heights[3]);
			hm.remesh();

			for(uint i=0; i<TransformedPos.size(); i++)
			{
				//TransformedPos[i].y += Bilerp(&hm,
				//	g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
				//	g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
				TransformedPos[i].y += hm.accheight(
					g_tilesize/2.0f + h->translation.x + TransformedPos[i].x,
					g_tilesize/2.0f + h->translation.z + TransformedPos[i].z);
			}

			//Regenerate normals:
			//Not possible, based on vertices alone, because we would also need to blend shared faces,
			//so leave this inaccuracy for now. TODO
		}

		for (uint i = 0 ; i < m->m_Entries.size() ; i++)
		{
			const unsigned int numindices = m->m_Entries[i].NumIndices;
			const unsigned int basevertex = m->m_Entries[i].BaseVertex;
			const unsigned int baseindex = m->m_Entries[i].BaseIndex;
			const unsigned int numunique = m->m_Entries[i].NumUniqueVerts;

			for(unsigned int indi=baseindex; indi<baseindex+numindices; indi++)
			{
			#if 0
				unsigned int ind0 = indi + 0;
				unsigned int ind1 = indi + 1;
				unsigned int ind2 = indi + 2;
				Vec3f v0 = TransformedPos[basevertex + ind0];
				Vec3f v1 = TransformedPos[basevertex + ind1];
				Vec3f v2 = TransformedPos[basevertex + ind2];
			#endif
				Vec3f v = TransformedPos[basevertex + m->Indices[indi]] + h->translation;
				Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

				if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
				{
					vmin->x = floor(v4.x+0.5f);
					setmm[0] = true;
				}
				if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
				{
					vmin->y = floor(v4.y+0.5f);
					setmm[1] = true;
				}
				if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
				{
					vmax->x = floor(v4.x+0.5f);
					setmm[2] = true;
				}
				if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
				{
					vmax->y = floor(v4.y+0.5f);
					setmm[3] = true;
				}
			}
		}
#if 0
		for(int i=0; i<h->model.Positions.size(); i++)
		{
			Vec3f v = h->model.Positions[i] + h->translation;
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = true;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = true;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = true;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = true;
			}
		}
#endif
#endif
	}


	if(g_tiletexs[TEX_DIFF] != 0)
	{
		VertexArray* va = &g_tileva[g_currincline];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i];
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = true;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = true;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = true;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = true;
			}
		}
	}
#ifdef DEBUG
	Log("rf"<<g_renderframe<<" setmm:"<<setmm[0]<<","<<setmm[1]<<","<<setmm[2]<<","<<setmm[3]<<std::endl;
	Log("rfvminmax "<<vmin->x<<","<<vmin->y<<"->"<<vmax->x<<","<<vmax->y<<std::endl;
#endif
#endif

#if 0
	//not needed anymore, we upscale the render resolution,
	//then downscale it to intended size.
	if(g_antialias)
	{
		//make room for "blur pixels" around the edges
		vmax->x++;
		vmax->y++;
	}
#endif
}

void PrepareRender(const char* fullpath, int rendtype)
{
	g_rendtopo = false;
	//g_mode = PREREND_ADJFRAME;
	g_rendertype = rendtype;
	strcpy(g_renderbasename, fullpath);
	g_gui.hideall();
	g_gui.show("render");
	//glClearColor(255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f, 1.0f);
	g_renderframe = 0;
	g_origlightpos = g_lightPos;
	g_origcam = g_cam;
	g_rendside = 0;
	g_rendpitch = 0;
	g_rendyaw = 0;
	g_rendroll = 0;
	g_renderframes = GetNumFrames();
	GetDoFrames();
	GetDoInclines();
	GetDoSides();
	GetDoRotations();
	g_origwidth = g_width;
	g_origheight = g_height;
	g_warned = false;
	ResetView(true);
	AdjustFrame();
	
	char sppath[SPE_MAX_PATH+1];
	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");
	sprintf(sppath, "%s_list.txt", g_renderbasename);
	FILE* fp = fopen(sppath, "w");
	if(g_doframes)
		fprintf(fp, "frames %d %d\r\n", g_doframes ? 1 : 0, g_renderframes);
	if(g_dosides && !g_dorots)
		fprintf(fp, "sides %d %d\r\n", g_dosides ? 1 : 0, g_nrendsides);
	else if(g_dorots)
		fprintf(fp, "rotations %d %d\r\n", g_dorots ? 1 : 0, g_nrendsides);
	if(g_doinclines)
		fprintf(fp, "inclines %d\r\n", g_doinclines ? 1 : 0);
	fclose(fp);
}

void AdjustFrame(bool resetview)
{
	g_mode = PREREND_ADJFRAME;
	//CallResize(MAX_TEXTURE, MAX_TEXTURE);
	if(resetview)
		ResetView(true);
	Vec2i vmin(g_width/2, g_height/2);
	Vec2i vmax(g_width/2, g_height/2);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" adjf1 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")");
#endif

	RotateView();
	AllScreenMinMax(&vmin, &vmax, g_width, g_height);

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" asmm adjf2 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")");
#endif

	//if(vmin.x < 0)
	//	ErrMess("asda","asdsad");
	//if(vmin.x < 0)
	//	exit(0);

	Log("fit %d,%d->%d,%d", vmin.x, vmin.y, vmax.x, vmax.y);

	if(!FitFocus(vmin, vmax))
		g_mode = RENDERING;
	g_mode = RENDERING;
}

void SaveRender(int rendstage)
{
#if 0
	SaveScreenshot();

	Log("sv r");
	
#endif

	LoadedTex prescreen;
	LoadedTex screen;

#if 0
	AllocTex(&screen, g_width, g_height, 3);
	memset(screen.data, 0, g_width * g_height * 3);

	// size must be multiple of 32 or else this will crash !!!!!
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screen.data);
	FlipImage(&screen);
#else
	int channels = 4;

	if(rendstage == RENDSTAGE_TEAM)
		channels = 1;

	//Must read RGBA from FBO, can't read GL_RED directly for team colour mask for some reason
	AllocTex(&prescreen, g_width, g_height, 4);
	memset(prescreen.data, 0, g_width * g_height * 4);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// size must be multiple of 32 or else this will crash !!!!!
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glReadPixels(0, 0, g_width, g_height, channels == 4 ? GL_RGBA : GL_RGBA, GL_UNSIGNED_BYTE, prescreen.data);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	FlipImage(&prescreen);
#endif

	AllocTex(&screen, g_width, g_height, channels);
	memset(screen.data, 0, g_width * g_height * channels);

	//Convert to 1-channel if team colour mask render
	for(int x=0; x<g_width; x++)
		for(int y=0; y<g_height; y++)
		{
			int i = ( x + y * g_width ) * 4;
			int i2 = ( x + y * g_width ) * channels;

			for(int c=0; c<channels; c++)
				screen.data[i2+c] = prescreen.data[i+c];
		}

	//if(g_clipmin.x < 0)
	//	ErrMess("asd","asda");
	//if(g_clipmin.x<0)
	//	exit(0);

	Vec2i clipsz;
	clipsz.x = g_clipmax.x - g_clipmin.x;
	clipsz.y = g_clipmax.y - g_clipmin.y;

#ifdef DEBUG
	char msg[128];
	sprintf(msg, "clipsz %d,%d->%d,%d sz(%d,%d)", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y, clipsz.x, clipsz.y);
	Log(msg<<std::endl;
	//InfoMess(msg, msg);
#endif

	int imagew;
	int imageh;

	if(!g_fit2pow)
	{
		imagew = clipsz.x;
		imageh = clipsz.y;
	}
	else
	{
		imagew = Max2Pow(clipsz.x);
		imageh = Max2Pow(clipsz.y);
	}

#if 0
	int downimagew = imagew;
	int downimageh = imageh;
	Vec2i downclipsz = clipsz;

	if(g_antialias)
	{
		//deal with non-power-of-2 upscales.
		downclipsz = clipsz / ANTIALIAS_UPSCALE;

	}
#endif

#ifdef DEBUG
	Log("rf"<<g_renderframe<<" gwh"<<g_width<<","<<g_height<<" clipxymm "<<g_clipmin.x<<","<<g_clipmin.y<<"->"<<g_clipmax.x<<","<<g_clipmax.y<<" clipsz "<<clipsz.x<<","<<clipsz.y<<" imgwh "<<imagew<<","<<imageh<<std::endl;
	

	Log("sv r 4");
	
#endif

	LoadedTex sprite;
	AllocTex(&sprite, imagew, imageh, channels);

	int transpkey[3] = {(int)(g_transpkey[0]*255), (int)(g_transpkey[1]*255), (int)(g_transpkey[2]*255)};

	int xoff = g_clipmin.x;
	int yoff = g_clipmin.y;

	//if(g_deswidth != g_width)	xoff += (g_width-g_deswidth)/2;
	//if(g_desheight != g_height)	yoff += (g_height-g_desheight)/2;

#ifdef DEBUG
	Log("des wh "<<g_deswidth<<","<<g_desheight<<" actl "<<g_width<<","<<g_height<<std::endl;
	Log("xy off "<<xoff<<","<<yoff<<std::endl;
	
#endif

#if 1
	for(int x=0; x<imagew; x++)
		for(int y=0; y<imageh; y++)
		{
			int index = channels * ( y * imagew + x );
#if 0
			sprite.data[index + 0] = transpkey[0];
			sprite.data[index + 1] = transpkey[1];
			sprite.data[index + 2] = transpkey[2];
			sprite.data[index + 3] = 0;
#else
			for(int c=0; c<channels; c++)
				sprite.data[index + c] = 0;
#endif
		}
#endif

#if 1
	for(int x=0; x<clipsz.x && x<g_width; x++)
		for(int y=0; y<clipsz.y && y<g_height; y++)
		{
			int index = channels * ( y * imagew + x );
			int index2 = channels * ( (y+yoff) * g_width + (x+xoff) );

			//index2 = imin( g_width * g_height, index2 );
			
				if(index < 0 || index >= clipsz.x * clipsz.y * 4)
					ErrMess("!12","!12");
				if(index < 0)
					ErrMess("!1111","!1111");
				if(index >= clipsz.x * clipsz.y * 4)
					ErrMess("!222","!222");
				if(index >= clipsz.x * clipsz.y * 4)
				{
					char mm[234];
					sprintf(mm, "cs %d,%d", clipsz.x, clipsz.y);
					ErrMess(mm,mm);
				}
				if(imagew < 0)
					ErrMess("!123","!123");
				
				if(index2 < 0 || index2 >= g_width * g_height * 4)
					ErrMess("!13","!13");

#if 0
			//should have RGBA in FBO
			if(channels >= 3
			&& screen.data[index2+0] == transpkey[0]
			&& screen.data[index2+1] == transpkey[1]
			&& screen.data[index2+2] == transpkey[2])
				continue;
#endif
			//Log(" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" ");
			//

			for(int c=0; c<channels; c++)
				sprite.data[index+c] = screen.data[index2+c];
		}
#endif

	int finalimagew = imagew;
	int finalimageh = imageh;
	Vec2i finalclipsz = clipsz;
	Vec2i finalclipmin = g_clipmin;
	Vec2i finalclipmax = g_clipmax;
	Vec2i finalcenter = g_spritecenter;

	LoadedTex finalsprite;

	if(g_antialias)
	{
		//downsample the sprite
		//and update the clip paramaters.

		int downimagew = imagew / ANTIALIAS_UPSCALE;
		int downimageh = imageh / ANTIALIAS_UPSCALE;
		Vec2i downclipsz = clipsz / ANTIALIAS_UPSCALE;
		Vec2i downclipmin = g_clipmin / ANTIALIAS_UPSCALE;
		Vec2i downclipmax = g_clipmax / ANTIALIAS_UPSCALE;

		AllocTex(&finalsprite, downimagew, downimageh, channels);

		for(int x=0; x<downimagew; x++)
			for(int y=0; y<downimageh; y++)
			{
				int index = channels * ( y * downimagew + x );

				if(index < 0 || index >= downimagew * downimageh * 4)
					ErrMess("!1","!1");
#if 0
				sprite.data[index + 0] = transpkey[0];
				sprite.data[index + 1] = transpkey[1];
				sprite.data[index + 2] = transpkey[2];
				sprite.data[index + 3] = 0;
#else
				for(int c=0; c<channels; c++)
					finalsprite.data[index + c] = 0;
#endif
			}

		for(int downx=0; downx<downclipsz.x; downx++)
			for(int downy=0; downy<downclipsz.y; downy++)
			{
				int downindex = channels * ( downy * downimagew + downx );
				//int upindex2 = 4 * ( (y+yoff) * g_width + (x+xoff) );

				unsigned int samples[4] = {0,0,0,0};

				int contributions = 0;

				for(int upx=0; upx<ANTIALIAS_UPSCALE; upx++)
					for(int upy=0; upy<ANTIALIAS_UPSCALE; upy++)
					{
						int upindex = channels * ( (downy*ANTIALIAS_UPSCALE + upy) * imagew + (downx*ANTIALIAS_UPSCALE + upx) );

						unsigned char* uppixel = &sprite.data[upindex];

						//if it's a transparent pixel, we don't want to
						//blend in the transparency key color.
						//instead, blend in black.
						if(channels == 4
						&&
							((uppixel[0] == transpkey[0]
							&& uppixel[1] == transpkey[1]
							&& uppixel[2] == transpkey[2])
							||
							(uppixel[3] == 0))
						)
						{
							samples[0] += 0;
							samples[1] += 0;
							samples[2] += 0;
							samples[3] += 0;
						}
						else
						{
							for(int c=0; c<channels; c++)
								samples[c] += uppixel[c];
							contributions++;
						}
					}

				//average the samples
				if(contributions > 0)
					for(int c=0; c<channels; c++)
						samples[c] /= contributions;
	#if 0
				//should have RGBA in FBO
				if(screen.data[index2+0] == transpkey[0]
				&& screen.data[index2+1] == transpkey[1]
				&& screen.data[index2+2] == transpkey[2])
					continue;
	#endif
				//Log(" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" ");
				//

				for(int c=0; c<channels; c++)
					finalsprite.data[downindex+c] = samples[c];
			}


		finalimagew = downimagew;
		finalimageh = downimageh;
		finalclipsz = downclipsz;
		finalclipmin = downclipmin;
		finalclipmax = downclipmax;
		finalcenter = finalcenter / ANTIALIAS_UPSCALE;
	}
	else
	{
		//no downsampling.
		AllocTex(&finalsprite, imagew, imageh, channels);

		for(int x=0; x<imagew; x++)
			for(int y=0; y<imageh; y++)
			{
				int index = channels * ( y * imagew + x );

				if(index < 0 || index >= imagew * imageh * 4)
					ErrMess("!11","!11");

				for(int c=0; c<channels; c++)
					finalsprite.data[index + c] = sprite.data[index + c];
			}
	}

	char fullpath[SPE_MAX_PATH+1];

	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");

	if(g_doframes)
		sprintf(frame, "_fr%03d", g_renderframe);

	if(g_dosides && !g_dorots)
		sprintf(side, "_si%d", g_rendside);
	else if(g_dorots)
		sprintf(side, "_y%dp%dr%d", g_rendyaw, g_rendpitch, g_rendroll);

	std::string incline = "";

	if(g_doinclines)
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
	else if(rendstage == RENDSTAGE_DEPTH)
		stage = "_depth";

	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &finalsprite);
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);
	
	finalclipmax.x = finalclipmax.x - finalclipmin.x;
	finalclipmax.y = finalclipmax.y - finalclipmin.y;
	finalclipmin.x = 0;
	finalclipmin.y = 0;

	sprintf(fullpath, "%s%s%s%s.txt", g_renderbasename, side, frame, incline.c_str());
	std::ofstream ofs(fullpath, std::ios_base::out);
	ofs<<finalcenter.x<<" "<<finalcenter.y<<std::endl;
	ofs<<finalimagew<<" "<<finalimageh<<std::endl;
	ofs<<finalclipsz.x<<" "<<finalclipsz.y<<std::endl;
	ofs<<finalclipmin.x<<" "<<finalclipmin.y<<" "<<finalclipmax.x<<" "<<finalclipmax.y;
}

void SpriteRender(int rendstage, Vec3f offset)
{
	glViewport(0, 0, g_width, g_height);
	if(rendstage == RENDSTAGE_TEAM)
		glClearColor(0,0,0,0);
		//glClearColor(g_transpkey[0],g_transpkey[1],g_transpkey[2],1);
	else if(rendstage == RENDSTAGE_COLOR)
		//glClearColor(g_transpkey[0],g_transpkey[1],g_transpkey[2],0);
		glClearColor(0,0,0,0);
	else if(rendstage == RENDSTAGE_DEPTH)
		glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef DEBUG
	CHECKGLERROR();
	Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
	Log(__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
#endif

	{
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection;

		VpWrap* v = &g_viewport[3];
		VpType* t = &g_vptype[v->m_type];

		bool persp = false;

#if 0
		if(g_projtype == PROJ_PERSP && v->m_type == VIEWPORT_ANGLE45O)
		{
			projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
			persp = true;
		}
		else if(g_projtype == PROJ_ORTHO || v->m_type != VIEWPORT_ANGLE45O)
#endif
		{
			projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
		}

		//Vec3f viewvec = g_focus;	//g_cam.m_view;
		//Vec3f viewvec = g_cam.m_view;
		Vec3f focusvec = v->focus() + offset;
		//Vec3f posvec = g_focus + t->m_offset;
		//Vec3f posvec = g_cam.m_pos;
		Vec3f posvec = v->pos() + offset;

		//if(v->m_type != VIEWPORT_ANGLE45O)
		//	posvec = g_cam.m_view + t->m_offset;

		//viewvec = posvec + Normalize(viewvec-posvec);
		//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
		//Vec3f upvec = t->m_up;
		//Vec3f upvec = g_cam.m_up;
		Vec3f upvec = v->up();

		//if(v->m_type != VIEWPORT_ANGLE45O)
		//	upvec = t->m_up;

		Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.setTranslation(translation);
		modelmat.setTranslation(translation);
		modelview.postmult(viewmat);

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postmult2(viewmat);

#if 0
		if(g_rendtopo)
		{
			Matrix orlon;	//up down
			Matrix orlat;	// left right
			
			orlon.InitRotateTransform(180 - 180.0 * g_rendlon / (float)g_orlons, 0, 0);

			mvpmat.postmult2(&orlon);

			orlat.InitRotateTransform(0, 360 - 360.0 * g_rendlat / (float)g_orlats, 0);

			mvpmat.postmult2(&orlat);
		}
#endif

#if 1
		if(v->m_type == VIEWPORT_ANGLE45O)
		{
			//RenderToShadowMap(projection, viewmat, modelmat, g_focus);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			//RenderToShadowMap(projection, viewmat, modelmat, g_cam.m_view);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			if(rendstage == RENDSTAGE_COLOR)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
			else if(rendstage == RENDSTAGE_TEAM)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawSceneTeam);
			else if(rendstage == RENDSTAGE_DEPTH)
				RenderToDepthMap(projection, viewmat, modelmat, g_cam.m_view, DrawSceneDepth);
		}
#endif
	}

#ifdef DEBUG
	CHECKGLERROR();
#endif
	glFlush();
#ifdef DEBUG
	CHECKGLERROR();
#endif
	//glFinish();
#ifdef DEBUG
	CHECKGLERROR();
#endif
}

void RotateView()
{
	
	if(!g_dorots)
	{
		g_lightPos = Rotate(g_lightPos, g_rendside*(DEGTORAD(360.0)/(float)g_nrendsides), 0, 1, 0);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendside*(DEGTORAD(360.0)/(float)g_nrendsides), 0, 1, 0);
	}
	else
	{
		Vec3f pitchaxis(1,0,0);
		Vec3f yawaxis(0,1,0);
		Vec3f rollaxis(0,0,1);
		
		//g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderyaw), 0, 1, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderpitch), 1, 0, 0);

#if 0
		//g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);
		
		pitchaxis = Rotate(pitchaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		yawaxis = Rotate(yawaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		rollaxis = Rotate(rollaxis, -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		
		pitchaxis = Rotate(pitchaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
		yawaxis = Rotate(yawaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
		rollaxis = Rotate(rollaxis, DEGTORAD(g_defrenderyaw), 0, 1, 0);
#endif
		
#if 0
		char m[123];
		sprintf(m, "before (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
			g_cam.m_pos.x, g_cam.m_pos.y, g_cam.m_pos.z,
			g_cam.m_view.x, g_cam.m_view.y, g_cam.m_view.z,
			g_cam.up2().x, g_cam.up2().y, g_cam.up2().z);
		InfoMess(m,m);
#endif

		//rollaxis = Rotate(rollaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		pitchaxis = Rotate(pitchaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		yawaxis = Rotate(yawaxis, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides), rollaxis.x, rollaxis.y, rollaxis.z);
		
#if 0
		sprintf(m, "about (%f,%f,%f),%f", 
			rollaxis.x, rollaxis.y, rollaxis.z, g_rendroll*(DEGTORAD(360.0)/(float)g_nrendsides));
		InfoMess(m,m);
#endif

#if 1
		rollaxis = Rotate(rollaxis, g_rendpitch*(DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		//pitchaxis = Rotate(pitchaxis, g_rendpitch*(DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		yawaxis = Rotate(yawaxis, g_rendpitch*(DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendpitch*(DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendpitch*(DEGTORAD(360.0)/(float)g_nrendsides), pitchaxis.x, pitchaxis.y, pitchaxis.z);

		rollaxis = Rotate(rollaxis, g_rendyaw*(DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		pitchaxis = Rotate(pitchaxis, g_rendyaw*(DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		//yawaxis = Rotate(yawaxis, g_rendyaw*(DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		g_lightPos = Rotate(g_lightPos, g_rendyaw*(DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
		g_cam.rotateabout(Vec3f(0,0,0), g_rendyaw*(DEGTORAD(360.0)/(float)g_nrendsides), yawaxis.x, yawaxis.y, yawaxis.z);
#endif
	
		//g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
		//g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);
		
#if 0
		//char m[123];
		sprintf(m, "after (%f,%f,%f),(%f,%f,%f),(%f,%f,%f)", 
			g_cam.m_pos.x, g_cam.m_pos.y, g_cam.m_pos.z,
			g_cam.m_view.x, g_cam.m_view.y, g_cam.m_view.z,
			g_cam.up2().x, g_cam.up2().y, g_cam.up2().z);
		InfoMess(m,m);
#endif
	}
}

void UpdateRender()
{
	ResetView(true);

	RotateView();

	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);

	/////AdjustFrame(false);
	//AllScreenMinMax needs to be called again because the pixels center of rendering depends on the window width and height, influencing the clip min/max
	AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);
	//FitFocus(g_clipmin, g_clipmax);
	//AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);

#if 0
	char msg[128];
	sprintf(msg, "clip %d,%d->%d,%d", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y);
	InfoMess(msg, msg);
#endif

	//Because we're always centered on origin, we can do this:
	g_spritecenter.x = g_width/2 - g_clipmin.x;
	g_spritecenter.y = g_height/2 - g_clipmin.y;

	glEnable(GL_BLEND);

	APPMODE oldmode = g_mode;
	g_mode = EDITOR;
	Draw();
	Draw();	//double buffered
	g_mode = oldmode;
#if 0
	Draw();
#elif 0
	Ortho(g_width, g_height, 1, 1, 1, 1);
	DrawViewport(3, 0, 0, g_width, g_height);
	EndS();
#else
	//glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
#ifdef DEBUG
//	Log(__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
	CHECKGLERROR();
#endif
	glViewport(0, 0, g_width, g_height);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
#ifdef DEBUG
	CHECKGLERROR();
#endif

	//get clip coordinates now that we've adjusted screen size (?)
	//AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);

#if 0
	char msg[128];
	sprintf(msg, "clip %d,%d->%d,%d", g_clipmin.x, g_clipmin.y, g_clipmax.x, g_clipmax.y);
	InfoMess(msg, msg);
#endif

	Vec3f offset;

	//g_1tilewidth
	//TILE_RISE

	//if(!g_antialias)
	{
		MakeFBO(0, RENDSTAGE_COLOR);
		glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
		if(!g_rendtopo)
		{
			SpriteRender(RENDSTAGE_COLOR, offset);
			SaveRender(RENDSTAGE_COLOR);
		}
		else
		{ 
			OrRender(RENDSTAGE_COLOR, offset);
		}
		DelFBO(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#ifdef DEBUG
		CHECKGLERROR();
	#endif
		if(g_rendertype != RENDER_TERRTILE &&
			g_exportteam)
		{
			MakeFBO(0, RENDSTAGE_TEAM);
			glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			if(!g_rendtopo)
			{
				SpriteRender(RENDSTAGE_TEAM, offset);
				SaveRender(RENDSTAGE_TEAM);
			}
			else
			{
				OrRender(RENDSTAGE_TEAM, offset);
			}
			DelFBO(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	#ifdef DEBUG
		CHECKGLERROR();
	#endif
		
		if(g_exportdepth)
		{
			MakeFBO(0, RENDSTAGE_DEPTH);
			glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb[0]);
			if(!g_rendtopo)
			{
				SpriteRender(RENDSTAGE_DEPTH, offset);
				SaveRender(RENDSTAGE_DEPTH);
			}
			else
			{
				OrRender(RENDSTAGE_DEPTH, offset);
			}
			DelFBO(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	#endif
	}

#if 0
	if(g_rendertype == RENDER_UNIT ||
		g_rendertype == RENDER_BUILDING ||
		g_doframes)
		g_renderframe++;
	else if(g_rendertype == RENDER_TERRTILE ||
		g_rendertype == RENDER_ROAD ||
		g_doinclines)
		g_currincline++;

	//If we're continuing the next render side or frame, or the end in other render types
	if(((g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING) && g_renderframe >= g_renderframes) ||
		((g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD) && g_currincline >= INCLINES))
	{
		if(g_rendside < g_nrendsides && g_rendertype == RENDER_UNIT)
		{
			g_renderframe = 0;
			g_rendside++;
			AdjustFrame();
		}
		else if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
		{
			g_currincline = 0;
			EndRender();
		}
		else
		{
			EndRender();
		}
	}
	else
	{
		AdjustFrame();
	}
#else
	//Advance render states
	//Are we doing sides and if so will the current side + 1 be valid? If so, advance
	if(g_dosides && !g_dorots && g_rendside+1 < g_nrendsides)
	{
		g_rendside++;
		AdjustFrame();
	}
	//Are we doing rotations?
	else if(g_dorots && g_rendroll+1 < g_nrendsides)
	{
		g_rendroll++;
		AdjustFrame();
	}
	else if(g_dorots && g_rendpitch+1 < g_nrendsides)
	{
		g_rendpitch++;
		g_rendroll = 0;
		AdjustFrame();
	}
	else if(g_dorots && g_rendyaw+1 < g_nrendsides)
	{
		g_rendyaw++;
		g_rendpitch = 0;
		g_rendroll = 0;
		AdjustFrame();
	}
	//Else, advance some other variable
	else
	{
		g_rendside = 0;
		g_rendroll = 0;
		g_rendpitch = 0;
		g_rendyaw = 0;

		if(g_doframes && g_renderframe+1 < g_renderframes)
		{
			g_renderframe++;
			AdjustFrame();
		}
		else
		{
			g_renderframe = 0;

			if(g_doinclines && g_currincline+1 < INCLINES)
			{
				g_currincline++;
				AdjustFrame();
			}
			else
			{
				g_currincline = 0;

#if 0
				if(g_rendtopo)
				{
					if(g_rendlon < g_orlons)
					{
						g_rendlon++;
						AdjustFrame();
					}
					else
					{
						g_rendlon = 0;

						if(g_rendlat < g_orlats)
						{
							g_rendlat++;
							AdjustFrame();
						}
						else
						{
							g_rendlat = 0;
							EndRender();
						}
					}
				}
				else
				{
					EndRender();
				}
#elif 1
				EndRender();
#endif
			}
		}
	}
#endif
}

void EndRender()
{
	g_mode = EDITOR;
	g_gui.hideall();
	//g_gui.show();
	g_gui.show("editor");
	g_renderframe = 0;
	g_lightPos = g_origlightpos;
	g_cam = g_origcam;
	CallResize(g_origwidth, g_origheight);
	ResetView(false);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	g_gui.reframe();
}
