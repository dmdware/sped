

#include "ordraw.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/tile.h"
#include "../render/sprite.h"
#include "../tool/rendersprite.h"
#include "../save/compilemap.h"

OrList g_orlist;

//orient view mode
void UpdateOrient()
{

}

//load based on globals
bool LoadOr1()
{
	float maxrad = 1;
	char infopath[SPE_MAX_PATH+1];
	NameRender(infopath, -1);
	strcat(infopath, "_info.txt");
	FILE* infofp = fopen(infopath, "r");
	fscanf(infofp, "maxrad %f\r\n", &maxrad);
	if(infofp)
		fclose(infofp);

	int ci = SpriteRef(g_dorots, g_dosides, g_nrendsides, g_doframes, g_renderframes,
		g_doinclines, 0,
		g_renderframe, g_currincline, g_rendpitch, g_rendyaw, g_rendroll,
		0, 0);

	Or *or = &g_orlist.ors[ci];

	or->maxrad = maxrad;

	char diffpath[SPE_MAX_PATH+1];
	NameRender(diffpath, -1);
	strcat(diffpath, "_diff.png");
	CreateTex(or->difftexi, diffpath, true, false);
	
#if 0
	unsigned int difftexi;
	unsigned int postexi[3];
	unsigned int jumptexi;
	float maxrad;
#endif

	char posxpath[SPE_MAX_PATH+1];
	NameRender(posxpath, -1);
	strcat(posxpath, "_posx.png");
	CreateTex(or->postexi[0], posxpath, true, false);

	char posypath[SPE_MAX_PATH+1];
	NameRender(posypath, -1);
	strcat(posypath, "_posy.png");
	CreateTex(or->postexi[1], posypath, true, false);

	char poszpath[SPE_MAX_PATH+1];
	NameRender(poszpath, -1);
	strcat(poszpath, "_posz.png");
	CreateTex(or->postexi[2], poszpath, true, false);

	char jumppath[SPE_MAX_PATH+1];
	NameRender(jumppath, -1);
	strcat(jumppath, "_isle.png");
	CreateTex(or->jumptexi, jumppath, true, false);

	return true;
}

bool LoadOr(const char* fullpath)
{
	if(!strstr(fullpath, "_list2.txt"))
	{
		char m[333];
		sprintf(m, "Is not \"_list2.txt\" orientability maps info file: %s", fullpath);
		ErrMess("Error", m);
		return false;
	}

	FILE *infofp = fopen(fullpath, "r");

	if(!infofp)
	{
		char m[333];
		sprintf(m, "Unable to load orientability maps: %s", fullpath);
		ErrMess("Error", m);
		return false;
	}

	g_rendtopo = true;

	//defaults
	//sim/tile.cpp/h
	g_currincline = 0;
	//compilemap.cpp/rendersprite.h
	g_renderframe = 0;
	//rendersprite.h/cpp
	g_renderframes = 1;
	g_doframes = false;
	g_dosides = false;
	g_doinclines = false;
	g_dorots = false;
	g_rendside = 0;
	g_rendpitch = g_rendyaw = g_rendroll = 0;
	g_nrendsides = 8;
	//rendertopo.h/cpp
	g_orwpx = 256;	//orientability map plane width pixels
	g_orhpx = 256;	//orientability map plane height pixels
	g_orlons = 16;	//orientability map longitude slices
	g_orlats = 16;	//orientability map latitude slices
	g_bigtex = 4096;	//orientability diffuse colors and surface positions map size

	while(!feof(infofp))
	{
		char line[256]="";
		//char a[256]="";
		//char b[256]="";

		//fscanf(infofp, "%s %s ", a, b);
		if(!fgets(line, 255, infofp))
			break;

		char a[256]="";
		char b[256]="";
		char c[256]="";

		sscanf(line, "%s %s %s ", a, b, c);

		if(strlen(a))
		{
			if(strcmp(a, "frames") == 0)
			{
				int doframes = 0;
				sscanf(b, "%d", &doframes);
				g_doframes = doframes;
				if(doframes)
					sscanf(c, "%d", &g_renderframes);
			}
			else if(strcmp(a, "sides") == 0)
			{
				int dosides = 0;
				sscanf(b, "%d", &dosides);
				g_dosides = dosides;
				if(dosides)
					sscanf(c, "%d", &g_nrendsides);
			}
			else if(strcmp(a, "rotations") == 0)
			{
				int dorots = 0;
				sscanf(b, "%d", &dorots);
				g_dorots = dorots;
				if(dorots)
					sscanf(c, "%d", &g_nrendsides);
			}
			else if(strcmp(a, "inclines") == 0)
			{
				int doincs = 0;
				sscanf(b, "%d", &doincs);
				g_doinclines = doincs;
			}
#if 0		
	g_orwpx = 256;	//orientability map plane width pixels
	g_orhpx = 256;	//orientability map plane height pixels
	g_orlons = 16;	//orientability map longitude slices
	g_orlats = 16;	//orientability map latitude slices
	g_bigtex = 4096;	//orientability diffuse colors and surface positions map size
#endif
			else if(strcmp(a, "orjpwpx") == 0)
			{
				sscanf(b, "%d", &g_orwpx);
			}
			else if(strcmp(a, "orjphpx") == 0)
			{
				sscanf(b, "%d", &g_orhpx);
			}
			else if(strcmp(a, "orjlons") == 0)
			{
				sscanf(b, "%d", &g_orlons);
			}
			else if(strcmp(a, "orjlats") == 0)
			{
				sscanf(b, "%d", &g_orlats);
			}
			else if(strcmp(a, "ormapsz") == 0)
			{
				sscanf(b, "%d", &g_bigtex);
			}
		}
	}

#if	00	
	//PrepareTopo() rendertopo.cpp
	char sppath[SPE_MAX_PATH+1];
	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");
	sprintf(sppath, "%s_list2.txt", g_renderbasename);
	FILE* fp = fopen(sppath, "w");
	if(g_doframes)
		fprintf(fp, "frames %d %d\r\n", g_doframes ? 1 : 0, g_renderframes);
	if(g_dosides && !g_dorots)
		fprintf(fp, "sides %d %d\r\n", g_dosides ? 1 : 0, g_nrendsides);
	else if(g_dorots)
		fprintf(fp, "rotations %d %d\r\n", g_dorots ? 1 : 0, g_nrendsides);
	if(g_doinclines)
		fprintf(fp, "inclines %d\r\n", g_doinclines ? 1 : 0);
	fprintf(fp, "orjpwpx %d\r\n", g_orwpx);
	fprintf(fp, "orjphpx %d\r\n", g_orhpx);
	fprintf(fp, "orjlons %d\r\n", g_orlons);
	fprintf(fp, "orjlats %d\r\n", g_orlats);
	fprintf(fp, "ormapsz %d\r\n", g_bigtex);
	if(fp)
		fclose(fp);
#endif


#if 0
	int32_t SpriteRef(bool rotations, bool sides, int nsides, bool frames, int nframes, bool inclines,
				  int nslices,
				  int32_t frame, int32_t incline, int32_t pitch, int32_t yaw, int32_t roll,
				  int slicex, int slicey)
#endif

	int ci = SpriteRef(g_dorots, g_dosides, g_nrendsides, g_doframes, g_renderframes,
		g_doinclines, 0,
		g_renderframes, INCLINES, g_nrendsides, g_nrendsides, g_nrendsides,
		0, 0);


	g_orlist.free();

	g_orlist.fullpath = fullpath;
	g_orlist.frames = g_doframes;
	g_orlist.inclines = g_doinclines;
	g_orlist.rotations = g_dorots;
	g_orlist.sides = g_dosides;
	g_orlist.nsides = g_nrendsides;
	g_orlist.nors = ci;
	g_orlist.ors = new Or [ci];
	g_orlist.on = true;

	do
	{
		LoadOr1();
	}while(AdvRender());
	EndRender();
}

void ViewTopo(const char* fullpath)
{
	

	g_mode = ORVIEW;
}