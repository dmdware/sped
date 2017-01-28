

#include "ordraw.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/tile.h"
#include "../tool/compilemap.h"

OrList g_orlist;

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

	return true;
}

bool LoadOr(const char* fullpath)
{
	if(!strstr(fullpath, "_list2.txt"))
	{
		char m[333];
		sprintf(m, "Is not \"_list2.txt\" orientability maps info file: %s", fullpath);
		ErrMess("Error", m);
		reurn false;
	}

	FILE *infofp = fopen(fullpath, "r");

	if(!infofp)
	{
		char m[333];
		sprintf(m, "Unable to load orientability maps: %s", fullpath);
		ErrMess("Error", m);
		reurn false;
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
			}
			else if(strcmp(a, "sides") == 0)
			{
			}
			else if(strcmp(a, "rotations") == 0)
			{
			}
			else if(strcmp(a, "inclines") == 0)
			{
			}
			else if(strcmp(a, "orjpwpx") == 0)
			{
			}
			else if(strcmp(a, "orjphpx") == 0)
			{
			}
			else if(strcmp(a, "orjlons") == 0)
			{
			}
			else if(strcmp(a, "orjlats") == 0)
			{
			}
			else if(strcmp(a, "ormapsz") == 0)
			{
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

	
}

void ViewTopo(const char* fullpath)
{
	

	g_appmode = ORVIEW;
}