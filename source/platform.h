#ifndef PLATFORM_H
#define PLATFORM_H

//#define USESTEAM			//uncomment this line for steam version

#ifdef _WIN32
#define PLATFORM_GL14
#define PLATFORM_WIN
#endif

#if __APPLE__

#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define PLATFORM_GL14
#define PLATFORM_MAC
#endif
#if TARGET_OS_IPHONE
#define PLATFORM_IOS
#define PLATFORM_IPHONE
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif
#if TARGET_OS_IPAD
#define PLATFORM_IOS
#define PLATFORM_IPAD
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif

#endif

#if defined( __GNUC__ )
//#define PLATFORM_LINUX
#endif
#if defined( __linux__ )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif
#if defined ( __linux )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#pragma warning( disable: 4996 )	//_stricmp ISO c++

#ifdef PLATFORM_WIN
#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <dirent.h>	//located in libs/win/dirent-1.20.1/include/
#include <assert.h>
#endif

#ifdef PLATFORM_LINUX
/* POSIX! getpid(), readlink() */
#include <sys/time.h>	//gettimeofday
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <arpa/inet.h>	//htonl()
#endif

#ifdef PLATFORM_MAC
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <math.h>
#include <cmath>

#ifdef PLATFORM_WIN
#include <jpeglib.h>
//extern "C" {
#include <png.h>
//}
//#include <zip.h>
#endif

#ifdef PLATFORM_LINUX
#include <jpeglib.h>
#include <png.h>
#endif
//#define NO_SDL_GLEXT


#ifdef PLATFORM_MAC

#if 0
// https://trac.macports.org/ticket/42710
#ifndef FALSE            /* in case these macros already exist */
#define FALSE   0        /* values of boolean */
#endif
#ifndef TRUE
#define TRUE    1
#endif
#define HAVE_BOOLEAN
#endif

#include <jpeglib.h>
#include <png.h>
//#include <zip.h>
#endif

#ifdef PLATFORM_WIN
#include <GL/glew.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#if 1
#ifdef PLATFORM_LINUX
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//#include <GL/glut.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_syswm.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

#ifdef PLATFORM_MAC
#ifdef __APPLE__
//# define __gl_h_
# define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#endif
#include <OpenGL/gl3.h>
//#include <GL/xglew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//#include <SDL2/SDL_net.h>
#include <SDL2/SDL_syswm.h>
#endif

#ifdef PLATFORM_WIN
#include <GL/wglew.h>
#include <SDL.h>
#include <SDL_opengl.h>
//#include <SDL_net.h>
#include <SDL_syswm.h>
#endif
#endif

#ifdef PLATFORM_WIN
#include <gl/glaux.h>
#endif

#if 1
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>      // C++ importer interface
//#include <Importer.hpp>      // C++ importer interface
//#include <scene.h>       // Output data structure
//#include <postprocess.h> // Post processing flags
#include <assimp/DefaultLogger.hpp>
#endif

#ifdef PLATFORM_WIN
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
//#pragma comment(lib, "SDL.lib")
//#pragma comment(lib, "SDLmain.lib")
//#pragma comment(lib, "lib32/assimp.lib")
//#pragma comment(lib, "assimp_release-dll_win32/assimp.lib")
//#pragma comment(lib, "assimp.lib")
//#pragma comment(lib, "assimp-vc110-mt.lib")
#pragma comment(lib, "assimp-vc90-mt.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "libpng15.lib")
//#pragma comment(lib, "zlib.lib")
//#pragma comment(lib, "zipstatic.lib")
#ifdef USESTEAM
#pragma comment(lib, "sdkencryptedappticket.lib")
#pragma comment(lib, "steam_api.lib")
#endif
#endif

#ifndef SPE_MAX_PATH
#define SPE_MAX_PATH 2600
#endif

#define DIRENT_FILE		0x8
#define DIRENT_FOLDER	0x4

#ifndef PLATFORM_WIN
#define SOCKET int
typedef unsigned char byte;
typedef unsigned int UINT;
typedef int16_t WORD;
#define _isnan isnan
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define APIENTRY
#define ERROR 0
#endif

typedef unsigned int uint;

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

/*
#ifndef _isnan
int _isnan(double x) { return x != x; }
#endif
*/

#ifdef PLATFORM_WIN
extern HINSTANCE g_hInstance;
#endif

extern SDL_Window *g_window;
extern SDL_Renderer* g_renderer;
extern SDL_GLContext g_glcontext;

#define SPECBUMPSHADOW

//#define GLDEBUG
//#define DEBUGLOG

#define CHECKGLERROR() CheckGLError(__FILE__,__LINE__)

#ifndef GLDEBUG
#define CheckGLError(a,b); (void)0;
#endif

#ifdef USESTEAM
#include <steam_api.h>
#include <isteamuserstats.h>
#include <isteamremotestorage.h>
#include <isteammatchmaking.h>
#include <steam_gameserver.h>
#endif

#ifdef USESTEAM
#include <steam.h>
#endif

#endif
