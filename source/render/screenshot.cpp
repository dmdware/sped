

#include "../platform.h"
#include "../window.h"
#include "../utils.h"
#include "../texture.h"

#if 0
void SaveScreenshot()
{
	LoadedTex screenshot;
#if 0
	screenshot.channels = 3;
	screenshot.sizex = g_width;
	screenshot.sizey = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );
#endif
	AllocTex(&screenshot, g_width, g_height, 3);

	memset(screenshot.data, 0, g_width * g_height * 3);

	// size must be multiple of 32 or else this will crash !!!!!
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);

	FlipImage(&screenshot);

	char relative[256];
	std::string datetime = FileDateTime();
	sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	char fullpath[SPE_MAX_PATH+1];
	FullPath(relative, fullpath);

	Log("Writing screenshot %s", fullpath);
	

	SaveJPEG(fullpath, &screenshot, 0.50);

#if 0
	free(screenshot.data);
#endif
	screenshot.destroy();
}
#endif

void SaveScreenshot()
{
#ifdef DEMO
	InfoMess("Demo", "Feature disabled ;)");
	return;
#endif

	LoadedTex screenshot;
	screenshot.channels = 3;
	screenshot.sizex = g_width;
	screenshot.sizey = g_height;
	screenshot.data = (unsigned char*)malloc( sizeof(unsigned char) * g_width * g_height * 3 );

	if(!screenshot.data)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	memset(screenshot.data, 0, g_width * g_height * 3);

	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screenshot.data);

	FlipImage(&screenshot);

	char relative[256];
	std::string datetime = FileDateTime();
	//sprintf(relative, "screenshots/%s.jpg", datetime.c_str());
	sprintf(relative, "screenshots/%s.png", datetime.c_str());
	char fullpath[SPE_MAX_PATH+1];
	FullPath(relative, fullpath);

	Log("Writing screenshot %s", fullpath);
	
	
	//SaveJPEG2(fullpath, &screenshot, 0.9f);
	SavePNG2(fullpath, &screenshot);

	//free(screenshot.data);
}
