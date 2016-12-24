

#include "savemap.h"
#include "compilemap.h"
#include "edmap.h"
#include "../texture.h"
#include "save.h"
#include "saveedm.h"
#include "../sim/map.h"
#include "../utils.h"

// chooses the brush texture from one of its sides
// that defines the attributes for the whole brush.
void BrushAttribTex(Brush* brush)
{
	int finaltex = 0;

	for(int sideidx=0; sideidx<brush->m_nsides; sideidx++)
	{
		BrushSide* side = &brush->m_sides[sideidx];

		int sidetex = side->m_diffusem;

		if(g_texture[sidetex].transp)
			finaltex = sidetex;

		if(g_texture[sidetex].grate)
			finaltex = sidetex;

		if(g_texture[sidetex].breakable)
			finaltex = sidetex;

		if(g_texture[sidetex].passthru)
			finaltex = sidetex;

		if(g_texture[sidetex].fabric)
			finaltex = sidetex;
	}

	brush->m_texture = finaltex;
}

// Compile a list of textures used by map brushes
// and save that table to file. Also, a list of
// texture references is set (texrefs) that
// indexes into the written texture table based
// on the diffuse texture index (which indexes into
// g_texture, the global texture array).
void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes)
{
	for(int i=0; i<TEXTURES; i++)
		texrefs[i] = -1;

	// the compiled index of textures used in the map
	std::list<TexRef> compilation;

	for(std::list<Brush>::iterator b=brushes.begin(); b!=brushes.end(); b++)
	{
		//List of sides we will check the textures of.
		//We make a list because we include not only
		//sides of the brush, but the door closed-state model
		//sides too, which might theoretically have different
		//textures if the door model/whatever wasn't updated.
		std::list<BrushSide*> sides;

		for(int i=0; i<b->m_nsides; i++)
			sides.push_back(&b->m_sides[i]);

		if(b->m_door)
		{
			EdDoor* door = b->m_door;

			for(int i=0; i<door->m_nsides; i++)
				sides.push_back(&door->m_sides[i]);
		}

		for(std::list<BrushSide*>::iterator si=sides.begin(); si!=sides.end(); si++)
		{
			BrushSide* s = *si;

			bool found = false;
			for(std::list<TexRef>::iterator j=compilation.begin(); j!=compilation.end(); j++)
			{
				if(j->diffindex == s->m_diffusem)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				Texture* t = &g_texture[s->m_diffusem];
				TexRef tr;
				tr.filepath = MakeRelative(t->fullpath.c_str());
				tr.texname = t->texname;
				tr.diffindex = s->m_diffusem;
				compilation.push_back(tr);
			}
		}

		bool found = false;
		for(std::list<TexRef>::iterator j=compilation.begin(); j!=compilation.end(); j++)
		{
			if(j->diffindex == b->m_texture)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			Texture* t = &g_texture[b->m_texture];
			TexRef tr;
			tr.filepath = MakeRelative(t->fullpath.c_str());
			tr.texname = t->texname;
			tr.diffindex = b->m_texture;
			compilation.push_back(tr);
		}
	}

	//Write the texture table to file.
	int nrefs = compilation.size();
	fwrite(&nrefs, sizeof(int), 1, fp);

#if 0
	Log("writing "<<nrefs<<" tex refs");
	
#endif

	int j=0;
	for(std::list<TexRef>::iterator i=compilation.begin(); i!=compilation.end(); i++, j++)
	{
		texrefs[ i->diffindex ] = j;
		int strl = i->filepath.length()+1;
		fwrite(&strl, sizeof(int), 1, fp);

#if 0
		Log("writing "<<strl<<"-long tex ref");
		
#endif

		fwrite(i->filepath.c_str(), sizeof(char), strl, fp);
	}
}

void ReadTexs(FILE* fp, TexRef** texrefs)
{
	int nrefs;
	fread(&nrefs, sizeof(int), 1, fp);

#if 0
	Log("reading "<<nrefs<<" tex refs");
	
#endif

#ifdef LOADMAP_DEBUG
	Log("nrefs = "<<nrefs<<std::endl;
	
#endif

	(*texrefs) = new TexRef[nrefs];

	for(int i=0; i<nrefs; i++)
	{
		TexRef* tr = &(*texrefs)[i];
		int strl;
		fread(&strl, sizeof(int), 1, fp);

#if 0
		Log("reading "<<strl<<"-long tex ref");
		
#endif

		char* filepath = new char[strl];
		fread(filepath, sizeof(char), strl, fp);
#ifdef LOADMAP_DEBUG
	Log("filepath = "<<filepath<<std::endl;
	
#endif
		tr->filepath = filepath;
		delete [] filepath;
		CreateTex(tr->diffindex, tr->filepath.c_str(), false, true);
		tr->texname = g_texture[tr->diffindex].texname;

		char basepath[SPE_MAX_PATH+1];
		strcpy(basepath, tr->filepath.c_str());
		StripExt(basepath);

		char specpath[SPE_MAX_PATH+1];
		SpecPath(basepath, specpath);

		CreateTex(tr->specindex, specpath, false, true);

		char normpath[SPE_MAX_PATH+1];
		NormPath(basepath, normpath);

		CreateTex(tr->normindex, normpath, false, true);

		char ownpath[SPE_MAX_PATH+1];
		OwnPath(basepath, ownpath);

		CreateTex(tr->ownindex, ownpath, false, true);
	}
}

void SaveBrushes(FILE* fp, int* texrefs, std::list<Brush>& brushes)
{
	int nbrush = brushes.size();
	fwrite(&nbrush, sizeof(int), 1, fp);

#if 0
	Log("writing "<<nbrush<<" brushes at "<<ftell(fp)<<std::endl;
	

	int i=0;
#endif

	for(std::list<Brush>::iterator b=brushes.begin(); b!=brushes.end(); b++)
	{
		SaveBrush(fp, texrefs, &*b);

#if 0
		Log("wrote brush "<<i<<" end at "<<ftell(fp)<<std::endl;
		i++;
#endif
	}
}

// We don't draw the sky textured brushes.
// They are replaced by a sky box.
// But we might want to know which
// ones are sky brushes for some reason.
// We need to save a table of transparent brushes
// because we need to alpha-sort them for proper drawing.
// Now that we saved the transparent brushes,
// we need to store the opaque ones.
// This is because we draw the opaque ones first,
// and then the transparent ones in the right order.
void SaveBrushRefs(FILE* fp, std::list<Brush>& brushes)
{
	std::list<int> opaqbrushrefs;
	std::list<int> transpbrushrefs;
	std::list<int> skybrushrefs;

	int brushidx = 0;
	for(std::list<Brush>::iterator brushitr=brushes.begin(); brushitr!=brushes.end(); brushitr++, brushidx++)
	{
		bool hassky = false;
		bool hastransp = false;

		//Check if any of the sides have a transparent texture
		for(int sideidx = 0; sideidx < brushitr->m_nsides; sideidx++)
		{
			BrushSide* pside = &brushitr->m_sides[sideidx];
			unsigned int sidetex = pside->m_diffusem;

			//If the brush has a sky texture attribute,
			//we want to write it.
			if(g_texture[sidetex].sky)
			{
				skybrushrefs.push_back(brushidx);
				hassky = true;
				break;
			}

			//If side has transparent texture,
			//the brush index will be added to the table.
			if(g_texture[sidetex].transp)
			{
				transpbrushrefs.push_back(brushidx);
				hastransp = true;
				break;
			}
		}

		if(!hassky && !hastransp)
		{
			opaqbrushrefs.push_back(brushidx);
		}
	}

	//Write the brush references tables.

	// Opaque brushes
	int nbrushrefs = opaqbrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = opaqbrushrefs.begin(); refitr != opaqbrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}

	// Transparent brushes
	nbrushrefs = transpbrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = transpbrushrefs.begin(); refitr != transpbrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}

	// Sky brushes
	nbrushrefs = skybrushrefs.size();
	fwrite(&nbrushrefs, sizeof(int), 1, fp);

	for(std::list<int>::iterator refitr = skybrushrefs.begin(); refitr != skybrushrefs.end(); refitr++)
	{
		fwrite(&*refitr, sizeof(int), 1, fp);
	}
}

//Read the brush references tables
void ReadBrushRefs(FILE* fp, Map* map)
{
	// Opaque brushes
	int nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->m_opaquebrush.push_back(ref);
	}

	// Transparent brushes
	nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->m_transpbrush.push_back(ref);
	}

	// Sky brushes
	nbrushrefs = 0;
	fread(&nbrushrefs, sizeof(int), 1, fp);
	for(int refindex = 0; refindex < nbrushrefs; refindex++)
	{
		int ref;
		fread(&ref, sizeof(int), 1, fp);
		map->m_skybrush.push_back(ref);
	}
}

void SaveMap(const char* fullpath, std::list<Brush>& brushes)
{
	FILE* fp = fopen(fullpath, "wb");

	char tag[] = TAG_MAP;
	fwrite(tag, sizeof(char), 5, fp);

	float version = MAP_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

#if 0
	Log("brushes to write: "<<brushes.size()<<std::endl;
#endif

	int texrefs[TEXTURES];
	SaveTexs(fp, texrefs, brushes);

#if 0
	Log("write brushes at "<<ftell(fp)<<std::endl;
	
#endif

	SaveBrushes(fp, texrefs, brushes);
	SaveBrushRefs(fp, brushes);

	fclose(fp);
}

void ReadBrushes(FILE* fp, TexRef* texrefs, Map* map)
{
	int nbrush;
	fread(&nbrush, sizeof(int), 1, fp);

	map->m_nbrush = nbrush;
	map->m_brush = new Brush[nbrush];

#ifdef LOADMAP_DEBUG
	Log("nbrush = "<<nbrush<<std::endl;
	
#endif

	for(int i=0; i<nbrush; i++)
	{
		ReadBrush(fp, texrefs, &map->m_brush[i]);

#ifdef LOADMAP_DEBUG
	Log("added b");
	
#endif

//		Log("read brush "<<i<<" end at "<<ftell(fp)<<std::endl;
	}
}

bool LoadMap(const char* fullpath, Map* map)
{
	map->destroy();

	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
	{
		Log("Failed to show map %s", fullpath);
		return false;
	}

	char tag[5];
	fread(tag, sizeof(char), 5, fp);

	char realtag[] = TAG_MAP;
	//if(false)
	if(tag[0] != realtag[0] ||  tag[1] != realtag[1] || tag[2] != realtag[2] || tag[3] != realtag[3] || tag[4] != realtag[4])
	{
		fclose(fp);
		ErrMess("Error", "Not a map file (invalid header tag).");
		return false;
	}

	float version;
	fread(&version, sizeof(float), 1, fp);

	if(version != MAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map's version (%f) doesn't match %f.", version, MAP_VERSION);
		ErrMess("Error", msg);
		return false;
	}

#ifdef LOADMAP_DEBUG
	Log("load map 1");
	
#endif

	TexRef* texrefs = NULL;

	ReadTexs(fp, &texrefs);

//	Log("read brushes at "<<ftell(fp)<<std::endl;

	ReadBrushes(fp, texrefs, map);
	ReadBrushRefs(fp, map);

//	Log("loaded "<<map->m_nbrush<<" brushes ");
	

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

	return true;
}
