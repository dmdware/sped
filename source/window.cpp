#include "window.h"
#include "math/math3d.h"

#ifndef MATCHMAKER
#include "texture.h"
#include "gui/gui.h"
#include "gui/font.h"
#include "gui/cursor.h"
#include "render/shader.h"
#include "render/shadow.h"
#endif

bool g_quit = false;
bool g_background = false;
double g_drawfrinterval = 0.0f;
bool g_fullscreen = true;
Resolution g_selres;
std::vector<Resolution> g_resolution;
std::vector<int> g_bpps;
double g_instantdrawfps = 0.0f;
//long long g_lasttime = GetTicks();
double g_instantupdfps = 0;
double g_updfrinterval = 0;

#ifndef MATCHMAKER
//Camera g_cam;
int g_currw;
int g_currh;
int g_width = INI_WIDTH;
int g_height = INI_HEIGHT;
int g_bpp = INI_BPP;
Vec2i g_mouse;
Vec2i g_mousestart;
bool g_keyintercepted = false;
bool g_keys[SDL_NUM_SCANCODES] = {0};
bool g_mousekeys[5] = {0};
float g_zoom = INI_ZOOM;
bool g_mouseout = false;
bool g_moved = false;
bool g_canplace = false;
int g_bpcol = -1;
Camera g_bpcam;
int g_bptype = -1;
float g_bpyaw = 0;
bool g_mouseoveraction = false;
int g_curst = CU_DEFAULT;	//cursor state
int g_kbfocus = 0;	//keyboad focus counter
#endif

void AddRes(int w, int h)
{
	Resolution r;
	r.width = w;
	r.height = h;
	g_resolution.push_back(r);
}

void EnumerateDisplay()
{
#ifdef PLATFORM_WIN
	DEVMODE dm;
	int index=0;
	while(0 != EnumDisplaySettings(NULL, index++, &dm))
	{
		Resolution r;
		r.width = dm.dmPelsWidth;
		r.height = dm.dmPelsHeight;

		bool found = false;

		for(int i=0; i<(int)g_resolution.size(); i++)
		{
			if(g_resolution[i].width == r.width && g_resolution[i].height == r.height)
			{
				found = true;
				break;
			}
		}

		if(!found)
			g_resolution.push_back(r);

		found = false;

		int bpp = dm.dmBitsPerPel;

		for(int i=0; i<(int)g_bpps.size(); i++)
		{
			if(g_bpps[i] == bpp)
			{
				found = true;
				break;
			}
		}

		if(!found)
			g_bpps.push_back(bpp);
	}
#endif // PLATFORM_WIN
}

#ifndef MATCHMAKER

void Resize(int width, int height)
{
#ifdef PLATFORM_MOBILE
	//width /= 2;
	//height /= 2;
#endif
	
	if(height == 0)
		height = 1;

	glViewport(0, 0, width, height);

	
	GUI* gui = &g_gui;

	//if(g_width != width || g_height != height)
	{
		g_width = width;
		g_height = height;

#ifdef PLATFORM_MOBILE
		//halve for mobile small screen so it appears twice as big
		g_width >>= 1;
		g_height >>= 1;
#endif
		
		//if(g_fullscreen)
		//Reload();
		//loadtex();
		gui->reframe();
		
#ifdef PLATFORM_MOBILE
		g_width = width;
		g_height = height;
#endif
	}
}

#endif

void CalcDrawRate()
{
	static unsigned long long frametime = 0;				// This stores the last frame's time
	static int framecounter = 0;
	static unsigned long long lasttime;

	// Get the current time in seconds
	unsigned long long currtime = GetTicks();

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
	g_drawfrinterval = (double)(currtime - frametime) / 1000.0f;	// + 0.005f;

	//g_instantdrawfps = 1.0f / (g_currentTime - frameTime);
	//g_instantdrawfps = 1.0f / g_drawfrinterval;

	frametime = currtime;

	// Increase the frame counter
	++framecounter;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if( currtime - lasttime > 1000 )
	{
		g_instantdrawfps = framecounter;

		// Here we set the lastTime to the currentTime
		lasttime = currtime;

		// Reset the frames per second
		framecounter = 0;
	}
}

bool DrawNextFrame()
{
	static unsigned long long lastdrawtick = GetTicks();
	static unsigned long long elapseddrawtime = 0;

#if 0
	unsigned long long currentTime = GetTicks(); // Get the time (milliseconds = seconds * .001)
	unsigned long long deltaTime = currentTime - lastdrawtick; // Get the slice of time
	int framedelay = 1000 / DRAW_FRAME_RATE; // Store 1 / desiredfps

	deltaTime ++;	//in case less than a millisecond passed
	
#if 1
	elapseddrawtime += deltaTime; // Add to the elapsed time
	lastdrawtick = currentTime; // Update lastdrawtick
#else
	elapseddrawtime = currentTime - lastdrawtick;
#endif

	// Check if the time since we last checked is greater than our framedelay
	if( elapseddrawtime > framedelay )
	{
		elapseddrawtime -= framedelay; // Adjust the elapsed time
		
		//lastdrawtick = currentTime;

		// Return true, to animate the next frame of animation
		return true;
	}

	// We don't animate right now.
	return false;
	
#else
	
	unsigned long long currentTime = GetTicks();
	float desiredFPMS = 1000.0f/(float)DRAW_FRAME_RATE;
	int deltaTime = (int)(currentTime - lastdrawtick);

	if(deltaTime >= desiredFPMS)
	{
		lastdrawtick = currentTime;
		return true;
	}

	return false;
#endif
}


void CalcUpdRate()
{
	static unsigned long long frametime = 0;				// This stores the last frame's time
	static int framecounter = 0;
	static unsigned long long lasttime;

	// Get the current time in seconds
	unsigned long long currtime = GetTicks();

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
	g_updfrinterval = (double)(currtime - frametime) / 1000.0f;	// + 0.005f;

	//g_instantdrawfps = 1.0f / (g_currentTime - frameTime);
	//g_instantdrawfps = 1.0f / g_drawfrinterval;

	frametime = currtime;

	// Increase the frame counter
	++framecounter;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if( currtime - lasttime > 1000 )
	{
		g_instantupdfps = framecounter;

		// Here we set the lastTime to the currentTime
		lasttime = currtime;

		// Reset the frames per second
		framecounter = 0;
	}
}

bool UpdNextFrame()
{
	static unsigned long long lastupdtick = GetTicks();
	static unsigned long long elapsedupdtime = 0;

#if 0
	//needs to be done elsewhere in UpdSim
	if(g_speed == SPEED_PAUSE)
	{
		lastupdtick = GetTicks();
		elapsedupdtime = 0;
		return false;
	}
#endif

#if 0
	unsigned long long currentTime = GetTicks(); // Get the time (milliseconds = seconds * .001)
	unsigned long long deltaTime = currentTime - lastupdtick; // Get the slice of time
	int framedelay = 1000 / SIM_FRAME_RATE; // Store 1 / desiredfps
	
	deltaTime ++;	//if less than a millisecond passed
	
#if 1
	elapsedupdtime += deltaTime; // Add to the elapsed time
	lastupdtick = currentTime; // Update lastupdtick
#else
	elapsedupdtime = currentTime - lastupdtick;
#endif

	// Check if the time since we last checked is greater than our framedelay
	if( elapsedupdtime > framedelay )
	{
		elapsedupdtime -= framedelay; // Adjust the elapsed time

		// Return true, to animate the next frame of animation
		
		//lastupdtick = currentTime;	//

		return true;
	}

	// We don't animate right now.
	return false;
	
#else
	
	unsigned long long currentTime = GetTicks();
	float desiredFPMS = 1000.0f/(float)SIM_FRAME_RATE;
	int deltaTime = (int)(currentTime - lastupdtick);

	if(deltaTime >= desiredFPMS)
	{
		lastupdtick = currentTime;
		return true;
	}

	return false;
#endif
}

#ifndef MATCHMAKER

bool InitWindow()
{
	//Log("Renderer1: %s\r\n", (char*)glGetString(GL_RENDERER));
	//Log("GL_VERSION1 = %s\r\n", (char*)glGetString(GL_VERSION));
	

#ifndef PLATFORM_MOBILE
	char path[SPE_MAX_PATH+1];
	FullPath("SpriteEd.png", path);
	LoadedTex* pixels = LoadPNG2(path);

	if(!pixels)
	{
		ErrMess("Error", "Couldn't load icon");
	}

	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(pixels->data, pixels->sizex, pixels->sizey, pixels->channels*8, pixels->channels*pixels->sizex, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	if(!surf)
	{
		char message[256];
		sprintf(message, "Couldn't create icon: %s", SDL_GetError());
		ErrMess("Error", message);
	}

	// The icon is attached to the window pointer
	SDL_SetWindowIcon(g_window, surf);

	delete pixels;
#endif
	
	//TODO check warnings

	// ...and the surface containing the icon pixel data is no longer required.
	//SDL_FreeSurface(surf);

#ifdef PLATFORM_GL14
	//glShadeModel(GL_SMOOTH);
#endif
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef PLATFORM_GL14
	glClearDepth(1.0f);
#endif
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
#ifdef PLATFORM_GL14
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	InitGLSL();
	InitShadows();
	LoadFonts();

	return true;
}

SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    
    if (flags & SDL_WINDOW_FULLSCREEN) return SDL_TRUE; // return SDL_TRUE if fullscreen
    
    return SDL_FALSE; // Return SDL_FALSE if windowed
}

/// Toggles On/Off FullScreen
/// @returns -1 on error, 1 on Set fullscreen successful, 0 on Set Windowed successful
int SDL_ToggleFS(SDL_Window *win)
{
    if (IsFullScreen(win))
    {
        // Swith to WINDOWED mode
        if (SDL_SetWindowFullscreen(win, SDL_FALSE) < 0)
        {
            Log("Setting windowed failed : %s\r\n", SDL_GetError());
            return -1;
        }
        
        return 0;
    }
    
    // Swith to FULLSCREEN mode
    if (SDL_SetWindowFullscreen(win, SDL_TRUE) < 0)
    {
        Log("Setting fullscreen failed : %s\r\n", SDL_GetError());
        return -1;
    }
    
    return 1; 
}

int SDL_ToggleFS(SDL_Window *win, int w, int h)
{
    Uint32 flags = (SDL_GetWindowFlags(win) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (SDL_SetWindowFullscreen(win, flags) < 0) return -1; // NOTE: this takes FLAGS as the second param, NOT true/false!
    SDL_SetWindowSize(win, w, h);
    return 0; 
}

void BreakWin(const char* title)
{
#ifdef PLATFORM_MOBILE
	glDeleteTextures(1, &g_zoomtex);
#endif
	
	FreeTextures();
	ReleaseShaders();
    
    //SDL_SetWindowFullscreen(g_window, SDL_FALSE);
    //Uint32 flags = (SDL_GetWindowFlags(g_window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
    Uint32 flags = (SDL_GetWindowFlags(g_window) & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_SetWindowFullscreen(g_window, flags); // NOTE: this takes FLAGS as the second param, NOT true/false!
        
    
	// Close and destroy the window
	SDL_GL_DeleteContext(g_glcontext);
#if 0
	SDL_DestroyRenderer(g_renderer);
#endif
	
#ifndef PLATFORM_IOS
	//Don't do this on iOS or else there's some exception later
	SDL_DestroyWindow(g_window);
#else
	if(!g_quit)
	{
		//only do this on iOS when we're recreating the window to change resolution
		SDL_DestroyWindow(g_window);
	}
#endif
	
    SDL_QuitSubSystem(SDL_INIT_VIDEO);  //exit black fullscreen on mac
    
    if(SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        char msg[1280];
        sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
        ErrMess("Error", msg);
    }
    
    SDL_ShowCursor(SDL_FALSE);
}

bool MakeWin(const char* title)
{

	Log("samw0");
	

	//Log("GL_VERSION: "<<(char*)glGetString(GL_VERSION));
	//

	Log("sa");
	

#if 1
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef PLATFORM_GL14
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
#ifdef PLATFORM_GLES20
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif

	unsigned int flags;
	int startx;
	int starty;

#ifdef PLATFORM_MOBILE
	startx = 0;
	starty = 0;
	//flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL;
	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS /* | SDL_WINDOW_ALLOW_HIGHDPI */;
#else
	if(g_fullscreen)
	{
		startx = SDL_WINDOWPOS_UNDEFINED;
		starty = SDL_WINDOWPOS_UNDEFINED;
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
	}
	else
	{
#if 0
		SDL_DisplayMode current;
		SDL_GetCurrentDisplayMode(0, &current);
		int screenw = current.w;
		int screenh = current.h;

		startx = screenw/2 - g_selres.width/2;
		starty = screenh/2 - g_selres.height/2;
#else
		startx = SDL_WINDOWPOS_UNDEFINED;
		starty = SDL_WINDOWPOS_UNDEFINED;
#endif
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	}
#endif
	
	//g_selres.width = 568;
	//g_selres.height = 320;

	// Create an application window with the following settings:
	g_window = SDL_CreateWindow(
	                   title,                  // window title
	                   startx,           // initial x position
	                   starty,           // initial y position
	                   g_selres.width,                               // width, in pixels
	                   g_selres.height,                               // height, in pixels
	                   flags                  // flags - see below
	           );

	// Check that the window was successfully made
	if (g_window == NULL)
	{
		// In the event that the window could not be made...
		char msg[256];
		sprintf(msg, "Could not create window: %s\n", SDL_GetError());
		ErrMess("Error", msg);
		return false;
	}

#if 0
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (g_renderer == NULL)
	{
		// In the event that the window could not be made...
		char msg[256];
		sprintf(msg, "Could not create renderer: %s\n", SDL_GetError());
		ErrMess("Error", msg);
		return false;
	}
#endif
	//SDL_GL_SetSwapInterval(0);

	g_glcontext = SDL_GL_CreateContext(g_window);

    //Can't call after 3.0? mac
	//Log("GL_VERSION: "<<glGetString(GL_VERSION));
	//

	if(!g_glcontext)
	{
		BreakWin(title);
		ErrMess("Error", "Couldn't create GL context");
		return false;
	}

	SDL_GL_MakeCurrent(g_window, g_glcontext);

#ifdef PLATFORM_MOBILE
	SDL_GL_SetSwapInterval(1);
#else
	SDL_GL_SetSwapInterval(0);
	//SDL_Delay(7000);
	//SDL_Delay(7000);
#endif

	Vec2i winsz;
	SDL_GetWindowSize(g_window, &winsz.x, &winsz.y);
	//SDL_GL_GetDrawableSize(g_window, &winsz.x, &winsz.y);
	Resize(winsz.x, winsz.y);

	
#ifdef PLATFORM_MOBILE
	int width = g_width;
	int height = g_height;
	//halve for mobile small screen so it appears twice as big
	g_width >>= 1;
	g_height >>= 1;
#endif
	
	if(!InitWindow())
	{
		BreakWin(title);
		ErrMess("Error", "Initialization failed");
		return false;
	}

	CenterMouse();
	
	LoadedTex blank;
	AllocTex(&blank, ZOOMBOX*2, ZOOMBOX*2, 4);
	CreateTex(&blank, &g_zoomtex, true, false);

#ifdef PLATFORM_MOBILE
	g_width = width;
	g_height = height;
#endif
	
	return true;
}

#endif
