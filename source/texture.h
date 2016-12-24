#ifndef TEXTURE_H
#define TEXTURE_H

#include "platform.h"
#include "math/vec2i.h"

class LoadedTex;

class Texture
{
public:
	unsigned int texname;
	std::string fullpath;
	bool loaded;
	int width, height;
	bool transp;
	bool sky;
	bool breakable;
	bool passthru;
	bool grate;
	bool ladder;
	bool water;
	bool fabric;
	bool clamp;
	bool mipmaps;

	LoadedTex* pixels;

	Texture()
	{
		pixels = NULL;
		fullpath = '\0';
		loaded = false;
	}
};

#define TEXTURES	4096
extern Texture g_texture[TEXTURES];

#define TGA_RGB		 2		// This tells us it's a normal RGB (really BGR) file
#define TGA_A		 3		// This tells us it's an ALPHA file
#define TGA_RLE		10		// This tells us that the targa is Run-Length Encoded (RLE)

#ifndef int_p_NULL
#define int_p_NULL (int*)NULL
#endif
#define png_infopp_NULL (png_infopp)NULL
#define png_voidp_NULL	(png_voidp)NULL

#define JPEG_BUFFER_SIZE (8 << 10)

#if 1
typedef struct
{
	struct jpeg_source_mgr  pub;
} JPEGSource;
#endif

class LoadedTex
{
public:
	int channels;			// The channels in the image (3 = RGB : 4 = RGBA)
	int sizex;				// The width of the image in pixels
	int sizey;				// The height of the image in pixels
	unsigned char *data;	// The image pixel data

	void destroy();
	LoadedTex();
	~LoadedTex();
};

struct TextureToLoad
{
	unsigned int* ptexindex;
	unsigned int texindex;
	char relative[SPE_MAX_PATH+1];
	bool clamp;
	bool reload;
	bool mipmaps;
};

extern std::vector<TextureToLoad> g_texload;

extern int g_texwidth;
extern int g_texheight;
//extern int g_texindex;
extern int g_lastLTex;

LoadedTex *LoadBMP(const char *fullpath);
LoadedTex *LoadTGA(const char *fullpath);
void DecodeJPG(jpeg_decompress_struct* cinfo, LoadedTex *pImageData);
LoadedTex *LoadJPG(const char *fullpath);
LoadedTex *LoadJPG2(const char *fullpath);
LoadedTex *LoadPNG(const char *fullpath);
LoadedTex *LoadPNG2(const char *fullpath);
bool FindTexture(unsigned int &texture, const char* relative);
int NewTexture();
bool TextureLoaded(unsigned int texture, const char* relative, bool transp, bool clamp, bool mipmaps, unsigned int& texindex);
void FindTextureExtension(char *relative);
void FreeTextures();
bool Load1Texture();
void QueueTexture(unsigned int* texindex, const char* relative, bool clamp, bool mipmaps);
void RequeueTexture(unsigned int texindex, const char* relative, bool clamp, bool mipmaps);
LoadedTex* LoadTexture(const char* full);
bool CreateTex(LoadedTex* pImage, unsigned int* texname, bool clamp, bool mipmaps);
bool CreateTex(unsigned int &texindex, const char* relative, bool clamp, bool mipmaps, bool reload=false);
void ReloadTextures();
void FreeTexture(const char* relative);
void FreeTexture(int i);
void DiffPath(const char* basepath, char* diffpath);
void DiffPathPNG(const char* basepath, char* diffpath);
void SpecPath(const char* basepath, char* specpath);
void NormPath(const char* basepath, char* normpath);
void OwnPath(const char* basepath, char* ownpath);
void AllocTex(LoadedTex* empty, int width, int height, int channels);
void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos);
void SaveJPEG(const char* fullpath, LoadedTex* image, float quality);
void SaveJPEG2(const char* fullpath, LoadedTex* image, float quality);
int SavePNG(const char* fullpath, LoadedTex* image);
int SavePNG2(const char* fullpath, LoadedTex* image);
void FlipImage(LoadedTex* image);
int SaveBMP(const char* fullpath, LoadedTex* image);
bool SaveRAW(const char* fullpath, LoadedTex* image);
void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim);
void Extract(LoadedTex* original, LoadedTex* empty, int x1, int y1, int x2, int y2);

#endif
