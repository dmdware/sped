

#include "../sim/map.h"
#include "save.h"
#include "../platform.h"
#include "../math/polygon.h"
#include "../texture.h"
#include "../render/vertexarray.h"
#include "../utils.h"
#include "saveedm.h"
#include "../save/edmap.h"
#include "savemap.h"
#include "../app/segui.h"
#include "../render/ordraw.h"

//#define LOADMAP_DEBUG

void SaveEdBrushSide(FILE* fp, BrushSide* s, int* texrefs)
{
	fwrite(&s->m_plane, sizeof(Plane3f), 1, fp);

	SaveVertexArray(fp, &s->m_drawva);

	fwrite(&texrefs[ s->m_diffusem ], sizeof(int), 1, fp);
	fwrite(&s->m_ntris, sizeof(int), 1, fp);
	fwrite(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fwrite(s->m_tceq, sizeof(Plane3f), 2, fp);

	SavePolygon(fp, &s->m_outline);

	fwrite(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	fwrite(&s->m_centroid, sizeof(Vec3f), 1, fp);
}

void SaveEdBrushSides(FILE* fp, Brush* b, int* texrefs)
{
	int nsides = b->m_nsides;
	fwrite(&nsides, sizeof(int), 1, fp);

	for(int i=0; i<nsides; i++)
	{
		/*
	Plane3f m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
		*/
	/*
	int m_ntris;
	Triangle2* m_tris;
	Plane3f m_tceq[2];	//tex coord uv equations
	Polyg m_outline;
	int* m_vindices;	//indices into parent brush's shared vertex array
	Vec3f m_centroid;
	*/
		BrushSide* s = &b->m_sides[i];

		SaveEdBrushSide(fp, s, texrefs);
	}
}

void ReadBrushSide(FILE* fp, BrushSide* s, TexRef* texrefs)
{
	fread(&s->m_plane, sizeof(Plane3f), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("s->m_plane = "<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<","<<s->m_plane.m_d<<std::endl;
	
#endif

	ReadVertexArray(fp, &s->m_drawva);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 1");
	
#endif

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	s->m_diffusem = texrefs[ texrefindex ].diffindex;
	s->m_specularm = texrefs[ texrefindex ].specindex;
	s->m_normalm = texrefs[ texrefindex ].normindex;
	s->m_ownerm = texrefs[ texrefindex ].ownindex;
	fread(&s->m_ntris, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 2");
	
#endif

	s->m_tris = new Triangle2[ s->m_ntris ];
	fread(s->m_tris, sizeof(Triangle2), s->m_ntris, fp);
	fread(s->m_tceq, sizeof(Plane3f), 2, fp);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 3");
	
#endif

	ReadPolygon(fp, &s->m_outline);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 4");
	
#endif

	s->m_vindices = new int[ s->m_outline.m_edv.size() ];
	fread(s->m_vindices, sizeof(int), s->m_outline.m_edv.size(), fp);
	fread(&s->m_centroid, sizeof(Vec3f), 1, fp);
}

void ReadBrushSides(FILE* fp, Brush* b, TexRef* texrefs)
{
	int nsides;
	fread(&nsides, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nsides = "<<nsides<<std::endl;
	
#endif

	if(b->m_sides)
	{
		delete [] b->m_sides;
		b->m_sides = NULL;
		b->m_nsides = 0;
	}

	for(int i=0; i<nsides; i++)
	{
		BrushSide s;

		ReadBrushSide(fp, &s, texrefs);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 5");
	
#endif

		b->add(s);

#ifdef LOADMAP_DEBUG
	Log("load ed brush side 6");
	
#endif
	}
}

Brush* GetBrushNum(int target, EdMap* map)
{
	int cnt = 0;
	for(std::list<Brush>::iterator i=map->m_brush.begin(); i!=map->m_brush.end(); i++, cnt++)
	{
		if(cnt == target)
			return &*i;
	}

	return NULL;
}

void SaveEdDoor(FILE* fp, EdDoor* door, int* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	bool startopen;
	Brush* brushp;
	Brush closedstate;*/

	fwrite(&door->axis, sizeof(Vec3f), 1, fp);
	fwrite(&door->point, sizeof(Vec3f), 1, fp);
	fwrite(&door->opendeg, sizeof(float), 1, fp);
	fwrite(&door->startopen, sizeof(bool), 1, fp);

	fwrite(&door->m_nsides, sizeof(int), 1, fp);
	for(int i=0; i<door->m_nsides; i++)
		SaveEdBrushSide(fp, &door->m_sides[i], texrefs);

#if 0
	Log("save ed door");
	for(int i=0; i<door->m_nsides; i++)
	{
		Log("side "<<i<<std::endl;
		Plane3f* p = &door->m_sides[i].m_plane;

		Log("plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void ReadEdDoor(FILE* fp, EdDoor* door, TexRef* texrefs)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	bool startopen;
	Brush* brushp;
	Brush closedstate;*/

#if 0
	MessageBox(g_hWnd, "read door", "aasd", NULL);
#endif

	fread(&door->axis, sizeof(Vec3f), 1, fp);
	fread(&door->point, sizeof(Vec3f), 1, fp);
	fread(&door->opendeg, sizeof(float), 1, fp);
	fread(&door->startopen, sizeof(bool), 1, fp);

	fread(&door->m_nsides, sizeof(int), 1, fp);


#if 0
	char msg[128];
	sprintf(msg, "door sides %d", door->m_nsides);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif

	door->m_sides = new BrushSide[door->m_nsides];
	for(int i=0; i<door->m_nsides; i++)
		ReadBrushSide(fp, &door->m_sides[i], texrefs);

#if 0
	Log("read ed door");
	for(int i=0; i<door->m_nsides; i++)
	{
		Log("side "<<i<<std::endl;
		Plane3f* p = &door->m_sides[i].m_plane;

		Log("plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void SaveBrush(FILE* fp, int* texrefs, Brush* b)
{
	/*
		int m_nsides;
		BrushSide* m_sides;
		int m_nsharedv;
		Vec3f* m_sharedv;	//shared vertices array
	*/

	SaveEdBrushSides(fp, b, texrefs);
	fwrite(&b->m_nsharedv, sizeof(int), 1, fp);
	fwrite(b->m_sharedv, sizeof(Vec3f), b->m_nsharedv, fp);
	fwrite(&texrefs[b->m_texture], sizeof(int), 1, fp);

	bool hasdoor = false;

	if(b->m_door)
		hasdoor = true;

	fwrite(&hasdoor, sizeof(bool), 1, fp);

	if(hasdoor)
		SaveEdDoor(fp, b->m_door, texrefs);
}

void SaveBrushes(FILE* fp, int* texrefs, EdMap* map)
{
	int nbrush = map->m_brush.size();
	fwrite(&nbrush, sizeof(int), 1, fp);

	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		SaveBrush(fp, texrefs, &*b);
	}
}

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b)
{
	ReadBrushSides(fp, b, texrefs);

	if(b->m_sharedv)
	{
		delete [] b->m_sharedv;
		b->m_sharedv = NULL;
		b->m_nsharedv = 0;
	}

	fread(&b->m_nsharedv, sizeof(int), 1, fp);
#ifdef LOADMAP_DEBUG
	Log("b->m_nsharedv = "<<b->m_nsharedv<<std::endl;
	
#endif

	b->m_sharedv = new Vec3f[ b->m_nsharedv ];
	fread(b->m_sharedv, sizeof(Vec3f), b->m_nsharedv, fp);

	//b.remaptex();	//comment this out

	int texrefindex;
	fread(&texrefindex, sizeof(int), 1, fp);
	b->m_texture = texrefs[texrefindex].diffindex;

	bool hasdoor = false;
	fread(&hasdoor, sizeof(bool), 1, fp);

	if(hasdoor)
	{
		b->m_door = new EdDoor();
		ReadEdDoor(fp, b->m_door, texrefs);
	}
}

void ReadBrushes(FILE* fp, TexRef* texrefs, EdMap* map)
{
	int nbrush;
	fread(&nbrush, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nbrush = "<<nbrush<<std::endl;
	
#endif

	for(int i=0; i<nbrush; i++)
	{
		Brush b;
		ReadBrush(fp, texrefs, &b);
		map->m_brush.push_back(b);

#ifdef LOADMAP_DEBUG
	Log("added b");
	
#endif
	}
}

void ReadEdTexs(FILE* fp, TexRef** texrefs)
{
	int nrefs;
	fread(&nrefs, sizeof(int), 1, fp);

#ifdef LOADMAP_DEBUG
	Log("nrefs = "<<nrefs<<std::endl;
	
#endif

	(*texrefs) = new TexRef[nrefs];

	for(int i=0; i<nrefs; i++)
	{
		TexRef* tr = &(*texrefs)[i];
		int strl;
		fread(&strl, sizeof(int), 1, fp);

		char* filepath = new char[strl];
		fread(filepath, sizeof(char), strl, fp);
#ifdef LOADMAP_DEBUG
	Log("filepath = "<<filepath<<std::endl;
	
#endif
		tr->filepath = filepath;
		delete [] filepath;
		CreateTex(tr->diffindex, tr->filepath.c_str(), false, false);
		tr->texname = g_texture[tr->diffindex].texname;

		char basepath[SPE_MAX_PATH+1];
		strcpy(basepath, tr->filepath.c_str());
		StripExt(basepath);

		char specpath[SPE_MAX_PATH+1];
		SpecPath(basepath, specpath);

		CreateTex(tr->specindex, specpath, false, false);

		char normpath[SPE_MAX_PATH+1];
		NormPath(basepath, normpath);

		CreateTex(tr->normindex, normpath, false, false);

		char ownpath[SPE_MAX_PATH+1];
		OwnPath(basepath, ownpath);

		CreateTex(tr->ownindex, ownpath, false, false);
	}
}

int BrushNum(Brush* b, EdMap* map)
{
	int cnt = 0;
	for(std::list<Brush>::iterator i=map->m_brush.begin(); i!=map->m_brush.end(); i++, cnt++)
	{
		if(&*i == b)
			return cnt;
	}

	return -1;
}

void SaveModelHolder(FILE* fp, ModelHolder* pmh)
{
	Model2* m = &g_model2[pmh->modeli];

	std::string relative = MakeRelative(m->m_fullpath.c_str());
	int nrelative = relative.size() + 1;

	fwrite(&nrelative, sizeof(int), 1, fp);
	fwrite(relative.c_str(), sizeof(char), nrelative, fp);

#if 0
	InfoMess("save relative", relative.c_str());
	InfoMess("save full", m->m_fullpath.c_str());
	char msg[128];
	sprintf(msg, "mi %d", pmh->modeli);
	InfoMess("modeli", msg);
#endif

	fwrite(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->translation, sizeof(Vec3f), 1, fp);
	fwrite(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void ReadModelHolder(FILE* fp, ModelHolder* pmh)
{
	int nrelative =0;
	fread(&nrelative, sizeof(int), 1, fp);

	char* relative = new char[nrelative];
	fread(relative, sizeof(char), nrelative, fp);
	pmh->modeli = LoadModel2(relative, Vec3f(1,1,1), Vec3f(0,0,0), true);
	delete [] relative;

	fread(&pmh->rotdegrees, sizeof(Vec3f), 1, fp);
	fread(&pmh->translation, sizeof(Vec3f), 1, fp);
	fread(&pmh->scale, sizeof(Vec3f), 1, fp);
}

void SaveModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = modelholders.size();

	fwrite(&nmh, sizeof(int), 1, fp);

	for(std::list<ModelHolder>::iterator iter = modelholders.begin(); iter != modelholders.end(); iter++)
	{
		SaveModelHolder(fp, &*iter);
	}
}

void ReadModelHolders(FILE* fp, std::list<ModelHolder>& modelholders)
{
	int nmh = 0;

	fread(&nmh, sizeof(int), 1, fp);

	for(int i = 0; i < nmh; i++)
	{
		ModelHolder mh;
		ReadModelHolder(fp, &mh);
		mh.retransform();
		modelholders.push_back(mh);
	}
}

void SaveEdMap(const char* fullpath, EdMap* map)
{
	FILE* fp = fopen(fullpath, "wb");

	char tag[] = TAG_EDMAP;
	fwrite(tag, sizeof(char), 5, fp);

	float version = EDMAP_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

	int texrefs[TEXTURES];
	SaveTexs(fp, texrefs, map->m_brush);

	int nframes = GetNumFrames();
	fwrite(&nframes, sizeof(int), 1, fp);

	SaveBrushes(fp, texrefs, map);
	SaveModelHolders(fp, g_modelholder);

	fclose(fp);
}

void ScaleAll(float factor)
{
	EdMap* map = &g_edmap;

	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		std::list<float> oldus;
		std::list<float> oldvs;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* s = &b->m_sides[i];

			Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = s->m_tceq[0].m_normal / factor;
			s->m_tceq[1].m_normal = s->m_tceq[1].m_normal / factor;
			Vec3f pop = PointOnPlane(s->m_plane);
			pop = pop * factor;
			s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);
		}

		b->collapse();

		std::list<float>::iterator oldu = oldus.begin();
		std::list<float>::iterator oldv = oldvs.begin();

		for(int i=0; i<b->m_nsides; i++, oldu++, oldv++)
		{
			BrushSide* s = &b->m_sides[i];

			Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];

			float newu = newsharedv.x*s->m_tceq[0].m_normal.x + newsharedv.y*s->m_tceq[0].m_normal.y + newsharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = newsharedv.x*s->m_tceq[1].m_normal.x + newsharedv.y*s->m_tceq[1].m_normal.y + newsharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->m_tceq[0].m_d -= changeu;
			s->m_tceq[1].m_d -= changev;
		}

		b->remaptex();
	}

	char msg[128];
	sprintf(msg, "scaled %f", factor);
	InfoMess("asd", msg);
}

bool LoadEdMap(const char* fullpath, EdMap* map)
{
	FreeEdMap(map);

	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
		return false;

	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	char realtag[] = TAG_EDMAP;
	//if(false)
	if(tag[0] != realtag[0] ||  tag[1] != realtag[1] || tag[2] != realtag[2] || tag[3] != realtag[3] || tag[4] != realtag[4])
	{
		fclose(fp);
		ErrMess("Error", "Not a project file (invalid header tag).");
		return false;
	}

	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != EDMAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		ErrMess("Error", msg);
		return false;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 1");
	
#endif

	TexRef* texrefs = NULL;

	ReadEdTexs(fp, &texrefs);

	
#ifdef LOADMAP_DEBUG
	Log("load map 1.01");
	
#endif


#if 1
	int nframes = 0;
	fread(&nframes, sizeof(int), 1, fp);
	SetNumFrames(nframes);
#endif

	
#ifdef LOADMAP_DEBUG
	Log("load map 1.1");
	
#endif

	ReadBrushes(fp, texrefs, map);
	
#ifdef LOADMAP_DEBUG
	Log("load map 1.2");
	
#endif

	ReadModelHolders(fp, g_modelholder);

#ifdef LOADMAP_DEBUG
	Log("load map 1.3");
	
#endif

	if(texrefs)
	{
		delete [] texrefs;
		texrefs = NULL;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 2");
	
#endif

	fclose(fp);

#ifdef LOADMAP_DEBUG
	Log("load map 3");
	
#endif

	//ScaleAll(1.75f);

	return true;
}

void FreeEdMap(EdMap* map)
{
	g_sel1b = NULL;
	g_selB.clear();
	g_dragV = -1;
	g_dragS = -1;
	g_dragD = -1;
	g_dragW = false;
	g_dragM = -1;
	g_selM.clear();

	for(std::list<Brush>::iterator b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* s = &b->m_sides[i];

			if(s->m_diffusem != 0)
				FreeTexture(s->m_diffusem);
			if(s->m_specularm != 0)
				FreeTexture(s->m_specularm);
			if(s->m_normalm != 0)
				FreeTexture(s->m_normalm);
			if(s->m_ownerm != 0)
				FreeTexture(s->m_ownerm);
		}
	}

	map->m_brush.clear();

	FreeModels2();
	FreeModelHolders();

	FreeOrList(&g_orlist);
}
