
#include "../platform.h"
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

Surf g_surf;
Surf g_fullsurf;


//#define BIGTEX	4096
//#define BIGTEX	256

int g_bigtex = 4096;

#define BIGTEX	g_bigtex


bool g_rendtopo = false;

int g_orwpx = 256;	//orientability map plane width pixels
int g_orhpx = 256;	//orientability map plane height pixels
int g_orlons = 16;	//orientability map longitude slices
int g_orlats = 16;	//orientability map latitude slices


int g_rendlon = 0;
int g_rendlat = 0;

void PrepareTopo(const char* fullpath, int rendtype)
{
	g_rendtopo = true;
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
}

#if 0

bool TraceRay(Vec3f line[2], 
			  Texture **retex,
			  Texture **retexs,
			  Texture **retexn,
			  Vec2f *retexc,
			  Vec3f *rep, Vec3f* ren)
{
	bool re = false;
	int nearesti = -1;
	float nearestd = Magnitude(line[1]-line[0]);
	char neartype = 0;
	Vec3f nearestv;
	Vec3f nearnorm;

	fprintf(g_applog, "ray%f,%f,%f->%f,%f,%f\r\n", 
		line[0].x, line[0].y, line[0].z,
		line[1].x, line[1].y, line[1].z);

//	fprintf(g_applog, "\r\nsta\r\n");
	

//	if(line[0].y < 0 && line[0].z > 0)
//	fprintf(g_applog, "\r\nstaaaaa\r\n");

	int ci = 0;

	//Texture *retex, *retexs, *retexn;

	unsigned char pr2,pg2,pb2,pa2,
		spr2,spg2,spb2,spa2,
		npr2,npg2,npb2,npa2;

	
				///if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!66!244","!2443!66");

	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
		mit!=g_modelholder.end(); 
		++mit, ++ci)
	{
		Vec3f origline[2];
		origline[0] = line[0];
		origline[1] = line[1];

		Vec3f p = mit->traceray(line, &pr2, &pg2, &pb2, &pa2, 
			&spr2, &spg2, &spb2, &spa2,
			&npr2, &npg2, &npb2, &npa2,
			retex, retexs, retexn,
			retexc,
			&nearnorm, true);

		//if(p == line[1])
		{
			
				//if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!!244","!2443!");
		}
		if(p != origline[1])
		{
		//	if(origline[0].y < 0)
		//	{
		//		ErrMess("!!!!","!!!!");
		//		fprintf(g_applog, "\r\n!!!!\r\n");
		//	}

			re = true;
			//fprintf(g_applog, "p1 %f,%f,%f \r\n", p.x, p.y, p.z);
			neartype = 1;
			//*ren = nearnorm;
			
				//if(
				//	line[0].y < 0)
				//		ErrMess("!!244","!2443!");

			float d = Magnitude(p - line[0]);

			if(d < nearestd || nearesti < 0)
			{
				*ren = nearnorm;
				*rep = p;
				//fprintf(g_applog, "p3 %f,%f,%f \r\n", p.x, p.y, p.z);

				nearestd = d;
				nearesti = ci;
#if 0
				pr = pr2;
				pg = pg2;
				pb = pb2;
				pa = pa2;
				npr = npr2;
				npg = npg2;
				npb = npb2;
				npa = npa2;
				spr = spr2;
				spg = spg2;
				spb = spb2;
				spa = spa2;
#endif
				nearestv = p;
				line[1] = nearestv;
			}
		}
	}
	ci = 0;
	for(std::list<Brush>::iterator bit=g_edmap.m_brush.begin();
		bit!=g_edmap.m_brush.end();
		bit++, ci++)
	{
		Vec3f origline[2];
		origline[0] = line[0];
		origline[1] = line[1];

		Vec3f p = bit->traceray(line, &pr2, &pg2, &pb2, &pa2, 
			&spr2, &spg2, &spb2, &spa2,
			&npr2, &npg2, &npb2, &npa2,
			retex, retexs, retexn,
			retexc,
			&nearnorm);

		if(p != origline[1])
		{
			re = true;
			//fprintf(g_applog, "p2 %f,%f,%f \r\n", p.x, p.y, p.z);

			float d = Magnitude(p - line[0]);

			if(d < nearestd || nearesti < 0)
			{
				*ren = nearnorm;
				*rep = p;
				//fprintf(g_applog, "p4 %f,%f,%f \r\n", p.x, p.y, p.z);

				neartype = 2;
				nearestd = d;
				nearesti = ci;
#if 0
				pr = pr2;
				pg = pg2;
				pb = pb2;
				pa = pa2;
				npr = npr2;
				npg = npg2;
				npb = npb2;
				npa = npa2;
				spr = spr2;
				spg = spg2;
				spb = spb2;
				spa = spa2;
#endif
				nearestv = p;
				line[1] = nearestv;
			}
		}
	}
//	fprintf(g_applog, "\r\nend\r\n");

	//*rep = nearestv;
	return re;

#if 0
	rendtex.data[3 * (px + py * wx) + 0] = pr;
	rendtex.data[3 * (px + py * wx) + 1] = pg;
	rendtex.data[3 * (px + py * wx) + 2] = pb;
	//	rendtex.data[3 * (px + py * wx) + 3] = pa;

	spectex.data[3 * (px + py * wx) + 0] = spr;
	spectex.data[3 * (px + py * wx) + 1] = spg;
	spectex.data[3 * (px + py * wx) + 2] = spb;

	normtex.data[3 * (px + py * wx) + 0] = npr;
	normtex.data[3 * (px + py * wx) + 1] = npg;
	normtex.data[3 * (px + py * wx) + 2] = npb;

	//unsigned int xc = (unsigned int)(50000+nearestv.x);
	//unsigned int yc = (unsigned int)(50000+nearestv.y);
	//unsigned int zc = (unsigned int)(50000+nearestv.z);

	xtex.data[3 * (px + py * wx) + 2] = 0;
	ytex.data[3 * (px + py * wx) + 2] = 0;
	ztex.data[3 * (px + py * wx) + 2] = 0;

	*(unsigned short*)&(xtex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.x);
	*(unsigned short*)&(ytex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.y);
	*(unsigned short*)&(ztex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.z);

	unsigned short neard = nearestd;
	*(unsigned short*)&(surfdepth.data[3 * (px + py * wx) + 0]) = neard;

	nearnorm = Normalize(nearnorm);

	surfnorm.data[3 * (px + py * wx) + 0] = (signed short)(nearnorm.x * 127);
	surfnorm.data[3 * (px + py * wx) + 1] = (signed short)(nearnorm.y * 127);
	surfnorm.data[3 * (px + py * wx) + 2] = (signed short)(nearnorm.z * 127);
#endif
}

void Split(SurfPt *sp, Surf *s,
		   int oi, std::list<SurfPt*>* outrim,
		   std::list<SurfPt*>* outrimnext)
{
	//n0 = inner
	//n1 = lefthind
	//n2=nextfor
	//n3..=out

	
}

void StartRay(Surf* surf, Tet* tet, Vec3f off, SurfPt **resp)
{
	Vec3f line[2];
	line[0] = off;
	line[1] = Vec3f(0,0,0);// - line[0];

#if 0
	if(surf->pts.size())
	{
		Vec3f origoff = surf->pts.begin()->pos;
		line[0] = line[0] + origoff;
		line[1] = line[1] + origoff;
	}
#endif

	Texture *retex, *retexs, *retexn;
	Vec2f retexc;
	Vec3f rep;
	Vec3f ren;

	if(!TraceRay(line,
		&retex, &retexs, &retexn,
		&retexc, &rep, &ren))
	{
		char m[123];
		sprintf(m, "No collision with start line (%f,%f,%f)->(%f,%f,%f)", 
			line[0].x,line[0].y,line[0].z,
			line[1].x,line[1].y,line[1].z);
		ErrMess("Error", m);
		return;
	}

	Vec3f rep2;
	Vec2f retexc2;

	surf->pts.push_back(SurfPt());
	SurfPt* sf = &*surf->pts.rbegin();
	sf->pos = rep;
	sf->texc = retexc;
	sf->norm = ren;
	//sf->off[0] = Vec3f(0,0,-1);

	*resp = sf;

#if 0
	tet->level = 0;
	for(int oi=0; oi<4; oi++)
	{
		if(!tet->neib[oi])
		{
			tet->neib[oi] = sf;
			break;
		}
	}
#endif
}

bool GetFatTri(Surf *surf, Tet **retet, SurfPt* refatt[3], SurfPt* resecfatt[3], 
			   float *refattriw, float *resecfattriw)
{
	SurfPt* fatt[3], *secfatt[3];
	float fattriw = -1, secfattriw = -1;
	Tet *ftet = NULL, *secftet = NULL;

	for(std::list<Tet>::iterator tit=surf->tets.begin();
		tit!=surf->tets.end();
		++tit)
	{
#if 0
		for(int p1it=0;
			tit->neib[p1it];
			p1it++)
		{
			int p2it = p1it;
			p2it++;

			for(;
				tit->neib[p2it];
				p2it++)
			{
				int p3it = p2it;
				p3it++;

				for(;
					tit->neib[p3it];
					p3it++)
				{
#endif
					int p1it = 0;
					int p2it = 1;
					int p3it = 2;

					SurfPt* p1 = tit->neib[p1it];
					SurfPt* p2 = tit->neib[p2it];
					SurfPt* p3 = tit->neib[p3it];

					Vec3f tri[3];
					tri[0] = p1->pos;
					tri[1] = p2->pos;
					tri[2] = p3->pos;

					/*
					Vec3f norm = Normal(tri);

					Vec3f cutter = Cross(norm, tri[1]-tri[2]);

					cutter = Normalize(cutter);

					float lencutter = Dot(cutter, 
					*/

					float a,b,c,s,area;

					a = Magnitude(tri[0]-tri[1]);
					b = Magnitude(tri[1]-tri[2]);
					c = Magnitude(tri[2]-tri[0]);

					s = (a+b+c)/2.0f;

					area = sqrt(s*(s-a)*(s-b)*(s-c));

					if(area != area)
						area = 0;

					if(fattriw <= -1 ||
						area > fattriw ||
						!ftet || !ftet->level)
					{
						secftet = ftet;
						ftet = &*tit;
						secfattriw = fattriw;
						fattriw = area;
						secfatt[0] = fatt[0];
						secfatt[1] = fatt[1];
						secfatt[2] = fatt[2];
						fatt[0] = p1;
						fatt[1] = p2;
						fatt[2] = p3;
						continue;
					}
					else if(secfattriw <= -1 ||
						area > secfattriw ||
						!secftet || !secftet->level)
					{
						secftet = &*tit;
						secfattriw = area;
						secfatt[0] = p1;
						secfatt[1] = p2;
						secfatt[2] = p3;
					}
#if 0
				}
			}
		}
#endif
	}

	if(fattriw >= 0 &&
		secfattriw >= 0 &&
		(fattriw >= 3.0f + secfattriw || surf->pts.size() < BIGTEX*BIGTEX/2/2))
	{
		refatt[0] = fatt[0];
		refatt[1] = fatt[1];
		refatt[2] = fatt[2];
		*refattriw = fattriw;
		*retet = ftet;
		return true;
	}

	fprintf(g_applog, "\r\n fattriw%f, secfatw%f \r\n", fattriw, secfattriw);
	fflush(g_applog);

	return false;
}

bool SplitRay(Surf *surf, Tet *tet,
	SurfPt* fatt[3], int fate,
	Vec3f off, Vec3f along, Vec3f midp, Vec3f norm,
	SurfPt** rep)
{
	Vec3f alongline[2];
	alongline[0] = Normalize(norm)*0 * 0.1 + midp;
	alongline[1] = Normalize(midp-fatt[fate]->pos)*10 + midp 
		+ Normalize(norm)*10 * 0.1;

	Vec3f downline[2];
	downline[0] = Normalize(midp-fatt[fate]->pos)*10 + midp 
		+ Normalize(norm)*10 * 0.1;
	downline[1] = Normalize(midp-fatt[fate]->pos)*10 + midp 
		- Normalize(norm)*10;

	Vec3f backalongline[2];
	backalongline[0] = Normalize(midp-fatt[fate]->pos)*10 + midp 
		- Normalize(norm)*10;
	backalongline[1] = Vec3f(0,0,0) - Normalize(norm)*10 + midp;
	
	Vec3f upline[2];
	upline[0] = Vec3f(0,0,0) - Normalize(norm)*10 + midp;
	upline[1] = Normalize(norm)*10 + midp;

	//second
	Vec3f line3[2];
	line3[0] = midp;
	line3[1] = midp + norm * 100;

	//third
	Vec3f line2[2];
	line2[0] = midp + Normalize(norm);
//	line2[1] = Vec3f(0,0,0) - line2[0];
	line2[1] = midp - norm * 100;

	//first
	Vec3f line[2];
//	line[0] = off;
//	line[1] = Vec3f(0,0,0) - line[0];
//	line[0] = midp;
//	line[1] = Vec3f(0,0,0) - midp;
//	line[0] = line[0] * 2;
	line[0] = Normalize(norm)*0 + fatt[(fate+1)%3]->pos;
	line[1] = Normalize(norm)*0 + fatt[(fate+2)%3]->pos;

	Texture *retex, *retexs, *retexn;
	Vec2f retexc;
	Vec3f rep2;
	Vec3f ren;

	if(*rep)
		goto got;
#if 0
	if(!TraceRay(alongline,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;

	if(!TraceRay(downline,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;

	if(!TraceRay(backalongline,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;

	if(!TraceRay(upline,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;
#endif

#if 0
	float firstd = Magnitude(line[0] - line[1]);
	int d = 1;

	/*
	line[0] = midp;
	line[1] = midp + Vec3f(rand()%1000-500,rand()%1000-500,rand()%1000-500)
		/1000.0f*(rand()%d)/firstd;
*/

	while(d <= firstd)
	{
		if(!TraceRay(line,
			&retex, &retexs, &retexn,
			&retexc, &rep2, &ren))
		{
			//char m[123];
			//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
			//ErrMess("Error", m);
			//return;
		}
		else
			goto got;

		d++;
		line[1] = midp + Vec3f(rand()%1000-500,rand()%1000-500,rand()%1000-500)
			/1000.0f*(rand()%d)/firstd;
	}
#endif

#if 1
	if(!TraceRay(line,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;

	
	if(!TraceRay(line3,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;

#if 1
	if(!TraceRay(line2,
		&retex, &retexs, &retexn,
		&retexc, &rep2, &ren))
	{
		//char m[123];
		//sprintf(m, "No collision with start line (%f,%f,%f)->(0,0,0)", off.x, off.y, off.z);
		//ErrMess("Error", m);
		//return;
	}
	else
		goto got;
#endif
#endif

	fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
	return false;

got:

	//Vec3f rep2;
	Vec2f retexc2;

	if(!*rep)
	{
		surf->pts.push_back(SurfPt());
		SurfPt* sf = &*surf->pts.rbegin();
		sf->pos = rep2;
		sf->texc = retexc;
		sf->norm = ren;
		//sf->off[0] = Vec3f(0,0,-1);
		*rep = sf;
	}

#if 0
	tet->level = 0;
	for(int oi=0; oi<4; oi++)
	{
		if(!tet->neib[oi])
		{
			tet->neib[oi] = *rep;
			break;
		}
	}
#endif

	return true;
}

bool SplitTri(Surf *surf, Tet *tet,
	SurfPt* fatt[3])
{
	int fate = -1;
	Vec3f p1 = fatt[0]->pos;
	Vec3f p2 = fatt[1]->pos;
	Vec3f p3 = fatt[2]->pos;
	float len1 = Magnitude(p3-p2);
	float len2 = Magnitude(p1-p3);
	float len3 = Magnitude(p2-p1);


		fprintf(g_applog, "\r\fa2[0,1,2]=%d,%d,%d\r\n", (int)fatt[0], (int)fatt[1], (int)fatt[2]);
		fflush(g_applog);

	if(len1>len2)
	{
		if(len1>len3)
		{
			fate=1-1;
		}
		else
			fate=3-1;
	}
	else
	{
		if(len2>len3)
		{
			fate=2-1;
		}
		else
			fate=3-1;
	}

	int e1 = (fate+1)%3;
	int e2 = (fate+2)%3;

	Vec3f tri[3];
	tri[0] = p1;
	tri[1] = p2;
	tri[2] = p3;

	Vec3f midp = (fatt[e1]->pos+fatt[e2]->pos)/2.0f;

		fprintf(g_applog, "\r\nmidp=(%f,%f,%f)>(%f,%f,%f)=(%f,%f,%f)\r\n",
			fatt[e1]->pos.x,fatt[e1]->pos.y,fatt[e1]->pos.z,
			fatt[e2]->pos.x,fatt[e2]->pos.y,fatt[e2]->pos.z,
			midp.x,midp.y,midp.z);

	//Vec3f norm = Normal(tri);
	Vec3f norm = (fatt[e1]->norm + fatt[e2]->norm)/2.0f;
	SurfPt* newp = NULL;

	
		fprintf(g_applog, "\r\nnorm=(%f,%f,%f)>(%f,%f,%f)=(%f,%f,%f)\r\n",
			fatt[e1]->norm.x,fatt[e1]->norm.y,fatt[e1]->norm.z,
			fatt[e2]->norm.x,fatt[e2]->norm.y,fatt[e2]->norm.z,
			norm.x,norm.y,norm.z);

	if(!SplitRay(surf, tet, fatt,
		fate, Vec3f(), Vec3f(), midp, norm, &newp))
	{
		fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
		return false;
	}

	fprintf(g_applog, "\r\nnewp%f,%f,%f\r\n", newp->pos.x, newp->pos.y, newp->pos.z);
	fprintf(g_applog, "\r\fa3[0,1,2]=%d,%d,%d\r\n fatt[0]=%f,%f,%f\r\n fatt[1]=%f,%f,%f\r\n fatt[2]=%f,%f,%f\r\n", 
		(int)fatt[0], (int)fatt[1], (int)fatt[2],
		fatt[0]->pos.x, fatt[0]->pos.y, fatt[0]->pos.z, 
		fatt[1]->pos.x, fatt[1]->pos.y, fatt[1]->pos.z, 
		fatt[2]->pos.x, fatt[2]->pos.y, fatt[2]->pos.z);
	fflush(g_applog);

	//SurfPt* fatt2[3] = {NULL};

	//GetSharedTri(fatt[e1], fatt[e2], &tet, fatt2);


	for(std::list<Tet>::iterator tit=surf->tets.begin();
		tit!=surf->tets.end();
		tit++)
	{
		if(&*tit == tet)
		{
			int lev2 = tet->level+1;
			int bef = surf->tets.size();
			surf->tets.erase(tit);

			if(surf->tets.size() < bef)
			{
				fprintf(g_applog, "\r\n%d->b%d tets\r\n", (int)bef, (int)surf->tets.size());
				fflush(g_applog);
			}

			Tet *newtet1, *newtet2;

			surf->tets.push_back(Tet());
			newtet1 = &*surf->tets.rbegin();
			surf->tets.push_back(Tet());
			newtet2 = &*surf->tets.rbegin();

			newtet1->level = lev2;
			newtet2->level = lev2;

			newtet1->neib[0] = fatt[fate];
			newtet1->neib[1] = newp;
			newtet1->neib[2] = fatt[e1];
			newtet1->neib[3] = NULL;

			newtet2->neib[0] = fatt[fate];
			newtet2->neib[1] = fatt[e2];
			newtet2->neib[2] = newp;
			newtet2->neib[3] = NULL;

			goto go1;
			//return true;
		}
	}

	return false;
go1:

	int share1 = -1;
	int share2 = -1;

	/////
	for(std::list<Tet>::iterator tit=surf->tets.begin();
		tit!=surf->tets.end();
		tit++)
	{
		share1 = -1;
		share2 = -1;

		for(int pin=0; pin<3; ++pin)
		{
			if(tit->neib[pin] == fatt[e1])
				share1 = pin;
			if(tit->neib[pin] == fatt[e2])
				share2 = pin;
			if(tit->neib[pin] == fatt[fate])
				goto gonext;
		}

		if(share1 >= 0 && share2 >= 0)
		{
			SurfPt* newp2 = newp;
			Vec3f tri2[3];
			SurfPt* fatt2[3] = {NULL};
			int fate2 = 0;
			while(fate2 == share1 || fate2 == share2)
				fate2++;
			fatt2[share1] = tit->neib[share1];
			fatt2[share2] = tit->neib[share2];
			fatt2[fate2] = tit->neib[fate2];
			Vec3f p12 = fatt2[0]->pos;
			Vec3f p22 = fatt2[1]->pos;
			Vec3f p32 = fatt2[2]->pos;
			tri2[0] = p12;
			tri2[1] = p22;
			tri2[2] = p32;
			//Vec3f norm2 = Normal(tri2);
			Vec3f norm2 = (fatt2[0]->norm + fatt2[1]->norm + fatt2[2]->norm)/3.0f;
#if 0
			if(!SplitRay(surf, &*tit, fatt2,
				fate2, Vec3f(), Vec3f(), midp, norm2, &newp2))
			{	
				fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
				fflush(g_applog);
				return false;
			}
#endif
			int lev2 = tet->level+1;
			int bef = surf->tets.size();
			surf->tets.erase(tit);

			if(surf->tets.size() < bef)
			{
				fprintf(g_applog, "\r\n%d->%d tets\r\n", (int)bef, (int)surf->tets.size());
				fflush(g_applog);
			}

			Tet *newtet1, *newtet2;

			surf->tets.push_back(Tet());
			newtet1 = &*surf->tets.rbegin();
			surf->tets.push_back(Tet());
			newtet2 = &*surf->tets.rbegin();

			newtet1->level = lev2;
			newtet2->level = lev2;

			newtet1->neib[0] = fatt2[share1];
			newtet1->neib[1] = newp2;
			newtet1->neib[2] = fatt2[fate2];
			newtet1->neib[3] = NULL;

			newtet2->neib[0] = fatt2[fate2];
			newtet2->neib[1] = newp2;
			newtet2->neib[2] = fatt2[share2];
			newtet2->neib[3] = NULL;

			goto go2;
		}

gonext:
		continue;
	}

				fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
				fflush(g_applog);
	return false;

go2:
	//if(tet->level)
	{
		return true;
	}

	return false;
}
#endif

/*
==================
ClipWinding
Clips the winding to the plane, returning the new winding on the positive side
Frees the input winding.
If keepon is true, an exactly on-plane winding will be saved, otherwise
it will be clipped away.
==================
*/
#if 0
winding_t *ClipWinding (winding_t *in, plane_t *split, qboolean keepon)
{
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*newtet;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = Dot (in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	if (keepon && !counts[0] && !counts[1])
		return in;
		
	if (!counts[0])
	{
		FreeWinding (in);
		return NULL;
	}
	if (!counts[1])
		return in;
	
	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors
	newtet = NewWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, newtet->points[newtet->numpoints]);
			newtet->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, newtet->points[newtet->numpoints]);
			newtet->numpoints++;
		}
		
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
	// generate a split point
		p2 = in->points[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (split->normal[j] == 1)
				mid[j] = split->dist;
			else if (split->normal[j] == -1)
				mid[j] = -split->dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		VectorCopy (mid, newtet->points[newtet->numpoints]);
		newtet->numpoints++;
	}
	
	if (newtet->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
		
// free the original winding
	FreeWinding (in);
	
	return newtet;
}
#endif

/*
Also must deal with case of
zero-depth/zero-volume two opposite
facing tris forming a double-sided triangle.
*/

/*
Initialize the full mesh
and clipped mesh.
Maybe step- by-step switch the latest
with the second latest.
*/
bool AddClipMesh(Surf *surf, Surf *fullsurf)
{
	int lastvi = 0;

	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
		mit!=g_modelholder.end();
		++mit)
	{
		mit->addclipmesh(surf);
		mit->addclipmesh(fullsurf);
	}

	for(std::list<Brush>::iterator bit=g_edmap.m_brush.begin();
		bit!=g_edmap.m_brush.end();
		++bit)
	{
		bit->addclipmesh(surf);
		bit->addclipmesh(fullsurf);
	}

	return true;
}


SurfPt* RepPt(Surf *surf,
			   SurfPt *sp, Tet *totet,
			   bool replacesp)
{
	for(int vi=0; vi<4; ++vi)
	{
		if(!totet->neib[vi])//remember remove parents of sp from sp2
		{
			SurfPt* sp2 = new SurfPt;
			surf->pts2.push_back(sp2);
			totet->neib[vi] = sp2;
	//		sp2->gen = sp->gen+1;
			sp2->holder.push_back(totet);
	//		sp2->norm = sp->norm;
			sp2->pos = sp->pos;
	//		sp2->tex = sp->tex;
	//		sp2->stex = sp->stex;
	//		sp2->ntex = sp->ntex;

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
			{
				sp2->holder.push_back(*hit);

				if(replacesp)
				{
					for(int vi2=0; vi2<4; ++vi2)
					{
						if((*hit)->neib[vi2] == sp)
							(*hit)->neib[vi2] = sp2;
					}
				}
			}

			//add sp2! (not sp) to previous tet's!!!

			RemDupTet(&sp2->holder);
			//sp2->holder.unique(UniqueTet);

			return sp2;
		}
	}
	ErrMess("!4","!4");
	return NULL;
}

/*
Replace all occurances of sp with sp2
Holders list of sp should already be added to sp2
*/
void RepPt2(Surf *surf,
			   SurfPt *sp,
			   SurfPt *sp2)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		for(int vi=0; vi<4; vi++)
		{
			if(tet->neib[vi] == sp)
			{
#if 0
				for(std::list<Tet*>::iterator hit=sp->holder.begin();
					hit!=sp->holder.end();
					++hit)
				{
					sp2->holder.push_back(*hit);
				}
#endif
				tet->neib[vi] = sp2;
				//sp2->holder.unique(UniqueTet);
			}
		}
	}
}

void AddPt(Surf *surf,
			   SurfPt *sp, Tet *totet)
{
	for(int vi=0; vi<4; ++vi)
	{
		if(!totet->neib[vi])//remember remove parents of sp from sp2
		{
		//	surf->pts2.push_back(sp);
			totet->neib[vi] = sp;
	//		sp2->gen = sp->gen+1;
			sp->holder.push_back(totet);
	//		sp2->norm = sp->norm;
		//	sp->pos = sp->pos;
	//		sp2->tex = sp->tex;
	//		sp2->stex = sp->stex;
	//		sp2->ntex = sp->ntex;

			//add sp2! (not sp) to previous tet's!!!

			//sp->holder.unique(UniqueTet);
			RemDupTet(&sp->holder);

			return;
		}
	}
	ErrMess("!4","!4");
}

SurfPt* CopyPt(Surf *surf,
			   SurfPt *sp, Tet *totet)
{
	for(int vi=0; vi<4; ++vi)
	{
		if(!totet->neib[vi])//remember remove parents of sp from sp2
		{
			SurfPt* sp2 = new SurfPt;
			surf->pts2.push_back(sp2);
			totet->neib[vi] = sp2;
	//		sp2->gen = sp->gen+1;
			sp2->holder.push_back(totet);
	//		sp2->norm = sp->norm;
			sp2->pos = sp->pos;
			sp2->wrappos = sp->wrappos;
			sp2->orc = sp->orc;
	//		sp2->tex = sp->tex;
	//		sp2->stex = sp->stex;
	//		sp2->ntex = sp->ntex;

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
			{
				sp2->holder.push_back(*hit);
#if 0
				if(replacesp)
				{
					for(int vi2=0; vi2<4; ++vi2)
					{
						if((*hit)->neib[vi2] == sp)
							(*hit)->neib[vi2] = sp2;
					}
				}
#endif	//RepSp
			}

			//add sp2! (not sp) to previous tet's!!!

			//sp2->holder.unique(UniqueTet);
			RemDupTet(&sp->holder);

			return sp2;
		}
	}
	ErrMess("!4","!4");
	return NULL;
}

//2=backward copy
SurfPt* CopyPt2(Surf *surf,
			   SurfPt *sp, Tet *totet)
{
	//return CopyPt(surf,sp,totet);
	for(int fvi=2; fvi>=-1; --fvi)
	{
		int vi = (fvi+4)%4;

		if(!totet->neib[vi])//remember remove parents of sp from sp2
		{
			SurfPt* sp2 = new SurfPt;
			surf->pts2.push_back(sp2);
			totet->neib[vi] = sp2;
	//		sp2->gen = sp->gen+1;
			sp2->holder.push_back(totet);
	//		sp2->norm = sp->norm;
			sp2->pos = sp->pos;
	//		sp2->tex = sp->tex;
	//		sp2->stex = sp->stex;
	//		sp2->ntex = sp->ntex;

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
				sp2->holder.push_back(*hit);

			//sp2->holder.unique(UniqueTet);
			RemDupTet(&sp2->holder);

			return sp2;
		}
	}
	ErrMess("!4","!4");
	return NULL;
}

void GenTexEq1(
	float &a,
	float &b,
	float &c,
	float &d,
	
	float &A,
	float &B,
	float &C,
	
	float &D,
	float &E,
	float &F,
	
	float &G,
	float &H,
	float &I,
	
	float &J,
	float &K,
	float &L)
{
	//u1 = ax1+by1+cz1+d
	//u2 = ax2+by2+cz2+d
	//u3 = ax3+by3+cz3+d

	//J = aA+bD+cG+d
	//K = aB+bE+cH+d
	//L = aC+bF+cI+d

	d = 0;

again:

	//d -= 55;

	c = -(((A*B-A*A)*F+(A*C-A*A)*E+
		(-B*C-A*B)*D)*d+
		(A*C*D*E-B*C*D*D)*b+
		(A*B*D-A*A*E)*L+
		A*A*F*K+
		(-A*B*F-A*C*E+B*C*D)*J)/
		((A*A*E-A*B*D)*I-A*A*F*H+(A*B*F+A*C*E-B*C*D)*G);

	//if(ISNAN(c))
	//	goto again;

	b = ((B-A)*d+(B*G-A*H)*c+A*K-B*J)/(A*E-B*D);

	//if(ISNAN(c))
	//	goto again;

	a =-(d+G*c+D*b-J)/A;

	//if(ISNAN(a))
	//	goto again;

/*

	d=-1.11;
	d -= 0.13f;

again:

	d += 0.13f;

	
	if(ISNAN(d))
	goto again;

	c = ((A*F-A*E+B*D)*d+(B*C*D-A*C*E)*a+(A*E-B*D)*L-A*F*K+B*F*J)/
		((A*E-B*D)*I-A*F*H+B*F*G+B*F);

	if(ISNAN(c))
	{
		goto again;
		//ErrMess("sfgsdfg", "isnan c gen");

		//char mm[123];
		//sprintf(mm, "((A*E-B*D)*I-A*F*H+B*F*G+B*F)=%f", ((A*E-B*D)*I-A*F*H+B*F*G+B*F));
		//ErrMess(mm,mm);
	}

	b=-(A*d+(A*H-B*G-B)*c-A*K+B*J)/(A*E-B*D);
	
	if(ISNAN(b))
	{
		//ErrMess("sfgsdfg", "isnan b gen");

		//char mm[123];
		//sprintf(mm, "(A*E-B*D)=%f", (A*E-B*D));
		//ErrMess(mm,mm);
		goto again;
	}

	a=-(d+G*c+D*b-J)/A;
	
	if(ISNAN(a))
	{
		//ErrMess("sfgsdfg", "isnan a gen");

		//char mm[123];
		//sprintf(mm, "A=%f", A);
		//ErrMess(mm,mm);
		goto again;
	}
*/

}

void GenTexEq(Tet *tet, Vec3f tri[3], Vec2f txc[3])
{
	//Vec3f uline[2];
	//Vec3f vline[2];

	//u1 = ax1+by1+cz1+d
	//u2 = ax2+by2+cz2+d
	//u3 = ax3+by3+cz3+d

	//J = aA+bD+cG+d
	//K = aB+bE+cH+d
	//L = aC+bF+cI+d

	tet->texc[0]=txc[0];
	tet->texc[1]=txc[1];
	tet->texc[2]=txc[2];
	tet->texcpos[0]=tri[0];
	tet->texcpos[1]=tri[1];
	tet->texcpos[2]=tri[2];

	for(int v1=0; v1<3; v1++)
	{
		for(int v2=v1+1; v2<3; v2++)
		{
			if(tri[v1] == tri[v2])
				ErrMess("sfgdsg","is eq gen tri");
		}
	}

	if(ISNAN(tri[0].x))
		ErrMess("sfgdfg","is nan gen tri 0 x");
	if(ISNAN(tri[0].y))
		ErrMess("sfgdfg","is nan gen tri 0 y");
	if(ISNAN(tri[0].z))
		ErrMess("sfgdfg","is nan gen tri 0 z");
	
	if(ISNAN(tri[1].x))
		ErrMess("sfgdfg","is nan gen tri 1 x");
	if(ISNAN(tri[1].y))
		ErrMess("sfgdfg","is nan gen tri 1 y");
	if(ISNAN(tri[1].z))
		ErrMess("sfgdfg","is nan gen tri 1 z");
	
	if(ISNAN(tri[2].x))
		ErrMess("sfgdfg","is nan gen tri 2 x");
	if(ISNAN(tri[2].y))
		ErrMess("sfgdfg","is nan gen tri 2 y");
	if(ISNAN(tri[2].z))
		ErrMess("sfgdfg","is nan gen tri 2 z");

	//tet->texceq[0]
#if 00
	//u1 = ax1+by1+cz1+d
	//u2 = ax2+by2+cz2+d
	//u3 = ax3+by3+cz3+d

	//J = aA+bD+cG+d
	//K = aB+bE+cH+d
	//L = aC+bF+cI+d
	GenTexEq1(
		tet->texceq[0].m_normal.x,
		tet->texceq[0].m_normal.y,
		tet->texceq[0].m_normal.z,
		tet->texceq[0].m_d,
		tri[0].x,
		tri[1].x,
		tri[2].x,
		tri[0].y,
		tri[1].y,
		tri[2].y,
		tri[0].z,
		tri[1].z,
		tri[2].z,
		txc[0].x,
		txc[1].x,
		txc[2].x);
#endif
#if 0
	GenTexEq1(
		tet->texceq[0].m_normal.x,
		tet->texceq[0].m_normal.y,
		tet->texceq[0].m_normal.z,
		tet->texceq[0].m_d,
		tri[2].x,
		tri[1].x,
		tri[0].x,
		tri[2].y,
		tri[1].y,
		tri[0].y,
		tri[2].z,
		tri[1].z,
		tri[0].z,
		txc[2].x,
		txc[1].x,
		txc[0].x);
#endif

	/*
	if(ISNAN(tet->texceq[0].m_normal.x))
	{
		ErrMess("dfgdg","is nan gen tet->texceq[0].m_normal.x");
	
		char mm[123];
		sprintf(mm, "5th=%f", tri[0].x);
		ErrMess(mm,mm);
	}
	if(ISNAN(tet->texceq[0].m_normal.y))
		ErrMess("dfgdg","is nan gen tet->texceq[0].m_normal.y");
	if(ISNAN(tet->texceq[0].m_normal.z))
		ErrMess("dfgdg","is nan gen tet->texceq[0].m_normal.z");
	if(ISNAN(tet->texceq[0].m_d))
		ErrMess("dfgdg","is nan gen tet->texceq[0].m_d");
#if 00
	GenTexEq1(
		tet->texceq[1].m_normal.x,
		tet->texceq[1].m_normal.y,
		tet->texceq[1].m_normal.z,
		tet->texceq[1].m_d,
		tri[0].x,
		tri[1].x,
		tri[2].x,
		tri[0].y,
		tri[1].y,
		tri[2].y,
		tri[0].z,
		tri[1].z,
		tri[2].z,
		txc[0].y,
		txc[1].y,
		txc[2].y);
#endif
#if 0
	GenTexEq1(
		tet->texceq[1].m_normal.x,
		tet->texceq[1].m_normal.y,
		tet->texceq[1].m_normal.z,
		tet->texceq[1].m_d,
		tri[2].x,
		tri[1].x,
		tri[0].x,
		tri[2].y,
		tri[1].y,
		tri[0].y,
		tri[2].z,
		tri[1].z,
		tri[0].z,
		txc[2].y,
		txc[1].y,
		txc[0].y);
#endif

	if(ISNAN(tet->texceq[1].m_normal.x))
		ErrMess("dfgdg","is nan gen tet->texceq[1].m_normal.x");
	if(ISNAN(tet->texceq[1].m_normal.y))
		ErrMess("dfgdg","is nan gen tet->texceq[1].m_normal.y");
	if(ISNAN(tet->texceq[1].m_normal.z))
		ErrMess("dfgdg","is nan gen tet->texceq[1].m_normal.z");
	if(ISNAN(tet->texceq[1].m_d))
		ErrMess("dfgdg","is nan gen tet->texceq[1].m_d");
*/
	/*
	c = ((A*F-A*E+B*D)*d+(B*C*D-A*C*E)*a+(A*E-B*D)*L-A*F*K+B*F*J)/
		((A*E-B*D)*I-A*F*H+B*F*G+B*F)
	*/

	/*
	d=0

	b=-(A*d+(A*H-B*G-B)*c-A*K+B*J)/(A*E-B*D)
	*/

	/*
	a=-(d+G*c+D*b-J)/A
	*/

//get:a,b,c,d
	//given: u1,u2,u3,x1,x2,x3,y1,y2,y3,z1,z2,z3
	
	//a=(u1-by1-cz1-d)/x1
	//b=(u2-ax2-cz2-d)/y2
	//c=(u3-ax3-by3-d)/z3

	//b->c
	//c=(u3-ax3-((u2-ax2-cz2-d)/y2)y3-d)/z3
	//c=(u3-ax3-d)/z3-(((u2-ax2-cz2-d)/y2)y3)/z3
	//c=(u3-ax3-d)/z3-(u2/y2*y3-ax2/y2*y3-cz2/y2*y3-d/y2*y3)/z3
	//c=(u3-ax3-d)/z3-(u2y3/y2/z3-ax2y3/y2/z3-cz2y3/y2/z3-dy3/y2/z3)
	//c=(u3-ax3-d)/z3-u2y3/y2/z3+ax2y3/y2/z3+cz2y3/y2/z3+dy3/y2/z3
	//c-cz2y3/y2/z3=(u3-ax3-d)/z3-u2y3/y2/z3+ax2y3/y2/z3+dy3/y2/z3
	//c(1-z2y3/y2/z3)=(u3-ax3-d)/z3-u2y3/y2/z3+ax2y3/y2/z3+dy3/y2/z3
	//c=( (u3-ax3-d)/z3-u2y3/y2/z3+ax2y3/y2/z3+dy3/y2/z3 )/(1-z2y3/y2/z3)

	//a->c
	//c=( (u3-((u1-by1-cz1-d)/x1)x3-d)/z3 - u2y3/y2/z3 + ((u1-by1-cz1-d)/x1)x2y3/y2/z3+dy3/y2/z3 ) / (1-z2y3/y2/z3)
	//c=( (u3-((u1-by1-cz1-d)/x1)x3-d)/z3 ) / (1-z2y3/y2/z3) 
	//		- (u2y3/y2/z3) / (1-z2y3/y2/z3) 
	//		+ ( ((u1-by1-cz1-d)/x1)x2y3/y2/z3+dy3/y2/z3 ) / (1-z2y3/y2/z3)
	//c - ( (u3-((u1-by1-cz1-d)/x1)x3-d)/z3 ) / (1-z2y3/y2/z3)  
	//		- ( ((u1-by1-cz1-d)/x1)x2y3/y2/z3+dy3/y2/z3 ) / (1-z2y3/y2/z3)
	//		= - (u2y3/y2/z3) / (1-z2y3/y2/z3) 
	
	//c - ( (u3-((u1-by1-cz1-d)/x1)x3-d)/z3 - ((u1-by1-cz1-d)/x1)x2y3/y2/z3+dy3/y2/z3 ) / (1-z2y3/y2/z3)
	//		= - (u2y3/y2/z3) / (1-z2y3/y2/z3) 
	
	//c - ( u3/z3-((u1-by1-cz1-d)/x1)x3/z3-d/z3 - ((u1-by1-cz1-d)/x1)x2y3/y2/z3+dy3/y2/z3 ) / (1-z2y3/y2/z3)
	//		= - (u2y3/y2/z3) / (1-z2y3/y2/z3) 

	//d=u1-ax1-by1-cz1=u2-ax2-by2-cz2=u3-ax3-by3-cz3
	//u1-ax1-by1-cz1=u2-ax2-by2-cz2=u3-ax3-by3-cz3=0=d

	//a=(u1-by1-cz1-d)/x1
	//a=(u2-by2-cz2-d)/x2
	//(u1-by1-cz1-d)/x1=(u2-by2-cz2-d)/x2
	//b=(u3-ax3-cz3-d)/y3
	//(u1-by1-cz1-d)/x1-(u2-by2-cz2-d)/x2=0
	//((u1-by1-cz1-d)x2-(u2-by2-cz2-d)x1)/(x1*x2)=0


	//
	//MakePlane(&tet->texceq[0].m_normal, &tet->texceq[0].m_d, 

	//Vec3f uaxis = Normalize(Cross(PlaneCrossAxis(m_plane.m_normal), m_plane.m_normal)) / STOREY_HEIGHT;
	//Vec3f vaxis = Normalize(Cross(uaxis, m_plane.m_normal)) / STOREY_HEIGHT;

	//tet->texceq[0] = Plane3f(uaxis.x, uaxis.y, uaxis.z, 0);
	//tet->texceq[1] = Plane3f(vaxis.x, vaxis.y, vaxis.z, 0);
}

void CheckTet(Tet *newtet, const char* file, int line)
{
	char mm[123];
	sprintf(mm, "%d %s", line, file);
	/*
	if(ISNAN( newtet->texceq[0].m_normal.x ))
		ErrMess(mm,"isnan txcv0x");
	if(ISNAN( newtet->texceq[0].m_normal.y ))
		ErrMess(mm,"isnan txcv0y");
	if(ISNAN( newtet->texceq[0].m_normal.z ))
		ErrMess(mm,"isnan txcv0z");
	if(ISNAN( newtet->texceq[0].m_d ))
		ErrMess(mm,"isnan txcv0d");

	if(ISNAN( newtet->texceq[1].m_normal.x ))
		ErrMess(mm,"isnan txcv1x");
	if(ISNAN( newtet->texceq[1].m_normal.y ))
		ErrMess(mm,"isnan txcv1y");
	if(ISNAN( newtet->texceq[1].m_normal.z ))
		ErrMess(mm,"isnan txcv1z");
	if(ISNAN( newtet->texceq[1].m_d ))
		ErrMess(mm,"isnan txcv1d");
		*/
}

void CopyTet(Surf *surf, Tet *fromtet)	//just copies tet, not pt's, which will be handled differently
{
	Tet *newtet = new Tet();
	surf->tets2.push_back(newtet);

	//newtet->level = fromtet->level+1;
	newtet->neib[0] = NULL;
	newtet->neib[1] = NULL;
	newtet->neib[2] = NULL;
	newtet->neib[3] = NULL;
	newtet->hidden = false;
	newtet->approved = false;
	newtet->tex = fromtet->tex;
	//newtet->stex = fromtet->stex;
	//newtet->ntex = fromtet->ntex;
	///newtet->texceq[0] = fromtet->texceq[0];
	////newtet->texceq[1] = fromtet->texceq[1];
	newtet->texc[0] = fromtet->texc[0];
	newtet->texc[1] = fromtet->texc[1];
	newtet->texc[2] = fromtet->texc[2];
	newtet->texcpos[0] = fromtet->texcpos[0];
	newtet->texcpos[1] = fromtet->texcpos[1];
	newtet->texcpos[2] = fromtet->texcpos[2];
/*
	if(ISNAN( newtet->texceq[0].m_normal.x ))
		ErrMess("sdfsd","isnan txcv0x");
	if(ISNAN( newtet->texceq[0].m_normal.y ))
		ErrMess("sdfsd","isnan txcv0y");
	if(ISNAN( newtet->texceq[0].m_normal.z ))
		ErrMess("sdfsd","isnan txcv0z");
	if(ISNAN( newtet->texceq[0].m_d ))
		ErrMess("sdfsd","isnan txcv0d");

	if(ISNAN( newtet->texceq[1].m_normal.x ))
		ErrMess("sdfsd","isnan txcv1x");
	if(ISNAN( newtet->texceq[1].m_normal.y ))
		ErrMess("sdfsd","isnan txcv1y");
	if(ISNAN( newtet->texceq[1].m_normal.z ))
		ErrMess("sdfsd","isnan txcv1z");
	if(ISNAN( newtet->texceq[1].m_d ))
		ErrMess("sdfsd","isnan txcv1d");*/
}
void FreeTet(Surf *surf, Tet *tet, bool freepts)	//entry must be freed from surf also
{
	if(freepts)
	{
		//if no other holder owns these pt's
		for(int vi=0; vi<4; ++vi)
		{
			if(tet->neib[vi])
			{
				SepPt(tet, tet->neib[vi], NULL);
				if(!tet->neib[vi]->holder.size())
				{
					if((tet)->neib[vi]->gone)
						ErrMess("g1","g1");
					tet->neib[vi]->gone=true;
					delete tet->neib[vi];

					std::list<SurfPt*>::iterator pit=surf->pts2.begin();
					while(
						pit!=surf->pts2.end())
					{
						if(*pit == tet->neib[vi])
							pit = surf->pts2.erase(pit);
						else
							++pit;
					}
				}
				tet->neib[vi] = NULL;
			}
		}
	}
	tet->gone=true;
	delete tet;
}

/*
Test for backwards facing normal (facing with back side to outside, not making any collisions
Might be a stray, disattached triangle, but looking for cases where perhaps the linkage
was done incorrectly when clipping a triangle in two or splitting a quad into tri's.
*/
bool WeirdTest(Surf *surf, Tet *testt)
{
	return false;
	if(!testt->neib[0])
		return false;
	if(!testt->neib[1])
		return false;
	if(!testt->neib[2])
		return false;

	Vec3f tri[3];

	tri[0] = testt->neib[0]->pos;
	tri[1] = testt->neib[1]->pos;
	tri[2] = testt->neib[2]->pos;

	Vec3f n = Normal(tri);

	Vec3f line[3][2][2];	//vert line,reverse order,line point,
	////////////////
	line[0][0][0] = (tri[0]+tri[1]+tri[2])/3.0f - n*0.2f;
	line[0][0][1] = line[0][0][0] - n*30000;

	line[0][1][1] = (tri[0]+tri[1]+tri[2])/3.0f  - n*0.2f;
	line[0][1][0] = line[0][1][1] - n*30000;
	/////////////////
#if 1
	line[1][0][0] = (tri[1]*2.0f/3.0f+tri[2]*1.0f/3.0f)/1.0f - n*0.2f;
	line[1][0][1] = line[1][0][0] - n*30000;

	line[1][1][1] = (tri[1]*2.0f/3.0f+tri[2]*1.0f/3.0f)/1.0f - n*0.2f;
	line[1][1][0] = line[1][1][1] - n*30000;
	/////////////////
	line[2][0][0] = (tri[2]*2.0f/3.0f+tri[0]*1.0f/3.0f)/1.0f - n*0.2f;
	line[2][0][1] = line[2][0][0] - n*30000;

	line[2][1][1] = (tri[2]*2.0f/3.0f+tri[0]*1.0f/3.0f)/1.0f - n*0.2f;
	line[2][1][0] = line[2][1][1] - n*30000;
	////////////
#endif

	LoadedTex *tex, *texn, *texs;
	Vec2f texc;
	Vec3f wp, rp, rn;
	Tet *rtet;
	double fU,fV;

	if( (!TraceRay4(surf,
		line[0][0],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV) &&
		!TraceRay4(surf,
		line[0][1],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV)
		&&
		!TraceRay4(surf,
		line[1][0],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV)&&
		!TraceRay4(surf,
		line[1][1],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV)&&
		!TraceRay4(surf,
		line[2][0],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV)&&
		!TraceRay4(surf,
		line[2][1],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV))
		)
	{
		char mm[1234];
		sprintf(mm, "weird n=%f,%f,%f \r\n"\
			"l0: %f,%f,%f \r\n"\
			"l1: %f,%f,%f \r\n",
			n.x,
			n.y,
			n.z,
			
			line[0][0][0].x,
			line[0][0][0].y,
			line[0][0][0].z,
			line[0][0][1].x,
			line[0][0][1].y,
			line[0][0][1].z);
		ErrMess(mm,mm);


		TraceRay4b(surf,
				line[0][0],
				&tex,
				&texs,
				&texn,
				&texc,
				&wp,
				&rp,
				&rn,
				&rtet,
				&fU, &fV);

		TraceRay4b(surf,
		line[0][1],
		&tex,
		&texs,
		&texn,
		&texc,
		&wp,
		&rp,
		&rn,
		&rtet,
		&fU, &fV);

		return true;	//weird
		//tet->approved = true;
		//	fprintf(g_applog, "app'd\r\n");
		//	fflush(g_applog);
	}
	else
	{
		return false;	//not weird
		//tet->approved = false;
		//fprintf(g_applog, "napp'd\r\n");
		//	fflush(g_applog);
	}


	return false;
}

/*
If there's 4 verts in this tet, break it into two 3-vert tets
The original tet should then be removed from the list because it is freed
Connections to other tets should be maintained correctly
*/
void SplitQuad(Surf *surf, Tet *fourtet,
			   std::list<SurfPt*> *substfrom,
			   std::list<SurfPt*> *substto)
{
	if(!fourtet)
		ErrMess("asd","!t1444");

	if(WeirdTest(surf, fourtet))
		ErrMess("w","w sq ft");

	for(int vi=0; vi<4; ++vi)
	{
		if(!fourtet->neib[vi])
			return;
		if(fourtet->neib[vi]->gone)
			ErrMess("g123123","n133sf");
	}
	CopyTet(surf, fourtet);
	Tet* tet1 = *surf->tets2.rbegin();
	CopyTet(surf, fourtet);
	Tet* tet2 = *surf->tets2.rbegin();

	if(!tet1)
		ErrMess("asd","!t1");
	if(!tet2)
		ErrMess("asd","!t2");
	if(tet1==tet2)
		ErrMess("asd","!t1t2");
	if(fourtet==tet1)
		ErrMess("asd","f!t1");
	if(fourtet==tet2)
		ErrMess("asd","f!t2");

	int fpts=0;
	int bpts=0;

	for(int vi=0; vi<4; ++vi)
	{
		//01
		//32
		//fourtet->neib[vi]->gone=true;
		//delete fourtet->neib[vi];
		//SurfPt *newp = CopyPt(surf, fourtet->neib[vi], tet1);
		//use this only with ClipTris
		SurfPt *newp;
		/*
		if(vi&&vi!=2)
		{
			newp = CopyPt(surf, fourtet->neib[vi], tet1);
			SepPt(fourtet, newp, NULL);
			//use this only with ClipTris
			AddPt(surf, newp, tet2);
			fpts++;
			bpts++;
		}
		else */
		if(vi)
		{
			//tet2->neib[vi-1] = new SurfPt;
			//SurfPt* newp = RepPt(surf, fourtet->neib[vi], tet2, true);
			newp = CopyPt(surf, fourtet->neib[vi], tet2);	
			//use this only with ClipTris
			if(!newp)
				ErrMess("!v","!v");
			SepPt(fourtet, newp, NULL);
			if(newp->gone)
				ErrMess("g123123","n133sfgn");
			fpts++;
		}
		//else 
		if(vi!=2)
		{
			//tet1->neib[vi] = new SurfPt;
			//SurfPt* newp = RepPt(surf, fourtet->neib[vi], tet1, true);
			newp = CopyPt(surf, fourtet->neib[vi], tet1);
			//use this only with ClipTris
			if(!newp)
				ErrMess("!v2","!v2");
			SepPt(fourtet, newp, NULL);
			if(newp->gone)
				ErrMess("g123123","sdf");
			bpts++;
		}
		//fourtet->neib[vi]=NULL;
		///substfrom->push_back(fourtet->neib[vi]);
		///substto->push_back(newp);
	}
	//FreeTet(surf, fourtet, false);
	FreeTet(surf, fourtet, true);
	//remember to remove fourtet from tets2 list
	//
	std::list<Tet*>::iterator tit=surf->tets2.begin();
	while(
		tit!=surf->tets2.end())
	{
		if(*tit == fourtet)
		{
			tit = surf->tets2.erase(tit);
			//return;
			continue;
		}
		++tit;
	}

#if 0
	std::list<SurfPt*>::iterator fromit = substfrom->begin();
	std::list<SurfPt*>::iterator toit = substto->begin();

	while(fromit != substfrom->end() &&
		toit != substto->end())
	{
		RepPt2(surf, *fromit, *toit);

		fromit++;
		toit++;
	}

	substfrom->clear();
	substto->clear();
#endif

	if(fpts<3)
		ErrMess("f<","f<ff");
	if(bpts<3)
		ErrMess("b<","b<bb");
}

bool VerifyTet(Surf *surf,
			   std::list<Tet*>::iterator tit)
{
	for(int vin=0; vin<3; ++vin)
	{
		if(!(*tit)->neib[vin])
		{
			Tet* tet = *tit;
			surf->tets2.erase(tit);
			FreeTet(surf, tet, true);
			std::list<Tet*>::iterator tit2=surf->tets2.begin();
			while(tit2!=surf->tets2.end())
			{
				if(*tit2 == tet)
					tit2 = surf->tets2.erase(tit2);
				else
					++tit2;
			}
			return false;
		}
	}
	return true;
}

bool TestDiscard(Surf *surf,
				 std::list<Tet*>::iterator tit)
{return false;
	//Test(surf);
	Tet *tet = *tit;
	for(int vin1=0; vin1<4; vin1++)
	{
		for(int vin2=vin1+1; vin2<4; ++vin2)
		{
			if(tet->neib[vin1] &&
				tet->neib[vin1] == tet->neib[vin2])
			{
				tet->neib[vin2]=NULL;
			}
		}
	}
	for(int vin1=0; vin1<3; vin1++)
	{
		if(!tet->neib[vin1])
			goto discard;
		for(int vin2=vin1+1; vin2<3; ++vin2)
		{
			if(!tet->neib[vin2])
				goto discard;
			if( Magnitude(tet->neib[vin1]->pos - tet->neib[vin2]->pos ) <= 1.0f )
				goto discard;
		}
	}
	//Test(surf);
	return false;

discard:

	//Test(surf);
	FreeTet(surf, tet, true);
	surf->tets2.erase(tit);
	tit=surf->tets2.begin();
	while(tit!=surf->tets2.end())
	{
		if(*tit== tet)
			tit = surf->tets2.erase(tit);
		else
			++tit;
	}

	//Test(surf);
	return true;
}

bool ShouldClip(Surf *surf, Tet *in, Plane3f split, 
				Vec3f* tri1, Vec3f* fulltri)
{
	Vec3f norm1 = Normal(tri1);
	//Vec3f fulltri[3];
	//fulltri[0] = (*fulltit)->neib[0]->pos;
	//fulltri[1] = (*fulltit)->neib[1]->pos;
	//fulltri[2] = (*fulltit)->neib[2]->pos;
	Vec3f fullnorm = Normal(fulltri);

	//if(fullnorm == norm1)
	//	continue;

	//Plane3f split;
	//MakePlane(&split.m_normal, &split.m_d, (fulltri[0]+fulltri[1]+fulltri[2])/3.0f,
	//	fullnorm);
	bool keepon = false;

#define MAX_POINTS_ON_WINDING 7

	float	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	float	dot;
	int		i, j;
	SurfPt	*p1, *p2;
	Vec3f	mid;
	Tet		*newtet;//, *backw;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

#define SIDE_BACK	0
#define SIDE_ON		1
#define SIDE_FRONT	2

	int fpts = 0;
//	int bpts = 0;

	// determine sides for each point
	for (i=0 ; i<3 ; i++)
	{
		dot = Dot (in->neib[i]->pos, split.m_normal);
		//dot -= split.m_d;
		dot += split.m_d;
		dists[i] = dot;
#define ON_EPSILONF	0.3f
		if (dot > ON_EPSILONF)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILONF)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if (//keepon && 
		!counts[0] && !counts[1])
		return false;

	//if(counts[1]>1)
	//	return false;

	if (!counts[0])
	{
		//FreeWinding (in);
		//return NULL;
		return false;	//don't know enough to say that it is truly hidden or if just random tri in front
		//goto again;
	}
	//if (!counts[1])
	//	continue;	//don't know enough to say that it is truly hidden or if just random tri in front
	if(!counts[2])
		return false;

	if(counts[1]==3)
		return false;

	//check tri-tri
	//if(!TriTri(tri1, fulltri))
	//	return false;

	//if(ShareEdge(tri1, fulltri))
	//	return false;

	return true;
}

/*
ShouldClip checks to make sure the triangles are intersecting
CanClip just checks the triangle to a PLANE
*/
bool CanClip(Surf *surf, Tet *in, Plane3f split, 
				Vec3f* tri1, Vec3f* fulltri)
{
	Vec3f norm1 = Normal(tri1);
	//Vec3f fulltri[3];
	//fulltri[0] = (*fulltit)->neib[0]->pos;
	//fulltri[1] = (*fulltit)->neib[1]->pos;
	//fulltri[2] = (*fulltit)->neib[2]->pos;
	Vec3f fullnorm = Normal(fulltri);

	//if(fullnorm == norm1)
	//	continue;

	//Plane3f split;
	//MakePlane(&split.m_normal, &split.m_d, (fulltri[0]+fulltri[1]+fulltri[2])/3.0f,
	//	fullnorm);
	bool keepon = false;

#define MAX_POINTS_ON_WINDING 7

	float	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	float	dot;
	int		i, j;
	SurfPt	*p1, *p2;
	Vec3f	mid;
	Tet		*newtet;//, *backw;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

#define SIDE_BACK	0
#define SIDE_ON		1
#define SIDE_FRONT	2

	int fpts = 0;
//	int bpts = 0;

	// determine sides for each point
	for (i=0 ; i<3 ; i++)
	{
		dot = Dot (in->neib[i]->pos, split.m_normal);
		//dot -= split.m_d;
		dot += split.m_d;
		dists[i] = dot;
#define ON_EPSILONF	0.3f
		if (dot > ON_EPSILONF)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILONF)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if (//keepon && 
		!counts[0] && !counts[1])
		return false;

	if(counts[2]+counts[1]>=3||counts[1]+counts[0]>=3)
		return false;

	if (!counts[0])
	{
		//FreeWinding (in);
		//return NULL;
		return false;	//don't know enough to say that it is truly hidden or if just random tri in front
		//goto again;
	}
	//if (!counts[1])
	//	continue;	//don't know enough to say that it is truly hidden or if just random tri in front
	if(!counts[2])
		return false;

	if(counts[1]==3)
		return false;

	//check tri-tri
	//if(!TriTri(tri1, fulltri))
	//	return false;

	//if(ShareEdge(tri1, fulltri))
	//	return false;

	for (i=0 ; i<3 ; i++)
	{
		if(!in->neib[i])
			ErrMess("44","44");
		if(in->neib[i]->gone)
			ErrMess("g123","g123");

		if (sides[i] == SIDE_ON)
		{
			//newvi++;
			//backvi++;

			//bpts++;
			fpts++;

			//if(bpts>4)
			//	ErrMess(">4",">4b");

			if(fpts>4)
				ErrMess(">4",">4f");

			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
		//	newvi++;

			fpts++;

			if(fpts>4)
				ErrMess(">4",">4fsdf");
		}
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		//bpts++;
		fpts++;

		//if(bpts>4)
		//	ErrMess(">4",">4bdsfgdfg");

		if(fpts>4)
			ErrMess(">4",">4fsdsdfsdf");
	}

	//if (newtet->numpoints > maxpts)
	//	Error ("ClipWinding: points exceeded estimate");

	// free the original winding
	//FreeWinding (in);
	/////FreeTet(surf, in, true);
	//surf->tets2.erase(tit);	//tit invalided by CopyPt etc.
/*
	tit=surf->tets2.begin();
	while(
		tit!=surf->tets2.end())
	{
		if(*tit == in)
		{
			tit=surf->tets2.erase(tit);
			//break;
		}
		else
			++tit;
	}

	Test(surf);
*/
	if(fpts<3)
		return false;

	return true;
}

/*
for all holders of sp, replace with sp2,
except "in" tet.
*/
/*
void RepSp()
{
}
*/

void ClipFront(Surf *surf, Tet *in, Plane3f split,
			   std::list<SurfPt*> *substfrom,
			   std::list<SurfPt*> *substto,
			   Tet **retet)
{
	
	if(WeirdTest(surf, in))
		ErrMess("w","w cf in");

	//Vec3f fulltri[3];
	//fulltri[0] = (*fulltit)->neib[0]->pos;
	//fulltri[1] = (*fulltit)->neib[1]->pos;
	//fulltri[2] = (*fulltit)->neib[2]->pos;
	//Vec3f fullnorm = Normal(fulltri);

	//if(fullnorm == norm1)
	//	continue;

//#define MAX_POINTS_ON_WINDING 7

	//split.m_d = -split.m_d;

	float	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	float	dot;
	int		i, j;
	SurfPt	*p1, *p2;
	Vec3f	mid;
	Tet		*newtet;//, *backw;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

//#define SIDE_BACK	0
//#define SIDE_ON		1
//#define SIDE_FRONT	2

	int fpts = 0;
	int bpts = 0;

	// determine sides for each point
	for (i=0 ; i<3 ; i++)
	{
		dot = Dot (in->neib[i]->pos, split.m_normal);
		//dot -= split.m_d;
		dot += split.m_d;
		//dot = Dot();
		/*
		fprintf(g_applog, "dot[%d] =%f= Dot((%f,%f,%f),(%f,%f,%f) - %f\r\n",
			i, dot,
			in->neib[i]->pos.x,
			in->neib[i]->pos.y,
			in->neib[i]->pos.z,
			split.m_normal.x,
			split.m_normal.y,
			split.m_normal.z,
			split.m_d);*/
		dists[i] = dot;
//#define ON_EPSILON	0.3f
		if (dot > ON_EPSILONF)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILONF)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	//if(counts[0]+counts[1]+counts[2] < 3)
	//	ErrMess("asdas","<333");

	//if (//keepon && 
	//	!counts[0] && !counts[1])
	//	return;

	//if (!counts[0])
	{
		//FreeWinding (in);
		//return NULL;
	//	return;	//don't know enough to say that it is truly hidden or if just random tri in front
		//goto again;
	}
	//if (!counts[1])
	//	continue;	//don't know enough to say that it is truly hidden or if just random tri in front
	//if(!counts[2])
	//	return;

	//check tri-tri
	//if(!TriTri(tri1, fulltri))
	//	continue;

	maxpts = 3+4;	// can't use counts[0]+2 because
	// of fp grouping errors
	//newtet = NewWinding (maxpts);
	//std::list<SurfPt*> neww2;
	CopyTet(surf, in);
	newtet = *surf->tets2.rbegin();
	*retet = newtet;
	//CopyTet(surf, in);
	//backw = *surf->tets2.rbegin();

	if(in->neib[3])
		ErrMess("i4","i4");
//
	//if(newtet == backw)
	//	ErrMess("nb","n=b");

	if(!newtet)
		ErrMess("n","N!");

	//if(!backw)
	//	ErrMess("b","B!");

	int newvi = 0;
	//int backvi = 0;

	//fprintf(g_applog, "clipfront:\r\n");

	//fprintf(g_applog, "split:(%f,%f,%f),%f\r\n",
	//	split.m_normal.x,
	//	split.m_normal.y,
	//	split.m_normal.z,
	//	split.m_d);

	char mm[123]="";

	for (i=0 ; i<3 ; i++)
	{
	//	fprintf(g_applog, "in->neib[%d]=%f,%f,%f\r\n",
	//		i,
	//		in->neib[i]->pos.x,
	//		in->neib[i]->pos.y,
	//		in->neib[i]->pos.z
	//		);

		if(!in->neib[i%3])
			ErrMess("44","44");
		if(in->neib[i%3]->gone)
			ErrMess("g123","g123");

		//if(i==3 && !(sides[0]==SIDE_BACK&&sides[1]==SIDE_BACK())
		//	continue;

		//p1 = in->points[i];
		p1 = in->neib[i%3];

		if (sides[i] == SIDE_ON)
		{
			strcat(mm, "on,");
			//VectorCopy (p1, newtet->points[newtet->numpoints]);
			//newtet->numpoints++;

			SurfPt* newp;
			if(!(newp=CopyPt(surf, p1, newtet)))
				ErrMess("123","12347");
			if(newp->gone)
				ErrMess("g123123","g123123");
			SepPt(in, newp, NULL);
			///substfrom->push_back(p1);
			///substto->push_back(newp);
			//if(!(newp=CopyPt(surf, p1, backw)))
			//	ErrMess("123","12348");
			//if(newp->gone)
			//	ErrMess("g123123","g12312355");
			//SepPt(in, newp, NULL);

			newvi++;
			//backvi++;

			//bpts++;
			fpts++;

			//if(bpts>4)
			//	ErrMess(">4",">4b");

			if(fpts>4)
				ErrMess(">4",">4f");

			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
			strcat(mm, "front,");
			//VectorCopy (p1, newtet->points[newtet->numpoints]);
			//newtet->numpoints++;
			SurfPt* newp;
			if(!(newp=CopyPt(surf, p1, newtet)))
				ErrMess("123","1234");
			if(newp->gone)
				ErrMess("g123123","g123123333");
			SepPt(in, newp, NULL);
			///substfrom->push_back(p1);
			///substto->push_back(newp);
			newvi++;

			fpts++;

			if(fpts>4)
				ErrMess(">4",">4fsdf");
		}
#if 0
		if(i==1 && sides[1]==SIDE_BACK && sides[2]==SIDE_BACK)
		{
			p1 = in->neib[2];
			p2 = in->neib[0];
			//ErrMess("!","!");
			goto setpt;
		}
#endif

		if ( (sides[i+1] == SIDE_ON || sides[i+1] == sides[i] ) //&&
		//if ( (sides[i+1] == SIDE_ON && sides[i+1] == sides[i] ) //&&
		  //( i!=0 || (i==0 && sides[0]!=SIDE_BACK || sides[1]!=SIDE_BACK) ) 
		  )
		{
			strcat(mm, "skip,");
			continue;
		}

		// generate a split point
		p2 = in->neib[(i+1)%3];

setpt:

		if(!p2)
			ErrMess("!2","22");
		if(p2->gone)
			ErrMess("g123123","g123123222");

#if 0
		if(i==0 && sides[0]==SIDE_BACK && sides[1]==SIDE_BACK)
		{
			//p2 = in->neib[(i+1)%3];
			dot = dists[2] / (dists[2]-dists[0]);
			for (j=0 ; j<3 ; j++)
			{
					mid[j] = p1->pos[j] + dot*(p2->pos[j]-p1->pos[j]);
			}
		}
		else
#endif
		{
			strcat(mm, "dotmid,");
			dot = dists[i] / (dists[i]-dists[i+1]);
			for (j=0 ; j<3 ; j++)
			{
					mid[j] = p1->pos[j] + dot*(p2->pos[j]-p1->pos[j]);
			}
		}
		Vec2f neworc = p1->orc + (p2->orc - p1->orc)*dot;
		Vec3f newwrap = p1->wrappos + (p2->wrappos - p1->wrappos)*dot;

		if(ISNAN(mid[0]))
			ErrMess("sfgdsfklg","nannanmidj0");
		if(ISNAN(mid[1]))
			ErrMess("sfgdsfklg","nannanmidj1");
		if(ISNAN(mid[2]))
			ErrMess("sfgdsfklg","nannanmidj2");

		//SurfPt *news1 = CopyPt(surf, p2, newtet);
		SurfPt *news1 = CopyPt(surf, p1, newtet);

		if(!news1)
			ErrMess("1","1");
		if(news1->gone)
			ErrMess("g123123","n1");

		news1->pos = mid;
		news1->wrappos = newwrap;
		news1->orc = neworc;
		////news2->pos = mid;

		SepPt(in, news1, NULL);
		///SepPt(in, news2, NULL);
		if(news1->gone)
			ErrMess("g123123","n133s");
		//if(news2->gone)
		//	ErrMess("g123123","n1233s");
		///substfrom->push_back(NULL);
		///substto->push_back(news1);

		//backvi++;
		newvi++;

		//bpts++;
		fpts++;

		//if(bpts>4)
		//	ErrMess(">4",">4bdsfgdfg");

		if(fpts>4)
			ErrMess(">4",">4fsdsdfsdf");
	}

	//if (newtet->numpoints > maxpts)
	//	Error ("ClipWinding: points exceeded estimate");

	// free the original winding
	//FreeWinding (in);
	/////FreeTet(surf, in, true);
	//surf->tets2.erase(tit);	//tit invalided by CopyPt etc.
/*
	tit=surf->tets2.begin();
	while(
		tit!=surf->tets2.end())
	{
		if(*tit == in)
		{
			tit=surf->tets2.erase(tit);
			//break;
		}
		else
			++tit;
	}

	Test(surf);
*/
	if(fpts<3)
		ErrMess("f<",mm);
	//if(bpts<3)
	//	ErrMess("b<","b<");

	//std::list<Tet*>::iterator backwtit=surf->tets2.end();
	//backwtit--;
	//std::list<Tet*>::iterator newwtit=backwtit;
	//newwtit--;

	////
#if 0
	std::list<SurfPt*>::iterator fromit = substfrom->begin();
	std::list<SurfPt*>::iterator toit = substto->begin();

	while(fromit != substfrom->end() &&
		toit != substto->end())
	{
		RepPt2(surf, *fromit, *toit);

		fromit++;
		toit++;
	}

	substfrom->clear();
	substto->clear();
#endif
	/////

	//if(VerifyTet(surf, newwtit))
	//SplitQuad(surf, newtet,
	//	substfrom, substto);
	//newtet = NULL;
	//else
	//{
	//	backwtit=surf->tets2.end();
	//	backwtit--;
	//}
	//if(VerifyTet(surf, backwtit))
	////SplitQuad(surf, backw);

	//backw = NULL;

	//return newtet;

	//return true;
	//goto again;
}

void TestD(Surf *surf, SurfPt *parp, SurfPt *neibp, SurfPt *newp)
{
	return;
	Vec3f line[6][2];
	Plane3f plane[3];
	Vec3f up[3];
	Vec3f along[3];
	Vec3f pnorm[3];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;
		int notsame = 0;

		for(int v=0; v<3; v++)
		{
			if(tet->neib[v] == parp)
				continue;
			if(tet->neib[v] == neibp)
				continue;
			if(tet->neib[v] == newp)
				continue;
			if(tet->neib[v]->ring < 0)
				goto next;

			notsame++;
		}

		if(notsame == 0)
			continue;

///////////////////////////

		for(int v=0; v<3; v++)
		{
			int e1 = v;
			int e2 = (v+1)%3;
			Vec3f midp = (tet->neib[e1]->wrappos + tet->neib[e2]->wrappos)/2.0f;
			midp = Normalize(midp);
			up[v] = midp;
			along[v] = Normalize( tet->neib[e2]->wrappos - tet->neib[e1]->wrappos );
			pnorm[v] = Cross( along[v], up[v] );//cw
			pnorm[v] = Normalize( pnorm[v] );
			MakePlane(&plane[v].m_normal, &plane[v].m_d, tet->neib[e1]->wrappos, pnorm[v]);
		}
		///////////////////
		SurfPt *in[3];
		in[0] = parp;
		in[1] = neibp;
		in[2] = newp;
		for(int v=0; v<3; v++)
		{
			int e1 = v;
			int e2 = (v+1)%3;
			
			Vec3f dir = in[e2]->wrappos - in[e1]->wrappos;
			Vec3f ndir = Normalize(dir);

			line[v*2+0][0] = in[e1]->wrappos + ndir * 0.2f;
			line[v*2+0][1] = in[e2]->wrappos - ndir * 0.2f;

			line[v*2+1][1] = line[v*2+0][0];
			line[v*2+1][0] = line[v*2+0][1];
		}
		///////////////////
		for(int vp=0; vp<3; vp++)
		{
			if(tet->neib[vp] == newp)
				continue;
			if(tet->neib[(vp+1)%3] == newp)
				continue;
			for(int vl=0; vl<3; vl++)
			{
				Vec3f i0;
				if(!LineInterPlane(line[vl*2+0], plane[vp].m_normal, plane[vp].m_d, &i0) //&&
				//	!LineInterPlane(line[vl*2+1], plane[vp].m_normal, plane[vp].m_d, &i0)
					)
					continue;
				Vec3f i1;
				if(!LineInterPlane(line[vl*2+0], plane[(vp+1)%3].m_normal, plane[(vp+1)%3].m_d, &i1) //&&
				//	!LineInterPlane(line[vl*2+1], plane[(vp+1)%3].m_normal, plane[vp].m_d, &i1)
					)
					continue;
				////
				if(PointOnOrBehindPlane(i0, plane[(vp+1)%3].m_normal, plane[(vp+1)%3].m_d))
					continue;
				if(PointOnOrBehindPlane(i1, plane[(vp+0)%3].m_normal, plane[(vp+0)%3].m_d))
					continue;
				////
				if(PointOnOrBehindPlane(i0, plane[(vp+2)%3].m_normal, plane[(vp+2)%3].m_d))
					continue;
				if(PointOnOrBehindPlane(i1, plane[(vp+2)%3].m_normal, plane[(vp+2)%3].m_d))
					continue;
				float indot = 
					Dot( Normalize(i0 - in[vl]->wrappos), Normalize( in[(vl+1)%3]->wrappos - in[vl]->wrappos ) );
				float outdot = 
					Dot( Normalize(i1 - in[vl]->wrappos), Normalize( in[(vl+1)%3]->wrappos - in[vl]->wrappos ) );
				char mm[123];
				sprintf(mm, "\r\ninov\r\np1:(%f,%f,%f),%f\r\np2:(%f,%f,%f),%f\r\nl1:(%f,%f,%f)\r\nl2:(%f,%f,%f)\r\ni0:(%f,%f,%f)\r\ni1:(%f,%f,%f)",
					plane[vp].m_normal.x,
					plane[vp].m_normal.y,
					plane[vp].m_normal.z,
					plane[vp].m_d,
					plane[(vp+1)%3].m_normal.x,
					plane[(vp+1)%3].m_normal.y,
					plane[(vp+1)%3].m_normal.z,
					plane[(vp+1)%3].m_d,
					line[vl*2+0][0].x,
					line[vl*2+0][0].y,
					line[vl*2+0][0].z,
					line[vl*2+0][1].x,
					line[vl*2+0][1].y,
					line[vl*2+0][1].z,
					i0.x,
					i0.y,
					i0.z,
					i1.x,
					i1.y,
					i1.z
					);
				ErrMess(mm,mm);
			}
		}
next:
		;
	}
}

void GetClipNeib(Tet *in, std::list<Tet*> *neibclip,
				 Vec3f *fulltri, Plane3f split, Plane3f backsplit, Surf *surf)
{
	for(int vi=0; vi<4; ++vi)
	{
		SurfPt *sp = in->neib[vi];

		if(!sp)
			continue;

		for(std::list<Tet*>::iterator nit=surf->tets2.begin();
			nit!=surf->tets2.end();
			++nit)
		{
			Tet *nitt = *nit;

			if(nitt == in)
				continue;

			for(int vi2=0; vi2<4; ++vi2)
			{
				SurfPt *sp2 = nitt->neib[vi2];

				if(!sp2)
					continue;

				if(Magnitude(sp2->pos - sp->pos) <= CLOSEPOSF)
					goto haveclose;
			}

			continue;

haveclose:

			Vec3f tri1[3];
			tri1[0] = nitt->neib[0]->pos;
			tri1[1] = nitt->neib[1]->pos;
			tri1[2] = nitt->neib[2]->pos;

			if(nitt->neib[0]->gone)
				ErrMess("sdf","gg0");
			if(nitt->neib[1]->gone)
				ErrMess("sdf","gg1");
			if(nitt->neib[2]->gone)
				ErrMess("sdf","gg2");
			
			if(!nitt->neib[0])
				ErrMess("sdf","gg0a");
			if(!nitt->neib[1])
				ErrMess("sdf","gg1a");
			if(!nitt->neib[2])
				ErrMess("sdf","gg2a");

			if(!CanClip(surf,
				nitt,
				split,
				tri1,
				fulltri))
				continue;
			if(!CanClip(surf,
				nitt,
				backsplit,
				tri1,
				fulltri))
				continue;

			neibclip->push_back(nitt);
			RemDupTet(neibclip);
			//neibclip->unique(UniqueTet);
		}
	}
}

/*
For removing hidden triangles
For each "approved" triangle (where a ray emerging from the front side doesn't touch anything)
draw rays from its back side (ONLY) to every other possible triangle.
If it hits that other target triangle first, and if 1) it is its front side, REMOVE that other triangle 
(it is inside), but if 2) it is its back side, we don't know enough to make a decision.

Also can do reverse: from each starting triangle, send ray forwad, and for first match, if hit behind,
go to next front ray intersection, until reaching last triangle. If only back sides were hit, the original
triangle is removed.

Make sure to remove free-floating, detatched triangles before running this test.
But before JoinPts, so each triangle can be removed indepedently. EDIT: nvm, FreeTet probably checks 
to make sure pt's have no more owners before deleting them.
*/
bool RemHid2(Surf *surf, bool checkring)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		tit++)
	{
		Tet *tet = *tit;

		tet->approved = false;
		tet->hidden = false;
	}

	int irem = 0;
	int irem2 = 0;

again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		tit++)
	{
		Tet *tet = *tit;

		Vec3f tri[3];

		tri[0] = tet->neib[0]->pos;
		tri[1] = tet->neib[1]->pos;
		tri[2] = tet->neib[2]->pos;

		Vec3f n = Normal(tri);

		Vec3f line[3][2][2];	//vert line,reverse order,line point,
		////////////////
		line[0][0][0] = (tri[0]+tri[1]+tri[2])/3.0f + n*0.2f;
		line[0][0][1] = line[0][0][0] + n*30000;
		
		line[0][1][1] = (tri[0]+tri[1]+tri[2])/3.0f  + n*0.2f;
		line[0][1][0] = line[0][1][1] + n*30000;
		/////////////////
		line[1][0][0] = (tri[1])/1.0f + n*0.2f;
		line[1][0][1] = line[1][0][0] + n*30000;
		
		line[1][1][1] = (tri[1])/1.0f + n*0.2f;
		line[1][1][0] = line[1][1][1] + n*30000;
		/////////////////
		line[2][0][0] = (tri[2])/1.0f + n*0.2f;
		line[2][0][1] = line[2][0][0] + n*30000;

		line[2][1][1] = (tri[2])/1.0f + n*0.2f;
		line[2][1][0] = line[2][1][1] + n*30000;
		////////////

		LoadedTex *tex, *texn, *texs;
		Vec2f texc;
		Vec3f wp, rp, rn;
		Tet *rtet;
		double fU,fV;

		if( (!TraceRay4(surf,
			line[0][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV) &&
		!TraceRay4(surf,
			line[0][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV))
#if 0
			
			||
		(!TraceRay2(surf,
			line[1][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV) &&
		!TraceRay2(surf,
			line[1][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV)) ||
		(!TraceRay2(surf,
			line[2][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV) &&
		!TraceRay2(surf,
			line[2][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV))
#endif
			)
		{
			tet->approved = true;
		//	fprintf(g_applog, "app'd\r\n");
		//	fflush(g_applog);
		}
		else
		{
			tet->approved = false;
			//fprintf(g_applog, "napp'd\r\n");
		//	fflush(g_applog);
		}
	}

		//	fprintf(g_applog, "p0'd\r\n");
		//	fflush(g_applog);

	//remove
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		tit++)
	{
		Tet *tet = *tit;
	
		if(!tet->approved)
			continue;

		if(checkring && tet->ring < 0)
			continue;

		if(tet->gone)
			ErrMess("gg","gg");
		
		//	fprintf(g_applog, "p00'd\r\n");
			//fflush(g_applog);
		
		Vec3f tri[3];
		tri[0] = tet->neib[0]->pos;
		tri[1] = tet->neib[1]->pos;
		tri[2] = tet->neib[2]->pos;
		Vec3f n = Normal(tri);

		for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
			tit2!=surf->tets2.end();
			tit2++)
		{
			Tet *tet2 = *tit2;

			if(tet2 == tet)
				continue;

			if(tet2->approved)
				continue;

		//	fprintf(g_applog, "p000'd\r\n");
			//fflush(g_applog);

			Vec3f tri2[3];
			tri2[0] = tet2->neib[0]->pos;
			tri2[1] = tet2->neib[1]->pos;
			tri2[2] = tet2->neib[2]->pos;
			Vec3f n2 = Normal(tri2);

			//from back side of tet, through tet2 if possible
			Vec3f line[2];
			line[0] = (tri[0]+tri[1]+tri[2])/3.0f - n * 1.25f;
			line[1] = (tri2[0]+tri2[1]+tri2[2])/3.0f - n2 * 10.25f;

		//	fprintf(g_applog, "p1'd\r\n");
			//fflush(g_applog);
			//is this line going FORWARD through tet1? Must go behind.
			if(Dot( Normalize(line[1]-line[0]) ,n) >= 0)
				continue;

		//	fprintf(g_applog, "p2'd\r\n");
			//fflush(g_applog);
			//is this line approaching from the FRONT of tet2? Must go from front.
			if(Dot( Normalize(line[1]-line[0]) ,n2) >= 0)
				continue;

		//	fprintf(g_applog, "p3'd\r\n");
			//fflush(g_applog);
			LoadedTex *tex, *texn, *texs;
			Vec2f texc;
			Vec3f wp, rp, rn;
			Tet *rtet;
			double fU,fV;

			if(TraceRay4(surf,
				line,
				&tex,
				&texs,
				&texn,
				&texc,
				&wp,
				&rp,
				&rn,
				&rtet,
				&fU, &fV))
			{
		//	fprintf(g_applog, "p4'd\r\n");
			//fflush(g_applog);
				//tet->approved = true;
				if(rtet == tet2)
				{
		//	fprintf(g_applog, "p5'd\r\n");
			//fflush(g_applog);
					FreeTet(surf, tet2, true);
					surf->tets2.erase(tit2);
					irem++;
					goto again;
				}
				else if(rtet != tet2)
				{
		//	fprintf(g_applog, "p6'd\r\n");
		//	fflush(g_applog);
					//something in between, can't determine
				}
			}
			else
			{
		//	fprintf(g_applog, "p7'd\r\n");
		//	fflush(g_applog);
				//no collision? something is wrong.
			}
		}
	}

	fprintf(g_applog, "RemHid2: %d removed\r\n\r\n", irem);
	return true;
}

void RemDupTet(std::list<Tet*>* tets)
{
	for(std::list<Tet*>::iterator tit1=tets->begin();
		tit1!=tets->end();
		tit1++)
	{
again:
		std::list<Tet*>::iterator tit2 = tit1;
		tit2++;

		for(; tit2!=tets->end(); ++tit2)
		{
			if(*tit1 == *tit2)
			{
				tit1 = tets->erase(tit1);
				goto again;
			}
		}
	}
}

/*
Use the full, unclipped mesh
to cut approved non-hidden
triangles into parts, and then
test the clipped parts for approve
or hidden.
*/
bool ClipTris(Surf *surf, Surf *fullsurf)
{	
again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		//if(!VerifyTet(surf, tit))
		//	goto again;

		Tet *in = *tit;
		//for(std::list<Tet*>::iterator fulltit=fullsurf->tets2.begin();
		//	fulltit!=fullsurf->tets2.end();
		//	++fulltit)

		if(TestDiscard(surf, tit))
			goto again;

		Vec3f tri1[3];
		tri1[0] = (*tit)->neib[0]->pos;
		tri1[1] = (*tit)->neib[1]->pos;
		tri1[2] = (*tit)->neib[2]->pos;
		Vec3f norm1 = Normal(tri1);


		
		for(std::list<Tet*>::iterator fulltit=fullsurf->tets2.begin();
			fulltit!=fullsurf->tets2.end();
			++fulltit)
		{
			Tet *fullin = *fulltit;
			//if(!VerifyTet(surf, fulltit))
			//	goto again;

			if(TestDiscard(surf, fulltit))
				goto again;

			//if(*fulltit == *tit)
			if(*fulltit == in)
				continue;

			int same = 0;

			for(int v1=0; v1<4; v1++)
			{
				if(!in->neib[v1])
					continue;

				for(int v2=0; v2<4; v2++)
				{
					if(!fullin->neib[v2])
						continue;

					if(Magnitude( in->neib[v1]->pos - fullin->neib[v2]->pos ) <= CLOSEPOSF )
						same++;
				}
			}

			if(same>=3)
				continue;

			Vec3f fulltri[3];
			fulltri[0] = (*fulltit)->neib[0]->pos;
			fulltri[1] = (*fulltit)->neib[1]->pos;
			fulltri[2] = (*fulltit)->neib[2]->pos;
			Vec3f fullnorm = Normal(fulltri);

			if(fullnorm == norm1)
				continue;

			if( (Vec3f(0,0,0)-fullnorm) == norm1 )
				continue;

			Plane3f split;
			MakePlane(&split.m_normal, &split.m_d, (fulltri[0]+fulltri[1]+fulltri[2])/3.0f,
				fullnorm);

			Plane3f backsplit;
			MakePlane(&backsplit.m_normal, &backsplit.m_d, 
				(fulltri[0]+fulltri[1]+fulltri[2])/3.0f,
				Vec3f(0,0,0) - fullnorm);

#if 00
			tri1[0] = in->neib[0]->pos;
			tri1[1] = in->neib[1]->pos;
			tri1[2] = in->neib[2]->pos;

			if(!CanClip(surf,
				in,
				split,
				tri1,
				fulltri))
				continue;

			Tet *front = NULL;
			Tet *back = NULL;

			std::list<SurfPt*> substfrom;
			std::list<SurfPt*> substto;

			ClipFront(surf, in, split,
				&substfrom, &substto, &front);
			ClipFront(surf, in, backsplit,
				&substfrom, &substto, &back);

			SplitQuad(surf, front,
				&substfrom, &substto);
			SplitQuad(surf, back,
				&substfrom, &substto);

			FreeTet(surf, in, true);
			//surf->tets2.erase(tit);	//tit invalided by CopyPt etc.

			tit=surf->tets2.begin();
			while(
				tit!=surf->tets2.end())
			{
				if(*tit == in)
				{
					tit=surf->tets2.erase(tit);
					//break;
				}
				else
					++tit;
			}
			goto again;
#endif
#if 01
			if(!ShouldClip(surf,
				in,
				split,
				tri1,
				fulltri))
				continue;
			if(!CanClip(surf,
				in,
				split,
				tri1,
				fulltri))
				continue;
/*
			std::list<Tet*> neibclip;

			GetClipNeib(in, &neibclip,
				fulltri, split, backsplit,
				surf);
*/
			std::list<SurfPt*> substfrom;
			std::list<SurfPt*> substto;

			Tet *front=NULL, *back=NULL;

			
	if(WeirdTest(surf, in))
		ErrMess("w","w ct i");

			ClipFront(surf, in, split,
				&substfrom, &substto, &front);

			
	if(WeirdTest(surf, front))
	{
		char mm[1234];
		sprintf(mm,"\r\n w ct f \r\n"\
			"in[0]=%f,%f,%f \r\n"\
			"in[1]=%f,%f,%f \r\n"\
			"in[2]=%f,%f,%f \r\n"\
			"f[0]=%f,%f,%f \r\n"\
			"f[1]=%f,%f,%f \r\n"\
			"f[2]=%f,%f,%f \r\n",
			in->neib[0]->pos.x,
			in->neib[0]->pos.y,
			in->neib[0]->pos.z,
			
			in->neib[1]->pos.x,
			in->neib[1]->pos.y,
			in->neib[1]->pos.z,
			
			in->neib[2]->pos.x,
			in->neib[2]->pos.y,
			in->neib[2]->pos.z,
			
			front->neib[0]->pos.x,
			front->neib[0]->pos.y,
			front->neib[0]->pos.z,
			
			front->neib[1]->pos.x,
			front->neib[1]->pos.y,
			front->neib[1]->pos.z,
			
			front->neib[2]->pos.x,
			front->neib[2]->pos.y,
			front->neib[2]->pos.z);
		ErrMess("w",mm);
	}

			ClipFront(surf, in, backsplit,
				&substfrom, &substto, &back);
			
	if(WeirdTest(surf, back))
	{
		char mm[1234];
		sprintf(mm,"\r\n w ct b \r\n"\
			"in[0]=%f,%f,%f \r\n"\
			"in[1]=%f,%f,%f \r\n"\
			"in[2]=%f,%f,%f \r\n"\
			"f[0]=%f,%f,%f \r\n"\
			"f[1]=%f,%f,%f \r\n"\
			"f[2]=%f,%f,%f \r\n",
			in->neib[0]->pos.x,
			in->neib[0]->pos.y,
			in->neib[0]->pos.z,
			
			in->neib[1]->pos.x,
			in->neib[1]->pos.y,
			in->neib[1]->pos.z,
			
			in->neib[2]->pos.x,
			in->neib[2]->pos.y,
			in->neib[2]->pos.z,
			
			front->neib[0]->pos.x,
			front->neib[0]->pos.y,
			front->neib[0]->pos.z,
			
			front->neib[1]->pos.x,
			front->neib[1]->pos.y,
			front->neib[1]->pos.z,
			
			front->neib[2]->pos.x,
			front->neib[2]->pos.y,
			front->neib[2]->pos.z);
		ErrMess("w",mm);
	}

			SplitQuad(surf, front,
				&substfrom, &substto);
			SplitQuad(surf, back,
				&substfrom, &substto);

			//if (newtet->numpoints > maxpts)
			//	Error ("ClipWinding: points exceeded estimate");

			//for all holders of old pt's in "in"
			//replace each pt to new ones

			// free the original winding
			//FreeWinding (in);
			FreeTet(surf, in, true);
			//surf->tets2.erase(tit);	//tit invalided by CopyPt etc.

			tit=surf->tets2.begin();
			while(
				tit!=surf->tets2.end())
			{
				if(*tit == in)
				{
					tit=surf->tets2.erase(tit);
					//break;
				}
				else
					++tit;
			}

#if 0
			RemDupTet(&neibclip);
			//neibclip.unique(UniqueTet);

			std::list<Tet*>::iterator cit=neibclip.begin();

			
			while(
				cit!=neibclip.end() )
			{
				Tet *ctit = *cit;

				
			std::list<Tet*>::iterator cit2=cit;
				cit2++;
				
				while(
					cit2!=neibclip.end() )
				{
					Tet *ctit2 = *cit2;

					if(ctit2 == ctit)
						ErrMess("rep","rep");

					cit2++;
				}

				//?
				if(!ctit->neib[0])
					ErrMess("sdf","gg0raa");
				//	continue;
				if(!ctit->neib[1])
					ErrMess("sdf","gg1raa");
				//	continue;
				if(!ctit->neib[2])
					ErrMess("sdf","gg2raa");

				++cit;
			}

			cit=neibclip.begin();


			while(
				cit!=neibclip.end() )
			{
				Tet *ctit = *cit;

				//?
				if(!ctit->neib[0])
					ErrMess("sdf","gg0ra");
				//	continue;
				if(!ctit->neib[1])
					ErrMess("sdf","gg1ra");
				//	continue;
				if(!ctit->neib[2])
					ErrMess("sdf","gg2ra");
				//	continue;
				if(ctit->neib[3])
					ErrMess("sdf","gg3ra");

			if(ctit->neib[0]->gone)
				ErrMess("sdf","gg0r");
			if(ctit->neib[1]->gone)
				ErrMess("sdf","gg1r");
			if(ctit->neib[2]->gone)
				ErrMess("sdf","gg2r");

				tri1[0] = ctit->neib[0]->pos;
				tri1[1] = ctit->neib[1]->pos;
				tri1[2] = ctit->neib[2]->pos;

				///*
				if(!CanClip(surf,
					ctit,
					split,
					tri1,
					fulltri))
					goto next;
//*/
				front = NULL;
				back = NULL;

				ClipFront(surf, ctit, split,
					&substfrom, &substto, &front);
				ClipFront(surf, ctit, backsplit,
					&substfrom, &substto, &back);
					
				SplitQuad(surf, front,
					&substfrom, &substto);
				SplitQuad(surf, back,
					&substfrom, &substto);

				FreeTet(surf, ctit, true);
				//surf->tets2.erase(tit);	//tit invalided by CopyPt etc.

				tit=surf->tets2.begin();
				while(
					tit!=surf->tets2.end())
				{
					if(*tit == ctit)
					{
						tit=surf->tets2.erase(tit);
						//break;
					}
					else
						++tit;
				}

next:
				cit = neibclip.erase(cit);
				//neibclip.unique(UniqueTet);
				RemDupTet(&neibclip);
				cit=neibclip.begin();
			}

			neibclip.clear();
#if 0
			//reverse iterate to get the latest change first?
			//or will there be a chain of changes?
			std::list<SurfPt*>::iterator fromit = substfrom.begin();
			std::list<SurfPt*>::iterator toit = substto.begin();

			while(fromit != substfrom.end() &&
				toit != substto.end())
			{
				RepPt2(surf, *fromit, *toit);

				fromit++;
				toit++;
			}
#endif
			Test(surf);

//			if(fpts<3)
//				ErrMess("f<","f<");
			//if(bpts<3)
			//	ErrMess("b<","b<");

			//std::list<Tet*>::iterator backwtit=surf->tets2.end();
			//backwtit--;
			//std::list<Tet*>::iterator newwtit=backwtit;
			//newwtit--;

			//if(VerifyTet(surf, newwtit))
			///	SplitQuad(surf, newtet);
			////	newtet = NULL;
			//else
			//{
			//	backwtit=surf->tets2.end();
			//	backwtit--;
			//}
			//if(VerifyTet(surf, backwtit))
			///	SplitQuad(surf, backw);

			///	backw = NULL;
#endif
			//return newtet;
			goto again;
#endif
	//return true;
			//goto again;
		}
	}

	Test(surf);

	return true;
}

bool UniqueTet(Tet *a, Tet *b)
{
	return (a==b);
}

//1gets2
void AddHolds(SurfPt *topt, SurfPt *frompt)
{	if(topt == frompt)
		return;

	for(std::list<Tet*>::iterator hit=frompt->holder.begin();
		hit!=frompt->holder.end();
		hit++)
	{
#if 0
		for(std::list<Tet*>::iterator tohit=topt->holder.begin();
			tohit!=topt->holder.end();
			++tohit)
		{
			if(*hit == *tohit)
				goto next;
		}
#endif
		topt->holder.push_back(*hit);
		//topt->holder.unique(UniqueTet);
		RemDupTet(&topt->holder);
next:
		;
	}
return;
	for(std::list<Tet*>::iterator remhit=frompt->holder.begin();
		remhit!=frompt->holder.end();
		++remhit)
	{
		//fromdel2
//1gets2hn
		for(int vin=0; vin<4; ++vin)
		{
			if((*remhit)->neib[vin] == frompt)
				(*remhit)->neib[vin] = topt;
		}
	}
}

void MigratePt(SurfPt* from, SurfPt *topt)
{
	for(std::list<Tet*>::iterator remhit=from->holder.begin();
		remhit!=from->holder.end();
		++remhit)
	{
		for(int vin=0; vin<4; ++vin)
		{
			if((*remhit)->neib[vin] == from)
				(*remhit)->neib[vin] = topt;
		}
	}
}

/*
First each triangle with vertices was created
with duplications of vertices for ease of 
cutting triangles.
Join all vertices that are close to each other
now.
*/
bool JoinPts(Surf *surf, Surf *fullsurf)
{//return 1;
	{
		int c = 0;
		std::list<Tet*>::iterator tit1=surf->tets2.begin();
		for(;
			tit1!=surf->tets2.end();
			++tit1)
		{
			
			for(int vin1=0; vin1<3; ++vin1)
			{
#if 0
		char mm[1234];
		sprintf(mm, "at (%f,%f,%f)", 
			(*tit1)->neib[vin1]->pos.x,
			(*tit1)->neib[vin1]->pos.y,
			(*tit1)->neib[vin1]->pos.z);
		ErrMess(mm,mm);
#endif

				if( Magnitude( (*tit1)->neib[vin1]->pos - Vec3f(-150,-150,400) ) <= CLOSEPOSF )
					c++;
			}
		}
#if 0
		char mm[1234];
		sprintf(mm, "at (-150,-150,400) c=%d", c);
		ErrMess(mm,mm);
#endif
	}


	int i1=0;
again:
	std::list<Tet*>::iterator tit1=surf->tets2.begin();

	for(int i11=0; i11<i1 && tit1!=surf->tets2.end(); i11++)
		tit1++;

	Test(surf);
	for(;
		tit1!=surf->tets2.end();
		++tit1, ++i1)
	{
		if(TestDiscard(surf, tit1))
			goto again;

		int i2=i1+1;
		std::list<Tet*>::iterator tit2;
		tit2=tit1;
		++tit2;
		for(;
			tit2!=surf->tets2.end();
			++tit2, ++i2)
		{
			if(*tit2 == *tit1)
				continue;

			if(TestDiscard(surf, tit2))
				goto again;

			////fprintf(g_applog, "\r\ni%d,%d\r\n", i1,i2);
			//fflush(g_applog);

			for(int vin1=0; vin1<3; ++vin1)
			{
				if(!(*tit1)->neib[vin1])
					ErrMess("1!","1!");
				if((*tit1)->neib[vin1]->gone)
					ErrMess("g123123","tt11");

				for(int vin2=0; vin2<3; vin2++)
				{
					if(!(*tit2)->neib[vin2])
						ErrMess("12!","12!");
					if((*tit2)->neib[vin2]->gone)
						ErrMess("g123123","tt2");

					if(Magnitude( (*tit1)->neib[vin1]->pos - (*tit2)->neib[vin2]->pos ) <= CLOSEPOSF &&
						(*tit1)->neib[vin1] != (*tit2)->neib[vin2] &&
						*tit1 != *tit2 )
					{

#if 0
						fprintf(g_applog, "\r\nt\nvin%d,%d:(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n",
							vin1,vin2,
							(*tit1)->neib[vin1]->pos.x,
							(*tit1)->neib[vin1]->pos.y,
							(*tit1)->neib[vin1]->pos.z,
							(*tit2)->neib[vin2]->pos.x,
							(*tit2)->neib[vin2]->pos.y,
							(*tit2)->neib[vin2]->pos.z,
							
							(*tit1)->neib[(vin1+1)%3]->pos.x,
							(*tit1)->neib[(vin1+1)%3]->pos.y,
							(*tit1)->neib[(vin1+1)%3]->pos.z,
							(*tit2)->neib[(vin2+1)%3]->pos.x,
							(*tit2)->neib[(vin2+1)%3]->pos.y,
							(*tit2)->neib[(vin2+1)%3]->pos.z,
							
							(*tit1)->neib[(vin1+2)%3]->pos.x,
							(*tit1)->neib[(vin1+2)%3]->pos.y,
							(*tit1)->neib[(vin1+2)%3]->pos.z,
							(*tit2)->neib[(vin2+2)%3]->pos.x,
							(*tit2)->neib[(vin2+2)%3]->pos.y,
							(*tit2)->neib[(vin2+2)%3]->pos.z);
						fflush(g_applog);
#endif

						if((*tit1)->neib[vin1]->gone)
							ErrMess("g13","11111g");
						//delete (*tit2)->neib[vin2];
#if 0
						if((*tit2)->neib[vin2]->holder.size() <= 1)
						{
							(*tit2)->neib[vin2]->gone=true;
							delete (*tit2)->neib[vin2];
							(*tit2)->neib[vin2]=NULL;
						}
#endif
						SurfPt* oldpt = (*tit2)->neib[vin2];

						SurfPt *topt = (*tit1)->neib[vin1];
						SurfPt *frompt = (*tit2)->neib[vin2];

						AddHolds((*tit1)->neib[vin1],
							(*tit2)->neib[vin2]);

#if 01
						for(std::list<Tet*>::iterator remhit=frompt->holder.begin();
								remhit!=frompt->holder.end();
								++remhit)
							{
								//fromdel2
						//1gets2hn
								for(int vin=0; vin<4; ++vin)
								{
									if((*remhit)->neib[vin] == frompt)
										(*remhit)->neib[vin] = topt;
								}
							}
#endif
#if 01
						RepPt2(surf, frompt, topt);
#endif
						oldpt->gone=true;
						delete oldpt;

						topt->holder.push_back(*tit2);
						//topt->holder.unique(UniqueTet);
						RemDupTet(&topt->holder);
//1gets2

						//MigratePt((*tit2)->neib[vin2],
						//	(*tit1)->neib[vin1]);
						if((*tit2)->neib[vin2]->gone)
							ErrMess("g13","g13");
						//(*tit2)->neib[vin2]->gone=true;
//1gets2
						//delete (*tit2)->neib[vin2];
						if(*tit1==*tit2)
							ErrMess("t12","t12=");
#if 0
						if((*tit1)->neib[vin1]==(*tit2)->neib[vin2])
							ErrMess("t12","t12=tn");
						if((*tit1)->neib[vin1]->gone)
							ErrMess("g13","111g");
#endif

						std::list<SurfPt*>::iterator pit=surf->pts2.begin();
						while(
							pit!=surf->pts2.end())
						{
							//if(*pit == (*tit2)->neib[vin2])
							if(*pit == frompt)
								pit = surf->pts2.erase(pit);
							else
								++pit;
						}

						//(*tit2)->neib[vin2] = (*tit1)->neib[vin1];
						(*tit2)->neib[vin2] = topt;
						if((*tit2)->neib[vin2]->gone)
							ErrMess("g123123","tt2tt");
						goto again;
					}
				}
			}
		}
	}
	Test(surf);

	//check join
	if(false)
	{

		tit1=surf->tets2.begin();
		i1=0;

		for(int i11=0; i11<i1 && tit1!=surf->tets2.end(); i11++)
			tit1++;

		Test(surf);
		for(;
			tit1!=surf->tets2.end();
			++tit1, ++i1)
		{
			if(TestDiscard(surf, tit1))
				goto again;

			int i2=i1+1;
			std::list<Tet*>::iterator tit2;
			tit2=tit1;
			++tit2;
			for(;
				tit2!=surf->tets2.end();
				++tit2, ++i2)
			{
				if(*tit2 == *tit1)
					continue;

				if(TestDiscard(surf, tit2))
					goto again;

				////fprintf(g_applog, "\r\ni%d,%d\r\n", i1,i2);
				//fflush(g_applog);

				for(int vin1=0; vin1<3; ++vin1)
				{
					if(!(*tit1)->neib[vin1])
						ErrMess("1!","1!");
					if((*tit1)->neib[vin1]->gone)
						ErrMess("g123123","tt11");

					for(int vin2=0; vin2<3; vin2++)
					{
						if(!(*tit2)->neib[vin2])
							ErrMess("12!","12!");
						if((*tit2)->neib[vin2]->gone)
							ErrMess("g123123","tt2");

						if(Magnitude( (*tit1)->neib[vin1]->pos - (*tit2)->neib[vin2]->pos ) <= CLOSEPOSF &&
							(*tit1)->neib[vin1] != (*tit2)->neib[vin2] &&
							*tit1 != *tit2 )
						{
							ErrMess("mj","MJ");

	#if 0
							fprintf(g_applog, "\r\nt\nvin%d,%d:(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n",
								vin1,vin2,
								(*tit1)->neib[vin1]->pos.x,
								(*tit1)->neib[vin1]->pos.y,
								(*tit1)->neib[vin1]->pos.z,
								(*tit2)->neib[vin2]->pos.x,
								(*tit2)->neib[vin2]->pos.y,
								(*tit2)->neib[vin2]->pos.z,
								
								(*tit1)->neib[(vin1+1)%3]->pos.x,
								(*tit1)->neib[(vin1+1)%3]->pos.y,
								(*tit1)->neib[(vin1+1)%3]->pos.z,
								(*tit2)->neib[(vin2+1)%3]->pos.x,
								(*tit2)->neib[(vin2+1)%3]->pos.y,
								(*tit2)->neib[(vin2+1)%3]->pos.z,
								
								(*tit1)->neib[(vin1+2)%3]->pos.x,
								(*tit1)->neib[(vin1+2)%3]->pos.y,
								(*tit1)->neib[(vin1+2)%3]->pos.z,
								(*tit2)->neib[(vin2+2)%3]->pos.x,
								(*tit2)->neib[(vin2+2)%3]->pos.y,
								(*tit2)->neib[(vin2+2)%3]->pos.z);
							fflush(g_applog);
	#endif

							if((*tit1)->neib[vin1]->gone)
								ErrMess("g13","11111g");
							//delete (*tit2)->neib[vin2];
	#if 0
							if((*tit2)->neib[vin2]->holder.size() <= 1)
							{
								(*tit2)->neib[vin2]->gone=true;
								delete (*tit2)->neib[vin2];
								(*tit2)->neib[vin2]=NULL;
							}
	#endif
							SurfPt* oldpt = (*tit2)->neib[vin2];

							SurfPt *topt = (*tit1)->neib[vin1];
							SurfPt *frompt = (*tit2)->neib[vin2];

							AddHolds((*tit1)->neib[vin1],
								(*tit2)->neib[vin2]);

	#if 01
							for(std::list<Tet*>::iterator remhit=frompt->holder.begin();
									remhit!=frompt->holder.end();
									++remhit)
								{
									//fromdel2
							//1gets2hn
									for(int vin=0; vin<4; ++vin)
									{
										if((*remhit)->neib[vin] == frompt)
											(*remhit)->neib[vin] = topt;
									}
								}
	#endif
	#if 01
							RepPt2(surf, frompt, topt);
	#endif
							oldpt->gone=true;
							delete oldpt;

							topt->holder.push_back(*tit2);
							//topt->holder.unique(UniqueTet);
							RemDupTet(&topt->holder);
	//1gets2

							//MigratePt((*tit2)->neib[vin2],
							//	(*tit1)->neib[vin1]);
							if((*tit2)->neib[vin2]->gone)
								ErrMess("g13","g13");
							//(*tit2)->neib[vin2]->gone=true;
	//1gets2
							//delete (*tit2)->neib[vin2];
							if(*tit1==*tit2)
								ErrMess("t12","t12=");
	#if 0
							if((*tit1)->neib[vin1]==(*tit2)->neib[vin2])
								ErrMess("t12","t12=tn");
							if((*tit1)->neib[vin1]->gone)
								ErrMess("g13","111g");
	#endif

							std::list<SurfPt*>::iterator pit=surf->pts2.begin();
							while(
								pit!=surf->pts2.end())
							{
								//if(*pit == (*tit2)->neib[vin2])
								if(*pit == frompt)
									pit = surf->pts2.erase(pit);
								else
									++pit;
							}

							//(*tit2)->neib[vin2] = (*tit1)->neib[vin1];
							(*tit2)->neib[vin2] = topt;
							if((*tit2)->neib[vin2]->gone)
								ErrMess("g123123","tt2tt");
							goto again;
						}
					}
				}
			}
		}
	}

	return true;
}




bool JoinPts2(Surf *surf, Surf *fullsurf)
{//return 1;
again:
	Test(surf);
	int i1=0;
	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1, ++i1)
	{
		if(TestDiscard(surf, tit1))
			goto again;

		int i2=i1+1;
		std::list<Tet*>::iterator tit2;
		tit2=tit1;
		++tit2;
		for(;
			tit2!=surf->tets2.end();
			++tit2, ++i2)
		{
			if(*tit2 == *tit1)
				continue;

			if(TestDiscard(surf, tit2))
				goto again;

			////fprintf(g_applog, "\r\ni%d,%d\r\n", i1,i2);
			//fflush(g_applog);

			for(int vin1=0; vin1<3; ++vin1)
			{
				if(!(*tit1)->neib[vin1])
					ErrMess("1!","1!");
				if((*tit1)->neib[vin1]->gone)
					ErrMess("g123123","tt11");

				for(int vin2=0; vin2<3; vin2++)
				{
					if(!(*tit2)->neib[vin2])
						ErrMess("12!","12!");
					if((*tit2)->neib[vin2]->gone)
						ErrMess("g123123","tt2");

					if(Magnitude( (*tit1)->neib[vin1]->pos - (*tit2)->neib[vin2]->pos ) <= CLOSEPOSF &&
						(*tit1)->neib[vin1] != (*tit2)->neib[vin2] &&
						*tit1 != *tit2 )
					{
#if 0
						fprintf(g_applog, "\r\nt\nvin%d,%d:(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n(%f,%f,%f),(%f,%f,%f)\r\n",
							vin1,vin2,
							(*tit1)->neib[vin1]->pos.x,
							(*tit1)->neib[vin1]->pos.y,
							(*tit1)->neib[vin1]->pos.z,
							(*tit2)->neib[vin2]->pos.x,
							(*tit2)->neib[vin2]->pos.y,
							(*tit2)->neib[vin2]->pos.z,
							
							(*tit1)->neib[(vin1+1)%3]->pos.x,
							(*tit1)->neib[(vin1+1)%3]->pos.y,
							(*tit1)->neib[(vin1+1)%3]->pos.z,
							(*tit2)->neib[(vin2+1)%3]->pos.x,
							(*tit2)->neib[(vin2+1)%3]->pos.y,
							(*tit2)->neib[(vin2+1)%3]->pos.z,
							
							(*tit1)->neib[(vin1+2)%3]->pos.x,
							(*tit1)->neib[(vin1+2)%3]->pos.y,
							(*tit1)->neib[(vin1+2)%3]->pos.z,
							(*tit2)->neib[(vin2+2)%3]->pos.x,
							(*tit2)->neib[(vin2+2)%3]->pos.y,
							(*tit2)->neib[(vin2+2)%3]->pos.z);
						fflush(g_applog);
#endif

						if((*tit1)->neib[vin1]->gone)
							ErrMess("g13","11111g");
						//delete (*tit2)->neib[vin2];
#if 0
						if((*tit2)->neib[vin2]->holder.size() <= 1)
						{
							(*tit2)->neib[vin2]->gone=true;
							delete (*tit2)->neib[vin2];
							(*tit2)->neib[vin2]=NULL;
						}
#endif
						SurfPt* oldpt = (*tit2)->neib[vin2];

						SurfPt *topt = (*tit1)->neib[vin1];
						SurfPt *frompt = (*tit2)->neib[vin2];

						AddHolds((*tit1)->neib[vin1],
							(*tit2)->neib[vin2]);

#if 01
						for(std::list<Tet*>::iterator remhit=frompt->holder.begin();
								remhit!=frompt->holder.end();
								++remhit)
							{
								//fromdel2
						//1gets2hn
								for(int vin=0; vin<4; ++vin)
								{
									if((*remhit)->neib[vin] == frompt)
										(*remhit)->neib[vin] = topt;
								}
							}
#endif
#if 01
						RepPt2(surf, frompt, topt);
#endif
						oldpt->gone=true;
						delete oldpt;

						topt->holder.push_back(*tit2);
						//topt->holder.unique(UniqueTet);
						RemDupTet(&topt->holder);
//1gets2

						//MigratePt((*tit2)->neib[vin2],
						//	(*tit1)->neib[vin1]);
						if((*tit2)->neib[vin2]->gone)
							ErrMess("g13","g13");
						//(*tit2)->neib[vin2]->gone=true;
//1gets2
						//delete (*tit2)->neib[vin2];
						if(*tit1==*tit2)
							ErrMess("t12","t12=");
#if 0
						if((*tit1)->neib[vin1]==(*tit2)->neib[vin2])
							ErrMess("t12","t12=tn");
						if((*tit1)->neib[vin1]->gone)
							ErrMess("g13","111g");
#endif

						std::list<SurfPt*>::iterator pit=surf->pts2.begin();
						while(
							pit!=surf->pts2.end())
						{
							//if(*pit == (*tit2)->neib[vin2])
							if(*pit == frompt)
								pit = surf->pts2.erase(pit);
							else
								++pit;
						}

						//(*tit2)->neib[vin2] = (*tit1)->neib[vin1];
						(*tit2)->neib[vin2] = topt;
						if((*tit2)->neib[vin2]->gone)
							ErrMess("g123123","tt2tt");
						goto again;
					}
				}
			}
		}
	}
	Test(surf);

	return true;
}

bool HolderHasPt(SurfPt* pt, Tet *holder)
{
	int c = 0;
	for(int vin=0; vin<4; ++vin)
	{
		if(holder->neib[vin] == pt)
			++c;
	}
	if(c>1)
	{
		ErrMess("asdasd","hp+++!");
		return false;
	}
	if(c)
		return true;
	ErrMess("asdasd","hp!");
	return false;
}

bool PtHasHolder(SurfPt* pt, Tet *holder)
{//neib not getting removed
	for(std::list<Tet*>::iterator tit=pt->holder.begin();
		tit!=pt->holder.end();
		++tit)
	{
		if(*tit == holder)
			goto next;
	}
	ErrMess("asd","pht!");
	return false;
next:
	//no repeats
	//tet
	for(std::list<Tet*>::iterator tit=pt->holder.begin();
		tit!=pt->holder.end();
		++tit)
	{
		std::list<Tet*>::iterator tit2=tit;
		for(tit2++;
			tit2!=pt->holder.end();
			++tit2)
		{
			if(*tit == *tit2)
			{
				ErrMess("asd","pht=pht2");
				return false;
			}
		}
	}
	return true;
}


bool Test(Surf *surf)
{
	return true;
}

bool Test2(Surf *surf)
{
	//return true;
	//return true;
	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		if((*pit)->gone)
			ErrMess("g0","g0");
		if((*pit)->holder.size() == 0)
		{
			ErrMess("asdasd","ph!0");
			return false;
		}
		for(std::list<Tet*>::iterator hit=(*pit)->holder.begin();
			hit!=(*pit)->holder.end();
			++hit)
		{
			//if(PtHasHolder(*pit, *hit))
			//	continue;
			if(HolderHasPt(*pit,*hit))
				continue;
			ErrMess("sadasd","ph!hp");
			return false;
		}
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		for(int vin=0; vin<4; ++vin)
		{
			if(!(*tit)->neib[vin])
				continue;
			if((*tit)->neib[vin]->gone)
				ErrMess("g0","g02");
			if(PtHasHolder((*tit)->neib[vin], *tit))
				continue;
			ErrMess("asdasd","ph!!");
			return false;
		}
	}
	//no repeats
	//pts
	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		std::list<SurfPt*>::iterator pit2=pit;
		for(pit2++;
			pit2!=surf->pts2.end();
			++pit2)
		{
			if(*pit == *pit2)
			{
				ErrMess("asd","p=p2");
				return false;
			}
		}
	}
	//tets
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		std::list<Tet*>::iterator tit2=tit;
		for(tit2++;
			tit2!=surf->tets2.end();
			++tit2)
		{
			if(*tit == *tit2)
			{
				ErrMess("asd","t=t2");
				return false;
			}
		}
	}
	return true;
}

/*
internal func called by RemTet
removes the holder tet from the list in the pt "sp".
does NOT remove the sp from the tet's neibs array.
*/
void SepPt(Tet *tet,
		   SurfPt *sp,
		   std::list<Tet*>* checkremove)
{
#if 0
	//find the pt entry in the neibs of the tet and remove it
	//tet maybe become less than a triangle
	//so add it to list to remove
	for(int vin=0; vin<3; ++vin)
	{
		if(tet->neib[vin] == sp)
		{
			tet->neib[vin] = NULL;
			checkremove->push_back(tet);
		}
	}
#endif	//wrong

	std::list<Tet*>::iterator hit=sp->holder.begin();
	while(hit!=sp->holder.end())
	{
		if(*hit == tet) 
			hit = sp->holder.erase(hit);
		else
			++hit;
	}
	if(sp->gone)
		ErrMess("g123123","g123123sss");
}

/*
pt [vin1] of tet tit found to have less than 3
owners, so remove the tet.
the pt will remain in the other owners/holders,
but they must be checked to have at least 3 pt's.
*/
void RemTet(Surf *surf,
		   std::list<Tet*>::iterator tit,
		   int vin1)
{
	//check these tets to see if they have less than 3 verts, and remove them
	//std::list<Tet*> checkremove;

	for(int vin=0; vin<4; ++vin)
	{
		//for(std::list<Tet*>::iterator hit=(*tit)->neib[vin]->holder.begin();
		//	hit!=(*tit)->neib[vin]->holder.end();
		//	++hit)
		//{
		//	if(*hit == *tit)
		//		continue;	//already going to remove this
			//remove pt from other holders
		if(!(*tit)->neib[vin])
			continue;

			SepPt(*tit, (*tit)->neib[vin], NULL);
		//}
				
		if((*tit)->neib[vin]->holder.size() == 0)
		{
			if((*tit)->neib[vin]->gone)
				ErrMess("g14","g14");
			(*tit)->neib[vin]->gone=true;
			delete (*tit)->neib[vin];

			std::list<SurfPt*>::iterator pit=surf->pts2.begin();
			while(
				pit!=surf->pts2.end())
			{
				if(*pit == (*tit)->neib[vin])
					pit = surf->pts2.erase(pit);
				else
					++pit;
			}
		}

		(*tit)->neib[vin] = NULL;
	}

	Tet *tet = *tit;
	FreeTet(surf, *tit, true);
	//surf->tets2.erase(tit);
	tit=surf->tets2.begin();
	while(tit!=surf->tets2.end())
	{
		if(*tit == tet)
			tit=surf->tets2.erase(tit);
		else
			++tit;
	}
}

/*
this fun musn't use NULL neibs in the tet.
it is assumed that neib[vin] is NULL.
*/
void RemTet2(Surf *surf,
		   std::list<Tet*>::iterator tit)
{
	//check these tets to see if they have less than 3 verts, and remove them
	//std::list<Tet*> checkremove;

	for(int vin=0; vin<4; ++vin)
	{
		//if(!(*tit)->neib[vin])
		//	contine;

		//for(std::list<Tet*>::iterator hit=(*tit)->neib[vin]->holder.begin();
		//	hit!=(*tit)->neib[vin]->holder.end();
		//	++hit)
		//{
		//	if(*hit == *tit && )
		//		continue;	//already going to remove this
			//remove pt from other holders
		if(!(*tit)->neib[vin])
			continue;

			SepPt(*tit, (*tit)->neib[vin], NULL);
		//}

		if((*tit)->neib[vin]->holder.size() == 0)
		{
			if((*tit)->neib[vin]->gone)
				ErrMess("g12","g12");
			(*tit)->neib[vin]->gone=true;
			delete (*tit)->neib[vin];
			
			std::list<SurfPt*>::iterator pit=surf->pts2.begin();
			while(
				pit!=surf->pts2.end())
			{
				if(*pit == (*tit)->neib[vin])
					pit = surf->pts2.erase(pit);
				else
					++pit;
			}
		}

		(*tit)->neib[vin] = NULL;
	}

	Tet *tet = *tit;
	FreeTet(surf, *tit, true);
	//surf->tets2.erase(tit);
	tit=surf->tets2.begin();
	while(tit!=surf->tets2.end())
	{
		if(*tit == tet)
			tit=surf->tets2.erase(tit);
		else
			++tit;
	}
}

void TestC(Surf* surf, const char* file, int line)
{
	
	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if(!(*tit1)->neib[vin])
				continue;
			if((*tit1)->neib[vin]->holder.size() < 3)
			{
				int c=0;
				for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
					tit2!=surf->tets2.end();
					tit2++)
				{
					for(int vin2=0; vin2<4; vin2++)
						if((*tit2)->neib[vin2] == (*tit1)->neib[vin])
							c++;
				}
				if(c>(*tit1)->neib[vin]->holder.size())
				{
					char mm[123];
					sprintf(mm, "%d %s >c1", line, file);
					ErrMess(">c1",mm);
				}
				//RemTet(surf, tit1, vin);
				//irem++;
				//goto again;
			}
		}
	}
}

//find edges between two surf pt's that are shared by more than 2 triangles
//(unusual non-triangle-fan connectivity around a nexus/pt)
bool RemOverEdges(Surf *surf, Surf *fullsurf)
{


	return true;
}

/*
Remove free-floating triangles
that are not attached to the main body mesh.
After having connected all the points together/welded,
we can see what is disattached.
*/
bool RemFloaters(Surf *surf, Surf *fullsurf)
{
//	return true;
	//if a surfpt has less than 3 holders/owners,
	//then it is a floater. test to confirm. TODO.

	int irem = 0;
	int irem2 = 0;

again:
#if 01
	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if(!(*tit1)->neib[vin])
				continue;
			if((*tit1)->neib[vin]->holder.size() < 3)
			{
#if 0
				char mm[1234];
				sprintf(mm, "\r\n hs<3 =%d \r\n"\
					"vin=%d \r\n"\
					"pos=%f,%f,%f \r\n",
					(int)(*tit1)->neib[vin]->holder.size(),
					vin,
					(*tit1)->neib[vin]->pos.x,
					(*tit1)->neib[vin]->pos.y,
					(*tit1)->neib[vin]->pos.z);
				ErrMess(mm,mm);
#endif
				int c=0;
				for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
					tit2!=surf->tets2.end();
					tit2++)
				{
					for(int vin2=0; vin2<4; vin2++)
						if((*tit2)->neib[vin2] == (*tit1)->neib[vin])
							c++;
				}
				if(c>(*tit1)->neib[vin]->holder.size())
					ErrMess(">c",">c");
				RemTet(surf, tit1, vin);
				irem++;
				goto again;
			}
		}
	}
#endif
#if 01
	//also after above, there may be tets with less than 3 verts, so remove them
	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
			tit1!=surf->tets2.end();
			++tit1)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if(!(*tit1)->neib[vin])
			{
				RemTet2(surf, tit1);
				++irem2;
				goto again;
			}
		}
	}
#endif
	//if three tets share an edge (a set of two pt's)
	//one of them must be covered by the others

	fprintf(g_applog, "RemFloaters: %d, %d removed\r\n",
		irem, irem2);

	return true;
}

bool TetRayCollides(Surf *surf,
					std::list<Tet*>::iterator tit,
					Vec3f line[2])
{
	for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
		tit2!=surf->tets2.end();
		++tit2)
	{
		if(*tit2 == *tit)
			continue;

		Vec3f tri[3];
		tri[0] = (*tit2)->neib[0]->pos;
		tri[1] = (*tit2)->neib[1]->pos;
		tri[2] = (*tit2)->neib[2]->pos;

		Vec3f vint;

		if(IntersectedPolygon(tri, line, 3, &vint))
		{
			//check if the line origin is in front of tit2
			Vec3f tnorm = Normal(tri);
			Plane3f tpl;
			MakePlane(&tpl.m_normal, &tpl.m_d, (tri[0]+tri[1]+tri[2])/3.0f,
				tnorm);

			if(PointOnOrBehindPlane(line[0], tpl, 1))
			{
				//just care if collides
				return true;
				//if true, then line origin is in the BACK
				/////continue;
			}
			else
			{
				//front
				//return true;
				//return false;
				continue;
				//collides with a front side of tet
			}
		}
	}

	return false;
}

bool InfectVis(Surf *surf, Surf *fullsurf)
{
again:
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
			tit2!=surf->tets2.end();
			++tit2)
		{
			if(*tit==*tit2)
				continue;

			if((*tit)->approved && (*tit2)->approved)
				continue;
			
			if(!(*tit)->approved && !(*tit2)->approved)
				continue;

			//check for same winding as approved
			//and a matching edge
				int samec = 0;
			for(int vin1=0; vin1<3; ++vin1)
			{
				for(int vin2=0; vin2<3; ++vin2)
				{
					if((*tit)->neib[vin1%3] == (*tit2)->neib[vin2%3])
						samec++;
					//else
					//	samec=0;
					
					if(samec >= 2)
					{
						//success
						(*tit)->approved = true;
						(*tit2)->approved = true;
						//goto nexttit2;
						goto again;
					}
				}
			}
nexttit2:
			;
		}
	}

	return true;
}

bool RemUnmarked(Surf *surf, Surf *fullsurf)
{
	int irem=0;
	//return true;
again:
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if(!(*tit)->approved)
			{
				irem++;
				RemTet2(surf, tit);
				goto again;
			}
		}
	}

	fprintf(g_applog, "RemUnmarked: %d removed\r\n",
		irem);

	return true;
}

/*
Draw a line from the outside to the first colliding
tet fronts. Then "spread the infection" to connected
but occluded to a straight line tets.
*/
bool MarkVis(Surf *surf, Surf *fullsurf)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		(*tit)->approved = false;
		(*tit)->hidden = false;
	}
	
again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Vec3f tri[3];
		tri[0] = (*tit)->neib[0]->pos;
		tri[1] = (*tit)->neib[1]->pos;
		tri[2] = (*tit)->neib[2]->pos;
		Vec3f tnorm = Normal(tri);

		//not necessary to get all because the "infection will spread"
		for(int attempt=0; attempt<3; ++attempt)
		{
			Vec3f line[2];
			line[0] = tri[attempt%3] + tnorm * 1.5f;
			line[1] = line[0] + tnorm * 3000;

			if(TetRayCollides(surf, tit, line))
			{
				(*tit)->approved = false;
				(*tit)->hidden = false;
				continue;	//inconclusive
			}
			else
			{
				(*tit)->approved = true;
				(*tit)->hidden = false;
				break;
			}
		}
	}

	//spread infection
	InfectVis(surf, fullsurf);

	//now for every approved (ie visible) tet,
	//draw line from it to every non-apporved inconclusive
	//tet and if it hits the front side of the still
	//non-approved tet, then it is hidden, so remove.
	//probably quicker to just remove non-approved,
	//but whatever.
	//also falsely approved (unknowingly) tets that
	//are actually obscured will be removed
	//in RemHidden because they share an edge (two pt's)
	//with two other tets, and one of them must be
	//inside/hidden.
	//
	//just remove non-approved here then.
	RemUnmarked(surf, fullsurf);

	return false;
}

/*
Test for completely hidden triangles.
An easy test is if all rays away from triangle lead
to back sides of other triangles.
What if there is an inner-facing chasm though?
Maybe after going to "map mesh" adding those
triangles to the map, ignore the chasm and
hidden triangles that won't be attached
in a 3-function loop, and if there's a
triangle below and triangle above coming from
two shared vertices, the one below can be identified
by projecting rays and checking for back sides.
Or: project rays inward, and approve the
first colliding triangles facing outward.
Infect neighbour triangles to test them if
projecting a ray outward doesn't meet any triangles
(and if so, make them also approved-non-hidden),
or if their front collides with a back side of an approved
(in that case they are hidden),
or their front side collides with a front side of an approved
(in that case they are approved),
in a loop one by one.
*/
bool RemHidden(Surf *surf, Surf *fullsurf)
{
	//if three tets share an edge (a set of two pt's)
	//one of them must be covered by the others

	int irem = 0;
again:

	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{
		//check the other holders of an edge
		//for two additional holders of an edge.
		for(int vin1=0; vin1<3; ++vin1)
		{
			for(int vin2=vin1+1; vin2<3; ++vin2)
			{
				//new vert combo
				int shared = 1;	//reset number of this edge's owners

				//find another owner with vin1 and vin2
				for(std::list<Tet*>::iterator hitv1=(*tit1)->neib[vin1]->holder.begin();
					hitv1!=(*tit1)->neib[vin1]->holder.end();
					++hitv1)
				{
					if(*hitv1 == *tit1)
						continue;	//counted already

					for(std::list<Tet*>::iterator hitv2=(*tit1)->neib[vin2]->holder.begin();
						hitv2!=(*tit1)->neib[vin2]->holder.end();
						++hitv2)
					{
						if(*hitv2 != *hitv1)
							continue;

						//found another owner of edge
						shared++;

						if(shared>=3)
						{
							//one of the three tets is hidden,
							//so remove it.
							//assuming at this point that all of them
							//are already marked "approved".

							Tet* tet[3];
							tet[0] = *tit1;
							tet[1] = *hitv1;
							tet[2] = *hitv2;

							//erase the one with the most back sides facing it by the others

							Vec3f tri[3][3];
							tri[0][0] = tet[0]->neib[0]->pos;
							tri[0][1] = tet[0]->neib[1]->pos;
							tri[0][2] = tet[0]->neib[2]->pos;
							tri[1][0] = tet[1]->neib[0]->pos;
							tri[1][1] = tet[1]->neib[1]->pos;
							tri[1][2] = tet[1]->neib[2]->pos;
							tri[2][0] = tet[2]->neib[0]->pos;
							tri[2][1] = tet[2]->neib[1]->pos;
							tri[2][2] = tet[2]->neib[2]->pos;

							Vec3f tnorm[3];
							tnorm[0] = Normal(tri[0]);
							tnorm[1] = Normal(tri[1]);
							tnorm[2] = Normal(tri[2]);

							Plane3f tpl[3];
							MakePlane(&tpl[0].m_normal, &tpl[0].m_d, (tri[0][0]+tri[0][1]+tri[0][2])/3.0f,
								tnorm[0]);
							MakePlane(&tpl[1].m_normal, &tpl[1].m_d, (tri[1][0]+tri[1][1]+tri[1][2])/3.0f,
								tnorm[1]);
							MakePlane(&tpl[2].m_normal, &tpl[2].m_d, (tri[2][0]+tri[2][1]+tri[2][2])/3.0f,
								tnorm[2]);

							float cover[3] = {0,0,0};

							for(int t1=0; t1<3; t1++)
							{
								for(int t2=0; t2<3; ++t2)
								{
									if(t1==t2)
										continue;

									if(PointOnOrBehindPlane( (tri[t1][0]+tri[t1][1]+tri[t1][2])/3.0f,
										tpl[t2]))
										cover[t1]+=1;
								}
							}

							int largest=0;

							for(int ti=1; ti<3; ++ti)
								if(cover[ti]>cover[largest])
									largest=ti;

							std::list<Tet*>::iterator ret;

							if(largest==0)
								ret = tit1;
							else if(largest ==1)
								ret = hitv1;
							else if(largest==2)
								ret = hitv2;

							irem++;
							RemTet2(surf, ret);
							//might have to go back to check for tets with less than 
							//three neighbours owners at each of its pts.

							goto again;
						}
					}
				}
			}
		}
	}
//new: if two tets share an edge with order p1,p2 (not p2,p1)
	//then one must be behind

	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{

		for(std::list<Tet*>::iterator tit2=surf->tets2.begin();
			tit2!=surf->tets2.end();
			++tit2)
		{
			if(*tit1 == *tit2)
				continue;

			Tet *tet1 = *tit1;
			Tet *tet2 = *tit2;

			int fate1 = -1;
			int fate2 = -1;

			int t1e1 = -1;
			int t1e2 = -1;

			int t2e1 = -1;
			int t2e2 = -1;

			for(int v1=0; v1<3; ++v1)
			{
				for(int v2=0; v2<3; ++v2)
				{
					if(tet1->neib[v1] == tet2->neib[v2])
					{
						if(t1e1 >= 0 || t2e1 >= 0)
						{
							t1e2 = v1;
							t2e2 = v2;
						}
						else
						{
							t1e1 = v1;
							t2e1 = v2;
						}
					}
				}
			}

			if( t1e1 >= 0 && t1e2 >= 0 &&
				t2e1 >= 0 && t2e2 >= 0 &&
				(((t1e2+3)%6)-((t1e1+3)%3)) == (((t2e2+3)%6)-((t2e1+3)%3)) )
			{

				//one of the three tets is hidden,
				//so remove it.
				//assuming at this point that all of them
				//are already marked "approved".

				Tet* tet[2];
				tet[0] = *tit1;
				tet[1] = *tit2;
				//tet[2] = *hitv2;

				//erase the one with the most back sides facing it by the others

				Vec3f tri[2][3];
				tri[0][0] = tet[0]->neib[0]->pos;
				tri[0][1] = tet[0]->neib[1]->pos;
				tri[0][2] = tet[0]->neib[2]->pos;
				tri[1][0] = tet[1]->neib[0]->pos;
				tri[1][1] = tet[1]->neib[1]->pos;
				tri[1][2] = tet[1]->neib[2]->pos;
				//tri[2][0] = tet[2]->neib[0]->pos;
				//tri[2][1] = tet[2]->neib[1]->pos;
				//tri[2][2] = tet[2]->neib[2]->pos;

				Vec3f tnorm[2];
				tnorm[0] = Normal(tri[0]);
				tnorm[1] = Normal(tri[1]);
				//tnorm[2] = Normal(tri[2]);

				Plane3f tpl[2];
				MakePlane(&tpl[0].m_normal, &tpl[0].m_d, (tri[0][0]+tri[0][1]+tri[0][2])/3.0f,
					tnorm[0]);
				MakePlane(&tpl[1].m_normal, &tpl[1].m_d, (tri[1][0]+tri[1][1]+tri[1][2])/3.0f,
					tnorm[1]);
				//MakePlane(&tpl[2].m_normal, &tpl[2].m_d, (tri[2][0]+tri[2][1]+tri[2][2])/3.0f,
				//	tnorm[2]);

				float cover[3] = {0,0,0};

#if 0
				for(int t1=0; t1<3; t1++)
				{
					for(int t2=0; t2<3; ++t2)
					{
						if(t1==t2)
							continue;

						if(PointOnOrBehindPlane( (tri[t1][0]+tri[t1][1]+tri[t1][2])/3.0f,
							tpl[t2]))
							cover[t1]+=1;
					}
				}
#else
				for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
					pit!=surf->pts2.end();
					++pit)
				{
					for(int ti=0; ti<2; ti++)
					{
						if(PointOnOrBehindPlane( (*pit)->pos,
							tpl[ti]))
							cover[ti]+=1;
					}
				}
#endif

				int largest=0;

				for(int ti=1; ti<2; ++ti)
					if(cover[ti]>cover[largest])
						largest=ti;

				std::list<Tet*>::iterator ret;

				if(largest==0)
					ret = tit1;
				else if(largest ==1)
					ret = tit2;
				//else if(largest==2)
				//	ret = hitv2;

				RemTet2(surf, ret);


				irem++;
				goto again;
			}
		}
	}

	fprintf(g_applog, "RemHidden: %d removed \r\n", irem);

	return true;
}

void ClassifyEdge(Vec2f p1orc,
				  Vec2f p2orc,
				  float *edgeposx,
				  bool *edgeplaced,
				  float *edgeposy)
{

	*edgeposx = p2orc.x - p1orc.x;
	*edgeposy = p2orc.y - p1orc.y;
	*edgeplaced = true;

	if(abs(*edgeposx) <= 0.01f && abs(*edgeposy) <= 0.01f)
		ErrMess("asd","<=exy");

#if 0
	if(p2orc.x > p1orc.x)
	{
		*edgeposx = true;
		*edgesamex = false;
	}
	else if(p2orc.x < p1orc.x)
	{
		*edgeposx = false;
		*edgesamex = false;
	}
	else
	{
		*edgeposx = false;
		*edgesamex = true;
	}
	
	if(p2orc.y > p1orc.y)
	{
		*edgeposy = true;
		*edgesamey = false;
	}
	else if(p2orc.y < p1orc.y)
	{
		*edgeposy = false;
		*edgesamey = false;
	}
	else
	{
		*edgeposy = false;
		*edgesamey = true;
	}
#endif
}

class EdgeRef
{
public:
	Tet* tet;
	int ev;
};

void ListEdges(Surf *surf, SurfPt *p1, SurfPt *p2, std::list<EdgeRef> *edgelist)
{
	//for(std::list<Tet*>::iterator tit=surf->tets2.begin();

}

//find the correct edges and update "updtet" edges
void UpdEdges2(Surf *surf, SurfPt *p1, SurfPt *p2, float* edgeposx, float* edgeposy,
			  Tet* updtet, bool *edgeplaced)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		//if(tet == not)
		//	continue;
		if(tet == updtet)
			continue;

		int e1 = -1;
		int e2 = -1;

		for(int v=0; v<3; ++v)
		{
			if(tet->neib[v] == p1)
				e1 = v;
			if(tet->neib[v] == p2)
				e2 = v;
		}

		if(e1 >= 0 && e2 >= 0)
		{
			int fate = 0;

			while(fate == e1 || fate == e2)
				fate++;

			int ei = (fate+1)%3;

			int e12 = (fate+1)%3;
			int e22 = (fate+2)%3;

			if(!tet->edgeplaced[ei])
				continue;

			//going forward?
			if(e12 == e1 && e22 == e2)
			{
				//tet->edgeposx[ei] = edgeposx;
				//tet->edgeposy[ei] = edgeposy;
				*edgeposx = tet->edgeposx[ei];
				*edgeposy = tet->edgeposy[ei];
				*edgeplaced = true;

				if(abs(*edgeposx) <= 0.01 &&
					abs(*edgeposy) <= 0.01)
					ErrMess("asdads","e<<<<<");
			}
			else
			{
				//tet->edgeposx[ei] = -edgeposx;
				//tet->edgeposy[ei] = -edgeposy;
				*edgeposx = -tet->edgeposx[ei];
				*edgeposy = -tet->edgeposy[ei];
				*edgeplaced = true;

				if(abs(*edgeposx) <= 0.01 &&
					abs(*edgeposy) <= 0.01)
					ErrMess("asdads","e<<<<<2");
			}
		}
	}
}

//update the obtained edge offsets (true) for neighbouring tets with the same edge
void UpdEdges(Surf *surf, SurfPt *p1, SurfPt *p2, float edgeposx, float edgeposy,
			  Tet* not)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		if(tet == not)
			continue;

		int e1 = -1;
		int e2 = -1;

		for(int v=0; v<3; ++v)
		{
			if(tet->neib[v] == p1)
				e1 = v;
			if(tet->neib[v] == p2)
				e2 = v;
		}

		if(e1 >= 0 && e2 >= 0)
		{
			int fate = 0;

			while(fate == e1 || fate == e2)
				fate++;

			int ei = (fate+1)%3;

			int e12 = (fate+1)%3;
			int e22 = (fate+2)%3;

			//going forward?
			if(e12 == e1 && e22 == e2)
			{
				tet->edgeposx[ei] = edgeposx;
				tet->edgeposy[ei] = edgeposy;
				tet->edgeplaced[ei] = true;
				

				if(abs(edgeposx) <= 0.01 &&
					abs(edgeposy) <= 0.01)
					ErrMess("asdads","e<<<<<333");

				return;
			}
			else
			{
				tet->edgeposx[ei] = -edgeposx;
				tet->edgeposy[ei] = -edgeposy;
				tet->edgeplaced[ei] = true;

				if(abs(edgeposx) <= 0.01 &&
					abs(edgeposy) <= 0.01)
					ErrMess("asdads","e<<<<<444");

				return;
			}
		}
	}

	ErrMess("asdads","e!!up333");
}

void PlaceTet(Surf *surf, Tet *tet, std::list<Tet*>* toplace, bool lasttry)
{
tryagain:
	if(!tet->placed)
	{
		int placec = 0;
		int fate = -1;

		SurfPt* placept[3] = {NULL,NULL,NULL};

		for(int vin=0; vin<3; ++vin)
		{
			if(!tet->neib[vin])
				ErrMess("pl","pl1111");
			if(tet->neib[vin]->placed)
			{
				if(!tet->neib[vin])
					ErrMess("pl","pl1");
				if(tet->neib[vin]->gone)
					ErrMess("pl","pl1g");
				placept[placec] = tet->neib[vin];
				placec++;
			}
			else
			{
				//placec=0;
				fate = vin;
			}
#if 0
			if(vin==2 && placec>=1 && placec<2 && lasttry)
			{
				Vec2f rp = Vec2f((rand()%100-50)/1000.0f + placept[0]->orc.x,
					(rand()%100-50)/1000.0f + placept[0]->orc.y);

				tet->neib[fate]->orc = rp;
				tet->neib[fate]->placed = true;
				//tet->placed = true;
				
				ClassifyEdge(tet->neib[0]->orc, tet->neib[1]->orc,
					&tet->edgeposx[0], &tet->edgesamex[0],
					&tet->edgeposy[0], &tet->edgesamey[0]);

				goto tryagain;
			}
#endif

			if(fate<0 && placec>=3)
			{
				//tet->placed = true;
				fate=0;

				UpdEdges2(surf,
					tet->neib[0],
					tet->neib[1],
					&tet->edgeposx[0],
					&tet->edgeposy[0],
					tet,
					&tet->edgeplaced[0]);
				UpdEdges2(surf,
					tet->neib[1],
					tet->neib[2],
					&tet->edgeposx[1],
					&tet->edgeposy[1],
					tet,
					&tet->edgeplaced[1]);
				UpdEdges2(surf,
					tet->neib[2],
					tet->neib[0],
					&tet->edgeposx[2],
					&tet->edgeposy[2],
					tet,
					&tet->edgeplaced[2]);

				for(std::list<Tet*>::iterator hit=tet->neib[fate]->holder.begin();
					hit!=tet->neib[fate]->holder.end();
					++hit)
				{
#if 0
					Tet *temptet = *hit;

					if(temptet->placed)
					{
						int e1 = -1;
						int e2 = -1;
						int e3 = -1;

						for(int v2=0; v2<3; v2++)
						{
							if(temptet->neib[v2] == tet->neib[0])
								e1 = v2;
							if(temptet->neib[v2] == tet->neib[1])
								e2 = v2;
							if(temptet->neib[v2] == tet->neib[2])
								e3 = v2;
						}

						if(e1 >= 0 && e2 >= 0)
						{
							tet->edgeposx[0] = 
								temptet->neib[e2]->orc.x - temptet->neib[e1]->orc.x;
							tet->edgeposy[0] = 
								temptet->neib[e2]->orc.y - temptet->neib[e1]->orc.y;
						}

						if(e2 >= 0 && e3 >= 0)
						{
							tet->edgeposx[1] = 
								temptet->neib[e3]->orc.x - temptet->neib[e2]->orc.x;
							tet->edgeposy[1] = 
								temptet->neib[e3]->orc.y - temptet->neib[e2]->orc.y;
						}

						if(e3 >= 0 && e1 >= 0)
						{
							tet->edgeposx[2] = 
								temptet->neib[e1]->orc.x - temptet->neib[e3]->orc.x;
							tet->edgeposy[2] = 
								temptet->neib[e1]->orc.y - temptet->neib[e3]->orc.y;
						}
					}
#endif
					//PlaceTet(surf, *hit);
					if(*hit != tet)
						toplace->push_back(*hit);
				}

				for(std::list<Tet*>::iterator hit=tet->neib[(fate+1)%3]->holder.begin();
					hit!=tet->neib[(fate+1)%3]->holder.end();
					++hit)
				{
					//PlaceTet(surf, *hit);
					if(*hit != tet)
						toplace->push_back(*hit);
				}

				for(std::list<Tet*>::iterator hit=tet->neib[(fate+2)%3]->holder.begin();
					hit!=tet->neib[(fate+2)%3]->holder.end();
					++hit)
				{
					//PlaceTet(surf, *hit);
					if(*hit != tet)
						toplace->push_back(*hit);
				}

				tet->placed = true;

				return;
			}

			if(placec==2&&fate>=0)
			{
				//if(vin==2 || !tet->neib[2]->placed)
				{
					int e1 = (fate+1)%3;
					int e2 = (fate+2)%3;

#if 0
					Vec3f p1a[4];
					Vec3f p2a[4];

					p1a[0] = Vec3f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y, 0);
					p1a[1] = Vec3f(tet->neib[e1]->orc.x+1, tet->neib[e1]->orc.y, 0);
					p1a[2] = Vec3f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y+1, 0);
					p1a[3] = Vec3f(tet->neib[e1]->orc.x+1, tet->neib[e1]->orc.y+1, 0);
					p2a[0] = Vec3f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y, 0);
					p2a[1] = Vec3f(tet->neib[e2]->orc.x+1, tet->neib[e2]->orc.y, 0);
					p2a[2] = Vec3f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y+1, 0);
					p2a[3] = Vec3f(tet->neib[e2]->orc.x+1, tet->neib[e2]->orc.y+1, 0);
					Vec3f up = Vec3f(0,0,-1);

					int p1n = -1;
					int p2n = -1;
					float pd = 9999999;

					for(int p1i=0; p1i<3; p1i++)
					{
						for(int p2i=0; p2i<3; p2i++)
						{
							float pdt = Magnitude(p1a[p1i] - p2a[p2i]);

							if(pdt < pd)
							{
								p1n = p1i;
								p2n = p2i;
								pd = pdt;
							}
						}
					}

					Vec3f p1 = p1a[p1n];
					Vec3f p2 = p2a[p2n];
#endif

#if 0
					Vec3f p1 = Vec2f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y, 0);
					Vec3f p2 = Vec2f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y, 0);

					if(tet->outx[e1])
						p1.x += 1;
					if(tet->outy[e1])
						p1.y += 1;
					
					if(tet->outx[e2])
						p2.x += 1;
					if(tet->outy[e2])
						p2.y += 1;
#endif
#if 1
					Vec2f p1a,p2a;

					p1a = tet->neib[e1]->orc;
					TrueNextPt(p1a, 
						tet->edgeposx[e1],
						false,
						tet->edgeposy[e1],
						false,
						tet->neib[e2]->orc,
						&p2a);

					Vec3f p1 = Vec3f(p1a.x, p1a.y, 0);
					Vec3f p2 = Vec3f(p2a.x, p2a.y, 0);
#endif

					Vec3f up = Vec3f(0,0,-1);
					Vec3f axis = Cross(up,p1-p2);

					if(ISNAN(axis.x))
						ErrMess("asd","axispnan!x");
					if(ISNAN(axis.y))
						ErrMess("asd","axispnan!y");
					if(ISNAN(axis.z))
						ErrMess("asd","axispnan!z");

					axis = Normalize(axis);

					Vec3f midp = (p1+p2)/2.0f;

					if(ISNAN(midp.x))
						ErrMess("asd","midpnan!x");
					if(ISNAN(midp.y))
						ErrMess("asd","midpnan!y");

					float d = Magnitude(p1-p2);
					
					if(ISNAN(d))
						ErrMess("asd","dnan!");

					Vec3f p3 = midp + axis * d;

					if(ISNAN(p3.x))
					{
						ErrMess("asd","nan!x");

						char m[123];
						sprintf(m, "midp%f,%f,%f axis%f,%f,%f d%f p1=%f,%f p2=%f,%f edgepos=%f,%f",
							midp.x, midp.y, midp.z,
							axis.x, axis.y, axis.z,
							d,
							p1.x, p1.y,
							p2.x, p2.y,
							tet->edgeposx[e1], tet->edgeposy[e1]);
						ErrMess(m,m);
					}
					if(ISNAN(p3.y))
						ErrMess("asd","nan!y");

					if(fate<0)
						ErrMess("fate","fate<0");
					if(fate>2)
						ErrMess("fate","fate>2");
/////////////////
					tet->edgedrawarea[fate] = Magnitude( tet->neib[fate]->pos - tet->neib[e1]->pos );
					//tet->edgeorarea[fate] = Magnitude( tet->neib[fate]->orc - tet->neib[e1]->orc );
					tet->edgeorarea[fate] = Magnitude( p3 - p1 );

					tet->edgedrawarea[e1] = Magnitude( tet->neib[e1]->pos - tet->neib[e2]->pos );
					//tet->edgeorarea[e1] = Magnitude( tet->neib[e1]->orc - tet->neib[e2]->orc );
					tet->edgeorarea[e1] = Magnitude( p1 - p2 );
					
					tet->edgedrawarea[e2] = Magnitude( tet->neib[e2]->pos - tet->neib[fate]->pos );
					//tet->edgeorarea[e1] = Magnitude( tet->neib[e1]->orc - tet->neib[e2]->orc );
					tet->edgeorarea[e2] = Magnitude( p2 - p3 );
	/////////////////
				
					ClassifyEdge(tet->neib[e2]->orc, Vec2f(p3.x,p3.y),
						&tet->edgeposx[e2], &tet->edgeplaced[e2],
						&tet->edgeposy[e2]);

					UpdEdges(surf,
						tet->neib[e2],
						tet->neib[fate],
						tet->edgeposx[e2],
						tet->edgeposy[e2],
						tet);

					if(tet->edgeposx[e2] == 0 &&
							tet->edgeposy[e2] == 0)
							ErrMess("asdasd","exy=0ttt");

					ClassifyEdge(Vec2f(p3.x,p3.y), tet->neib[e1]->orc,
						&tet->edgeposx[fate], &tet->edgeplaced[fate],
						&tet->edgeposy[fate]);
					
					UpdEdges(surf,
						tet->neib[fate],
						tet->neib[e1],
						tet->edgeposx[fate],
						tet->edgeposy[fate],
						tet);

					if(tet->edgeposx[fate] == 0 &&
							tet->edgeposy[fate] == 0)
							ErrMess("asdasd","exy=0gggg");

					UpdEdges(surf,
						tet->neib[e1],
						tet->neib[e2],
						tet->edgeposx[e1],
						tet->edgeposy[e1],
						tet);

					if(tet->edgeposx[e1] == 0 &&
						tet->edgeposy[e1] == 0)
						ErrMess("asdasd","exy=0eeeeee");

					while(p3.x < 0)
						p3.x += 1.0f;
					while(p3.y < 0)
						p3.y += 1.0f;

					//bool outx = false;
					//bool outy = false;

					while(p3.x >= 1)
					{
						//outx = true;
						p3.x -= 1.0f;
					}
					while(p3.y >= 1)
					{
						//outy = true;
						p3.y -= 1.0f;
					}

					tet->neib[fate]->orc = Vec2f(p3.x,p3.y);
					tet->neib[fate]->placed = true;
					///tet->edgeoutx[fate] = outx;
					///tet->edgeouty[fate] = outy;
					tet->placed = true;

#if 0
					if(OrNorm(tet->neib,
						tet->edgeposx,
						tet->edgeposy,
						tet->edgesamex,
						tet->edgesamey).z >= 0)
					{
						p3 = midp - axis * d;

						//outx = false;
						//outy = false;
						
						ClassifyEdge(tet->neib[e2]->orc, Vec2f(p3.x,p3.y),
							&tet->edgeposx[e2], &tet->edgesamex[e2],
							&tet->edgeposy[e2], &tet->edgesamey[e2]);
						ClassifyEdge(Vec2f(p3.x,p3.y), tet->neib[e1]->orc,
							&tet->edgeposx[fate], &tet->edgesamex[fate],
							&tet->edgeposy[fate], &tet->edgesamey[fate]);

						while(p3.x < 0)
						{
							//outx = true;
							p3.x += 1.0f;
						}
						while(p3.y < 0)
						{
							//outy = true;
							p3.y += 1.0f;
						}
						while(p3.x >= 1)
						{
							//outx = true;
							p3.x -= 1.0f;
						}
						while(p3.y >= 1)
						{
							//outy = true;
							p3.y -= 1.0f;
						}

						tet->neib[fate]->orc = Vec2f(p3.x,p3.y);
						tet->neib[fate]->placed = true;
//					tet->edgeoutx[fate] = outx;
		//				tet->edgeouty[fate] = outy;
						tet->placed = true;
					}
#endif
/////////////////

					for(std::list<Tet*>::iterator hit=tet->neib[fate]->holder.begin();
						hit!=tet->neib[fate]->holder.end();
						++hit)
					{
						//PlaceTet(surf, *hit);
						if(*hit != tet)
							toplace->push_back(*hit);
					}

					for(std::list<Tet*>::iterator hit=tet->neib[(fate+1)%3]->holder.begin();
						hit!=tet->neib[(fate+1)%3]->holder.end();
						++hit)
					{
						//PlaceTet(surf, *hit);
						if(*hit != tet)
							toplace->push_back(*hit);
					}

					for(std::list<Tet*>::iterator hit=tet->neib[(fate+2)%3]->holder.begin();
						hit!=tet->neib[(fate+2)%3]->holder.end();
						++hit)
					{
						//PlaceTet(surf, *hit);
						if(*hit != tet)
							toplace->push_back(*hit);
					}
				}

				break;
			}
		}
	}
}

void TrueBackEdge(SurfPt* p2, 
				  //these booleans apply to p1 going to p2
				  bool edgeposx, bool edgesamex,
				  bool edgeposy, bool edgesamey,
				  //getting the coords of p1
				  Vec2f startorc, Vec2f* trueorc)
{
	
}

#define SAME_EPSILON	0.1f

void TrueNextPt(Vec2f p1orc, //prev point
			  //for edge with second point =trueorc
				  float edgeposx, bool edgesamex,
				  float edgeposy, bool edgesamey,
				  Vec2f startorc, Vec2f* trueorc)
{
	trueorc->x = startorc.x;
	trueorc->y = startorc.y;
	//trueorc->x = p1orc.x += edgeposx;
	//trueorc->y = p1orc.y += edgeposy;

#if 0
	trueorc->x = startorc.x;
	trueorc->y = startorc.y;

	while(edgeposx && trueorc->x <= p1orc.x)
	{
		trueorc->x += 1;
	}
	
	while(!edgeposx && !edgesamex && trueorc->x >= p1orc.x)
	{
		trueorc->x -= 1;
	}
	
	if(!edgeposx && edgesamex)
	{
		trueorc->x = p1orc.x;
	}
	/////////
	while(edgeposy && trueorc->y <= p1orc.y)
	{
		trueorc->y += 1;
	}
	
	while(!edgeposy && !edgesamey && trueorc->y >= p1orc.y)
	{
		trueorc->y -= 1;
	}
	
	if(!edgeposy && edgesamey)
	{
		trueorc->y = p1orc.y;
	}
#endif
}

/*
Go from triangle to triangle, mapping to the orientability map.
Try moves that require adding only one vertex, or then two.
All the identical points must be single without repeats,
so join any <1 distance points before hand.
*/
bool GrowMapMesh(Surf *surf, Surf *fullsurf, Vec2f *vmin, Vec2f *vmax)
{
	//std::list<Tet*>::iterator starttit=surf->tets2.begin();
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		//if((*tit)->approved)
		//{
		//	starttit = tit;
		//	break;
		//}

		(*tit)->placed = false;
	}

	if(!surf->tets2.size())
		ErrMess("pp","pp1");

	Tet *tet = *surf->tets2.begin();

	if(!tet->neib[0])
		ErrMess("ssasd","s11");
	if(tet->neib[0]->gone)
		ErrMess("ssasd","s11g");
	if(!tet->neib[1])
		ErrMess("ssasd","s111");
	if(tet->neib[1]->gone)
		ErrMess("ssasd","s111g");

	tet->neib[0]->orc.x = 0.5f;
	tet->neib[0]->orc.y = 0.5f + 1.0f/(float)surf->pts2.size();
	tet->neib[0]->placed = true;
	tet->neib[1]->orc.x = 0.5f + 1.0f/(float)surf->pts2.size();
	tet->neib[1]->orc.y = 0.5f + 1.0f/(float)surf->pts2.size();
	tet->neib[1]->placed = true;
	ClassifyEdge(tet->neib[0]->orc, tet->neib[1]->orc,
		&tet->edgeposx[0], &tet->edgeplaced[0],
		&tet->edgeposy[0]);

	UpdEdges(surf,
		tet->neib[0],
		tet->neib[1],
		tet->edgeposx[0],
		tet->edgeposy[0],
		tet);

	//tet->edgeposx[0] = true;
	//tet->edgesamex[0] = false;
	//tet->edgeposy[0] = false;
	//tet->edgesamey[0] = true;
	//tet->edgeoutx[0] = false;
	//tet->edgeouty[0] = false;
	//tet->edgeoutx[1] = false;
	//tet->edgeouty[1] = false;
	//tet->neib[2]->orc.x = 0.52f;
	//tet->neib[2]->orc.y = 0.5f;
	tet->edgedrawarea[0] = Magnitude( tet->neib[0]->pos - tet->neib[1]->pos );
	tet->edgeorarea[0] = Magnitude( tet->neib[0]->orc - tet->neib[1]->orc );

	if(tet->edgeposx[0] == 0 &&
		tet->edgeposy[0] == 0)
		ErrMess("asdasd","exy=0");

	//maybe split some tri's?
	std::list<Tet*> toplace;
	PlaceTet(surf, tet, &toplace, false);

//placeagain:
	while(toplace.size())
	{
		PlaceTet(surf, *toplace.begin(), &toplace, false);
		toplace.erase(toplace.begin());
	}

#if 0
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		if(!(*tit)->placed)
			toplace.push_back(*tit);
	}

	//if(toplace.size())
	{
		while(toplace.size())
		{
			PlaceTet(surf, *toplace.begin(), &toplace, false);
			toplace.erase(toplace.begin());
		}
	}
		//goto placeagain;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		if((*tit)->placed)
		{
dotet:
			Tet * tet = *tit;
			SurfPt *fatt[3];

			fatt[0] = tet->neib[0];
			fatt[1] = tet->neib[1];
			fatt[2] = tet->neib[2];

			if(fatt[0]->placed &&
				fatt[1]->placed &&
				fatt[2]->placed)
			{
dopt:
				for(int v=0; v<3; ++v)
				{
					if(fatt[v]->orc.x < vmin->x)
						vmin->x = fatt[v]->orc.x;
					if(fatt[v]->orc.y < vmin->y)
						vmin->y = fatt[v]->orc.y;
					
					if(fatt[v]->orc.x > vmax->x)
						vmax->x = fatt[v]->orc.x;
					if(fatt[v]->orc.y > vmax->y)
						vmax->y = fatt[v]->orc.y;
				}
			}
			else
			{
				ErrMess("p","!p");
				PlaceTet(surf, tet, &toplace, false);

				while(toplace.size())
				{
					PlaceTet(surf, *toplace.begin(), &toplace, false);
					toplace.erase(toplace.begin());
				}

				goto dopt;
			}
		}
		else
		{
			ErrMess("p","!p2");
			PlaceTet(surf, tet, &toplace, false);

			while(toplace.size())
			{
				PlaceTet(surf, *toplace.begin(), &toplace, false);
				toplace.erase(toplace.begin());
			}

			goto dotet;
		}
	}
#endif
	/////////
	return true;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
		
		SurfPt *fatt[3];

		fatt[0] = tet->neib[0];
		fatt[1] = tet->neib[1];
		fatt[2] = tet->neib[2];

		for(int v=0; v<3; ++v)
		{
			fatt[v]->placed = false;
		}
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
		SurfPt *fatt[3];

		fatt[0] = tet->neib[0];
		fatt[1] = tet->neib[1];
		fatt[2] = tet->neib[2];

		for(int v=0; v<3; ++v)
		{
			///tet->edgeposx[v] /= (vmax->x - vmin->x);
			///tet->edgeposy[v] /= (vmax->y - vmin->y);
		}

		for(int v=0; v<3; ++v)
		{
			if(fatt[v]->placed)
				continue;
			//fatt[v]->orc.x = (fatt[v]->orc.x - vmin->x) / ((vmax->x - vmin->x));
			//fatt[v]->orc.y = (fatt[v]->orc.y - vmin->y) / ((vmax->y - vmin->y));
			///fatt[v]->orc.x = (fatt[v]->orc.x - vmin->x) / ((vmax->x - vmin->x));
			///fatt[v]->orc.y = (fatt[v]->orc.y - vmin->y) / ((vmax->y - vmin->y));
			//tet->edgeposx[v] /= (vmax->x - vmin->x);
			//tet->edgeposy[v] /= (vmax->y - vmin->y);
			fatt[v]->placed = true;
		}

		for(int v=0; v<3; ++v)
		{
			int e1 = v;
			int e2 = (v+1)%3;

			tet->edgeorarea[v] = Magnitude( tet->neib[e1]->orc - tet->neib[e2]->orc );
		}
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
	
		if(OrNorm(tet->neib,
			tet->edgeposx,
			tet->edgeposy,
			tet->edgeplaced,
			tet->edgeplaced).z >= 0)
		{
			SurfPt* oldpt = tet->neib[0];
			tet->neib[0] = tet->neib[1];
			tet->neib[1] = oldpt;

			float oldval1 = tet->edgedrawarea[1];
			tet->edgedrawarea[1] = tet->edgedrawarea[2];
			tet->edgedrawarea[2] = oldval1;

			oldval1 = tet->edgeorarea[1];
			tet->edgeorarea[1] = tet->edgeorarea[2];
			tet->edgeorarea[2] = oldval1;
		}
	}

	return true;
}

bool SplitEdge3(Surf *surf, Tet *tet,
			   //std::list<Tet*>::iterator tit,
			   SurfPt **remidp,
			   int fate//,
			  // Tet **retet
			  )
{
	//fate=far vertex from midp, the one behind the one behind midp in a clockwise turn
	SurfPt* fatt[3];

	fatt[0] = tet->neib[0];
	fatt[1] = tet->neib[1];
	fatt[2] = tet->neib[2];

	if(fatt[0]->gone)
		ErrMess("g00000","g0f0");
	if(fatt[1]->gone)
		ErrMess("g00000","g0f1");
	if(fatt[2]->gone)
		ErrMess("g00000","g0f2");

	//int fate = -1;

	int e1 = (fate+1)%3;
	int e2 = (fate+2)%3;

	Vec3f midp = (fatt[e1]->pos+fatt[e2]->pos)/2.0f;
	Vec3f midwp = (fatt[e1]->wrappos+fatt[e2]->wrappos)/2.0f;
	float midfile = (fatt[e1]->file + fatt[e2]->file)/2.0f;
	int midring = max(fatt[e1]->ring, fatt[e2]->ring);

	//01
	//32

	//surf->tets2.erase(tit);

	CopyTet(surf, tet);
	Tet *newt1 = *surf->tets2.rbegin();
	//CopyTet(surf, tet);
	//Tet *newt2 = *surf->tets2.rbegin();
	newt1->placed = false;

	if(*remidp)	//already created?
	{
		(*remidp)->holder.push_back(newt1);
		(*remidp)->holder.push_back(tet);
		
		//(*remidp)->holder.unique(UniqueTet);
		RemDupTet(&(*remidp)->holder);
		
		newt1->neib[0] = *remidp;
		newt1->neib[1] = fatt[e2];
		newt1->neib[2] = fatt[fate];
		SepPt(newt1, fatt[e1], NULL);
		
		if(newt1->neib[0]->gone)
			ErrMess("g00000","g0f0n");
		if(newt1->neib[1]->gone)
			ErrMess("g00000","g0f1n");
		if(newt1->neib[2]->gone)
			ErrMess("g00000","g0f2n");
		
		//fatt[e1]->holder.push_back(newt1);
		fatt[e2]->holder.push_back(newt1);
		fatt[fate]->holder.push_back(newt1);
		
		RemDupTet(&fatt[e2]->holder);
		RemDupTet(&fatt[fate]->holder);

		tet->neib[0] = *remidp;
		tet->neib[1] = fatt[fate];
		tet->neib[2] = fatt[e1];
		SepPt(tet, fatt[e2], NULL);
		
		if(tet->neib[0]->gone)
			ErrMess("g00000","g0f0nt");
		if(tet->neib[1]->gone)
			ErrMess("g00000","g0f1nt");
		if(tet->neib[2]->gone)
			ErrMess("g00000","g0f2nt");
	}
	else
	{
		//(*remidp) = CopyPt(surf, fatt[(fate+1)%3], newt1);
		//(*remidp) = CopyPt(surf, fatt[e1], newt1);
		(*remidp) = new SurfPt;
		surf->pts2.push_back(*remidp);
		(*remidp)->placed = false;
		(*remidp)->ring = midring;
		(*remidp)->file = midfile;
		//SepPt(tet, *remidp, NULL);
		//SurfPt* newp2 = CopyPt(surf, fatt[(fate+1)%3], newt2);
		//SepPt(tet, newp2, NULL);
		(*remidp)->pos = midp;
		(*remidp)->orc = (tet->neib[e1]->orc + tet->neib[e2]->orc)/2.0f;
		(*remidp)->wrappos = (tet->neib[e1]->wrappos + tet->neib[e2]->wrappos)/2.0f;
		(*remidp)->pressure = (tet->neib[e1]->pressure + tet->neib[e2]->pressure)/2.0f;
		//newp2->pos = midp;
		(*remidp)->holder.clear();
		(*remidp)->holder.push_back(newt1);
		(*remidp)->holder.push_back(tet);
		//(*remidp)->holder.unique(UniqueTet);
		RemDupTet(&(*remidp)->holder);

		//Tet *sharet = NULL;

		//sharet also gets midp
		
		//12
		//3

		//23
		//1*

		newt1->neib[0] = *remidp;
		newt1->neib[1] = fatt[e2];
		newt1->neib[2] = fatt[fate];
		SepPt(newt1, fatt[e1], NULL);
			
		if(newt1->neib[0]->gone)
			ErrMess("g00000","g0f0nb");
		if(newt1->neib[1]->gone)
			ErrMess("g00000","g0f1nb");
		if(newt1->neib[2]->gone)
			ErrMess("g00000","g0f2nb");
		
		//fatt[e1]->holder.push_back(newt1);
		fatt[e2]->holder.push_back(newt1);
		fatt[fate]->holder.push_back(newt1);
		
		RemDupTet(&fatt[e2]->holder);
		RemDupTet(&fatt[fate]->holder);

		tet->neib[0] = *remidp;
		tet->neib[1] = fatt[fate];
		tet->neib[2] = fatt[e1];
		SepPt(tet, fatt[e2], NULL);

		if(tet->neib[0]->gone)
			ErrMess("g00000","g0f0nt2");
		if(tet->neib[1]->gone)
			ErrMess("g00000","g0f1nt2");
		if(tet->neib[2]->gone)
			ErrMess("g00000","g0f2nt2");
	}

	//*retet = newt1;

	//fatt[e1]->holder.push_back(newt1);
	//fatt[e2]->holder.push_back(newt1);
	//fatt[fate]->holder.push_back(newt1);

	//std::list<Tet*> toplace;
	//PlaceTet(surf, newt1, &toplace, false);

	//while(toplace.size())
	{
	//	PlaceTet(surf, *toplace.begin(), &toplace, false);
	//	toplace.erase(toplace.begin());
	}

	return true;
}

bool SplitEdge2(Surf *surf, SurfPt *epts[2])
{
	
	return true;
}

/*
make triangles tesselated
*/
bool SplitEdges(Surf *surf, Surf *fullsurf, Vec2f *vmin, Vec2f *vmax, int amt=BIGTEX*BIGTEX)
{//return true;
	int times = 0;
again:

	++times;

	if(times > amt)
		return true;

	float smallestratio = 9999999;
	//std::list<Tet*>::iterator titrec=surf->tets2.end();

	Tet *tet = NULL;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Vec3f tri[3];
		tri[0] = (*tit)->neib[0]->pos;
		tri[1] = (*tit)->neib[1]->pos;
		tri[2] = (*tit)->neib[2]->pos;

		float a,b,c,s,areadraw,areaor;

		a = Magnitude(tri[0]-tri[1]);
		b = Magnitude(tri[1]-tri[2]);
		c = Magnitude(tri[2]-tri[0]);

		//(*tit)->edgedrawarea[0] = Magnitude( a );
		//(*tit)->edgedrawarea[1] = Magnitude( b );
		//(*tit)->edgedrawarea[2] = Magnitude( c );

		s = (a+b+c)/2.0f;

		areadraw = sqrt(s*(s-a)*(s-b)*(s-c));

		areadraw = fmax(a, fmax(b, c));
#if 0
		Vec2f orc[3];
		orc[0] = (*tit)->neib[0]->orc;
		orc[1] = (*tit)->neib[1]->orc;
		orc[2] = (*tit)->neib[2]->orc;
		
		a = Magnitude(orc[0]-orc[1]);
		b = Magnitude(orc[1]-orc[2]);
		c = Magnitude(orc[2]-orc[0]);
		
		s = (a+b+c)/2.0f;
#endif
		//areaor = sqrt(s*(s-a)*(s-b)*(s-c));
		areaor = 1;

		if(areadraw / areaor > smallestratio || areaor == 0 || ISNAN(areaor) )
		{
			//titrec = tit;
			tet = *tit;
			smallestratio = areadraw / areaor;
			if(ISNAN(smallestratio))
				smallestratio = 0;
		}
	}

	SurfPt *midp = NULL;

	if(tet)
	{
		int fate = -1;
		float bigd = 0;

		if((tet)->neib[0]->gone)
			ErrMess("gasd","gasd0");
		if((tet)->neib[1]->gone)
			ErrMess("gasd","gasd1");
		if((tet)->neib[2]->gone)
			ErrMess("gasd","gasd2");

		for(int v=0; v<3; ++v)
		{
			float d = Magnitude((tet)->neib[v]->pos - (tet)->neib[(v+1)%3]->pos);

			if(d > bigd || fate < 0)
			{
				bigd=d;
				fate=(v+2)%3;
			}
		}

		Tet *sharet = NULL;
		int sharefate = -1;
		SurfPt *p1 = tet->neib[(fate+1)%3];
		SurfPt *p2 = tet->neib[(fate+2)%3];

		for(std::list<Tet*>::iterator sharetit=surf->tets2.begin();
			sharetit!=surf->tets2.end();
			++sharetit)
		{
			if( (*sharetit) == tet)
				continue;

			for(int v=0; v<3; v++)
			{
				if(// ((*sharetit)->neib[v] == p1 &&
					//(*sharetit)->neib[(v+1)%3] == p2) ||
					 ((*sharetit)->neib[v] == p2 &&
					(*sharetit)->neib[(v+1)%3] == p1) )
				{
					sharet = *sharetit;
					sharefate = (v+2)%3;
					goto share;
				}
			}
		}

		ErrMess("me","missedge");

		return true;

share:

		if(sharet->neib[0]->gone)
			ErrMess("gasdasd","gg0");
		if(sharet->neib[1]->gone)
			ErrMess("gasdasd","gg1");
		if(sharet->neib[2]->gone)
			ErrMess("gasdasd","gg2");

		Tet *sharet2=NULL, *tet2=NULL;

		SplitEdge3(surf, tet,
		//titrec,
		&midp, //&midwp,
		fate//,&tet2
		);
		
		SplitEdge3(surf, sharet,
		//sharetit,
		&midp, //&midwp,
		sharefate//,&sharet2
		);

		
		//std::list<Tet*> toplace;
		//PlaceTet(surf, tet, &toplace, false);
		//PlaceTet(surf, tet2, &toplace, false);
		//PlaceTet(surf, sharet, &toplace, false);
		//PlaceTet(surf, sharet2, &toplace, false);

		//while(toplace.size())
		{
			//PlaceTet(surf, *toplace.begin(), &toplace, false);
			//toplace.erase(toplace.begin());
		}


		goto again;
	}

	return true;
}




/*
make triangles tesselated
based on WRAPpos edges, as they are added one by one(check)
*/
bool SplitEdges2(Surf *surf, Surf *fullsurf, Vec2f *vmin, Vec2f *vmax, float minlen)
{//return true;
	int times = 0;
again:

	//++times;

	//if(times > amt)
	//	return true;

	float smallestratio = 9999999;
	//std::list<Tet*>::iterator titrec=surf->tets2.end();

	Tet *tet = NULL;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		//if((*tit)->neib[0]->ring < 0)
		//	continue;
		//if((*tit)->neib[1]->ring < 0)
		//	continue;
		//if((*tit)->neib[2]->ring < 0)
		//	continue;

		Vec3f tri[3];
		tri[0] = (*tit)->neib[0]->pos;
		tri[1] = (*tit)->neib[1]->pos;
		tri[2] = (*tit)->neib[2]->pos;

		float a,b,c,s,areadraw,areaor;

		a = Magnitude(tri[0]-tri[1]);
		b = Magnitude(tri[1]-tri[2]);
		c = Magnitude(tri[2]-tri[0]);

		//(*tit)->edgedrawarea[0] = Magnitude( a );
		//(*tit)->edgedrawarea[1] = Magnitude( b );
		//(*tit)->edgedrawarea[2] = Magnitude( c );

		s = (a+b+c)/2.0f;

		areadraw = sqrt(s*(s-a)*(s-b)*(s-c));

		areadraw = fmax(a, fmax(b, c));
#if 0
		Vec2f orc[3];
		orc[0] = (*tit)->neib[0]->orc;
		orc[1] = (*tit)->neib[1]->orc;
		orc[2] = (*tit)->neib[2]->orc;
		
		a = Magnitude(orc[0]-orc[1]);
		b = Magnitude(orc[1]-orc[2]);
		c = Magnitude(orc[2]-orc[0]);
		
		s = (a+b+c)/2.0f;
#endif
		//areaor = sqrt(s*(s-a)*(s-b)*(s-c));
		areaor = 1;

		if( (areadraw / areaor > smallestratio || areaor == 0 || ISNAN(areaor))
			&& areadraw / areaor >= minlen)
		{
			//titrec = tit;
			tet = *tit;
			smallestratio = areadraw / areaor;
			if(ISNAN(smallestratio))
				smallestratio = 0;
		}
	}

	SurfPt *midp = NULL;

	if(tet)
	{
		int fate = -1;
		float bigd = 0;

		if((tet)->neib[0]->gone)
			ErrMess("gasd","gasd0");
		if((tet)->neib[1]->gone)
			ErrMess("gasd","gasd1");
		if((tet)->neib[2]->gone)
			ErrMess("gasd","gasd2");

		for(int v=0; v<3; ++v)
		{
			float d = Magnitude((tet)->neib[v]->pos - (tet)->neib[(v+1)%3]->pos);

			if(d > bigd || fate < 0)
			{
				bigd=d;
				fate=(v+2)%3;
			}
		}

		Tet *sharet = NULL;
		int sharefate = -1;
		SurfPt *p1 = tet->neib[(fate+1)%3];
		SurfPt *p2 = tet->neib[(fate+2)%3];

		for(std::list<Tet*>::iterator sharetit=surf->tets2.begin();
			sharetit!=surf->tets2.end();
			++sharetit)
		{
			if( (*sharetit) == tet)
				continue;

			for(int v=0; v<3; v++)
			{
				if(// ((*sharetit)->neib[v] == p1 &&
					//(*sharetit)->neib[(v+1)%3] == p2) ||
					 ((*sharetit)->neib[v] == p2 &&
					(*sharetit)->neib[(v+1)%3] == p1) )
				{
					sharet = *sharetit;
					sharefate = (v+2)%3;
					goto share;
				}
			}
		}

		ErrMess("me","missedge");

		return true;

share:

		if(sharet->neib[0]->gone)
			ErrMess("gasdasd","gg0");
		if(sharet->neib[1]->gone)
			ErrMess("gasdasd","gg1");
		if(sharet->neib[2]->gone)
			ErrMess("gasdasd","gg2");

		Tet *sharet2=NULL, *tet2=NULL;

		SplitEdge3(surf, tet,
		//titrec,
		&midp, //&midwp,
		fate//,&tet2
		);
		
		SplitEdge3(surf, sharet,
		//sharetit,
		&midp, //&midwp,
		sharefate//,&sharet2
		);

		
		//std::list<Tet*> toplace;
		//PlaceTet(surf, tet, &toplace, false);
		//PlaceTet(surf, tet2, &toplace, false);
		//PlaceTet(surf, sharet, &toplace, false);
		//PlaceTet(surf, sharet2, &toplace, false);

		//while(toplace.size())
		{
			//PlaceTet(surf, *toplace.begin(), &toplace, false);
			//toplace.erase(toplace.begin());
		}


		goto again;
	}

	return true;
}

void UpdStrains(std::list<Tet*>* tets)
{
	for(std::list<Tet*>::iterator tit=tets->begin();
		tit!=tets->end();
		++tit)
	{
		Tet *tet = *tit;
		for(int fate=0; fate<3; ++fate)
		{
			int e1 = (fate+1)%3;
			int e2 = (fate+2)%3;

			Vec3f p1a[4];
			Vec3f p2a[4];

			p1a[0] = Vec3f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y, 0);
			p1a[1] = Vec3f(tet->neib[e1]->orc.x+1, tet->neib[e1]->orc.y, 0);
			p1a[2] = Vec3f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y+1, 0);
			p1a[3] = Vec3f(tet->neib[e1]->orc.x+1, tet->neib[e1]->orc.y+1, 0);
			p2a[0] = Vec3f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y, 0);
			p2a[1] = Vec3f(tet->neib[e2]->orc.x+1, tet->neib[e2]->orc.y, 0);
			p2a[2] = Vec3f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y+1, 0);
			p2a[3] = Vec3f(tet->neib[e2]->orc.x+1, tet->neib[e2]->orc.y+1, 0);
			Vec3f up = Vec3f(0,0,-1);

			int p1n = -1;
			int p2n = -1;
			float pd = 9999999;

			for(int p1i=0; p1i<3; p1i++)
			{
				for(int p2i=0; p2i<3; p2i++)
				{
					float pdt = Magnitude(p1a[p1i] - p2a[p2i]);

					if(pdt < pd)
					{
						p1n = p1i;
						p2n = p2i;
						pd = pdt;
					}
				}
			}

			Vec3f p1 = p1a[p1n];
			Vec3f p2 = p2a[p2n];

			/////////////////
			//tet->edgedrawarea[fate] = Magnitude( tet->neib[fate]->pos - tet->neib[e1]->pos );
			//tet->edgeorarea[fate] = Magnitude( tet->neib[fate]->orc - tet->neib[e1]->orc );
			//tet->edgeorarea[fate] = Magnitude( p3 - p1 );

			tet->edgedrawarea[e1] = Magnitude( tet->neib[e1]->pos - tet->neib[e2]->pos );
			//tet->edgeorarea[e1] = Magnitude( tet->neib[e1]->orc - tet->neib[e2]->orc );
			tet->edgeorarea[e1] = Magnitude( p1 - p2 );
			/////////////////
		}
	}

	tets->clear();
}

Vec3f OrNorm(SurfPt *fatt[3], float *edgeposx, float *edgeposy, bool *edgesamex, bool *edgesamey)
{
#if 0
	Vec2f orc0a[4];
	Vec2f orc1a[4];
	Vec2f orc2a[4];
	orc0a[0] = Vec2f(fatt[0]->orc.x,fatt[0]->orc.y);
	orc0a[1] = Vec2f(fatt[0]->orc.x+1,fatt[0]->orc.y);
	orc0a[2] = Vec2f(fatt[0]->orc.x,fatt[0]->orc.y+1);
	orc0a[3] = Vec2f(fatt[0]->orc.x+1,fatt[0]->orc.y+1);
	orc1a[0] = Vec2f(fatt[1]->orc.x,fatt[1]->orc.y);
	orc1a[1] = Vec2f(fatt[1]->orc.x+1,fatt[1]->orc.y);
	orc1a[2] = Vec2f(fatt[1]->orc.x,fatt[1]->orc.y+1);
	orc1a[3] = Vec2f(fatt[1]->orc.x+1,fatt[1]->orc.y+1);
	orc2a[0] = Vec2f(fatt[2]->orc.x,fatt[2]->orc.y);
	orc2a[1] = Vec2f(fatt[2]->orc.x+1,fatt[2]->orc.y);
	orc2a[2] = Vec2f(fatt[2]->orc.x,fatt[2]->orc.y+1);
	orc2a[3] = Vec2f(fatt[2]->orc.x+1,fatt[2]->orc.y+1);
	float od0 = 9999999;
	float od1 = 9999999;
	float od2 = 9999999;
	int o0n = 0;
	int o1n = 0;
	int o2n = 0;

	for(int o0i=0; o0i<3; o0i++)
	{
		for(int o1i=0; o1i<3; o1i++)
		{
			for(int o2i=0; o2i<3; o2i++)
			{
				float odt0 = Magnitude(orc0a[o0i] - orc1a[o1i]);
				float odt1 = Magnitude(orc1a[o1i] - orc2a[o2i]);
				float odt2 = Magnitude(orc2a[o2i] - orc0a[o0i]);

				if(odt0 < od0 || odt1 < od1 || odt2 < od2)
				{
					od0 = odt0;
					od1 = odt1;
					od2 = odt2;
					o0n = o0i;
					o1n = o1i;
					o2n = o2i;
				}
			}
		}
	}

	Vec2f orcf[3];
	orcf[0] = orc0a[o0n] * 1;
	orcf[1] = orc1a[o1n] * 1;
	orcf[2] = orc2a[o2n] * 1;

#endif

#if 0
	Vec2f orcf[3];
	for(int v=0; v<3; v++)
	{
		orcf[v] = fatt[v]->orc;
		if(outx[v])
		{
			while(orcf[v].x < 1)
				orcf[v].x += 1;
		}
		
		if(outy[v])
		{
			while(orcf[v].y < 1)
				orcf[v].y += 1;
		}
	}
#endif

	Vec3f tri3[3];

	//for(int v=0; v<3; v++)
	//	tri3[v] = Vec3f(orcf[v].x, orcf[v].y, 0);

	Vec2f trueorc[3];

	trueorc[0] = fatt[0]->orc;
	trueorc[1] = fatt[1]->orc;
	trueorc[2] = fatt[2]->orc;

	for(int v=0; v<4; ++v)
	{
		TrueNextPt(trueorc[v%3], 
			edgeposx[v%3], edgesamex[v%3], edgeposy[v%3], edgesamey[v%3], 
			trueorc[(v+1)%3],
			&trueorc[(v+1)%3]);
	}

	for(int v=0; v<3; ++v)
	{
		tri3[v] = Vec3f(trueorc[v].x, trueorc[v].y, 0);
	}

	return Normal(tri3);
}

bool BalanceMesh(Surf *surf, Surf *fullsurf, Vec2f *vmin, Vec2f *vmax)
{
	//check for updating edge-areas
	std::list<Tet*> tocheck;

	std::list<Tet*>::iterator mosttit;
	Tet* mosttet = NULL;
	int mostvin = -1;
	SurfPt *sp[3] = {NULL,NULL,NULL};
	float moststrain = 0;

	int tries = 0;

again:

	moststrain = 0;
	sp[0] = NULL;
	sp[1] = NULL;
	sp[2] = NULL;
	mostvin = -1;
	mosttet = NULL;
	mosttit = surf->tets2.end();
	float *edgeposx[3], *edgeposy[3];
	bool *edgesamex[3], *edgesamey[3];
	bool *edgeplaced[3];

	bool updown = false;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		//Vec3f tri3[3];
		//for(int v=0; v<3; ++v)
		//	tri3[v] = Vec3f(tet->neib[v].x, tet->neib[v].y, 0);

		//Vec3f norm = Normal(tri3);
		Vec3f norm = OrNorm(tet->neib,
			tet->edgeposx,
			tet->edgeposy,
			tet->edgeplaced,
			tet->edgeplaced);

		//if(rand()%3!=1)
		//	continue;

		//for(int vin=0; vin<3; ++vin)
		for(int ein=0; ein<3; ++ein)
		{
			if(norm.z >= 0 && EdgeFront(surf, tet->neib[ein], tet->neib[(ein+1)%3], tet))
				continue;

			float strain = tet->edgedrawarea[ein] / tet->edgeorarea[ein];

			if(ISNAN(strain) || tet->edgeorarea[ein] == 0)
				strain = 1;

			if(strain > moststrain ||
				(!updown && norm.z >= 0) ||
				(updown && norm.z >= 0 && strain > moststrain))
			{
				if(norm.z >= 0)updown=true;
				moststrain = strain;
				sp[ein] = tet->neib[ein];
				sp[(ein+1)%3] = tet->neib[(ein+1)%3];
				sp[(ein+2)%3] = tet->neib[(ein+2)%3];

				for(int v=0; v<3; v++)
				{
					edgeposx[(ein+v)%3] = &tet->edgeposx[(ein+v)%3];
					edgeposy[(ein+v)%3] = &tet->edgeposy[(ein+v)%3];
					//edgesamex[(ein+v)%3] = &tet->edgeplaced[(ein+v)%3];
					//edgesamey[(ein+v)%3] = &tet->edgeplaced[(ein+v)%3];
					edgeplaced[(ein+v)%3] = &tet->edgeplaced[(ein+v)%3];
				}
				mostvin = ein;
				mosttet = tet;
				mosttit = tit;
			}
		}
	}

	if(mostvin >= 0 && sp[mostvin])	//edge to balance?
	{
		//if(updown)
		{

		}
		//else
		{
			float strain1 = 0.00001f;
			float strain2 = 0.00001f;

			std::list<Tet*> holders;
			
			for(std::list<Tet*>::iterator hit=sp[mostvin]->holder.begin();
				hit!=sp[mostvin]->holder.end();
				++hit)
				if(*hit != mosttet)
					holders.push_back(*hit);

			for(std::list<Tet*>::iterator hit=sp[(mostvin+1)%3]->holder.begin();
				hit!=sp[(mostvin+1)%3]->holder.end();
				++hit)
				if(*hit != mosttet)
					holders.push_back(*hit);

			for(std::list<Tet*>::iterator hit=holders.begin();
				hit!=holders.end();
				++hit)
			{
				if(*hit == mosttet)
					continue;

				for(int vin=0; vin<3; vin++)
				{
					bool a = ( (*hit)->neib[vin] == sp[0] );
					//bool b = (*hit)->neib[vin] == sp[1];
					//bool c = (*hit)->neib[(vin+1)%3] == sp[0];
					bool d = ( (*hit)->neib[(vin+1)%3] == sp[1] );
					
					float ch1 = (*hit)->edgedrawarea[vin] / (*hit)->edgeorarea[vin];
					float ch2 = (*hit)->edgedrawarea[(vin+2)%3] / (*hit)->edgeorarea[(vin+2)%3];

					if(ISNAN(ch1) || (*hit)->edgeorarea[vin] == 0)
						ch1 = 0.001;
					if(ISNAN(ch2) || (*hit)->edgeorarea[(vin+2)%3] == 0)
						ch2 = 0.001;

					//if( (a||b||c||d) && !((a||b)&&(c||d)) )
					if( (a) && !(a&&d) )
					{
						strain1 += ch1;
						strain2 += ch2;
					}
					if( (d) && !(a&&d) )
					{
						strain1 += ch2;
						strain2 += ch1;
					}

					if( (a||d) && !(a&&d) )
						tocheck.push_back(*hit);
				}
			}
			
			tocheck.push_back(mosttet);
		
			float strain1to2out = (moststrain/strain1)/(moststrain/strain2);

#if 0
			Vec2f orc0a[4];
			Vec2f orc1a[4];
			orc0a[0] = Vec2f(sp[0]->orc.x,sp[0]->orc.y);
			orc0a[1] = Vec2f(sp[0]->orc.x+1,sp[0]->orc.y);
			orc0a[2] = Vec2f(sp[0]->orc.x,sp[0]->orc.y+1);
			orc0a[3] = Vec2f(sp[0]->orc.x+1,sp[0]->orc.y+1);
			orc1a[0] = Vec2f(sp[1]->orc.x,sp[1]->orc.y);
			orc1a[1] = Vec2f(sp[1]->orc.x+1,sp[1]->orc.y);
			orc1a[2] = Vec2f(sp[1]->orc.x,sp[1]->orc.y+1);
			orc1a[3] = Vec2f(sp[1]->orc.x+1,sp[1]->orc.y+1);
			float od = 9999999;
			int o0n = 0;
			int o1n = 0;

			for(int o0i=0; o0i<3; o0i++)
			{
				for(int o1i=0; o1i<3; o1i++)
				{
					float odt = Magnitude(orc0a[o0i] - orc1a[o1i]);

					if(odt < od)
					{
						od = odt;
						o0n = o0i;
						o1n = o1i;
					}
				}
			}

			Vec2f orc0 = orc0a[o0n];
			Vec2f orc1 = orc1a[o1n];
#endif
#if 1
			Vec2f orc0 = sp[mostvin]->orc;
			Vec2f orc1 = sp[(mostvin+1)%3]->orc;

			TrueNextPt(sp[(mostvin+2)%3]->orc,
				*edgeposx[(mostvin+2)%3],
				*edgesamex[(mostvin+2)%3],
				*edgeposy[(mostvin+2)%3],
				*edgesamey[(mostvin+2)%3],
				orc0,
				&orc0);

			TrueNextPt(orc0,
				*edgeposx[(mostvin)%3],
				*edgesamex[(mostvin)%3],
				*edgeposy[(mostvin)%3],
				*edgesamey[(mostvin)%3],
				orc1,
				&orc1);

#endif
			//Vec2f out1dir = sp[0]->orc - sp[1]->orc;
			Vec2f out1dir = orc0 - orc1;
			out1dir = out1dir / Magnitude(out1dir);

			float strain1to2out2 = strain1/strain2;

			if(updown)
			{
				if(strain2>strain1)
					strain1to2out2*=-1;

				//float force = 0.1f * strain1to2out;
				orc0 = orc0 - out1dir * 0.1f;// * strain1to2out2;
				orc1 = orc1 + out1dir * 0.1f;// * strain1to2out2;
			}//reverse
			else
			{
				if(strain2>strain1)
					strain1to2out2*=-1;

				orc0 = orc0 + out1dir * 0.1f;// * strain1to2out2;
				orc1 = orc1 - out1dir * 0.1f;// * strain1to2out2;
			}

			ClassifyEdge(orc0, orc1,
				edgeposx[mostvin],
				edgeplaced[mostvin],
				edgeposy[mostvin]);

			//
		//	Vec2f orc0 = sp[mostvin]->orc;
		//	Vec2f orc1 = sp[(mostvin+1)%3]->orc;

			UpdEdges(surf,
				mosttet->neib[mostvin],
				mosttet->neib[(mostvin+1)%3],
				mosttet->edgeposx[mostvin],
				mosttet->edgeposy[mostvin],
				mosttet);
			//////////////

			ClassifyEdge(orc1, sp[(mostvin+2)%3]->orc,
				edgeposx[(mostvin+1)%3],
				edgeplaced[(mostvin+1)%3],
				edgeposy[(mostvin+1)%3]);
			
			UpdEdges(surf,
				mosttet->neib[(mostvin+1)%3],
				mosttet->neib[(mostvin+2)%3],
				mosttet->edgeposx[(mostvin+1)%3],
				mosttet->edgeposy[(mostvin+1)%3],
				mosttet);
			//////////////
			
			ClassifyEdge(sp[(mostvin+2)%3]->orc, orc0,
				edgeposx[(mostvin+2)%3],
				edgeplaced[(mostvin+2)%3],
				edgeposy[(mostvin+2)%3]);
			
			UpdEdges(surf,
				mosttet->neib[(mostvin+2)%3],
				mosttet->neib[(mostvin+3)%3],
				mosttet->edgeposx[(mostvin+2)%3],
				mosttet->edgeposy[(mostvin+2)%3],
				mosttet);
			//////////////

			if(ISNAN(orc0.x))
				orc0.x = 0;
			if(ISNAN(orc0.y))
				orc0.y = 0;
			if(ISNAN(orc1.x))
				orc1.x = 0;
			if(ISNAN(orc1.y))
				orc1.y = 0;

#if 011

			if(orc0.x < 0)
				orc0.x += 1;
			if(orc0.y < 0)
				orc0.y += 1;
			if(orc0.x > 1)
				orc0.x -= 1;
			if(orc0.y > 1)
				orc0.y -= 1;
			
			if(orc1.x < 0)
				orc1.x += 1;
			if(orc1.y < 0)
				orc1.y += 1;
			if(orc1.x > 1)
				orc1.x -= 1;
			if(orc1.y > 1)
				orc1.y -= 1;
#endif

			sp[mostvin]->orc = orc0;
			sp[(mostvin+1)%3]->orc = orc1;

			UpdStrains(&tocheck);

			tries++;
			//if(tries < BIGTEX*BIGTEX*BIGTEX)
			if(tries < surf->tets2.size()*3*20)
				goto again;
		}
	}

	return true;
}

#if 0
bool GrowMapMesh2(Surf *surf, Surf *fullsurf)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		(*tit)->neib[0]->orc.x = 0.5f;
		(*tit)->neib[0]->orc.y = 0.5f;

		(*tit)->neib[1]->orc.x = 0.52f;
		(*tit)->neib[1]->orc.y = 0.52f;

		(*tit)->neib[2]->orc.x = 0.5f;
		(*tit)->neib[2]->orc.y = 0.52f;
	}

	int times = 0;
	
again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Vec3f tri[3];
		tri[0] = (*tit)->neib[0]->pos;
		tri[1] = (*tit)->neib[1]->pos;
		tri[2] = (*tit)->neib[2]->pos;

		float a,b,c,s,area;

		a = Magnitude(tri[0]-tri[1]);
		b = Magnitude(tri[1]-tri[2]);
		c = Magnitude(tri[2]-tri[0]);

		(*tit)->edgedrawarea[0] = Magnitude( a );
		(*tit)->edgedrawarea[1] = Magnitude( b );
		(*tit)->edgedrawarea[2] = Magnitude( c );

		s = (a+b+c)/2.0f;

		area = sqrt(s*(s-a)*(s-b)*(s-c));

		(*tit)->drawarea = area;

		/////////////

		a = fabs( (*tit)->neib[0]->orc - (*tit)->neib[1]->orc );
		b = fabs( (*tit)->neib[1]->orc - (*tit)->neib[2]->orc );
		c = fabs( (*tit)->neib[2]->orc - (*tit)->neib[0]->orc );
		
		(*tit)->edgeorarea[0] = Magnitude( a );
		(*tit)->edgeorarea[1] = Magnitude( b );
		(*tit)->edgeorarea[2] = Magnitude( c );

		s = (a+b+c)/2.0f;

		area = sqrt(s*(s-a)*(s-b)*(s-c));

		(*tit)->ormaparea = area;
	}


	std::list<Tet*>::iterator titstrain=surf->tets2.begin();
	float moststrain = 0;
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		float invstrain = (*tit)->ormaparea / (*tit)->drawarea;
		if(mostinvstrain <= 0 ||
			invstrain < mostinvstrain)
		{
			titstrain = tit;
			mostinvstrain = invstrain;
		}
	}



	times++;

	if(times > 30000)
		return true;

	goto again;

	return false;
}
#endif

//is the angle with the edge p1-p2 facing correctly upward (clockwise)?
bool EdgeFront(Surf *surf, SurfPt* p1, SurfPt *p2, Tet *not)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		tit++)
	{
		Tet *tet = *tit;

		if(tet == not)
			continue;

		bool got[2] = {false,false};

		for(int v=0; v<3; v++)
		{
			if(tet->neib[v] == p1)
				got[0] = true;
			if(tet->neib[v] == p2)
				got[1] = true;
		}

		if(got[0] && got[1])
		{
			Vec3f ornorm = OrNorm(tet->neib,
				tet->edgeposx,
				tet->edgeposy,
				tet->edgeplaced,
				tet->edgeplaced);

			return (ornorm.z<0);
		}
	}

	return false;
}

float sign (Vec2f p1, Vec2f p2, Vec2f p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle (Vec2f pt, Vec2f v1, Vec2f v2, Vec2f v3)
{
	bool b1, b2, b3;

	b1 = sign(pt, v1, v2) < 0.0f;
	b2 = sign(pt, v2, v3) < 0.0f;
	b3 = sign(pt, v3, v1) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
}

void GenTexC(Vec2f &retexc,
			 Vec3f ir,
			 Vec2f *texc,
			 Vec3f *tri)
{
	// compute vectors
	//Vec2f v0 = tri[1] - tri[0], 
	//	v1 = tri[2] - tri[0],
	//	v2 = pout - tri[0];
	Vec3f v0 = tri[1] - tri[0],
		v1 = tri[2] - tri[0],
		v2 = ir - tri[0];

	// do bounds test for each position
	double f00 = Dot( v0, v0 );
	double f01 = Dot( v0, v1 );
	double f11 = Dot( v1, v1 );

	double f02 = Dot( v0, v2 );
	double f12 = Dot( v1, v2 );

	// Compute barycentric coordinates
	double invDenom = 1 / ( f00 * f11 - f01 * f01 );
	if(ISNAN(invDenom))
		invDenom = 1;
	double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
	double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

	// Check if point is in triangle
	//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
	//	goto dotex;
	//continue;

dotex:

	retexc = texc[0] * (1 - fU - fV) + 
		texc[1] * (fU) + 
		texc[2] * (fV);
}

bool TraceRay3(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV)
{
	bool re = false;
	int nearesti = -1;
	float nearestd = Magnitude(line[1]-line[0]);
	char neartype = 0;
	//Vec3f nearestv;
	//Vec3f nearnorm;

	//fprintf(g_applog, "ray%f,%f,%f->%f,%f,%f\r\n", 
	//	line[0].x, line[0].y, line[0].z,
	//	line[1].x, line[1].y, line[1].z);

//	fprintf(g_applog, "\r\nsta\r\n");
	

//	if(line[0].y < 0 && line[0].z > 0)
//	fprintf(g_applog, "\r\nstaaaaa\r\n");

	//int ci = 0;

	//Texture *retex, *retexs, *retexn;

	//unsigned char pr2,pg2,pb2,pa2,
	//	spr2,spg2,spb2,spa2,
	//	npr2,npg2,npb2,npa2;

	
				///if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!66!244","!2443!66");

//	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
	//	mit!=g_modelholder.end(); 
//		++mit, ++ci)

	Tet *itet = NULL;
	Vec3f ir = line[1];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		if(tet != *retet)
			continue;

		Vec3f tr[3];
		tr[0] = tet->neib[0]->wrappos;
		tr[1] = tet->neib[1]->wrappos;
		tr[2] = tet->neib[2]->wrappos;
		
		Vec3f tr2[3];
		tr2[1] = tet->neib[0]->wrappos;
		tr2[0] = tet->neib[1]->wrappos;
		tr2[2] = tet->neib[2]->wrappos;

		ir = line[1];

		if(IntersectedPolygon(tr, line, 3, &ir))
		{
			goto interc;
		}
		//if(IntersectedPolygon(tr2, line, 3, &ir))
		{
		//	goto interc;
		}

		continue;

interc:
		re = true;
		line[1] = ir;
		itet = tet;
	}

	if(itet)
	{
		Vec3f tr[3];
		tr[0] = itet->neib[0]->wrappos;
		tr[1] = itet->neib[1]->wrappos;
		tr[2] = itet->neib[2]->wrappos;
		Vec3f rtr[3];
		rtr[0] = itet->neib[0]->pos;
		rtr[1] = itet->neib[1]->pos;
		rtr[2] = itet->neib[2]->pos;

		Vec2f txc[3];

	CheckTet(itet, __FILE__, __LINE__);

		for(int v=0; v<3; v++)
		{
			GenTexC(txc[v],
				tr[v],
				itet->texc,
				itet->texcpos);
			//txc[v].x = 
				/*
				itet->neib[v]->pos.x * itet->texceq[0].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[0].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[0].m_normal.z +
				itet->texceq[0].m_d;*/
			
			//txc[v].y = 
			/*
				itet->neib[v]->pos.x * itet->texceq[1].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[1].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[1].m_normal.z +
				itet->texceq[1].m_d;
*/
			/*
			if(ISNAN(itet->neib[v]->pos.x))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.x");
			if(ISNAN(itet->neib[v]->pos.y))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.y");
			if(ISNAN(itet->neib[v]->pos.z))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.z");

			if(ISNAN(itet->texceq[0].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.x");
			if(ISNAN(itet->texceq[0].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.y");
			if(ISNAN(itet->texceq[0].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.z");
			if(ISNAN(itet->texceq[0].m_d))
				ErrMess("dsfgdfg", "itet->texceq[0].m_d");

			if(ISNAN(itet->texceq[1].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.x");
			if(ISNAN(itet->texceq[1].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.y");
			if(ISNAN(itet->texceq[1].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.z");
			if(ISNAN(itet->texceq[1].m_d))
				ErrMess("dsfgdfg", "itet->texceq[1].m_d");
*/

			if(ISNAN(txc[v].x))//2//
			{
				ErrMess("sgfs","isnan txc v x2");
/*
				char mm[1234];
				sprintf(mm,
					"%f=%f*%f+\r\n%f*%f+\r\n%f*%f+\r\n%f",
					itet->texceq[0].m_normal.x, itet->neib[v]->pos.x,
					itet->texceq[0].m_normal.y, itet->neib[v]->pos.y,
					itet->texceq[0].m_normal.z, itet->neib[v]->pos.z,
					itet->texceq[0].m_d);
				ErrMess(mm,mm);
				*/
	CheckTet(itet, __FILE__, __LINE__);
			}
			if(ISNAN(txc[v].y))
				ErrMess("sgfs","isnan txc v y");
		}

		Vec3f v0 = tr[1] - tr[0],
			v1 = tr[2] - tr[0],
			v2 = ir - tr[0];

		// do bounds test for each position
		double f00 = Dot( v0, v0 );
		double f01 = Dot( v0, v1 );
		double f11 = Dot( v1, v1 );

		double f02 = Dot( v0, v2 );
		double f12 = Dot( v1, v2 );

		// Compute barycentric coordinates
		double invDenom = 1 / ( f00 * f11 - f01 * f01 );
		if(ISNAN(invDenom))
			invDenom = 1;
		double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
		double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

		// Check if point is in triangle
		//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
		//	goto dotex;
		//continue;

//dotex:

		Vec2f txcf = txc[0] * (1 - fU - fV) + 
			txc[1] * (fU) + 
			txc[2] * (fV);

		*refU = fU;
		*refV = fV;

		if(ISNAN(fU))
			ErrMess("sgfdfg","fUnandfdfg");
		if(ISNAN(fV))
			ErrMess("sgfdfg","fVnandfdfg");

		if(ISNAN(txc[0].x))
			ErrMess("dfdfgsd","nantxcf.x[0]1");
		if(ISNAN(txc[0].y))
			ErrMess("dfdfgsd","nantxcf.y[0]1");
		
		if(ISNAN(txc[1].x))
			ErrMess("dfdfgsd","nantxcf.x[1]1");
		if(ISNAN(txc[1].y))
			ErrMess("dfdfgsd","nantxcf.y[1]1");
		
		if(ISNAN(txc[2].x))
			ErrMess("dfdfgsd","nantxcf.x[2]1");
		if(ISNAN(txc[2].y))
			ErrMess("dfdfgsd","nantxcf.y[2]1");

		LoadedTex *diff = itet->tex->pixels;

		*retex = diff;
		*rewp = ir;
		*rerp = rtr[0] * (1 - fU - fV) + 
			rtr[1] * (fU) + 
			rtr[2] * (fV);
		*retet = itet;

		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x1");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y1");

#if 01
		if(txcf.x >= 1)
			txcf.x -= 1;
		if(txcf.y >= 1)
			txcf.y -= 1;

		if(txcf.x < 0)
			txcf.x += 1;
		if(txcf.y < 0)
			txcf.y += 1;
#endif
		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y");

		*retexc = txcf;
		*ren = Normal(rtr);
	}

	return re;
}

bool TraceRay2(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV)
{
	bool re = false;
	int nearesti = -1;
	float nearestd = Magnitude(line[1]-line[0]);
	char neartype = 0;
	//Vec3f nearestv;
	//Vec3f nearnorm;

	//fprintf(g_applog, "ray%f,%f,%f->%f,%f,%f\r\n", 
	//	line[0].x, line[0].y, line[0].z,
	//	line[1].x, line[1].y, line[1].z);

//	fprintf(g_applog, "\r\nsta\r\n");
	

//	if(line[0].y < 0 && line[0].z > 0)
//	fprintf(g_applog, "\r\nstaaaaa\r\n");

	//int ci = 0;

	//Texture *retex, *retexs, *retexn;

	//unsigned char pr2,pg2,pb2,pa2,
	//	spr2,spg2,spb2,spa2,
	//	npr2,npg2,npb2,npa2;

	
				///if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!66!244","!2443!66");

//	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
	//	mit!=g_modelholder.end(); 
//		++mit, ++ci)

	Tet *itet = NULL;
	Vec3f ir = line[1];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		Vec3f tr[3];
		tr[0] = tet->neib[0]->wrappos;
		tr[1] = tet->neib[1]->wrappos;
		tr[2] = tet->neib[2]->wrappos;
		
		Vec3f tr2[3];
		tr2[1] = tet->neib[0]->wrappos;
		tr2[0] = tet->neib[1]->wrappos;
		tr2[2] = tet->neib[2]->wrappos;

		ir = line[1];

		if(IntersectedPolygon(tr, line, 3, &ir))
		{
			goto interc;
		}
		//if(IntersectedPolygon(tr2, line, 3, &ir))
		{
		//	goto interc;
		}

		continue;

interc:
		re = true;
		line[1] = ir;
		itet = tet;
	}

	if(itet)
	{
		Vec3f tr[3];
		tr[0] = itet->neib[0]->wrappos;
		tr[1] = itet->neib[1]->wrappos;
		tr[2] = itet->neib[2]->wrappos;
		Vec3f rtr[3];
		rtr[0] = itet->neib[0]->pos;
		rtr[1] = itet->neib[1]->pos;
		rtr[2] = itet->neib[2]->pos;

		Vec2f txc[3];
				CheckTet(itet, __FILE__, __LINE__);

		for(int v=0; v<3; v++)
		{
			GenTexC(txc[v],
				tr[v],
				itet->texc,
				itet->texcpos);
			//txc[v].x = 
				/*
				itet->neib[v]->pos.x * itet->texceq[0].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[0].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[0].m_normal.z +
				itet->texceq[0].m_d;*/
			
			//txc[v].y = 
			/*
				itet->neib[v]->pos.x * itet->texceq[1].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[1].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[1].m_normal.z +
				itet->texceq[1].m_d;
*/
			/*

			if(ISNAN(itet->neib[v]->pos.x))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.x");
			if(ISNAN(itet->neib[v]->pos.y))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.y");
			if(ISNAN(itet->neib[v]->pos.z))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.z");

			if(ISNAN(itet->texceq[0].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.x");
			if(ISNAN(itet->texceq[0].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.y");
			if(ISNAN(itet->texceq[0].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.z");
			if(ISNAN(itet->texceq[0].m_d))
				ErrMess("dsfgdfg", "itet->texceq[0].m_d");

			if(ISNAN(itet->texceq[1].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.x");
			if(ISNAN(itet->texceq[1].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.y");
			if(ISNAN(itet->texceq[1].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.z");
			if(ISNAN(itet->texceq[1].m_d))
				ErrMess("dsfgdfg", "itet->texceq[1].m_d");
*/
			if(ISNAN(txc[v].x))//1//
			{
				ErrMess("sgfs","isnan txc v x");
/*
				char mm[1234];
				sprintf(mm,
					"%f,\r\n%f,%f,%f\r\n%f,%f,%f,%f\r\n%f,%f,%f,%f",
					txc[v].x,
					itet->neib[v]->pos.x,itet->neib[v]->pos.y,itet->neib[v]->pos.z,
					itet->texceq[0].m_normal.x,
					itet->texceq[0].m_normal.y,
					itet->texceq[0].m_normal.z,
					itet->texceq[0].m_d,
					itet->texceq[1].m_normal.x,
					itet->texceq[1].m_normal.y,
					itet->texceq[1].m_normal.z,
					itet->texceq[1].m_d);
				ErrMess(mm,mm);
*/
				CheckTet(itet, __FILE__, __LINE__);
			}
			if(ISNAN(txc[v].y))
				ErrMess("sgfs","isnan txc v y");
		}

		Vec3f v0 = tr[1] - tr[0],
			v1 = tr[2] - tr[0],
			v2 = ir - tr[0];

		// do bounds test for each position
		double f00 = Dot( v0, v0 );
		double f01 = Dot( v0, v1 );
		double f11 = Dot( v1, v1 );

		double f02 = Dot( v0, v2 );
		double f12 = Dot( v1, v2 );

		// Compute barycentric coordinates
		double invDenom = 1 / ( f00 * f11 - f01 * f01 );
		if(ISNAN(invDenom))
			invDenom = 1;
		double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
		double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

		// Check if point is in triangle
		//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
		//	goto dotex;
		//continue;

//dotex:

		Vec2f txcf = txc[0] * (1 - fU - fV) + 
			txc[1] * (fU) + 
			txc[2] * (fV);

		*refU = fU;
		*refV = fV;

		if(ISNAN(fU))
			ErrMess("sgfdfg","fUnandfdfg");
		if(ISNAN(fV))
			ErrMess("sgfdfg","fVnandfdfg");

		if(ISNAN(txc[0].x))
			ErrMess("dfdfgsd","nantxcf.x[0]1");
		if(ISNAN(txc[0].y))
			ErrMess("dfdfgsd","nantxcf.y[0]1");
		
		if(ISNAN(txc[1].x))
			ErrMess("dfdfgsd","nantxcf.x[1]1");
		if(ISNAN(txc[1].y))
			ErrMess("dfdfgsd","nantxcf.y[1]1");
		
		if(ISNAN(txc[2].x))
			ErrMess("dfdfgsd","nantxcf.x[2]1");
		if(ISNAN(txc[2].y))
			ErrMess("dfdfgsd","nantxcf.y[2]1");

		LoadedTex *diff = itet->tex->pixels;

		*retex = diff;
		*rewp = ir;
		*rerp = rtr[0] * (1 - fU - fV) + 
			rtr[1] * (fU) + 
			rtr[2] * (fV);
		*retet = itet;

		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x1");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y1");

#if 01
		while(txcf.x >= 1)
			txcf.x -= 1;
		while(txcf.y >= 1)
			txcf.y -= 1;

		while(txcf.x < 0)
			txcf.x += 1;
		while(txcf.y < 0)
			txcf.y += 1;
#endif
		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y");

		*retexc = txcf;
		*ren = Normal(rtr);
	}

	return re;
}

//this uses neib->pos's instead of neib->wrappos's!
bool TraceRay4(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV)
{
	bool re = false;
	int nearesti = -1;
	float nearestd = Magnitude(line[1]-line[0]);
	char neartype = 0;
	//Vec3f nearestv;
	//Vec3f nearnorm;

	//fprintf(g_applog, "ray%f,%f,%f->%f,%f,%f\r\n", 
	//	line[0].x, line[0].y, line[0].z,
	//	line[1].x, line[1].y, line[1].z);

//	fprintf(g_applog, "\r\nsta\r\n");
	

//	if(line[0].y < 0 && line[0].z > 0)
//	fprintf(g_applog, "\r\nstaaaaa\r\n");

	//int ci = 0;

	//Texture *retex, *retexs, *retexn;

	//unsigned char pr2,pg2,pb2,pa2,
	//	spr2,spg2,spb2,spa2,
	//	npr2,npg2,npb2,npa2;

	
				///if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!66!244","!2443!66");

//	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
	//	mit!=g_modelholder.end(); 
//		++mit, ++ci)

	Tet *itet = NULL;
	Vec3f ir = line[1];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		//if(tet->ring < 0)
		//	continue;

		Vec3f tr[3];
		tr[0] = tet->neib[0]->pos;
		tr[1] = tet->neib[1]->pos;
		tr[2] = tet->neib[2]->pos;
		
		Vec3f tr2[3];
		tr2[1] = tet->neib[0]->pos;
		tr2[0] = tet->neib[1]->pos;
		tr2[2] = tet->neib[2]->pos;

		ir = line[1];

		if(IntersectedPolygon(tr, line, 3, &ir))
		{
			goto interc;
		}
		//if(IntersectedPolygon(tr2, line, 3, &ir))
		{
		//	goto interc;
		}

		continue;

interc:
		re = true;
		line[1] = ir;
		itet = tet;
	}

	if(itet)
	{
		Vec3f tr[3];
		tr[0] = itet->neib[0]->pos;
		tr[1] = itet->neib[1]->pos;
		tr[2] = itet->neib[2]->pos;
		Vec3f rtr[3];
		rtr[0] = itet->neib[0]->wrappos;
		rtr[1] = itet->neib[1]->wrappos;
		rtr[2] = itet->neib[2]->wrappos;

		Vec2f txc[3];
				CheckTet(itet, __FILE__, __LINE__);

		for(int v=0; v<3; v++)
		{
			GenTexC(txc[v],
				tr[v],
				itet->texc,
				itet->texcpos);
			//txc[v].x = 
				/*
				itet->neib[v]->pos.x * itet->texceq[0].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[0].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[0].m_normal.z +
				itet->texceq[0].m_d;*/
			
			//txc[v].y = 
			/*
				itet->neib[v]->pos.x * itet->texceq[1].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[1].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[1].m_normal.z +
				itet->texceq[1].m_d;
*/
			/*

			if(ISNAN(itet->neib[v]->pos.x))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.x");
			if(ISNAN(itet->neib[v]->pos.y))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.y");
			if(ISNAN(itet->neib[v]->pos.z))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.z");

			if(ISNAN(itet->texceq[0].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.x");
			if(ISNAN(itet->texceq[0].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.y");
			if(ISNAN(itet->texceq[0].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.z");
			if(ISNAN(itet->texceq[0].m_d))
				ErrMess("dsfgdfg", "itet->texceq[0].m_d");

			if(ISNAN(itet->texceq[1].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.x");
			if(ISNAN(itet->texceq[1].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.y");
			if(ISNAN(itet->texceq[1].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.z");
			if(ISNAN(itet->texceq[1].m_d))
				ErrMess("dsfgdfg", "itet->texceq[1].m_d");
*/
			if(ISNAN(txc[v].x))//1//
			{
				ErrMess("sgfs","isnan txc v x");
/*
				char mm[1234];
				sprintf(mm,
					"%f,\r\n%f,%f,%f\r\n%f,%f,%f,%f\r\n%f,%f,%f,%f",
					txc[v].x,
					itet->neib[v]->pos.x,itet->neib[v]->pos.y,itet->neib[v]->pos.z,
					itet->texceq[0].m_normal.x,
					itet->texceq[0].m_normal.y,
					itet->texceq[0].m_normal.z,
					itet->texceq[0].m_d,
					itet->texceq[1].m_normal.x,
					itet->texceq[1].m_normal.y,
					itet->texceq[1].m_normal.z,
					itet->texceq[1].m_d);
				ErrMess(mm,mm);
*/
				CheckTet(itet, __FILE__, __LINE__);
			}
			if(ISNAN(txc[v].y))
				ErrMess("sgfs","isnan txc v y");
		}

		Vec3f v0 = tr[1] - tr[0],
			v1 = tr[2] - tr[0],
			v2 = ir - tr[0];

		// do bounds test for each position
		double f00 = Dot( v0, v0 );
		double f01 = Dot( v0, v1 );
		double f11 = Dot( v1, v1 );

		double f02 = Dot( v0, v2 );
		double f12 = Dot( v1, v2 );

		// Compute barycentric coordinates
		double invDenom = 1 / ( f00 * f11 - f01 * f01 );
		if(ISNAN(invDenom))
			invDenom = 1;
		double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
		double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

		// Check if point is in triangle
		//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
		//	goto dotex;
		//continue;

//dotex:

		Vec2f txcf = txc[0] * (1 - fU - fV) + 
			txc[1] * (fU) + 
			txc[2] * (fV);

		*refU = fU;
		*refV = fV;

		if(ISNAN(fU))
			ErrMess("sgfdfg","fUnandfdfg");
		if(ISNAN(fV))
			ErrMess("sgfdfg","fVnandfdfg");

		if(ISNAN(txc[0].x))
			ErrMess("dfdfgsd","nantxcf.x[0]1");
		if(ISNAN(txc[0].y))
			ErrMess("dfdfgsd","nantxcf.y[0]1");
		
		if(ISNAN(txc[1].x))
			ErrMess("dfdfgsd","nantxcf.x[1]1");
		if(ISNAN(txc[1].y))
			ErrMess("dfdfgsd","nantxcf.y[1]1");
		
		if(ISNAN(txc[2].x))
			ErrMess("dfdfgsd","nantxcf.x[2]1");
		if(ISNAN(txc[2].y))
			ErrMess("dfdfgsd","nantxcf.y[2]1");

		LoadedTex *diff = itet->tex->pixels;

		*retex = diff;
		*rerp = ir;
		*rewp = rtr[0] * (1 - fU - fV) + 
			rtr[1] * (fU) + 
			rtr[2] * (fV);
		*retet = itet;

		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x1");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y1");

#if 01
		if(txcf.x >= 1)
			txcf.x -= 1;
		if(txcf.y >= 1)
			txcf.y -= 1;

		if(txcf.x < 0)
			txcf.x += 1;
		if(txcf.y < 0)
			txcf.y += 1;
#endif
		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y");

		*retexc = txcf;
		*ren = Normal(tr);
	}

	return re;
}

//this uses neib->pos's instead of neib->wrappos's!
//diagnostic func to determine why there is no intersection
bool TraceRay4b(Surf* surf,
			   Vec3f line[2], 
			  LoadedTex **retex,
			  LoadedTex **retexs,
			  LoadedTex **retexn,
			  Vec2f *retexc,
			  Vec3f *rewp, Vec3f *rerp, 
			  Vec3f* ren,
			  Tet **retet,
			  double *refU, double *refV)
{
	bool re = false;
	int nearesti = -1;
	float nearestd = Magnitude(line[1]-line[0]);
	char neartype = 0;
	//Vec3f nearestv;
	//Vec3f nearnorm;

	//fprintf(g_applog, "ray%f,%f,%f->%f,%f,%f\r\n", 
	//	line[0].x, line[0].y, line[0].z,
	//	line[1].x, line[1].y, line[1].z);

//	fprintf(g_applog, "\r\nsta\r\n");
	

//	if(line[0].y < 0 && line[0].z > 0)
//	fprintf(g_applog, "\r\nstaaaaa\r\n");

	//int ci = 0;

	//Texture *retex, *retexs, *retexn;

	//unsigned char pr2,pg2,pb2,pa2,
	//	spr2,spg2,spb2,spa2,
	//	npr2,npg2,npb2,npa2;

	
				///if(
				//	line[0] == Vec3f(0,-30000,30000))
				//		ErrMess("!66!244","!2443!66");

//	for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
	//	mit!=g_modelholder.end(); 
//		++mit, ++ci)

	Tet *itet = NULL;
	Vec3f ir = line[1];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		//if(tet->ring < 0)
		//	continue;

		Vec3f tr[3];
		tr[0] = tet->neib[0]->pos;
		tr[1] = tet->neib[1]->pos;
		tr[2] = tet->neib[2]->pos;
		
		Vec3f tr2[3];
		tr2[1] = tet->neib[0]->pos;
		tr2[0] = tet->neib[1]->pos;
		tr2[2] = tet->neib[2]->pos;

		ir = line[1];

		if( (line[0].x <= tr[0].x || line[0].x <= tr[1].x || line[0].x <= tr[2].x) &&
			(line[0].x >= tr[0].x || line[0].x >= tr[1].x || line[0].x >= tr[2].x) &&
			(line[0].z <= tr[0].z || line[0].z <= tr[1].z || line[0].z <= tr[2].z) &&
			(line[0].z >= tr[0].z || line[0].z >= tr[1].z || line[0].z >= tr[2].z) )
		{
			fprintf(g_applog, 
				"\r\n try tri \r\n"\
				"l[0]=%f,%f,%f \r\n"\
				"l[1]=%f,%f,%f \r\n"\
				"tr[0]=%f,%f,%f \r\n"\
				"tr[1]=%f,%f,%f \r\n"\
				"tr[2]=%f,%f,%f \r\n",
				line[0].x,
				line[0].y,
				line[0].z,
				line[1].x,
				line[1].y,
				line[1].z,
				tr[0].x,
				tr[0].y,
				tr[0].z,
				tr[1].x,
				tr[1].y,
				tr[1].z,
				tr[2].x,
				tr[2].y,
				tr[2].z
				);
			fflush(g_applog);
		}

		if(IntersectedPolygon(tr, line, 3, &ir))
		{
			fprintf(g_applog, "int@!\r\n");
			fflush(g_applog);
			goto interc;
		}
		
			fprintf(g_applog, "NOint@!\r\n");
			fflush(g_applog);


		if(IntersectedPolygon(tr2, line, 3, &ir))
		{
			fprintf(g_applog, "intinv@!\r\n");
			fflush(g_applog);
			continue;
		//	goto interc;
		}
		
			fprintf(g_applog, "NOintinv@!\r\n");
			fflush(g_applog);
		//if(IntersectedPolygon(tr2, line, 3, &ir))
		{
		//	goto interc;
		}

		continue;

interc:
		re = true;
		line[1] = ir;
		itet = tet;
	}

	if(itet)
	{
		Vec3f tr[3];
		tr[0] = itet->neib[0]->pos;
		tr[1] = itet->neib[1]->pos;
		tr[2] = itet->neib[2]->pos;
		Vec3f rtr[3];
		rtr[0] = itet->neib[0]->wrappos;
		rtr[1] = itet->neib[1]->wrappos;
		rtr[2] = itet->neib[2]->wrappos;

		Vec2f txc[3];
				CheckTet(itet, __FILE__, __LINE__);

		for(int v=0; v<3; v++)
		{
			GenTexC(txc[v],
				tr[v],
				itet->texc,
				itet->texcpos);
			//txc[v].x = 
				/*
				itet->neib[v]->pos.x * itet->texceq[0].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[0].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[0].m_normal.z +
				itet->texceq[0].m_d;*/
			
			//txc[v].y = 
			/*
				itet->neib[v]->pos.x * itet->texceq[1].m_normal.x +
				itet->neib[v]->pos.y * itet->texceq[1].m_normal.y +
				itet->neib[v]->pos.z * itet->texceq[1].m_normal.z +
				itet->texceq[1].m_d;
*/
			/*

			if(ISNAN(itet->neib[v]->pos.x))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.x");
			if(ISNAN(itet->neib[v]->pos.y))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.y");
			if(ISNAN(itet->neib[v]->pos.z))
				ErrMess("sfgdfg","is nan itet->neib[v]->pos.z");

			if(ISNAN(itet->texceq[0].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.x");
			if(ISNAN(itet->texceq[0].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.y");
			if(ISNAN(itet->texceq[0].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[0].m_normal.z");
			if(ISNAN(itet->texceq[0].m_d))
				ErrMess("dsfgdfg", "itet->texceq[0].m_d");

			if(ISNAN(itet->texceq[1].m_normal.x))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.x");
			if(ISNAN(itet->texceq[1].m_normal.y))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.y");
			if(ISNAN(itet->texceq[1].m_normal.z))
				ErrMess("dsfgdfg", "itet->texceq[1].m_normal.z");
			if(ISNAN(itet->texceq[1].m_d))
				ErrMess("dsfgdfg", "itet->texceq[1].m_d");
*/
			if(ISNAN(txc[v].x))//1//
			{
				ErrMess("sgfs","isnan txc v x");
/*
				char mm[1234];
				sprintf(mm,
					"%f,\r\n%f,%f,%f\r\n%f,%f,%f,%f\r\n%f,%f,%f,%f",
					txc[v].x,
					itet->neib[v]->pos.x,itet->neib[v]->pos.y,itet->neib[v]->pos.z,
					itet->texceq[0].m_normal.x,
					itet->texceq[0].m_normal.y,
					itet->texceq[0].m_normal.z,
					itet->texceq[0].m_d,
					itet->texceq[1].m_normal.x,
					itet->texceq[1].m_normal.y,
					itet->texceq[1].m_normal.z,
					itet->texceq[1].m_d);
				ErrMess(mm,mm);
*/
				CheckTet(itet, __FILE__, __LINE__);
			}
			if(ISNAN(txc[v].y))
				ErrMess("sgfs","isnan txc v y");
		}

		Vec3f v0 = tr[1] - tr[0],
			v1 = tr[2] - tr[0],
			v2 = ir - tr[0];

		// do bounds test for each position
		double f00 = Dot( v0, v0 );
		double f01 = Dot( v0, v1 );
		double f11 = Dot( v1, v1 );

		double f02 = Dot( v0, v2 );
		double f12 = Dot( v1, v2 );

		// Compute barycentric coordinates
		double invDenom = 1 / ( f00 * f11 - f01 * f01 );
		if(ISNAN(invDenom))
			invDenom = 1;
		double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
		double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

		// Check if point is in triangle
		//if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
		//	goto dotex;
		//continue;

//dotex:

		Vec2f txcf = txc[0] * (1 - fU - fV) + 
			txc[1] * (fU) + 
			txc[2] * (fV);

		*refU = fU;
		*refV = fV;

		if(ISNAN(fU))
			ErrMess("sgfdfg","fUnandfdfg");
		if(ISNAN(fV))
			ErrMess("sgfdfg","fVnandfdfg");

		if(ISNAN(txc[0].x))
			ErrMess("dfdfgsd","nantxcf.x[0]1");
		if(ISNAN(txc[0].y))
			ErrMess("dfdfgsd","nantxcf.y[0]1");
		
		if(ISNAN(txc[1].x))
			ErrMess("dfdfgsd","nantxcf.x[1]1");
		if(ISNAN(txc[1].y))
			ErrMess("dfdfgsd","nantxcf.y[1]1");
		
		if(ISNAN(txc[2].x))
			ErrMess("dfdfgsd","nantxcf.x[2]1");
		if(ISNAN(txc[2].y))
			ErrMess("dfdfgsd","nantxcf.y[2]1");

		LoadedTex *diff = itet->tex->pixels;

		*retex = diff;
		*rerp = ir;
		*rewp = rtr[0] * (1 - fU - fV) + 
			rtr[1] * (fU) + 
			rtr[2] * (fV);
		*retet = itet;

		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x1");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y1");

#if 01
		if(txcf.x >= 1)
			txcf.x -= 1;
		if(txcf.y >= 1)
			txcf.y -= 1;

		if(txcf.x < 0)
			txcf.x += 1;
		if(txcf.y < 0)
			txcf.y += 1;
#endif
		if(ISNAN(txcf.x))
			ErrMess("dfdfgsd","nantxcf.x");
		if(ISNAN(txcf.y))
			ErrMess("dfdfgsd","nantxcf.y");

		*retexc = txcf;
		*ren = Normal(tr);
	}

	return re;
}


//bounce rays with "globe" to get intersections and map those by long,lat to orientability map
void OutTex2(Surf *surf, LoadedTex* out)
{
#if 0
	int t=0;
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		LoadedTex ltex;
		AllocTex(&ltex, BIGTEX, BIGTEX, 3);


		for(int outpx=0; outpx<BIGTEX; outpx++)
		{
			for(int outpy=0; outpy<BIGTEX; outpy++)
			{
				Vec3f line[2];

				line[1] = Vec3f(0,0,0);
				line[0] = Vec3f(30000,0,0);

				float lat = M_PI*
					(float)outpy/(float)BIGTEX;
				line[0] = Rotate(line[0], lat, 0, 0, 1);
				//fprintf(g_applog, "prepos3 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
				float yaw = M_PI*
					(float)2.0f*(float)outpx/(float)BIGTEX;
				line[0] = Rotate(line[0], -yaw, 0, 1, 0);

				LoadedTex *tex=NULL, *ntex=NULL, *stex=NULL;
				Vec2f texc;
				Vec3f wp, rp, n;
				Tet *tet2 = tet;
				double fU, fV;

				if(TraceRay3(surf,
					line,
					&tex,
					&stex,
					&ntex,
					&texc,
					&wp,&rp,
					&n,
					&tet2,
					&fU, &fV))
				{
					if(tet2!=tet)
						continue;

					unsigned char c[4];

					//texc.x = texc.x - (int)texc.x;
					//texc.y = texc.y - (int)texc.y;

					int intx = texc.x * tex->sizex;
					int inty = texc.y * tex->sizey;

					while(intx < 0)
						intx += tex->sizex;
					while(inty < 0)
						inty += tex->sizey;
					while(intx >= tex->sizex)
						intx -= tex->sizex;
					while(inty >= tex->sizey)
						inty -= tex->sizey;

					c[0] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 0 ];
					c[1] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 1 ];
					c[2] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 2 ];

					unsigned short vx = 30000 + rp.x;
					unsigned short vy = 30000 + rp.y;
					unsigned short vz = 30000 + rp.z;

					//	unsigned short vx = 30000 + wp.x;
					//		unsigned short vy = 30000 + wp.y;
					//		unsigned short vz = 30000 + wp.z;

					Vec3f trg = Vec3f(255,0,0) * (1 - fU - fV) + 
						Vec3f(0,255,0) * (fU) + 
						Vec3f(0,0,255) * (fV);

					c[0] = trg.x;
					c[1] = trg.y;
					c[2] = trg.z;

					ltex.data[ 3 * (outpx + outpy * ltex.sizex) + 0 ] = c[0];
					ltex.data[ 3 * (outpx + outpy * ltex.sizex) + 1 ] = c[1];
					ltex.data[ 3 * (outpx + outpy * ltex.sizex) + 2 ] = c[2];

					//*((unsigned short*)&(out[1].data[ 3 * (outpx + outpy * out[1].sizex) + 0 ])) = vx;
					//out[1].data[ 3 * (outpx + outpy * out[1].sizex) + 2 ] = 0;
				}
			}
		}

		char file[32];
		sprintf(file, "renders/outaa%d.png", t);
		char full[SPE_MAX_PATH+1];
		FullPath(file, full);
		SavePNG2(full, &ltex);
		++t;

	}
	return;
#endif

	float maxrad = 1;

	//maximum radius (for constructing the "billboard" plane quad)
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		for(int v=0; v<3; v++)
		{
			SurfPt *sp = tet->neib[v];

			if(Magnitude(sp->pos) >= maxrad)
				maxrad = Magnitude(sp->pos);
		}
	}

	//for good measure
	maxrad *= 2;

	char infopath[SPE_MAX_PATH+1];
	NameRender(infopath, -1);
	strcat(infopath, "_info.txt");
	FILE* infofp = fopen(infopath, "w");
	fprintf(infofp, "maxrad %f\r\n", maxrad);
	if(infofp)
		fclose(infofp);

	//islands / jump lookup map
	for(int orlon=0; orlon<g_orlons; orlon++)
	{
		for(int orlat=0; orlat<g_orlats; orlat++)
		{
			Vec3f up = Vec3f(0,1,0);
			Vec3f side = Vec3f(1,0,0);
			Vec3f view = Vec3f(0,0,-1);

			up = Rotate(up, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);
			side = Rotate(side, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);
			view = Rotate(view, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);

			up = Rotate(up, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);
			side = Rotate(side, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);
			view = Rotate(view, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);

			for(int orxpx=0; orxpx<g_orwpx; orxpx++)
			{
				for(int orypx=0; orypx<g_orhpx; orypx++)
				{
#if 0
					int tabi = orxpx + 
						orypx * g_orwpx +
						orlat * g_orwpx * g_orhpx + 
						orlon * g_orwpx * g_orhpx * g_orlats;
					
					int tabx = tabi % (g_orwpx * g_orlons);
					int tabx = tabi % (g_orwpx * g_orlons);
#endif

					//jump table (islands) index coords
					int tabx = orxpx + orlon * g_orwpx;
					int taby = orypx + orlat * g_orhpx;

					//set to "nothing" first (no island known yet)
					out[1].data[ ( tabx + taby * out[1].sizex ) * 3 + 0] = 0;
					out[1].data[ ( tabx + taby * out[1].sizex ) * 3 + 1] = 0;
					out[1].data[ ( tabx + taby * out[1].sizex ) * 3 + 2] = 0;

					//out[1].data
					Vec3f line[2];

					line[0] = up * -1 * maxrad + side * -1 * maxrad + 
						up * (float)orypx / (float)g_orhpx * maxrad +
						side * (float)orxpx / (float)g_orwpx * maxrad;

					line[1] = line[0] +
						view * maxrad * 1.1;

					line[0] = line[0] - 
						view * maxrad * 1.1;
	
					LoadedTex *tex=NULL, *ntex=NULL, *stex=NULL;
					Vec2f texc;
					Vec3f wp, rp, n;
					Tet *tet;
					double fU, fV;

					//intersection by "pos" (real positions, not wrap positions of unwrap globe)
					if(TraceRay4(surf,
						line,
						&tex,
						&stex,
						&ntex,
						&texc,
						&wp,&rp,
						&n,
						&tet,
						&fU, &fV))
					{
						unsigned char c[4];

						//get wrappos map tex coord from pos intersection tet
						//and assign that "island's" coord index at the RG(B) at the (tabx,taby) table lookup

						//GenTexC(txc[v],tri3[v],
						//	tet->texc,tet->texcpos);

						SurfPt *sp[3];
						sp[0] = tet->neib[0];
						sp[1] = tet->neib[1];
						sp[2] = tet->neib[2];

						//Vec2f orc = sp[0]->orc * (1 - fU - fV) + 
						//	sp[1]->orc * (fU) + 
						//	sp[2]->orc * (fV);
						
						Vec2f orc = tet->texc[0] * (1 - fU - fV) + 
							tet->texc[1] * (fU) + 
							tet->texc[2] * (fV);

						while(orc.x < 0)
							orc.x += 1;
						while(orc.x >= 1)
							orc.x -= 1;
						while(orc.y < 0)
							orc.y += 1;
						while(orc.y >= 1)
							orc.y -= 1;

						//pixel index
						unsigned int orci = (g_bigtex-1) * orc.x + 
							g_bigtex * (g_bigtex-1) * orc.y;

						unsigned char *outorc = 
						(unsigned char*)&(out[1].data[ (tabx + taby * out[1].sizex) * 3 + 0]);

						//*outorc = orci;
						//assume little endian
						*(outorc+0) = *(((unsigned char*)&orci)+0);
						*(outorc+1) = *(((unsigned char*)&orci)+1);
						*(outorc+2) = *(((unsigned char*)&orci)+2);

						//out[1].data[ (tabx + taby * out[1].sizex) * 3 + 2] = 0;
					}
				}
			}
		}
	}
#if 0
			AllocTex(&outtex[0], g_bigtex, g_bigtex, 3);	//diffuse
			AllocTex(&outtex[1], g_orwpx * g_orlons, g_orhpx * g_orlats, 3);	//jump/islands map
			AllocTex(&outtex[2], g_bigtex, g_bigtex, 3);	//posx
			AllocTex(&outtex[3], g_bigtex, g_bigtex, 3);	//posy
			AllocTex(&outtex[4], g_bigtex, g_bigtex, 3);	//posz
#endif
	for(int outpx=0; outpx<g_bigtex; outpx++)
	{
		for(int outpy=0; outpy<g_bigtex; outpy++)
		{
			//bool focus = false;

			//if(outpx == 148 && 61 == outpy)
			//	focus = true;
			//if(outpx == 133 && 58 == outpy)
			//	focus = true;

			Vec3f line[2];

			line[1] = Vec3f(0,0,0);
			line[0] = Vec3f(30000,0,0);

			float lat = M_PI*
				(float)outpy/(float)BIGTEX;
			line[0] = Rotate(line[0], lat, 0, 0, 1);
			//fprintf(g_applog, "prepos3 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			float yaw = M_PI*
				(float)2.0f*(float)outpx/(float)BIGTEX;
			line[0] = Rotate(line[0], -yaw, 0, 1, 0);

			LoadedTex *tex=NULL, *ntex=NULL, *stex=NULL;
			Vec2f texc;
			Vec3f wp, rp, n;
			Tet *tet;
			double fU, fV;

			//collision by globe wrap pos
			if(TraceRay2(surf,
				line,
				&tex,
				&stex,
				&ntex,
				&texc,
				&wp,&rp,
				&n,
				&tet,
				&fU, &fV))
			{
				fprintf(g_applog, "\r\n out texc=%f,%f \r\n",
					texc.x,
					texc.y);
				fflush(g_applog);

				unsigned char c[4];

				//texc.x = texc.x - (int)texc.x;
				//texc.y = texc.y - (int)texc.y;

				int intx = texc.x * tex->sizex;
				int inty = texc.y * tex->sizey;

				while(intx < 0)
					intx += tex->sizex;
				while(inty < 0)
					inty += tex->sizey;
				while(intx >= tex->sizex)
					intx -= tex->sizex;
				while(inty >= tex->sizey)
					inty -= tex->sizey;

				c[0] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 0 ];
				c[1] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 1 ];
				c[2] = tex->data[ tex->channels * (intx + inty * tex->sizex) + 2 ];

				unsigned short vx = 30000 + rp.x;
				unsigned short vy = 30000 + rp.y;
				unsigned short vz = 30000 + rp.z;
				
			//	unsigned short vx = 30000 + wp.x;
		//		unsigned short vy = 30000 + wp.y;
		//		unsigned short vz = 30000 + wp.z;

//				Vec3f trg = Vec3f(255,0,0) * (1 - fU - fV) + 
//					Vec3f(0,255,0) * (fU) + 
//					Vec3f(0,0,255) * (fV);

	//			c[0] = trg.x;
	//			c[1] = trg.y;
	//			c[2] = trg.z;

				out[0].data[ 3 * (outpx + outpy * out[0].sizex) + 0 ] = c[0];
				out[0].data[ 3 * (outpx + outpy * out[0].sizex) + 1 ] = c[1];
				out[0].data[ 3 * (outpx + outpy * out[0].sizex) + 2 ] = c[2];
				
				*((unsigned short*)&(out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 0 ])) = vx;
				out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 2 ] = 0;
				
				*((unsigned short*)&(out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 0 ])) = vy;
				out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 2 ] = 0;
			
				*((unsigned short*)&(out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 0 ])) = vz;
				out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 2 ] = 0;
				
#if 0
			AllocTex(&outtex[0], g_bigtex, g_bigtex, 3);	//diffuse
			AllocTex(&outtex[1], g_orwpx * g_orlons, g_orhpx * g_orlats, 3);	//jump/islands map
			AllocTex(&outtex[2], g_bigtex, g_bigtex, 3);	//posx
			AllocTex(&outtex[3], g_bigtex, g_bigtex, 3);	//posy
			AllocTex(&outtex[4], g_bigtex, g_bigtex, 3);	//posz
#endif
/*
				if(focus)
				{
					fprintf(g_applog, "focus(%d,%d): rp:(%f,%f,%f) wp:(%f,%f,%f)\r\n",
						outpx, outpy,
						rp.x,rp.y,rp.z,
						wp.x,wp.y,wp.z);
					for(int vii=0; vii<4; ++vii)
					{
						if(!tet->neib[vii])
							continue;
						fprintf(g_applog, "tet->neib[%d] = \r\n\tpos=(%f,%f,%f)\r\n\twrap=(%f,%f,%f)\r\n",
							vii,
							tet->neib[vii]->pos.x,
							tet->neib[vii]->pos.y,
							tet->neib[vii]->pos.z,
							tet->neib[vii]->wrappos.x,
							tet->neib[vii]->wrappos.y,
							tet->neib[vii]->wrappos.z);
					}
					fflush(g_applog);
				}
*/
#if 0
				unsigned short vd = 30000 - Magnitude(rp);

				//if(vx == 30000)
			//		vx=0;
		//		if(vy == 30000)
	//				vy=0;
//				if(vz == 30000)
//					vz=0;
				
				*((unsigned short*)&(out[1].data[ 3 * (outpx + outpy * out[1].sizex) + 0 ])) = vx;
				out[1].data[ 3 * (outpx + outpy * out[1].sizex) + 2 ] = 0;
				
				*((unsigned short*)&(out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 0 ])) = vy;
				out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 2 ] = 0;
				
				*((unsigned short*)&(out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 0 ])) = vz;
				out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 2 ] = 0;

				float nx = n.x;
				float ny = n.y;
				float nz = n.z;
				
				out[5].data[ 3 * (outpx + outpy * out[5].sizex) + 0 ] = (unsigned char)(nx*127)+127;
				out[5].data[ 3 * (outpx + outpy * out[5].sizex) + 1 ] = (unsigned char)(ny*127)+127;
				out[5].data[ 3 * (outpx + outpy * out[5].sizex) + 2 ] = (unsigned char)(nz*127)+127;

				///*
			//	out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 0 ] = (unsigned char)((rp.x+1000/4+1)/(10));
			//	out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 1 ] = (unsigned char)((rp.y+1000/4+1)/(10));
			//	out[3].data[ 3 * (outpx + outpy * out[3].sizex) + 2 ] = (unsigned char)((rp.z+1000/4+1)/(10));
				
			//	out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 0 ] = (unsigned char)((wp.x+1000/4+1)/(10));
			//	out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 1 ] = (unsigned char)((wp.y+1000/4+1)/(10));
			//	out[2].data[ 3 * (outpx + outpy * out[2].sizex) + 2 ] = (unsigned char)((wp.z+1000/4+1)/(10));
//*/
				//*((unsigned short*)&(out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 0 ])) = vd;
				//out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 2 ] = 0;

				unsigned short hash = 1;

				unsigned int ti=0;

				for(std::list<Tet*>::iterator tit=surf->tets2.begin();
					tit!=surf->tets2.end();
					++tit)
				{
					if(*tit == tet)
						break;
					++ti;
				}

				struct PlayerColor
				{
					unsigned char color[3];
					char name[32];
				};

				#define PLAYER_COLORS	48

				//if(ti>=PLAYER_COLORS)
				//	ErrMess(">pc",">pc");

				//extern PlayerColor g_pycols[PLAYER_COLORS];

				PlayerColor g_pycols[PLAYER_COLORS] =
				{
					{{0x7e, 0x1e, 0x9c}, "Purple"},
					{{0x15, 0xb0, 0x1a}, "Green"},
					{{0x03, 0x43, 0xdf}, "Blue"},
					{{0xff, 0x81, 0xc0}, "Pink"},
					{{0x65, 0x37, 0x00}, "Brown"},
					{{0xe5, 0x00, 0x00}, "Red"},
					{{0x95, 0xd0, 0xfc}, "Light Blue"},
					{{0x02, 0x93, 0x86}, "Teal"},
					{{0xf9, 0x73, 0x06}, "Orange"},
					{{0x96, 0xf9, 0x7b}, "Light Green"},
					{{0xc2, 0x00, 0x78}, "Magenta"},
					{{0xff, 0xff, 0x14}, "Yellow"},
					{{0x75, 0xbb, 0xfd}, "Sky Blue"},
					{{0x92, 0x95, 0x91}, "Grey"},
					{{0x89, 0xfe, 0x05}, "Lime Green"},
					{{0xbf, 0x77, 0xf6}, "Light Purple"},
					{{0x9a, 0x0e, 0xea}, "Violet"},
					{{0x33, 0x35, 0x00}, "Dark Green"},
					{{0x06, 0xc2, 0xac}, "Turquoise"},
					{{0xc7, 0x9f, 0xef}, "Lavender"},
					{{0x00, 0x03, 0x5b}, "Dark Blue"},
					{{0xd1, 0xb2, 0x6f}, "Tan"},
					{{0x00, 0xff, 0xff}, "Cyan"},
					{{0x13, 0xea, 0xc9}, "Aqua"},
					{{0x06, 0x47, 0x0c}, "Forest Green"},
					{{0xae, 0x71, 0x81}, "Mauve"},
					{{0x35, 0x06, 0x3e}, "Dark Purple"},
					{{0x01, 0xff, 0x07}, "Bright Green"},
					{{0x65, 0x00, 0x21}, "Maroon"},
					{{0x6e, 0x75, 0x0e}, "Olive"},
					{{0xff, 0x79, 0x6c}, "Salmon"},
					{{0xe6, 0xda, 0xa6}, "Beige"},
					{{0x05, 0x04, 0xaa}, "Royal Blue"},
					{{0x00, 0x11, 0x46}, "Navy Blue"},
					{{0xce, 0xa2, 0xfd}, "Lilac"},
					{{0x00, 0x00, 0x00}, "Black"},
					{{0xff, 0x02, 0x8d}, "Hot Pink"},
					{{0xad, 0x81, 0x50}, "Light Brown"},
					{{0xc7, 0xfd, 0xb5}, "Pale Green"},
					{{0xff, 0xb0, 0x7c}, "Peach"},
					{{0x67, 0x7a, 0x04}, "Olive Green"},
					{{0xcb, 0x41, 0x6b}, "Dark Pink"},
					{{0x8e, 0x82, 0xfe}, "Periwinkle"},
					{{0x53, 0xfc, 0xa1}, "Sea Green"},
					{{0xaa, 0xff, 0x32}, "Lime"},
					{{0x38, 0x02, 0x82}, "Indigo"},
					{{0xce, 0xb3, 0x01}, "Mustard"},
					{{0xff, 0xd1, 0xdf}, "Light Pink"}
				};

				out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 0 ] = g_pycols[ti%PLAYER_COLORS].color[0];
				out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 1 ] = g_pycols[ti%PLAYER_COLORS].color[1];
				out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 2 ] = g_pycols[ti%PLAYER_COLORS].color[2];

				//for(int hti=0; hti<ti; hti++)
				//	hash = ((hash<<11)|(hash>>(16-11)))^(111+hti);
				
				//*((unsigned short*)&(out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 0 ])) = hash;
				//out[4].data[ 3 * (outpx + outpy * out[4].sizex) + 2 ] = 0;
#endif
			}
			else
			{
				//ErrMess("tr!~","tr!");
			}

		}
	}

	for(int t=0; t<6; ++t)
	{
		//char file[32];
		//sprintf(file, "renders/outaa%d.png", t);
		//char full[SPE_MAX_PATH+1];
		//FullPath(file, full);
		//SavePNG2(full, &out[t]);
	}
}

void OutTex(Surf *surf, LoadedTex out[6])
{
	int ti=0;
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		ti++;
		LoadedTex out1;
		LoadedTex out2;
		LoadedTex out3;
		LoadedTex out4;
		LoadedTex out5;
		AllocTex(&out1, BIGTEX, BIGTEX, 3);
		AllocTex(&out2, BIGTEX, BIGTEX, 3);
		AllocTex(&out3, BIGTEX, BIGTEX, 3);
		AllocTex(&out4, BIGTEX, BIGTEX, 3);
		AllocTex(&out5, BIGTEX, BIGTEX, 3);

		Tet* tet = *tit;
		Texture *diff = tet->tex;

		if(tet->gone)
			ErrMess("ssdf","tet->gone\r\n");

		if(!tet->placed)
			continue;

		Vec3f tri3[3];
		tri3[0] = tet->neib[0]->pos;
		tri3[1] = tet->neib[1]->pos;
		tri3[2] = tet->neib[2]->pos;

		//Vec2f txc[3];
		//txc[0] = tet->neib[0]->texc;
		//txc[1] = tet->neib[1]->texc;
		//txc[2] = tet->neib[2]->texc;
		Vec2f txc[3];
		for(int v=0; v<3; v++)
		{
			/*
			txc[v].x = tri3[v].x * tet->texceq[0].m_normal.x +
				tri3[v].y * tet->texceq[0].m_normal.y +
				tri3[v].z * tet->texceq[0].m_normal.z +
				tet->texceq[0].m_d ;
			txc[v].y = tri3[v].x * tet->texceq[1].m_normal.x +
				tri3[v].y * tet->texceq[1].m_normal.y +
				tri3[v].z * tet->texceq[1].m_normal.z +
				tet->texceq[1].m_d ;

			*/
			GenTexC(txc[v],tri3[v],
				tet->texc,tet->texcpos);
		}

		//Vec2f tri[3];
		//tri[0] = tet->neib[0]->orc * BIGTEX;
		//tri[1] = tet->neib[1]->orc * BIGTEX;
		//tri[2] = tet->neib[2]->orc * BIGTEX;

#if 0
		while(tri[0].x < 0 || tri[1].x < 0 || tri[2].x < 0)
		{
			tri[0].x += BIGTEX;
			tri[1].x += BIGTEX;
			tri[2].x += BIGTEX;
		}
		while(tri[0].y < 0 || tri[1].y < 0 || tri[2].y < 0)
		{
			tri[0].y += BIGTEX;
			tri[1].y += BIGTEX;
			tri[2].y += BIGTEX;
		}
#elif 0
		SurfPt *sp[3];
		sp[0] = tet->neib[0];
		sp[1] = tet->neib[1];
		sp[2] = tet->neib[2];

		
		
					fprintf(g_applog, "->neib=sp[0,1,2]->orc.xy=(%f,%f)\r\n(%f,%f)\r\n(%f,%f)\r\n",
						sp[0]->orc.x,sp[0]->orc.y,
						sp[1]->orc.x,sp[1]->orc.y,
						sp[2]->orc.x,sp[2]->orc.y);
					fflush(g_applog);

		Vec2f orc0a[4];
		Vec2f orc1a[4];
		Vec2f orc2a[4];
		orc0a[0] = Vec2f(sp[0]->orc.x,sp[0]->orc.y);
		orc0a[1] = Vec2f(sp[0]->orc.x+1,sp[0]->orc.y);
		orc0a[2] = Vec2f(sp[0]->orc.x,sp[0]->orc.y+1);
		orc0a[3] = Vec2f(sp[0]->orc.x+1,sp[0]->orc.y+1);
		orc1a[0] = Vec2f(sp[1]->orc.x,sp[1]->orc.y);
		orc1a[1] = Vec2f(sp[1]->orc.x+1,sp[1]->orc.y);
		orc1a[2] = Vec2f(sp[1]->orc.x,sp[1]->orc.y+1);
		orc1a[3] = Vec2f(sp[1]->orc.x+1,sp[1]->orc.y+1);
		orc2a[0] = Vec2f(sp[2]->orc.x,sp[2]->orc.y);
		orc2a[1] = Vec2f(sp[2]->orc.x+1,sp[2]->orc.y);
		orc2a[2] = Vec2f(sp[2]->orc.x,sp[2]->orc.y+1);
		orc2a[3] = Vec2f(sp[2]->orc.x+1,sp[2]->orc.y+1);
		float od0 = 9999999;
		float od1 = 9999999;
		float od2 = 9999999;
		int o0n = 0;
		int o1n = 0;
		int o2n = 0;

		for(int o0i=0; o0i<3; o0i++)
		{
			for(int o1i=0; o1i<3; o1i++)
			{
				for(int o2i=0; o2i<3; o2i++)
				{
					float odt0 = Magnitude(orc0a[o0i] - orc1a[o1i]);
					float odt1 = Magnitude(orc1a[o1i] - orc2a[o2i]);
					float odt2 = Magnitude(orc2a[o2i] - orc0a[o0i]);

					Vec3f tri31[3];

					tri31[0] = Vec3f(orc0a[o0i].x, orc0a[o0i].y, 0);
					tri31[1] = Vec3f(orc1a[o1i].x, orc1a[o1i].y, 0);
					tri31[2] = Vec3f(orc2a[o2i].x, orc2a[o2i].y, 0);

					Vec3f norm = Normal(tri31);

					if(norm.z >= 0)
						continue;

					if(odt0 < od0 || odt1 < od1 || odt2 < od2)
					{
						od0 = odt0;
						od1 = odt1;
						od2 = odt2;
						o0n = o0i;
						o1n = o1i;
						o2n = o2i;
					}
				}
			}
		}

		Vec2f orcf[3];
		orcf[0] = orc0a[o0n] * 1;
		orcf[1] = orc1a[o1n] * 1;
		orcf[2] = orc2a[o2n] * 1;

					fprintf(g_applog, "orcf[0,1,2].xy=(%d,%d,%d)=\r\n(%f,%f)\r\n(%f,%f)\r\n(%f,%f)\r\n",
						o0n,o1n,o2n,
						orcf[0].x,orcf[0].y,
						orcf[1].x,orcf[1].y,
						orcf[2].x,orcf[2].y);
					fflush(g_applog);

#if 01
		while(orcf[0].x >= 1 &&
			orcf[1].x >= 1 &&
			orcf[2].x >= 1)
		{
			orcf[0].x -= 1;
			orcf[1].x -= 1;
			orcf[2].x -= 1;
		}
		while(orcf[0].y >= 1 &&
			orcf[1].y >= 1 &&
			orcf[2].y >= 1)
		{
			orcf[0].y -= 1;
			orcf[1].y -= 1;
			orcf[2].y -= 1;
		}

		while(orcf[0].x < 0 ||
			orcf[1].x < 0 ||
			orcf[2].x < 0)
		{
			orcf[0].x += 1;
			orcf[1].x += 1;
			orcf[2].x += 1;
		}
		while(orcf[0].y < 0 ||
			orcf[1].y < 0 ||
			orcf[2].y < 0)
		{
			orcf[0].y += 1;
			orcf[1].y += 1;
			orcf[2].y += 1;
		}

#if 0
		for(int v=0; v<3; v++)
		{
			while(orcf[v].x >= 1)
			{
				orcf[v].x -= 1;
			}
			
			while(orcf[v].x < 0)
			{
				orcf[v].x += 1;
			}
		}
#endif
#endif
		
					fprintf(g_applog, "orcf[0,1,2].xy=(%f,%f)\r\n(%f,%f)\r\n(%f,%f)\r\n",
						orcf[0].x,orcf[0].y,
						orcf[1].x,orcf[1].y,
						orcf[2].x,orcf[2].y);
					fflush(g_applog);
#endif

		SurfPt *sp[3];
		sp[0] = tet->neib[0];
		sp[1] = tet->neib[1];
		sp[2] = tet->neib[2];

		Vec2f orcf[3];

		orcf[0] = sp[0]->orc;
		orcf[1] = sp[1]->orc;
		orcf[2] = sp[2]->orc;
/*
		for(int v=0; v<3; ++v)
		{
			fprintf(g_applog, "start pl%d,%d tet%d v%d = %f,%f  edge(pos)(x,y),(same)(x,y)=%f,%f,%d,%d\r\n",
				(int)tet->neib[v]->placed,
				(int)tet->placed,
				ti,
				v,
				orcf[v].x,
				orcf[v].y,
				(float)(tet->edgeposx[v]),
				(float)(tet->edgeposy[v]),
				(int)(tet->edgeplaced[v]),
				(int)(tet->edgeplaced[v]));
			fflush(g_applog);
		}
*/
		for(int v=0; v<3; v++)
		{
			TrueNextPt(orcf[v%3],
				tet->edgeposx[v%3],
				0,
				tet->edgeposy[v%3],
				0,
				orcf[(v+1)%3], &orcf[(v+1)%3]);
			/*
		//	for(int v=0; v<3; ++v)
			{
				fprintf(g_applog, "mid tet%d v%d = %f,%f  edge(pos)(x,y),(same)(x,y)=%f,%f,%d,%d\r\n",
					ti,
					v,
					orcf[v].x,
					orcf[v].y,
				(float)(tet->edgeposx[v]),
				(float)(tet->edgeposy[v]),
				(int)(tet->edgeplaced[v]),
				(int)(tet->edgeplaced[v]));
				fflush(g_applog);
			}*/
		}
		/*
		for(int v=0; v<3; ++v)
		{
			fprintf(g_applog, "final tet%d v%d = %f,%f  edge(pos)(x,y),(same)(x,y)=%f,%f,%d,%d\r\n",
				ti,
				v,
				orcf[v].x,
				orcf[v].y,
				(float)(tet->edgeposx[v]),
				(float)(tet->edgeposy[v]),
				(int)(tet->edgeplaced[v]),
				(int)(tet->edgeplaced[v]));
			fflush(g_applog);
		}*/

		//float fminz3 = fmin(tri3[0].z, fmin(tri3[1].z, tri3[2].z));
		//float fmaxz3 = fmax(tri3[0].z, fmax(tri3[1].z, tri3[2].z));
#if 0
		float fminxout = fmin(tri[0].x, fmin(tri[1].x, tri[2].x));
		float fmaxxout = fmax(tri[0].x, fmax(tri[1].x, tri[2].x));
		float fminyout = fmin(tri[0].y, fmin(tri[1].y, tri[2].y));
		float fmaxyout = fmax(tri[0].y, fmax(tri[1].y, tri[2].y));
#else
		float fminxout = fmin(orcf[0].x, fmin(orcf[1].x, orcf[2].x));
		float fmaxxout = fmax(orcf[0].x, fmax(orcf[1].x, orcf[2].x));
		float fminyout = fmin(orcf[0].y, fmin(orcf[1].y, orcf[2].y));
		float fmaxyout = fmax(orcf[0].y, fmax(orcf[1].y, orcf[2].y));
#endif

#if 0
		int iminx = (BIGTEX + fminx)%BIGTEX;
		int imaxx = (BIGTEX + fmaxx)%BIGTEX;
		int iminy = (BIGTEX + fminy)%BIGTEX;
		int imaxy = (BIGTEX + fmaxy)%BIGTEX;
#else
		//int iminxout = (int)(fminxout);
		//int imaxxout = (int)(fmaxxout);
		//int iminyout = (int)(fminyout);
		//int imaxyout = (int)(fmaxyout);
#if 0
		while(fminxout < 0)
			fminxout += 1;
		while(fminyout < 0)
			fminyout += 1;
		while(fmaxxout < fminxout)
			fmaxxout += 1;
		while(fmaxyout < fminyout)
			fmaxyout += 1;
#endif
#endif

		static int c=0;
		++c;

		fprintf(g_applog, "%d/%d\r\n", c, (int)surf->tets2.size());
		fflush(g_applog);

		float xch = 1.0f/BIGTEX;
		float ych = 1.0f/BIGTEX;

		Vec3f ornorm = OrNorm(tet->neib,
			tet->edgeposx,
			tet->edgeposy,
			tet->edgeplaced,
			tet->edgeplaced);

		if(ornorm.z >= 0)
		{
			//xch *= -1;
			//ych *= -1;

			//pxout=fmaxxout;
			//pyout=fmaxyout;
		}
		
		for(float pxout=fminxout; pxout <= fmaxxout /* && pxout <= fminxout+1 */; pxout+=xch)
		{

			for(float pyout=fminyout; pyout <= fmaxyout /* && pyout <= fminyout+1 */; pyout+=ych)
			{
				Vec2f pout = Vec2f(pxout,pyout);
			//	fprintf(g_applog, "pout=%f,%f\r\n", pout.x, pout.y);
			//	fflush(g_applog);
			//	Vec3f ir = 
				//if(!PointInTriangle(pout, tri[0], tri[1], tri[2]))
				//if(!PointInTriangle(pout, orcf[0], orcf[1], orcf[2]))
				{
					//fprintf(g_applog, "out\r\n");
					//fflush(g_applog);
				//	continue;
				}
					//fprintf(g_applog, "in\r\n");
					//fflush(g_applog);

				LoadedTex* difpx = diff->pixels;

				//Vec2f ratio = Vec2f(pxout,pyout)/(float)BIGTEX;

				// compute vectors
				//Vec2f v0 = tri[1] - tri[0], 
				//	v1 = tri[2] - tri[0],
				//	v2 = pout - tri[0];
				Vec2f v0 = orcf[1] - orcf[0],
					v1 = orcf[2] - orcf[0],
					v2 = pout - orcf[0];

				// do bounds test for each position
				double f00 = Dot( v0, v0 );
				double f01 = Dot( v0, v1 );
				double f11 = Dot( v1, v1 );

				double f02 = Dot( v0, v2 );
				double f12 = Dot( v1, v2 );

				// Compute barycentric coordinates
				double invDenom = 1 / ( f00 * f11 - f01 * f01 );
				if(ISNAN(invDenom))
					invDenom = 1;
				double fU = ( f11 * f02 - f01 * f12 ) * invDenom;
				double fV = ( f00 * f12 - f01 * f02 ) * invDenom;

				// Check if point is in triangle
				if( ( fU >= 0.0 ) && ( fV >= 0.0 ) && ( fU + fV <= 1.0 ) )
					goto dotex;
				continue;

dotex:

				Vec3f ff3 = tri3[0] * (1 - fU - fV) + 
					tri3[1] * (fU) + 
					tri3[2] * (fV);

				float ffU = txc[0].x * (1 - fU - fV) + 
					txc[1].x * (fU) + 
					txc[2].x * (fV);

				float ffV = txc[0].y * (1 - fU - fV) + 
					txc[1].y * (fU) + 
					txc[2].y * (fV);

				Vec4f color;

				Vec4f red = Vec4f(1,0,0,1);
				Vec4f green = Vec4f(0,1,0,1);
				Vec4f blue = Vec4f(0,0,1,1);
				
				color = red * (1 - fU - fV) + 
					green * (fU) + 
					blue * (fV);

				ffU = ffU - (int)ffU;
				ffV = ffV - (int)ffV;

				int intx = ffU * difpx->sizex;
				int inty = ffV * difpx->sizey;

				if(intx < 0)
					intx = (intx%difpx->sizex);
				if(inty < 0)
					inty = (inty%difpx->sizey);

				while(intx < 0)
					intx += difpx->sizex;
				while(intx >= difpx->sizex)
					intx %= difpx->sizex;
				while(inty < 0)
					inty += difpx->sizey;
				while(inty >= difpx->sizey)
					inty %= difpx->sizey;

				int pxin = (intx + difpx->sizex * inty);

				Vec4f rgba;

				rgba.x = difpx->data[ pxin * difpx->channels + 0 ];
				rgba.y = difpx->data[ pxin * difpx->channels + 1 ];
				rgba.z = difpx->data[ pxin * difpx->channels + 2 ];
				rgba.w = 255;

				int pxout2 = pxout * out->sizex;
				int pyout2 = pyout * out->sizey;
				
				while(pxout2 < 0)
					pxout2 += out->sizex;
				while(pxout2 >= out->sizex)
					pxout2 %= out->sizex;
				while(pyout2 < 0)
					pyout2 += out->sizey;
				while(pyout2 >= out->sizey)
					pyout2 %= out->sizey;

				rgba = color;

				out[0].data[ out[0].channels * ( pxout2 + pyout2 * BIGTEX ) + 0 ] = (int)rgba.x;
				out[0].data[ out[0].channels * ( pxout2 + pyout2 * BIGTEX ) + 1 ] = (int)rgba.y;
				out[0].data[ out[0].channels * ( pxout2 + pyout2 * BIGTEX ) + 2 ] = (int)rgba.z;

				LoadedTex *depthout = &out[1];
				LoadedTex *xout = &out[2];
				LoadedTex *yout = &out[3];
				LoadedTex *zout = &out[4];

				//unsigned short depthi = (unsigned short)Magnitude(ff3);
				//unsigned char depthb[2];
				//depthb[0] = *(((unsigned char*)&depthi)+0);
				//depthb[1] = *(((unsigned char*)&depthi)+1);

				*((unsigned short*)&depthout->data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ]) =
					(unsigned short)Magnitude(ff3);
				depthout->data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)0;

				//*((unsigned short*)&out1.data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ]) =
				//	(unsigned short)Magnitude(ff3);
				//out1.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)0;
				out1.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ] = (int)(color.x*255);
				out1.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 1 ] = (int)(color.y*255);
				out1.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)(color.z*255);


				*((unsigned short*)&xout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 0 ]) =
					(unsigned short)ff3.x + 30000;
				xout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 2 ] = (int)0;

				*((unsigned short*)&out2.data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ]) =
					(unsigned short)ff3.x + 30000;
				out2.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)0;

				

				*((unsigned short*)&yout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 0 ]) =
					(unsigned short)ff3.y + 30000;
				yout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 2 ] = (int)0;

				*((unsigned short*)&out3.data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ]) =
					(unsigned short)ff3.y + 30000;
				out3.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)0;

				
				*((unsigned short*)&zout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 0 ]) =
					(unsigned short)ff3.z + 30000;
				zout->data[ out->channels * ( pxout2 + pyout2 * out->sizex ) + 2 ] = (int)0;
				
				*((unsigned short*)&out4.data[ depthout->channels * ( pxout2 + pyout2 * depthout->sizex ) + 0 ]) =
					(unsigned short)ff3.z + 30000;
				out4.data[ out1.channels * ( pxout2 + pyout2 * depthout->sizex ) + 2 ] = (int)0;
			}
		}

		char full[SPE_MAX_PATH+1];

		char file[32];

		sprintf(file, "renders/tet%d_1.png", ti);
		FullPath(file, full);
		SavePNG2(full, &out1);
		
		sprintf(file, "renders/tet%d_2.png", ti);
		FullPath(file, full);
		//SavePNG2(full, &out2);
		
		sprintf(file, "renders/tet%d_3.png", ti);
		FullPath(file, full);
		//SavePNG2(full, &out3);
		
		sprintf(file, "renders/tet%d_4.png", ti);
		FullPath(file, full);
		//SavePNG2(full, &out4);
		
		sprintf(file, "renders/tet%d_5.png", ti);
		FullPath(file, full);
		//SavePNG2(full, &out5);
	}
}

void DrawClip()
{
	Surf *surf = &g_surf;
	Shader *s = &g_shader[g_curS];

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		tit++)
	{
		Tet *tet = *tit;
		Vec3f tri[3];
		tri[0] = tet->neib[0]->pos;
		tri[1] = tet->neib[1]->pos;
		tri[2] = tet->neib[2]->pos;

		Vec2f texc[3];
		Vec3f norm[3];

		for(int i=0; i<3; i++)
		{
			norm[i] = Normal(tri);
/*
			texc[i].x =
				tet->texceq[0].m_normal.x * tri[i].x +
				tet->texceq[0].m_normal.y * tri[i].y +
				tet->texceq[0].m_normal.z * tri[i].z +
				tet->texceq[0].m_d;
			
			texc[i].y =
				tet->texceq[1].m_normal.x * tri[i].x +
				tet->texceq[1].m_normal.y * tri[i].y +
				tet->texceq[1].m_normal.z * tri[i].z +
				tet->texceq[1].m_d;
				*/

			GenTexC(texc[i],tri[i],
				tet->texc,tet->texcpos);
		}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tet->tex->texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tet->tex->texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tet->tex->texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, tet->tex->texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);


		//const unsigned int numindices = m_Entries[i].NumIndices;
		//const unsigned int basevertex = m_Entries[i].BaseVertex;
		//const unsigned int baseindex = m_Entries[i].BaseIndex;
		//const unsigned int numunique = m_Entries[i].NumUniqueVerts;

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, (float*)&tri);
		glTexCoordPointer(2, GL_FLOAT, 0, (float*)&texc);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, (float*)&norm);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		//glDrawRangeElements(GL_TRIANGLES, 0, 3, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);

	}

	//UseS(SHADER_MAPPERSP);
}

void GetToEmerge(Surf *surf,
				 Tet *etet,
				 Vec3f *emline,
				 SurfPt **esp)
{
	*esp = NULL;

	int numem = 0;

	bool occluded = false;
	bool facingdown = false;

	//////////////////
	Vec3f tri[3];
	tri[0] = etet->neib[0]->wrappos;
	tri[1] = etet->neib[1]->wrappos;
	tri[2] = etet->neib[2]->wrappos;

	Vec3f norm = Normal(tri);

	float dot = Dot( norm, Normalize( (tri[0]+tri[1]+tri[2])/3.0f ) );

	if( dot <= 0.0f )
		facingdown = true;

	////////////////
	Vec3f line[3][2];

	line[0][0] = norm * 0.1f + tri[0];
	line[0][1] = norm * 60000.0f + tri[0];

	line[1][0] = norm * 0.1f + tri[1];
	line[1][1] = norm * 60000.0f + tri[1];

	line[2][0] = norm * 0.1f + tri[2];
	line[2][1] = norm * 60000.0f + tri[2];

	LoadedTex *retex, *retexn, *retexs;
	Vec2f retexc;
	Vec3f rewp, rerp, ren;
	Tet *retet;
	double refU, refV;

	//and no occluders?
#if 0
	if( TraceRay2(surf,
		line[0],
		&retex,
		&retexs,
		&retexn,
		&retexc,
		&rewp,
		&rerp,
		&ren,
		&retet,
		&refU, &refV) ||
		TraceRay2(surf,
		line[1],
		&retex,
		&retexs,
		&retexn,
		&retexc,
		&rewp,
		&rewp,
		&ren,
		&retet,
		&refU, &refV) ||
		TraceRay2(surf,
		line[2],
		&retex,
		&retexs,
		&retexn,
		&retexc,
		&rewp,
		&rewp,
		&ren,
		&retet,
		&refU, &refV))
	{
		occluded = true;
	}
#endif
////////////////////

	for(int vi=0; vi<3; ++vi)
	{
		if(etet->neib[vi]->emerged)
			numem++;
	}

	for(int vi=0; vi<3; ++vi)
	{
		if(!etet->neib[vi]->emerged)
		{
			//*esp = etet->neib[vi];
			break;
		}
	}

	if(numem == 1)
	{
		SurfPt* spe;
		SurfPt *dsp[2];
		int cursp = 0;

		Vec3f tri[3];

		for(int vi=0; vi<3; ++vi)
		{
			tri[vi] = etet->neib[vi]->wrappos;
			if(etet->neib[vi]->emerged)
				spe = etet->neib[vi];
			else
			{
				dsp[cursp] = etet->neib[vi];
				cursp++;
			}
		}

		float sign = 1;

		//if(facingdown)
		{
			sign = -1;
		}
		//else
		{
		//	return;
		}
		
		//Vec3f vdir = dsp[0]->wrappos - (spe->wrappos + dsp[1]->wrappos)/2.0f;
		//float cdist = Magnitude( (spe->wrappos + dsp[1]->wrappos)/2.0f );
		Vec3f vdir = (dsp[0]->wrappos + dsp[1]->wrappos)/2.0f - spe->wrappos;
		float cdist = Magnitude( spe->wrappos );
		//vdir = Normalize(vdir) //* cdist / 10000.0f
		//	;

		emline[0] = dsp[0]->wrappos;
		//emline[1] = Normalize ( vdir * sign + (spe->wrappos + dsp[1]->wrappos)/2.0f ) * cdist;
		//emline[1] = Normalize ( vdir * sign + spe->wrappos ) * cdist;
		Vec3f n = Normal(tri);
		//cdist = 1000;
		//emline[1] = Normalize( n ) * cdist + vdir;
		*esp = dsp[0];
		GetCen(surf, *esp, &emline[1]);
		emline[1] = Normalize(emline[1]) * cdist;
	}
	else if(numem == 2)
	{
		SurfPt* spe[2]={NULL,NULL};
		SurfPt *dsp;
		int cursp=0;

		Vec3f tri[3];
		
		for(int vi=0; vi<3; ++vi)
		{
			tri[vi] = etet->neib[vi]->wrappos;
			if(etet->neib[vi]->emerged)
			{
				spe[cursp] = etet->neib[vi];
				cursp++;
			}
			else
			{
				dsp = etet->neib[vi];
			}
		}

		float sign = 1;

		//if(facingdown)
		{
			sign = -1;
		}
		//else
		{
		//	return;
		}

		Vec3f vdir = dsp->wrappos - (spe[0]->wrappos + spe[1]->wrappos)/2.0f;
		float cdist = Magnitude( (spe[0]->wrappos + spe[1]->wrappos)/2.0f );
		//vdir = Normalize(vdir) //* cdist 
		//	/ 10000.0f
		//;

		emline[0] = dsp->wrappos;
		//emline[1] = Normalize ( vdir * sign + (spe[0]->wrappos + spe[1]->wrappos)/2.0f ) * cdist;
		Vec3f n = Normal(tri);
		//cdist = 1000;
		//emline[1] = Normalize( n ) * cdist + vdir;
		*esp = dsp;
		GetCen(surf, *esp, &emline[1]);
		emline[1] = Normalize(emline[1]) * cdist;
	}
	else
	{
		//error
		return;
	}
}

void GetCen(Surf *surf,
			SurfPt *sp,
			Vec3f *cen)
{
	float count = 1;
	*cen = sp->wrappos;

	for(std::list<Tet*>::iterator hit=sp->holder.begin();
		hit!=sp->holder.end();
		++hit)
	{
		Tet *tet = *hit;
		Vec3f tri[3];
		for(int v=0; v<3; ++v)
		{
			tri[v] = tet->neib[v]->wrappos;
			if(tet->neib[v] == sp)
				continue;
			//if(!tet->neib[v]->emerged)
			//	continue;
			//*cen = *cen * count / (count + 1) + tet->neib[v]->wrappos / (count + 1);
			//count += 1;
		}
		Vec3f n = Normal(tri);
		*cen = *cen * count / (count + 1) + n * 1000 / (count + 1);
		count += 1;
	}
	*cen = Normalize(*cen) * 1000;
}

void Emerge(Surf *surf,
			SurfPt *esp,
			Vec3f emline[2])
{
	for(std::list<SurfPt*>::iterator sit=surf->pts2.begin();
		sit!=surf->pts2.end();
		++sit)
	{
		SurfPt *sp = *sit;

		if(sp == esp)
			continue;

		Vec3f sidevec = Cross( Normalize(emline[1]), Normalize(sp->wrappos) );
		//Vec3f sidevec = Cross( Normalize(sp->wrappos), Normalize(emline[1]) );
		sidevec = Normalize( sidevec );
		//Vec3f dirmove = Cross( Normalize(sp->w=rappos), sidevec );
		
		float amt = (1.0f + Dot( Normalize(sp->wrappos), Normalize(emline[1]) ))/2.0f;

		sp->wrappos = Rotate(sp->wrappos, M_PI * amt / 2000.0f, sidevec.x, sidevec.y, sidevec.z);
	}

	esp->wrappos = emline[1];
}

void CheckEmerged(Surf *surf, Tet** halfemerged)
{
	*halfemerged = NULL;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		tet->neib[0]->emerged = true;
		tet->neib[1]->emerged = true;
		tet->neib[2]->emerged = true;
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		Vec3f tri[3];
		tri[0] = tet->neib[0]->wrappos;
		tri[1] = tet->neib[1]->wrappos;;
		tri[2] = tet->neib[2]->wrappos;;

		Vec3f norm = Normal(tri);

		//takes just 1 to be not-emerged pt of tets

		//facing up?
		if( Dot(norm, Normalize( (tri[0]+tri[1]+tri[2])/3.0f )) > 0.0f )
		{
			//tet->neib[0]
			continue;
		}

#if 0	//necessary?

		Vec3f line[3][2];

		line[0][0] = norm * 0.1f + tri[0];
		line[0][1] = norm * 60000.0f + tri[0];
		
		line[1][0] = norm * 0.1f + tri[1];
		line[1][1] = norm * 60000.0f + tri[1];
		
		line[2][0] = norm * 0.1f + tri[2];
		line[2][1] = norm * 60000.0f + tri[2];

		LoadedTex *retex, *retexn, *retexs;
		Vec2f retexc;
		Vec3f rewp, rerp, ren;
		Tet *retet;
		double refU, refV;

		//and no occluders?
		if( TraceRay2(surf,
			line[0],
			&retex,
			&retexs,
			&retexn,
			&retexc,
			&rewp,
			&rerp,
			&ren,
			&retet,
			&refU, &refV) ||
		TraceRay2(surf,
			line[1],
			&retex,
			&retexs,
			&retexn,
			&retexc,
			&rewp,
			&rewp,
			&ren,
			&retet,
			&refU, &refV) ||
		TraceRay2(surf,
			line[2],
			&retex,
			&retexs,
			&retexn,
			&retexc,
			&rewp,
			&rewp,
			&ren,
			&retet,
			&refU, &refV))
		{
			continue;
		}
#endif

		tet->neib[0]->emerged = false;
		tet->neib[1]->emerged = false;
		tet->neib[2]->emerged = false;
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		int emd = 0;

		for(int v=0; v<3; v++)
		{
			if(tet->neib[v]->emerged)
				emd++;
		}

		if(emd <= 2)
		{
			*halfemerged = tet;	//pick with 1 submerged

			if(rand()%3000==1)
				return;
		}

#if 0

		//find tet with some emerged and some not pt's
		if( !(tet->neib[0]->emerged &&
			tet->neib[1]->emerged &&
			tet->neib[2]->emerged) &&
			(tet->neib[0]->emerged ||
			tet->neib[1]->emerged ||
			tet->neib[2]->emerged) )
		{
			*halfemerged = tet;

			if(rand()%300==1)
				return;
		}
#endif
	}

	//no 1 sub?
	if(!*halfemerged)
	{	
		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet *tet = *tit;

			int emd = 0;

			for(int v=0; v<3; v++)
			{
				if(tet->neib[v]->emerged)
					emd++;
			}

			if(emd == 1)
			{
				*halfemerged = tet;	//pick with 2 submerged

				if(rand()%3000==1)
					return;
			}
		}
	}

	if(!*halfemerged)
	{	
		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet *tet = *tit;

			int emd = 0;

			for(int v=0; v<3; v++)
			{
				if(tet->neib[v]->emerged)
					emd++;
			}

			if(emd == 0)
			{
				*halfemerged = tet;	//pick with 2 submerged

				if(rand()%3000==1)
					return;
			}
		}
	}
}

//get the sideways order
void MapRing(std::list<SurfPt*> *rlist,
			 float *exts,
			 SurfPt *extstarts)
{
#if 0
	exts[0] = -1;
	exts[1] = -1;

	extstarts[0] = NULL;
	extstarts[1] = NULL;

	std::list<SurfPt*>::list nextfiles;
	bool changed = false;

	nextfiles.push_back( *rlist->begin() );

again:

	changed = false;

	//for(std::list<SurfPt*>::iterator pit=rlist->begin();
	//	pit!=rlist->end();
	//	++pit)
	for(std::list<SurfPt*>::iterator pit=nextfiles.begin();
		pit=nextfiles.begin() && pit!=nextfiles.end();
		)
	{
		SurfPt *ringneibs[9];
		SurfPt *ringpars[9];
		int ringneibc = 0;
		int ringparc = 0;
		SurfPt *p = *pit;

		nextfiles.erase(pit);

		if(p->file >= 0)
			continue;

		for(std::list<Tet*>::iterator hit=p->holder.begin();
			hit!=p->holder.end();
			++hit)
		{
			//get neibs
			//get parents and their sideways order

			Tet *het = *hit;

			for(int v=0; v<3; v++)
			{
				SurfPt *p2 = het->neib[v];

				if(p2 == p)
					continue;

				if(p2->ring == p->ring)
				{
					for(int ri=0; ri<ringneibc; ++ri)
					{
						if(ringneib[ri]==p2)
						{
							goto found;
						}
					}
						ringneib[ringneibc] = p2;
						ringneibc++;
found:
					;
				}
				else if(p2->ring == p->ring-1)
				{
					//parent
					for(int ri=0; ri<ringparc; ++ri)
					{
						if(ringpars[ri]==p2)
						{
							goto found2;
						}
					}
						ringpars[ringparc] = p2;
						ringparc++;
found2:
					;
				}
			}
		}

		if(!ringneibc)
			ErrMess("asdasd", "ringnc!");
		if(!ringparc)
			ErrMess("asdasd", "ringparc!");

		/////////////////////
		float *neibfiles[9];
		int nunqfiles = 0;
		for(int vi=0; vi<ringneibc; vi++)
		{
			if(ringneibs[vi]->file < 0)
				continue;
			for(int vi2=0; vi2<nunqfiles; vi2++)
			{
				if(ringneibs[vi]->file == *neibfiles[vi2])
					goto found3;
			}

			neibfiles[nunqfiles] = &ringneibs[vi]->file;
			nunqfiles++;

found3:
			;
		}
		float *parfiles[9];
		int nunqparfiles = 0;
		for(int vi=0; vi<ringparc; vi++)
		{
			if(ringpars[vi]->file < 0)
				continue;
			for(int vi2=0; vi2<nunqparfiles; vi2++)
			{
				if(ringpars[vi]->file == *parfiles[vi2])
					goto found4;
			}

			pariles[nunqparfiles] = &ringpars[vi]->file;
			nunqparfiles++;

found4:
			;
		}
		////////////////////////////

		if(nunqfiles >= 3)
		{
			float high = 0;
			for(int vi=0; vi<nunqfiles; vi++)
			{
				if(*neibfiles[vi] > high)
					high = *neibfiles[vi];
			}

			p->file = high + 1;
		}
		else if(nunqfiles == 2)
		{
			//if( abs( *neibfiles[0] - *neibfiles[1] ) >= 2 )
#if 0
			if( abs( *neibfiles[0] - *neibfiles[1] ) > 0.0f )
			{
				p->file = ( *neibfiles[0] + *neibfiles[1] ) / 2.0f;
			}
#endif

			float high = 0;
			for(int vi=0; vi<nunqfiles; vi++)
			{
				if(*neibfiles[vi] > high)
					high = *neibfiles[vi];
			}

			p->file = high + 1;
		}
		else if(nunqfiles == 1)
		{
			p->file = *neibfiles[0] + 1;

#if 0
			if(nunqparfiles == 0)
			{
				p->file = *neibfiles[0] + 1;
			}
			if(nunqparfiles == 1
#endif
		}
		else if(nunqfiles == 0)
		{
			p->file = 0;
		}

		for(int vi=0; vi<ringneibc; vi++)
		{
			nextfiles.push_back(ringneibs[vi]);
		}
	}

	//if(triedsearch && !changed)
	if(changed)
		goto again;
#endif
}

void GetExts(float *exts, Surf *surf, std::list<SurfPt*>* curring)
{
	//std::list<SurfPt*
}

void HuntCoords(Surf *surf)
{
	int ring = 0;
	std::list<SurfPt*> nextring;
	std::list<SurfPt*> curring;

	if(!surf->tets2.size())
		return;

	Tet *starttet = *surf->tets2.begin();
	
	SurfPt *startpt = starttet->neib[0];
	startpt->ring = ring;
	startpt->file = 0;
	//ring++;

	curring.push_back(startpt);

	float fileexts[2];

	do
	{
		fileexts[0] = 0;
		fileexts[1] = 0;

		nextring.clear();
		
		for(std::list<SurfPt*>::iterator pit=curring.begin();
			pit!=curring.end();
			++pit)
		{
			SurfPt *p = *pit;

			//get extents
		}
		
		for(std::list<SurfPt*>::iterator pit=curring.begin();
			pit!=curring.end();
			++pit)
		{
			SurfPt *p = *pit;
			//p->ring = ring;
			//p->file = -1;
		}

		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			tit++)
		{
			Tet *tet = *tit;

			if(tet->ring >= 0)
				continue;

			bool hasfree = false;
			bool hascurr = false;

			for(int v=0; v<3; v++)
			{
				if(tet->neib[v]->ring < 0)
					hasfree = true;
				if(tet->neib[v]->ring == ring)
					hascurr = true;
			}

			if(hasfree && hascurr)
			{
				tet->ring = ring;

				for(int v=0; v<3; v++)
				{
					if(tet->neib[v]->ring < 0)
					{
						nextring.push_back(tet->neib[v]);
					}
				}
			}
		}

		ring++;
		curring = nextring;
	}while(curring.size());

}

//case 1: jump down to sub-ring, have two parents at least
void DownEmerge()
{
}

//case 2: jump down to sub-ring from starting point, only one parent
void StartEmerge()
{
}

//walk around the ring's neighbour pt's and make sure the rules are always followed:
//1. always have 1 parent (ring#=ring-1)
//2. always have 1 hind neighbour (ring#=ring, file#=file-1)
//any tet's with strange mixtures of rings shouldn't exist, otherwise there's a non-triangle-fan non-flat
//configuration of connections/triangles with edges shared by more then 2 triangles
bool CheckCompleteRing(Surf *surf,
					   int ring)
{
	return true;////////////////////
	//shouldn't have any <-1 or >+1 pt's in contact with ring pt's
	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		SurfPt *p = *pit;

		if(p->ring != ring)
			continue;

		for(std::list<Tet*>::iterator hit=p->holder.begin();
			hit!=p->holder.end();
			++hit)
		{
			Tet *het = *hit;

			for(int v=0; v<3; v++)
			{
				SurfPt *p2 = het->neib[v];

				if(p2->ring < 0)
					continue;

				if(p2 == p)
					continue;

				if(p2->ring < ring-1)
				{
					//ErrMess("asdsd","c<-1");
					goto desc;
				}
				if(p2->ring > ring+1)
				{
					//ErrMess("asdasd","c>+1");
					goto desc;
				}

				continue;

desc:
				char mm[1234];
				sprintf(mm, 
					"ring=%d,%d,%d file=%d,%d,%d",
					het->neib[0]->ring,
					het->neib[1]->ring,
					het->neib[2]->ring,
					het->neib[0]->file,
					het->neib[1]->file,
					het->neib[2]->file);
				//ErrMess(mm,mm);
				return false;
			}
		}
	}

	return true;
}

/*
get axis-angle rotation to turn p1 to p2,
which is also the shortest path along the sphere
between p1 and p2.
*/
void ShortestPath(Vec3f p1, Vec3f p2, float *angle, Vec3f *axis)
{
	*angle = AngleBetweenVectors(p1, p2);
	*axis = Cross(p1, p2);

	if( Magnitude( p1 - Rotate(p2, *angle, axis->x, axis->y, axis->z) ) <
		Magnitude( p2 - Rotate(p1, *angle, axis->x, axis->y, axis->z) ))
	{
		*axis = Vec3f(0,0,0) - *axis;	//invert
	}
}

void ShortestPathSigned(Vec3f p1, Vec3f p2, float *angle, Vec3f *axis, bool *inv)
{
	*angle = AngleBetweenVectors(p1, p2);
	*axis = Cross(p1, p2);
	*inv = false;

#if 01
	if( Magnitude( p1 - Rotate(p2, *angle, axis->x, axis->y, axis->z) ) <
		Magnitude( p2 - Rotate(p1, *angle, axis->x, axis->y, axis->z) ))
	{
		*inv = true;
		*axis = Vec3f(0,0,0) - *axis;	//invert
	}
#endif
	//don't try to get absolute value here
	//this is for determining winding (clockwise/counter) using order of points of spherical triangle
}

/*
assuming there is an intersection between these two paths along the sphere,
get their intersecion.
*/
void PathIntersect(Vec3f start1, Vec3f start2, 
					Vec3f end1, Vec3f end2,
					float angle1, float angle2,
					Vec3f axis1, Vec3f axis2,
					float *subangle1, float *subangle2,
					Vec3f *subvec)
{
	/////////////////////
	float min1angle = 0;
	float min2angle = 0;
	float max1angle = angle1;
	float max2angle = angle2;
	Vec3f min1pt = start1;
	Vec3f min2pt = start2;
	Vec3f max1pt = end1;
	Vec3f max2pt = end2;

	while( abs(max1angle - min1angle) >= M_PI*2.0f/10000.0f || 
		abs(max2angle - min2angle) >= M_PI*2.0f/10000.0f)
	{
		float mid1angle = (min1angle + max1angle)/2.0f;
		float mid2angle = (min2angle + max2angle)/2.0f;

		Vec3f mid1pt = Rotate(start1, mid1angle, axis1.x, axis1.y, axis1.z);
		Vec3f mid2pt = Rotate(start2, mid2angle, axis2.x, axis2.y, axis2.z);

		Vec3f tri1low[3];
		Vec3f tri1high[3];
		
		Vec3f tri2low[3];
		Vec3f tri2high[3];

		tri1low[0] = Rotate(start1, min1angle, axis1.x, axis1.y, axis1.z);
		tri1low[1] = Rotate(start1, mid1angle, axis1.x, axis1.y, axis1.z);
		tri1low[2] = Normalize( (tri1low[0]+tri1low[1])/2.0f );
		
		tri1high[0] = Rotate(start1, mid1angle, axis1.x, axis1.y, axis1.z);
		tri1high[1] = Rotate(start1, max1angle, axis1.x, axis1.y, axis1.z);
		tri1high[2] = Normalize( (tri1high[0]+tri1high[1])/2.0f );

		/////////////
		
		tri2low[0] = Rotate(start2, min2angle, axis2.x, axis2.y, axis2.z);
		tri2low[1] = Rotate(start2, mid2angle, axis2.x, axis2.y, axis2.z);
		tri2low[2] = Normalize( (tri2low[0]+tri2low[1])/2.0f );
		
		tri2high[0] = Rotate(start2, mid2angle, axis2.x, axis2.y, axis2.z);
		tri2high[1] = Rotate(start2, max2angle, axis2.x, axis2.y, axis2.z);
		tri2high[2] = Normalize( (tri2high[0]+tri2high[1])/2.0f );
		
		/////////////

		if(TriTri(tri1low, tri2low))
		{
			max1angle = mid1angle;
			max2angle = mid2angle;
		}
		else if(TriTri(tri1low, tri2high))
		{
			max1angle = mid1angle;
			min2angle = mid2angle;
		}
		else if(TriTri(tri1high, tri2low))
		{
			min1angle = mid1angle;
			max2angle = mid2angle;
		}
		else if(TriTri(tri1high, tri2high))
		{
			min1angle = mid1angle;
			min2angle = mid2angle;
		}
		else
		{
			break;
		}
	}

	*subangle1 = (min1angle+max1angle)/2.0f;
	*subangle2 = (min2angle+max2angle)/2.0f;
	*subvec = Rotate(start1, *subangle1, axis1.x, axis1.y, axis1.z);
}

/*
mid-point along shortest path between p1 and p2 on sphere
*/
Vec3f SphMidPt(Vec3f p1, Vec3f p2)
{

}

/*
check for emerging upside-down facing triangle in wrap sphere
as pt's emerge
also check for irregular combinations of ring numbers in tets:
according to the theory/rules, there shouldn't be more ring numbers
in a tet than a parent (#-1) and a neighbour (#) number(s)
*/
bool TestE(Surf *surf,
		   Vec3f place,
		   //SurfPt *esp,
		   SurfPt *ig1,
		   SurfPt *ig2,
		   bool silent)
{
	return false;///////////////
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		if(tet->neib[0]->ring < 0)
			continue;
		if(tet->neib[1]->ring < 0)
			continue;
		if(tet->neib[2]->ring < 0)
			continue;

		bool weird = false;

		if( abs( tet->neib[0]->ring - tet->neib[1]->ring ) > 1 )
			weird = true;
		if( abs( tet->neib[1]->ring - tet->neib[2]->ring ) > 1 )
			weird = true;
		if( abs( tet->neib[0]->ring - tet->neib[2]->ring ) > 1 )
			weird = true;

		Vec3f teste[3];
		teste[0] = tet->neib[0]->wrappos;
		teste[1] = tet->neib[1]->wrappos;
		teste[2] = tet->neib[2]->wrappos;

		//if(!cw)
		{
		//	teste[0] = parpt->wrappos;
		//	teste[1] = frompt->wrappos;
		}

		Vec3f trinorm = Normal( teste );
		Vec3f cennorm = Normalize( (teste[0] + teste[1] + teste[2])/3.0f );

		//backwards norm?
		bool discw = (Dot(cennorm, trinorm) <= 0.0f);

		if(discw || weird)
		{
			Vec3f sidevec[3];
			sidevec[0] = Cross( Normalize(place), Normalize(tet->neib[0]->prevwrap) );
			sidevec[1] = Cross( Normalize(place), Normalize(tet->neib[1]->prevwrap) );
			sidevec[2] = Cross( Normalize(place), Normalize(tet->neib[2]->prevwrap) );
			//Vec3f sidevec = Cross( Normalize(sp->wrappos), Normalize(emline[1]) );
			sidevec[0] = Normalize( sidevec[0] );
			sidevec[1] = Normalize( sidevec[1] );
			sidevec[2] = Normalize( sidevec[2] );
			//Vec3f dirmove = Cross( Normalize(sp->w=rappos), sidevec );
			
			float amt[3];
			amt[0] = (1.0f + Dot( Normalize(tet->neib[0]->prevwrap), Normalize(place) ))/2.0f;
			amt[1] = (1.0f + Dot( Normalize(tet->neib[1]->prevwrap), Normalize(place) ))/2.0f;
			amt[2] = (1.0f + Dot( Normalize(tet->neib[2]->prevwrap), Normalize(place) ))/2.0f;

	#if 0
			amt = exp(amt);
			amt -= 1;
	#define E	(2.7182818284590452353602874713526624977572470937L)
			amt /= (E-1);
	#endif

		//sp->wrappos = Rotate(sp->wrappos, M_PI * amt / div, sidevec.x, sidevec.y, sidevec.z);

			if(!silent)
			{
				fprintf(g_applog,
					"\r\n discw=%d weird=%d \r\n"\
					" ig1==tetneib[0]=%d \r\n"
					" ig1==tetneib[1]=%d \r\n"
					" ig1==tetneib[2]=%d \r\n"
					" ig2==tetneib[0]=%d \r\n"
					" ig2==tetneib[1]=%d \r\n"
					" ig2==tetneib[2]=%d \r\n"
					//" (not in tet->) \r\n"
					//" teste ring=%d,%d,%d file=%d,%d,%d \r\n"
					" (<- in tet ->) \r\n"\
					" ring=%d,%d,%d file=%d,%d,%d \r\n"\
					" cennorm=%f,%f,%f \r\n"\
					" trinorm=%f,%f,%f \r\n"\
					" dot(cennorm, trinorm)=%f \r\n "\
					"test[0]=%f,%f,%f from=%f,%f,%f \r\n"\
					"test[1]=%f,%f,%f from=%f,%f,%f \r\n"\
					"test[2]=%f,%f,%f from=%f,%f,%f \r\n"\
					"rp[0]=%f,%f,%f \r\n"\
					"rp[1]=%f,%f,%f \r\n"\
					"rp[2]=%f,%f,%f \r\n"\
					"place=%f,%f,%f \r\n"\
					"sidevec0=%f,%f,%f amt=%f \r\n"\
					"sidevec1=%f,%f,%f amt=%f \r\n"\
					"sidevec2=%f,%f,%f amt=%f \r\n",

					(int)discw, (int)weird,

					(int)(ig1==tet->neib[0]),
					(int)(ig1==tet->neib[1]),
					(int)(ig1==tet->neib[2]),
					(int)(ig2==tet->neib[0]),
					(int)(ig2==tet->neib[1]),
					(int)(ig2==tet->neib[2]),

					//esp->ring,
					//ig1->ring,
					//ig2->ring,

					tet->neib[0]->ring,
					tet->neib[1]->ring,
					tet->neib[2]->ring,
					tet->neib[0]->file,
					tet->neib[1]->file,
					tet->neib[2]->file,

					cennorm.x,
					cennorm.y,
					cennorm.z,

					trinorm.x,
					trinorm.y,
					trinorm.z,

					Dot(cennorm, trinorm),

					teste[0].x,
					teste[0].y,
					teste[0].z,
					tet->neib[0]->prevwrap.x,
					tet->neib[0]->prevwrap.y,
					tet->neib[0]->prevwrap.z,
					
					teste[1].x,
					teste[1].y,
					teste[1].z,
					tet->neib[1]->prevwrap.x,
					tet->neib[1]->prevwrap.y,
					tet->neib[1]->prevwrap.z,
					
					teste[2].x,
					teste[2].y,
					teste[2].z,
					tet->neib[2]->prevwrap.x,
					tet->neib[2]->prevwrap.y,
					tet->neib[2]->prevwrap.z,

					tet->neib[0]->pos.x,
					tet->neib[0]->pos.y,
					tet->neib[0]->pos.z,

					tet->neib[1]->pos.x,
					tet->neib[1]->pos.y,
					tet->neib[1]->pos.z,

					tet->neib[2]->pos.x,
					tet->neib[2]->pos.y,
					tet->neib[2]->pos.z,

					place.x,
					place.y,
					place.z,
					
					sidevec[0].x,
					sidevec[0].y,
					sidevec[0].z,
					amt[0],
					
					sidevec[1].x,
					sidevec[1].y,
					sidevec[1].z,
					amt[1],
					
					sidevec[2].x,
					sidevec[2].y,
					sidevec[2].z,
					amt[2]);
				fflush(g_applog);
				ErrMess("pnn","pnn!2");
			}

			return false;
		}
	}

	return true;
}

//case 3: next point along ring, have previous neighbour and parent
void NextEmerge(Surf *surf, SurfPt *frompt, SurfPt *parpt, SurfPt *topt, bool cw, Vec3f *emerge,
				Vec3f *emerge2)
{
	//cw doesn't refer to the order of these pt arguments
	//but their arrangment in the tet
	//if cw=false then frompt,parpt actually go parpt,frompt

	static int call = 0;
	call++;
	
			if(parpt->gone)
				ErrMess("hgp","hgp");

	//make par->neib offset more important, so sideways less space is used in fan
	Vec3f along = parpt->wrappos - frompt->wrappos;
	Vec3f midp = (parpt->wrappos + frompt->wrappos)/2.0f;
	Vec3f midp2 = (parpt->wrappos*1.0f/11.0f + frompt->wrappos*10.0f/11.0f);
	midp2 = midp;
	Vec3f out = Normalize( midp );
	//along = Normalize( along );
	Vec3f cross = Cross( Normalize(along), out );

	if(ISNAN(midp.x))
		ErrMess("asdasd","isnanex2midp");
	if(ISNAN(midp.y))
		ErrMess("asdasd","isnaney2midp");
	if(ISNAN(midp.z))
		ErrMess("asdasd","isnanez2midp");

	if(ISNAN(along.x))
		ErrMess("asdasd","isnanex2along");
	if(ISNAN(along.y))
		ErrMess("asdasd","isnaney2along");
	if(ISNAN(along.z))
		ErrMess("asdasd","isnanez2along");

	if(ISNAN(out.x))
		ErrMess("asdasd","isnanex2out");
	if(ISNAN(out.y))
		ErrMess("asdasd","isnaney2out");
	if(ISNAN(out.z))
		ErrMess("asdasd","isnanez2out");

	if(ISNAN(cross.x))
		ErrMess("asdasd","isnanex2cross");
	if(ISNAN(cross.y))
		ErrMess("asdasd","isnaney2cross");
	if(ISNAN(cross.z))
		ErrMess("asdasd","isnanez2cross");

	Vec3f dir = Normalize( cross );

	if(!cw)
		dir = Vec3f(0,0,0) - dir;

	midp = Normalize( midp );
	midp2 = Normalize( midp2 );
	
	if(ISNAN(midp.x))
		ErrMess("asdasd","isnanex2");
	if(ISNAN(midp.y))
		ErrMess("asdasd","isnaney2");
	if(ISNAN(midp.z))
		ErrMess("asdasd","isnanez2");

	float len = Magnitude(along);
	//len = 1/12.0f;
	len = 10;
	float baselen = len;

	if(ISNAN(len))
	{
		ErrMess("asdasd","isnanez22");

		char mm[123];
		sprintf(mm, "len%f=%f,%f,%f",
			len,
			along.x,along.y,along.z);
		ErrMess("asd",mm);
	}

	//len = 1;
	//*emerge = midp * 1000 + dir * len * 1;
	//*emerge2 = midp * 1000 - dir * len * 1;
	*emerge = midp2 * 1000 + dir * len * 1;
	*emerge2 = midp2 * 1000 - dir * len * 1;

	Vec3f ldir[2];
	ldir[0] = frompt->wrappos - *emerge;
	ldir[1] = parpt->wrappos - *emerge;

	{
		Vec3f teste[3];
		teste[0] = frompt->wrappos;
		teste[1] = parpt->wrappos;
		teste[2] = *emerge;

		if(!cw)
		{
			teste[0] = parpt->wrappos;
			teste[1] = frompt->wrappos;
		}

		Vec3f trinorm = Normal( teste );
		Vec3f cennorm = Normalize( (teste[0] + teste[1] + teste[2])/3.0f );

		//backwards norm?
		bool discw = (Dot(cennorm, trinorm) <= 0.0f);

		if(discw)
			ErrMess("pnn","pnn!");
	}

	Vec3f ndir[2];
	ndir[0] = Normalize( ldir[0] );
	ndir[1] = Normalize( ldir[1] );

	Vec3f line[4][2];
	line[0][0] = frompt->wrappos + ndir[0] * 0.1f;
	line[0][1] = ( *emerge ) - ndir[0] * 0.1f;
	line[1][0] = parpt->wrappos + ndir[1] * 0.1f;
	line[1][1] = ( *emerge ) - ndir[1] * 0.1f;
	
	line[2][0] = frompt->wrappos + ndir[0] * 0.1f;
	line[2][1] = ( *emerge2 ) - ndir[0] * 0.1f;
	line[3][0] = parpt->wrappos + ndir[1] * 0.1f;
	line[3][1] = ( *emerge2 ) - ndir[1] * 0.1f;
	//Vec3f triline[2][2];
	//triline[0][0] = frompt->wrappos;
	//triline[0][1] = 

	Plane3f plane[3];
	Vec3f pup[3];
	Vec3f palong[3];
	Vec3f pnorm[3];

again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;
		int notsame = 0;

		for(int v=0; v<3; v++)
		{
			if(tet->neib[v]->ring < 0)
				goto next;
			if(tet->neib[v] == parpt)
				continue;
			if(tet->neib[v] == frompt)
				continue;
			if(tet->neib[v] == topt)
				//continue;
				goto next;

			notsame++;
		}

		if(notsame <= 0)
			continue;

///////////////////////////

#if 01	//exit//
		Vec3f test[3];
		test[0] = tet->neib[0]->wrappos;
		test[1] = tet->neib[1]->wrappos;
		test[2] = tet->neib[2]->wrappos;

#if 0
		Vec3f trinorm = Normal( test );
		Vec3f cennorm = Normalize( (tet->neib[0]->wrappos + tet->neib[1]->wrappos + tet->neib[2]->wrappos)/3.0f );

		//backwards norm?
		bool discw = (Dot(cennorm, trinorm) <= 0.0f);

		if(discw)
		{
			fprintf(g_applog,
				"\r\n trinorm=%f,%f,%f wp=(%f,%f,%f),(%f,%f,%f),(%f,%f,%f) "\
				"\r\n cennorm=%f,%f,%f ring=%d,%d,%d file=%d,%d,%d "\
				"\r\n dot=%f \r\n",
				trinorm.x,
				trinorm.y,
				trinorm.z,
				test[0].x,
				test[0].y,
				test[0].z,
				test[1].x,
				test[1].y,
				test[1].z,
				test[2].x,
				test[2].y,
				test[2].z,
				cennorm.x,
				cennorm.y,
				cennorm.z,
				tet->neib[0]->ring,
				tet->neib[1]->ring,
				tet->neib[2]->ring,
				tet->neib[0]->file,
				tet->neib[1]->file,
				tet->neib[2]->file,
				Dot(cennorm, trinorm));
			fflush(g_applog);
		}
#endif
		//if(discw)	//error
		//	ErrMess("asd","nn!");

		for(int v=0; v<3; v++)
		{
			int e1 = v;
			int e2 = (v+1)%3;
			Vec3f midp3 = (tet->neib[e1]->wrappos + tet->neib[e2]->wrappos)/2.0f;
			midp3 = Normalize(midp3);
			pup[v] = midp3;
			palong[v] = Normalize( tet->neib[e2]->wrappos - tet->neib[e1]->wrappos );
			pnorm[v] = Cross( palong[v], pup[v] );//cw
			pnorm[v] = Normalize( pnorm[v] );
			///if(discw)
			/////	pnorm[v] = Vec3f(0,0,0) - pnorm[v];
			Vec3f inoff = pnorm[v] * 0.4f;
			MakePlane(&plane[v].m_normal, &plane[v].m_d, tet->neib[e1]->wrappos + inoff, pnorm[v]);
			//bool discw = false;
		}
		///////////////////
		for(int vp=0; vp<3; vp++)
		//for(int vp=0; vp<1; vp++)
		{
			if(tet->neib[vp] == topt)
				continue;
			if(tet->neib[(vp+1)%3] == topt)
				continue;
			for(int vl=0; vl<2; vl++)
			{
				Vec3f i0;
				if(!LineInterPlane(line[vl], plane[vp].m_normal, plane[vp].m_d, &i0) //&&
				//	!LineInterPlane(line[vl*2+1], plane[vp].m_normal, plane[vp].m_d, &i0)
					)
					continue;
				Vec3f i1;
				if(!LineInterPlane(line[vl], plane[(vp+1)%3].m_normal, plane[(vp+1)%3].m_d, &i1) //&&
				//	!LineInterPlane(line[vl*2+1], plane[(vp+1)%3].m_normal, plane[vp].m_d, &i1)
					)
					continue;
				////
				if(PointOnOrBehindPlane(i0, plane[(vp+1)%3].m_normal, plane[(vp+1)%3].m_d))
					continue;
				if(PointOnOrBehindPlane(i1, plane[(vp+0)%3].m_normal, plane[(vp+0)%3].m_d))
					continue;
				////
				if(PointOnOrBehindPlane(i0, plane[(vp+2)%3].m_normal, plane[(vp+2)%3].m_d))
					continue;
				if(PointOnOrBehindPlane(i1, plane[(vp+2)%3].m_normal, plane[(vp+2)%3].m_d))
					continue;
				//Vec3f i2;
				//if(!LineInterPlane(line[vl], plane[(vp+2)%3].m_normal, plane[(vp+2)%3].m_d, &i2) //&&
				//	!LineInterPlane(line[vl*2+1], plane[(vp+1)%3].m_normal, plane[vp].m_d, &i1)
				//	)
				//	continue;
				//float indot = 
				//	Dot( Normalize(i0 - in[vl]->wrappos), Normalize( in[(vl+1)%3]->wrappos - in[vl]->wrappos ) );
				//float outdot = 
				//	Dot( Normalize(i1 - in[vl]->wrappos), Normalize( in[(vl+1)%3]->wrappos - in[vl]->wrappos ) );
#if 0
				char mm[1233];
				sprintf(mm, "\r\n call%d inov2 vp=%d vl=%d cw=%d notsame=%d \r\n "\
					"collider discw=%d ring=%d,%d,%d file=%d,%d,%d \r\n "\
					"n1: %f,%f,%f \r\n "\
					"n2: %f,%f,%f \r\n "\
					"n3: %f,%f,%f \r\n "\
					"ring=%d,%d,%d file=%d,%d,%d len=%f dir=(%f,%f,%f)\r\n"\
					"p1:(%f,%f,%f),%f\r\np2:(%f,%f,%f),%f\r\n p3:(%f,%f,%f),%f \r\n"\
					"l1:(%f,%f,%f)\r\nl2:(%f,%f,%f)\r\ni0:(%f,%f,%f)\r\ni1:(%f,%f,%f)",
					call,
					(int)vp, (int)vl,
					(int)cw, notsame,
					(int)discw,
					tet->neib[0]->ring,
					tet->neib[1]->ring,
					tet->neib[2]->ring,
					tet->neib[0]->file,
					tet->neib[1]->file,
					tet->neib[2]->file,
					tet->neib[0]->wrappos.x,
					tet->neib[0]->wrappos.y,
					tet->neib[0]->wrappos.z,
					tet->neib[1]->wrappos.x,
					tet->neib[1]->wrappos.y,
					tet->neib[1]->wrappos.z,
					tet->neib[2]->wrappos.x,
					tet->neib[2]->wrappos.y,
					tet->neib[2]->wrappos.z,
					frompt->ring,
					parpt->ring,
					topt->ring,
					frompt->file,
					parpt->file,
					topt->file,
					len,
					dir.x, dir.y, dir.z,
					plane[vp].m_normal.x,
					plane[vp].m_normal.y,
					plane[vp].m_normal.z,
					plane[vp].m_d,
					plane[(vp+1)%3].m_normal.x,
					plane[(vp+1)%3].m_normal.y,
					plane[(vp+1)%3].m_normal.z,
					plane[(vp+1)%3].m_d,
					plane[(vp+2)%3].m_normal.x,
					plane[(vp+2)%3].m_normal.y,
					plane[(vp+2)%3].m_normal.z,
					plane[(vp+2)%3].m_d,
					line[vl*2+0][0].x,
					line[vl*2+0][0].y,
					line[vl*2+0][0].z,
					line[vl*2+0][1].x,
					line[vl*2+0][1].y,
					line[vl*2+0][1].z,
					i0.x,
					i0.y,
					i0.z,
					i1.x,
					i1.y,
					i1.z
					);
				//ErrMess(mm,mm);
				fprintf(g_applog, "%s", mm);
				fflush(g_applog);
#endif

				//////////////correct emerge pt

				len += baselen;
				//*emerge = midp * 1000 + dir * len * 1;
				//*emerge2 = midp * 1000 - dir * len * 1;
				*emerge = midp2 * 1000 + dir * len * 1;
				*emerge2 = midp2 * 1000 - dir * len * 1;

				//Vec3f off = Vec3f(rand()%21-10, rand()%21-10, rand()%21-10);
				//*emerge = Normalize( *emerge + off ) * 1000;

				//Vec3f line[2][2];
				line[0][0] = frompt->wrappos + ndir[0] * 0.1f;
				line[0][1] = ( *emerge ) - ndir[0] * 0.1f;
				line[1][0] = parpt->wrappos + ndir[1] * 0.1f;
				line[1][1] = ( *emerge ) - ndir[1] * 0.1f;
				
				line[2][0] = frompt->wrappos + ndir[0] * 0.1f;
				line[2][1] = ( *emerge2 ) - ndir[0] * 0.1f;
				line[3][0] = parpt->wrappos + ndir[1] * 0.1f;
				line[3][1] = ( *emerge2 ) - ndir[1] * 0.1f;

				//exit(0);
				goto again;
			}

		}
#endif

		static int app = 0;
		app++;
		
#if 0
				char mm[1233];
				sprintf(mm, "\r\n call%d app%d inov3 vp=%d vl=%d cw=%d notsame=%d \r\n collider ring=%d,%d,%d file=%d,%d,%d \r\n ring=%d,%d,%d file=%d,%d,%d len=%f dir=(%f,%f,%f)\r\np1:(%f,%f,%f),%f\r\np2:(%f,%f,%f),%f\r\nl1:(%f,%f,%f)\r\nl2:(%f,%f,%f)\r\ni0:(%f,%f,%f)\r\ni1:(%f,%f,%f)",
					call, app,
					//(int)vp, (int)vl,
					0,0,
					(int)cw, notsame,
					tet->neib[0]->ring,
					tet->neib[1]->ring,
					tet->neib[2]->ring,
					tet->neib[0]->file,
					tet->neib[1]->file,
					tet->neib[2]->file,
					frompt->ring,
					parpt->ring,
					topt->ring,
					frompt->file,
					parpt->file,
					topt->file,
					len,
					dir.x, dir.y, dir.z,
					0.0f,//plane[vp].m_normal.x,
					0.0f,//plane[vp].m_normal.y,
					0.0f,//plane[vp].m_normal.z,
					0.0f,//plane[vp].m_d,
					0.0f,//plane[(vp+1)%3].m_normal.x,
					0.0f,//plane[(vp+1)%3].m_normal.y,
					0.0f,//plane[(vp+1)%3].m_normal.z,
					0.0f,//plane[(vp+1)%3].m_d,
					0.0f,//line[vl*2+0][0].x,
					0.0f,//line[vl*2+0][0].y,
					0.0f,//line[vl*2+0][0].z,
					0.0f,//line[vl*2+0][1].x,
					0.0f,//line[vl*2+0][1].y,
					0.0f,//line[vl*2+0][1].z,
					0.0f,//i0.x,
					0.0f,//i0.y,
					0.0f,//i0.z,
					0.0f,//i1.x,
					0.0f,//i1.y,
					0.0f//i1.z
					);
				//ErrMess(mm,mm);
				fprintf(g_applog, "%s", mm);
				fflush(g_applog);
#endif

next:
		;
	}

#if 0
	LoadedTex *retex, *retexn, *retexs;
	Vec2f retexc;
	Vec3f rewp, rerp, ren;
	Tet *retet;
	double refU, refV;

	while(TraceRay4(surf,
		line,
		&retex,
		&retexs,
		&retexn,
		&retexc,
		&rewp,
		&rerp,
		&ren,
		&retet,
		&refU, &refV))
	{
		len+=1;
		*emerge = midp * 1000 + dir * len * 1;
		line[1] = Normalize( *emerge ) * 30000;
	}
#endif
	if(ISNAN(emerge->x))
		ErrMess("asdasd","isnanex");
	if(ISNAN(emerge->y))
		ErrMess("asdasd","isnaney");
	if(ISNAN(emerge->z))
		ErrMess("asdasd","isnanez");
}

bool TestEmerge2(Surf *surf,
			SurfPt *esp,
			SurfPt *ig1,
			SurfPt *ig2,
			Vec3f place,
			float div=19)
{
//////////////////////////////
	Tet *tet;
	Vec3f teste[3];
	teste[0] = tet->neib[0]->wrappos;
	teste[1] = tet->neib[1]->wrappos;
	teste[2] = tet->neib[2]->wrappos;

	//if(!cw)
	{
		//	teste[0] = parpt->wrappos;
		//	teste[1] = frompt->wrappos;
	}

	Vec3f trinorm = Normal( teste );
	Vec3f cennorm = Normalize( (teste[0] + teste[1] + teste[2])/3.0f );

	//backwards norm?
	bool discw = (Dot(cennorm, trinorm) <= 0.0f);

	if(discw)
	{
		ErrMess("te","te2");
		return false;
	}

	return true;
}


//test before Emerge2
//check if emerging place will intersect any triangles
void PreEmTest(Surf *surf,
			SurfPt *esp,
			SurfPt *ig1,
			SurfPt *ig2,
			Vec3f place,
			Vec3f place2)
{
	return;////////

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		if(tet->neib[0]->ring < 0)
			continue;
		if(tet->neib[1]->ring < 0)
			continue;
		if(tet->neib[2]->ring < 0)
			continue;

		Vec3f tri[3];
		tri[0] = tet->neib[0]->wrappos;
		tri[1] = tet->neib[1]->wrappos;
		tri[2] = tet->neib[2]->wrappos;

		Vec3f line[2];
		line[0] = Normalize(place) * 30000;
		line[1] = Vec3f(0,0,0);

		Vec3f vint;

		if(IntersectedPolygon(tri, line,
			3, &vint))
		{
			fprintf(g_applog,
				"\r\n pem \r\n"\
				"ring=%d,%d,%d file=%d,%d,%d \r\n"\
				"tri[0]=%f,%f,%f \r\n"\
				"tri[1]=%f,%f,%f \r\n"\
				"tri[2]=%f,%f,%f \r\n"\
				"place=%f,%f,%f \r\n",
				tet->neib[0]->ring,
				tet->neib[1]->ring,
				tet->neib[2]->ring,
				tet->neib[0]->file,
				tet->neib[1]->file,
				tet->neib[2]->file,

				tri[0].x,
				tri[0].y,
				tri[0].z,

				tri[1].x,
				tri[1].y,
				tri[1].z,

				tri[2].x,
				tri[2].y,
				tri[2].z,

				place.x,
				place.y,
				place.z);
			fflush(g_applog);
			ErrMess("pem","pem");
		}
	}
}

//make clearing room as we get closer to the emerging pt
//but save tests of incorrect normals 'til the end
void Emerge3(Surf *surf,
			 Vec3f place,
			 float div=19)
{
	//fprintf(g_applog, "\r\n em3 \r\n");
	//fflush(g_applog);

	int pin = 0;

	for(std::list<SurfPt*>::iterator sit=surf->pts2.begin();
		sit!=surf->pts2.end();
		++sit, ++pin)
	{
		SurfPt *sp = *sit;

		if(sp->ring < 0)
			continue;
		
		//fprintf(g_applog, "\r\b em3 pin%d ring%d \r\n", pin, sp->ring);
		//fflush(g_applog);

		sp->prevwrap = sp->wrappos;

		//if(sp == esp)
		//	continue;
		//if(sp == ig1)
		//	continue;
		//if(sp == ig2)
		//	continue;
		//if(sp->ring < 0)
		//	continue;

		Vec3f sidevec = Cross( Normalize(place), Normalize(sp->wrappos) );

		sidevec = Normalize(sidevec);

		//if(Magnitude(sidevec) < 0.3f)
		//	continue;
		if(ISNAN(sidevec.x) || ISNAN(sidevec.y) || ISNAN(sidevec.z))
			continue;

		//Vec3f sidevec = Cross( Normalize(sp->wrappos), Normalize(emline[1]) );
		sidevec = Normalize( sidevec );
		//Vec3f dirmove = Cross( Normalize(sp->w=rappos), sidevec );
		
		float amt = (1.0f + Dot( Normalize(sp->wrappos), Normalize(place) ))/2.0f;

#if 01
		amt = exp(amt);
		amt -= 1;
#define E	(2.7182818284590452353602874713526624977572470937L)
		amt /= (E-1);
#endif

		sp->wrappos = Rotate(sp->wrappos, M_PI * amt / div, sidevec.x, sidevec.y, sidevec.z);

#if 01
		fprintf(g_applog, "\r\n em3 r%d f%d sp->wrappos %f,%f,%f \r\n from %f,%f,%f \r\n",
			sp->ring,
			sp->file,
			sp->wrappos.x,
			sp->wrappos.y,
			sp->wrappos.z,
			sp->prevwrap.x,
			sp->prevwrap.y,
			sp->prevwrap.z);
		fflush(g_applog);
#endif
		
		TestE(surf, place, NULL, NULL);
	}

	TestE(surf, place, NULL, NULL);
}

//make room for placement of new points in a sub-ring
//or perhaps a line forming a ring with part of itself 
//(degenerate case where the ring closes off in one spot)
void Emerge2(Surf *surf,
			SurfPt *esp,
			SurfPt *ig1,
			SurfPt *ig2,
			Vec3f place,
			Vec3f place2,
			float div=19)
{
#if 0	//ignore for use in MapGlobe4
	PreEmTest(surf,
		esp,
		ig1,
		ig2,
		place,
		place2);
#endif

	for(std::list<SurfPt*>::iterator sit=surf->pts2.begin();
		sit!=surf->pts2.end();
		++sit)
	{
		SurfPt *sp = *sit;

#if 0	//MapGlobe4()....
		if(sp->ring < 0)
			continue;
#endif

		sp->prevwrap = sp->wrappos;

		if(sp == esp)
			continue;
		//if(sp == ig1)
		//	continue;
		//if(sp == ig2)
		//	continue;
		//if(sp->ring < 0)
		//	continue;

		Vec3f sidevec = Cross( Normalize(place), Normalize(sp->wrappos) );

		sidevec = Normalize(sidevec);

		//if(Magnitude(sidevec) < 0.3f)
		//	continue;
		if(ISNAN(sidevec.x) || ISNAN(sidevec.y) || ISNAN(sidevec.z))
			continue;

		//Vec3f sidevec = Cross( Normalize(sp->wrappos), Normalize(emline[1]) );
		sidevec = Normalize( sidevec );
		//Vec3f dirmove = Cross( Normalize(sp->w=rappos), sidevec );
		
		float amt = (1.0f + Dot( Normalize(sp->wrappos), Normalize(place) ))/2.0f;

#if 01
		amt = exp(amt);
		amt -= 1;
#define E	(2.7182818284590452353602874713526624977572470937L)
		amt /= (E-1);
#endif

		sp->wrappos = Rotate(sp->wrappos, M_PI * amt / div, sidevec.x, sidevec.y, sidevec.z);

#if 0
		fprintf(g_applog, "\r\n em2 sp->wrappos %f,%f,%f from %f,%f,%f \r\n",
			sp->wrappos.x,
			sp->wrappos.y,
			sp->wrappos.z,
			sp->prevwrap.x,
			sp->prevwrap.y,
			sp->prevwrap.z);
		fflush(g_applog);
#endif
	}

	if(esp)
	{
		//esp->prevwrap = esp->wrappos;
		esp->prevwrap = place;
		esp->wrappos = place;
		
#if 0
		fprintf(g_applog, "\r\n em2 esp->wrappos %f,%f,%f from %f,%f,%f \r\n",
			esp->wrappos.x,
			esp->wrappos.y,
			esp->wrappos.z,
			esp->prevwrap.x,
			esp->prevwrap.y,
			esp->prevwrap.z);
		fflush(g_applog);
#endif
	}

	//TestE(surf, place, ig1, ig2);
}

//jump along ring, add link
bool TryJump(Surf *surf, SurfPt *frompt, SurfPt **topt)
{

	fprintf(g_applog, "\r\n tryjump \r\n");
	fflush(g_applog);

	*topt = NULL;
	//assumed, since this is within "frompt" linkhood
	//bool haveneib = false;

	//test: bool havepar = false;

	//bool havefile = false;

	//if(topt->ring >= 0)
	//	return false;

	//if(topt->file >= 0)
	//	return false;
	
	for(std::list<Tet*>::iterator hit=frompt->holder.begin();
		hit!=frompt->holder.end();
		++hit)
	{
		int parvi = -1;
		int tovi = -1;
		int fromvi = -1;
		//neighbour of frompt, so will have fromvi
		//not sure if will have parvi (parent)
		//tovi is to be determined, must be unclaimed
		Tet *het = *hit;
		for(int v=0; v<3; v++)
		{
			SurfPt *p = het->neib[v];

			if(p == frompt)
			{
				fromvi = v;
			}
			//else if(p == topt)
			else if(p->ring < 0)
			{
				tovi = v;
			}
			else if(p->ring >= 0 &&
				p->ring == frompt->ring - 1)
			{
				parvi = v;
			}
		}

		if(parvi >= 0 &&
			fromvi >= 0 &&
			tovi >= 0)
		{
			bool cw = (( (fromvi==parvi-1) || (fromvi==parvi+3-1) ) 
				//&& 
				//( (parvi==tovi-1) || (parvi==tovi+3-1) )
				);

			if(het->neib[parvi]->gone)
				ErrMess("hg","hg");

			//p = parent
#if 0
			*topt = het->neib[tovi];
			(*topt)->ring = frompt->ring;
			(*topt)->file = frompt->file + 1;
#endif
			*topt = het->neib[tovi];
			SurfPt *parpt = het->neib[parvi];
			Vec3f emerge;
			Vec3f emerge2;

			fprintf(g_applog, "\r\n fromvi=%d parvi=%d tovi=%d (<-in tet->) \r\n"\
				"ring=%d,%d,%d file=%d,%d,%d \r\n",
				fromvi, parvi, tovi,
				het->neib[0]->ring,
				het->neib[1]->ring,
				het->neib[2]->ring,
				//frompt->ring,
				//het->neib[parvi]->ring,
				//(*topt)->ring,
				//frompt->file,
				//het->neib[parvi]->file,
				//(*topt)->file
				
				het->neib[0]->file,
				het->neib[1]->file,
				het->neib[2]->file
				);
			fflush(g_applog);

			//cw doesn't refer to the order of these pt arguments
			//but their arrangment in the tet
			//if cw=false then frompt,parpt actually go parpt,frompt
			NextEmerge(surf, frompt, het->neib[parvi], *topt, cw, &emerge, &emerge2);
				//pre-emerge clearing
			//for(float ci=0; ci<11; ci+=1)
			//	Emerge3(surf, (frompt->wrappos*ci+parpt->wrappos*(11-ci))/11.0f, 15);
			//for(float ci=0; ci<11; ci+=1)
			//	Emerge3(surf, (emerge*(ci)+frompt->wrappos*(11-ci))/11.0f, 150);
			Emerge2(surf, *topt, frompt, het->neib[parvi], emerge, emerge2);

#if 01
			//*topt = het->neib[tovi];
			(*topt)->ring = frompt->ring;
			(*topt)->file = frompt->file + 1;
#endif

			//CheckFan(surf, *topt);

			if(!CheckCompleteRing(surf, (*topt)->ring))
			{
				//ErrMess("!c","!ctj");
				
				char mm[1234];
				sprintf(mm, 
					"up ring=%d,%d,%d file=%d,%d,%d",
					het->neib[0]->ring,
					het->neib[1]->ring,
					het->neib[2]->ring,
					het->neib[0]->file,
					het->neib[1]->file,
					het->neib[2]->file);
				//ErrMess(mm,mm);
			}

			TestD(surf, frompt, het->neib[parvi], *topt);

			return true;
		}
	}

	return false;
}

/*
check nexus of sp to be a flattanable fan of triangles,
with no "3d" configurations with edges shared by three or more
triangles.
only run this when TryJump no longer works, and requires a LowJump (to a new ring count).
*/
void CheckFan(Surf *surf,
			  SurfPt *sp)
{
	for(std::list<Tet*>::iterator hit=sp->holder.begin();
		hit!=sp->holder.end();
		++hit)
	{
		Tet *het = *hit;

		for(int v=0; v<3; v++)
		{
			het->neib[v]->checked = false;
		}
	}

	SurfPt *curpt = sp;
	sp->checked = true;

	//get first pt, set
	//jump to second, set (necessary so there's only one front going side with free unchecked pt's

	for(std::list<Tet*>::iterator hit=sp->holder.begin();
		hit!=sp->holder.end();
		++hit)
	{
		Tet *het = *hit;

		for(int v=0; v<3; v++)
		{
			if(het->neib[v] != curpt)
			{
				curpt = het->neib[v];
				curpt->checked = true;
				goto sec;
			}
		}
	}

sec:
	//second

	for(std::list<Tet*>::iterator hit=sp->holder.begin();
		hit!=sp->holder.end();
		++hit)
	{
		Tet *het = *hit;

		for(int v=0; v<3; v++)
		{
			if(het->neib[v] != curpt &&
				het->neib[v] != sp)
			{
				curpt = het->neib[v];
				curpt->checked = true;
				goto again;
			}
		}
	}

again:

	SurfPt *free1 = NULL;
	SurfPt *free2 = NULL;	//not fan if have 2

	for(std::list<Tet*>::iterator hit=sp->holder.begin();
		hit!=sp->holder.end();
		++hit)
	{
		Tet *het = *hit;
		bool havecur = false;

		for(int v=0; v<3; v++)
		{
			SurfPt *hp = het->neib[v];

			if(hp == curpt)
			{
				havecur = true;
				goto tryadd;
			}
		}

tryadd:
		if(!havecur)
			continue;

		for(int v=0; v<3; v++)
		{
			SurfPt *hp = het->neib[v];

			if(hp->checked)
				continue;

			if(!free1)
				free1 = hp;
			else if(!free2)
				free2 = hp;
					
			if(free1 && free2)
			{
				ErrMess("fb","fb");
				return;
			}
		}
	}

tryset:

	if(!free1)
	{
		//check for gap

		for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
		{
			Tet *het = *hit;

			for(int v=0; v<3; v++)
			{
				SurfPt *hp = het->neib[v];

				if(!hp->checked)
				{
					//ErrMess("gap","gap, possibly non-flat non-fan, \r\n possibly unfinished ring");
				}
			}
		}

		return;
	}

	free1->checked = true;
	curpt = free1;
	goto again;
}

//jump to next ring down
bool LowJump(Surf *surf, SurfPt *frompt, SurfPt **rep)
{
	
	fprintf(g_applog, "\r\n lowjump \r\n");
	fflush(g_applog);

	*rep = NULL;

	CheckFan(surf, frompt);

	for(std::list<Tet*>::iterator hit=frompt->holder.begin();
		hit!=frompt->holder.end();
		++hit)
	{
		Tet *het = *hit;
		int tovi = -1;
		int par1vi = -1;
		int par2vi = -1;
		SurfPt *par1 = NULL;
		SurfPt *par2 = NULL;
		for(int v=0; v<3; v++)
		{
			SurfPt *p = het->neib[v];

			//frompt is the last file number addition of previous ring, going clockwise
			//so frompt will always be 
			//? is always par2 from first init'd tet
			if(p == frompt)
			{
				//par1vi = v;
				//par1 = p;
				par2vi = v;
				par2 = p;
				continue;
			}
			//else if(p->ring >= 0)
			else if(p->ring >= 0 &&
				p->ring == frompt->ring)
			{
				//par2vi = v;
				//par2 = p;
				par1vi = v;
				par1 = p;
				continue;
			}
			else if(p->ring < 0)
			{
				tovi = v;
				continue;
			}
			//if(p->file >= 0)
			//	continue;

			//p->ring = curpt->ring + 1;
			//p->file = 0;
			//*nextpt = p;
			//return true;
		}
///////////////
		if(par1vi >= 0 &&
			par2vi >= 0 &&
			tovi >= 0)
		{
			bool cw = (( (par1vi==par2vi-1) || (par1vi==par2vi+3-1) ) //&& 
			//	( (par2vi==tovi-1) || (par2vi==tovi+3-1) )
				);

			//cw = !cw;	//this is from the outside of the topt's tet (het),
			//so the winding will be opposite

			//p = parent
#if 0
			*rep = het->neib[tovi];
			(*rep)->ring = frompt->ring + 1;
			(*rep)->file = 0;
#endif
			*rep = het->neib[tovi];
			Vec3f emerge;
			Vec3f emerge2;
			NextEmerge(surf, par1, par2, *rep, cw, &emerge, &emerge2);
				//pre-emerge clearing
			//for(float ci=0; ci<11; ci+=1)
			//	Emerge3(surf, (par2->wrappos*ci+par1->wrappos*(11-ci))/11.0f, 15);
			//for(float ci=0; ci<11; ci+=1)
			//	Emerge3(surf, (emerge*ci+frompt->wrappos*(11-ci))/11.0f, 150);
			Emerge2(surf, *rep, par1, par2, emerge, emerge2);
			
			
#if 01
			//*rep = het->neib[tovi];
			(*rep)->ring = frompt->ring + 1;
			(*rep)->file = 0;
#endif

			CheckFan(surf, *rep);
			//CheckFan(surf, par1);

			if(!CheckCompleteRing(surf, (*rep)->ring))
			{
				//ErrMess("!c","!clj");
			}


			TestD(surf, par1, par2, *rep);

			TryJump(surf, *rep, rep);
			return true;
		}
	}
	return false;
}

//check if there's a missing region for a new ring
bool MissJump(Surf *surf, SurfPt **rep)
{
	
	fprintf(g_applog, "\r\n missjump \r\n");
	fflush(g_applog);

	int parring = -1;
	//SurfPt *parp = NULL;
	//*parpt = NULL;
	//*startpt = NULL;
	*rep = NULL;

	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		//bool havefile = false;
		//bool haveparent = false;
		
		SurfPt *p = *pit;

		if(p->ring >= 0)
			continue;
		if(p->file >= 0)
			continue;

		for(std::list<Tet*>::iterator hit=p->holder.begin();
			hit!=p->holder.end();
			++hit)
		{
			Tet *het = *hit;
			int par1vi = -1;
			int par2vi = -1;
			int tovi = -1;
			SurfPt *par1 = NULL;
			SurfPt *par2 = NULL;
			for(int v=0; v<3; v++)
			{
				SurfPt *p2 = het->neib[v];

				if(p2 == p)
				{
					tovi = v;
					continue;
				}
				else if(p2->ring >= 0)
				{
					if(par1vi < 0)
					{
						par1vi = v;
						par1 = p2;
						continue;
					}
					else if(par2vi < 0)
					{
						par2 = p2;
						//if(par2->ring != par1->ring &&
						//	abs(par2->ring - par1->ring) != 1)
						//	goto nexttet;
						par2vi = v;
						continue;
					}
				}

#if 0
				//if(p2->file >= 0)
				if(p2->ring >= 0)
				{
					//haveparent = true;
					parring = p2->ring;
					*parpt = p2;
					*startpt = p;
					return true;
				}
#endif
			}

			if(par1vi >= 0 &&
				par2vi >= 0 &&
				tovi >= 0)
			{
				bool cw = (( (par1vi==par2vi-1) || (par1vi==par2vi+3-1) )// && 
					//( (par2vi==tovi-1) || (par2vi==tovi+3-1) )
					);

				//p = parent
#if 0
				*rep = het->neib[tovi];
				(*rep)->ring = max(par1->ring, par2->ring) + 1;
				(*rep)->file = 0;
#endif
				*rep = het->neib[tovi];
				Vec3f emerge;
				Vec3f emerge2;
				NextEmerge(surf, par1, par2, *rep, cw, &emerge, &emerge2);
				//pre-emerge clearing
				//for(float ci=0; ci<11; ci+=1)
				//	Emerge3(surf, (par1->wrappos*ci+par2->wrappos*(11-ci))/11.0f, 15);
				//for(float ci=0; ci<11; ci+=1)
				//	Emerge3(surf, (emerge*ci+par2->wrappos*(11-ci))/11.0f, 150);
				Emerge2(surf, *rep, par1, par2, emerge, emerge2);
				
#if 01
				//*rep = het->neib[tovi];
				(*rep)->ring = max(par1->ring, par2->ring) + 1;
				(*rep)->file = 0;
#endif
			//CheckFan(surf, *rep);

				
			if(!CheckCompleteRing(surf, (*rep)->ring))
			{
				//ErrMess("!c","!cmj");
			}

				
				TestD(surf, par1, par2, *rep);

				TryJump(surf, *rep, rep);
				return true;
			}
//nexttet:
//			;
		}
	}

	return false;
}

bool MapGlobe3(Surf *surf)
{
	SplitEdges2(surf, NULL, NULL, NULL, 10);

	//jump by one pt each iteration, keeping at least one parent and one neighbour in linkage
	SurfPt* curpt;// = *surf->pts2.begin();
	Tet* startt = *surf->tets2.begin();
	for(int v=0; v<3; v++)
	{
		SurfPt *p = startt->neib[v];
		p->ring = 0;
		p->file = v;
		curpt = p;
	}
	startt->neib[0]->wrappos = Vec3f(-1,1000,-1);
	startt->neib[1]->wrappos = Vec3f(1,1000,-1);
	startt->neib[2]->wrappos = Vec3f(0,1000,0);

	Vec3f test[3];
	test[0] = startt->neib[0]->wrappos;
	test[1] = startt->neib[1]->wrappos;
	test[2] = startt->neib[2]->wrappos;

	//if(Normal(test).y <= 0)

	if( Dot( Normal(test), Normalize(test[0]+test[1]+test[2]) ) <= 0.0f )
		ErrMess("asd","ny<0");

	//curpt->file = 0;

	SurfPt *nextpt = NULL;

	if(!LowJump(surf, curpt, &nextpt))
		ErrMess("asdasd", "!lj");

	curpt = nextpt;

	bool didmiss = false;

	while(curpt)
	{
foundjump:
		//SplitEdges2(surf, NULL, NULL, NULL, 0.2);
		int bestupdown = -1;
		//Balance3(surf, bestupdown);
		//Balance4(surf);
		//SplitEdges2(surf, NULL, NULL, NULL, 0.2);
		//Balance4(surf);

		if(TryJump(surf, curpt, &nextpt))
		{
			curpt = nextpt;
			continue;
		}

		//if(!didmiss)
		//	CheckCompleteRing(surf, curpt->ring);

		nextpt = NULL;
		if(LowJump(surf, curpt, &nextpt))
		{
			curpt = nextpt;
			continue;
		}

		break;
	}

trymiss:
	if(MissJump(surf, &curpt))
	{
		didmiss = true;
		if(curpt)
			goto foundjump;
		else
			goto trymiss;
	}

	return true;
}

//by method of up and pull attraction of tets
bool MapGlobe4(Surf *surf)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;
		tet->hidden = true;
		tet->strength = 0;

		tet->neib[0]->wrappos = tet->neib[0]->pos;
		tet->neib[1]->wrappos = tet->neib[1]->pos;
		tet->neib[2]->wrappos = tet->neib[2]->pos;
	}

	//return true;

again:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;
		tet->hidden = true;
		tet->strength = 0;

		Vec3f tri[3];

		tri[0] = tet->neib[0]->wrappos;
		tri[1] = tet->neib[1]->wrappos;
		tri[2] = tet->neib[2]->wrappos;

		Vec3f n = Normal(tri);

		Vec3f line[3][2][2];	//vert line,reverse order,line point,
		////////////////
#if 0
		line[0][0][0] = (tri[0]+tri[1]+tri[2])/3.0f + n*0.2f;
		line[0][0][1] = line[0][0][0] - n*30000;

		line[0][1][1] = (tri[0]+tri[1]+tri[2])/3.0f  + n*0.2f;
		line[0][1][0] = line[0][1][1] + n*30000;
		/////////////////
	#if 1
		line[1][0][0] = (tri[1]*2.0f/3.0f+tri[2]*1.0f/3.0f)/1.0f + n*0.2f;
		line[1][0][1] = line[1][0][0] + n*30000;

		line[1][1][1] = (tri[1]*2.0f/3.0f+tri[2]*1.0f/3.0f)/1.0f + n*0.2f;
		line[1][1][0] = line[1][1][1] + n*30000;
		/////////////////
		line[2][0][0] = (tri[2]*2.0f/3.0f+tri[0]*1.0f/3.0f)/1.0f + n*0.2f;
		line[2][0][1] = line[2][0][0] + n*30000;

		line[2][1][1] = (tri[2]*2.0f/3.0f+tri[0]*1.0f/3.0f)/1.0f + n*0.2f;
		line[2][1][0] = line[2][1][1] + n*30000;
		////////////
	#endif
#else
		
		line[0][0][0] = (tri[0]) + n*30000;
		line[0][0][1] = Vec3f(0,0,0);

		line[0][1][1] = (tri[0]) + n*30000;
		line[0][1][0] = Vec3f(0,0,0);
		/////////////////
		line[1][0][0] = tri[1] + n*30000;
		line[1][0][1] = Vec3f(0,0,0);

		line[1][1][1] = tri[1] + n*30000;
		line[1][1][0] = Vec3f(0,0,0);
		/////////////////
		line[2][0][0] = tri[2] + n*30000;
		line[2][0][1] = Vec3f(0,0,0);

		line[2][1][1] = tri[2] + n*30000;
		line[2][1][0] = Vec3f(0,0,0);
#endif

		LoadedTex *tex, *texn, *texs;
		Vec2f texc;
		Vec3f wp, rp, rn;
		Tet *rtet;
		double fU,fV;

		if( (!TraceRay3(surf,
			line[0][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV) &&
			!TraceRay3(surf,
			line[0][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV)
			&&
			!TraceRay3(surf,
			line[1][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV)&&
			!TraceRay3(surf,
			line[1][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV)&&
			!TraceRay3(surf,
			line[2][0],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV)&&
			!TraceRay3(surf,
			line[2][1],
			&tex,
			&texs,
			&texn,
			&texc,
			&wp,
			&rp,
			&rn,
			&rtet,
			&fU, &fV))
			)
		{
			tet->hidden = false;
			tet->strength = 1;
		}
	}

iteratepull:

	bool chpull = false;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet *tet = *tit;

		if(!tet->hidden)
			continue;

		Tet *nearout = NULL;

		for(int v=0; v<3; v++)
		{
			SurfPt *sp = tet->neib[v];

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
			{
				Tet *het = *hit;

				if(!het->hidden)
				{
					nearout = het;
					goto getinfo;
				}
			}
		}

getinfo:

		if(!nearout)
			continue;

		bool sharep[3] = {false,false,false};

		for(int v=0; v<3; v++)
		{
			SurfPt *sp = tet->neib[v];

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
			{
				Tet *het = *hit;

				if(het == nearout)
					sharep[v] = true;
			}
		}

pull:

		Vec3f outdir;
		Vec3f updir;

		Vec3f closerin;
		Vec3f closerout;

		float ninc = 0;
		float noutc = 0;

		for(int v=0; v<3; v++)
		{
			if(sharep[v])
			{
				closerout = (closerout * noutc + tet->neib[v]->wrappos * 1) / (noutc + 1);
				noutc += 1;
			}
			else
			{
				closerin = (closerin * ninc + tet->neib[v]->wrappos * 1) / (ninc + 1);
				ninc += 1;
			}
		}

		outdir = closerout - closerin;
	
		Vec3f noutdir = Normalize(outdir);
#if 0
		Vec3f surfdir = tet->neib[1]->wrappos - tet->neib[0]->wrappos;

		if(Magnitude(surfdir - outdir) <= 0.1f)
			surfdir = tet->neib[2]->wrappos - tet->neib[1]->wrappos;

		Vec3f nsurfdir = Normalize(surfdir);
#endif

		Vec3f tri[3];
		tri[0] - tet->neib[0]->wrappos;
		tri[1] - tet->neib[1]->wrappos;
		tri[2] - tet->neib[2]->wrappos;

		updir = Normal( tri );

		for(int v=0; v<3; v++)
		{
			if(sharep[v])
				continue;
	
			tet->neib[v]->wrappos = 
				tet->neib[v]->wrappos + updir * 0.01f + outdir;
		}

		chpull = true;
	}

	//if(chpull)
	//	goto iteratepull;
	if(chpull)
	{
		Balance4(surf);
		goto again;
	}

	return true;
}

//ring,file
bool MapGlobe2(Surf *surf)
{
	int ring = 0;
	std::list<SurfPt*> nextring;
	std::list<SurfPt*> curring;

	if(!surf->tets2.size())
		return false;

	Tet *starttet = *surf->tets2.begin();
	starttet->ring = ring;
	starttet->file = 0;

	SurfPt *startpt = starttet->neib[0];
	startpt->ring = ring;
	startpt->file = 0;
	ring++;

	curring.push_back(startpt);

	do
	{
		nextring.clear();
		
		for(std::list<SurfPt*>::iterator pit=curring.begin();
			pit!=curring.end();
			++pit)
		{
			SurfPt *p = *pit;
			p->ring = ring;
			p->file = -1;
		}

		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			tit++)
		{
			Tet *tet = *tit;

			if(tet->ring >= 0)
				continue;

			bool hasfree = false;
			bool hascurr = false;

			for(int v=0; v<3; v++)
			{
				if(tet->neib[v]->ring < 0)
					hasfree = true;
				if(tet->neib[v]->ring == ring)
					hascurr = true;
			}

			if(hasfree && hascurr)
			{
				tet->ring = ring;

				for(int v=0; v<3; v++)
				{
					if(tet->neib[v]->ring < 0)
					{
						nextring.push_back(tet->neib[v]);
					}
				}
			}
		}

		ring++;
		curring = nextring;
	}while(curring.size());
	return true;
}

//give even linkage distribution between pt's (by cutting wedges into mid pt's)
//actually the edge opposite to the additional link site is cut, but it shares
//that far pt with both new triangles, so in effect a new link is added by adding a low-linkage pt
//mm
void Balance5(Surf *surf)
{
	int maxnexus = 5;

	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		SurfPt *p = *pit;
	
		int nexus = p->holder.size();

		if(nexus > maxnexus)
			maxnexus = nexus;
	}

again:

	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		SurfPt *p = *pit;

		int nexus = p->holder.size();

		if(nexus >= maxnexus)
			continue;

		goto again;
	}

	return;
}

void Balance4(Surf *surf)
{
	int t = 0;

again:
	Vec3f strong = Vec3f(0,0,0);
	float c = 0;

	for(std::list<SurfPt*>::iterator pit=surf->pts2.begin();
		pit!=surf->pts2.end();
		++pit)
	{
		SurfPt *p = *pit;

#if 0	//ignore for use in MapGlobe4();
		if(p->ring < 0)
			continue;
#endif

		strong = strong * c / (c+1) + p->wrappos / (c+1);
		c++;
	}

	float mag = Magnitude(strong) / 1000.0f;

	float div = 1.0f / (mag/4.0f);

	Emerge2(surf, NULL, NULL, NULL, Normalize(strong) * 1000, Normalize(strong) * 1000, div);

	t++;
	if(t < mag*20 || t < c+1)
		goto again;
}

void Balance3(Surf *surf, int bestupdown)
{
	int currupdown = 0;
	bool haveupdown = false;
#if 01
	bool progress = true;
	
	if(progress)
	{
			for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet * tet = *tit;
		
			for(int v=0; v<3; v++)
			{
				tet->neib[v]->pressure = Vec3f(0,0,0);
			}
		}

		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet * tet = *tit;

			SurfPt *fatt[3];

			fatt[0] = tet->neib[0];
			fatt[1] = tet->neib[1];
			fatt[2] = tet->neib[2];

			if(fatt[0]->ring < 0 ||
				fatt[1]->ring < 0 ||
				fatt[2]->ring < 0)
				continue;

			Vec3f wrappos[3];
			wrappos[0] = fatt[0]->wrappos;
			wrappos[1] = fatt[1]->wrappos;
			wrappos[2] = fatt[2]->wrappos;

			//for(int v=0; v<3; v++)
			//	fprintf(g_applog, "wrappos[%d]%f,%f,%f\r\n",
			//	v,
			//	wrappos[v].x,
			//	wrappos[v].y,
			//	wrappos[v].z);

			Vec3f cen = (wrappos[0] + wrappos[1] + wrappos[2])/3.0f;
			Vec3f norm = Normal(wrappos);

				if(ISNAN(norm.x))
					ErrMess("yhjgjf","nannormx");
				if(ISNAN(norm.y))
					ErrMess("yhjgjf","nannormy");
				if(ISNAN(norm.z))
					ErrMess("yhjgjf","nannormz");

			Vec3f correctnorm = Normalize(cen);

			
				if(ISNAN(correctnorm.x))
					ErrMess("yhjgjf","nannormxcc");
				if(ISNAN(correctnorm.y))
					ErrMess("yhjgjf","nannormycc");
				if(ISNAN(correctnorm.z))
					ErrMess("yhjgjf","nannormzcc");

			//fprintf(g_applog, "corr%f,%f,%f nor%f,%f,%f\r\n", 
			//	correctnorm.x,
			//	correctnorm.y,
			//	correctnorm.z,
			//	norm.x,
			//	norm.y,
			//	norm.z);

			float dot = Dot(correctnorm, norm);

			//fprintf(g_applog, "dot%f\r\n", 
			//	dot);

			if(ISNAN(dot))
			{
				ErrMess("asfdgsdfg","dotnan");
			}

			if(dot < 0)
			{
				currupdown++;
				haveupdown = true;
				//dot *= 2;
				float dot2 = 1000.0f / (dot - 0.1f);
				//dot = fmax(dot, -10);

				if(ISNAN(dot2))
				{
					dot = -1000.0f / (0.001f);
				}
				else
					dot = dot2;
			}
			else
			{
				//float dot2 = 1.0 / dot;

				//if(ISNAN(dot2))
				//	dot = 1.0 / 0.001f;
				float dot2 = 10.0f / (0.1f + dot);
				//dot = fmax(dot, -10);

				if(ISNAN(dot2))
				{
					dot = 1.0f / (0.01f);
				}
				else
					dot = dot2;
			}
			//if(dot == 0)
			//	dot += 1;


			//if(dot >= 0.5f)
			//	;
			//else if(dot > 0 && dot < 0.5f)
			//	dot *= 2.6f;

			
			float a,b,c,s,area;

			a = Magnitude(wrappos[0]-wrappos[1]);
			b = Magnitude(wrappos[1]-wrappos[2]);
			c = Magnitude(wrappos[2]-wrappos[0]);

			s = (a+b+c)/2.0f;

			area = sqrt(s*(s-a)*(s-b)*(s-c));

			if(area == 0)
				area = 1;

			float strength = 10000.0f / area;

			if(!ISNAN(strength))
				dot *= strength;

			dot = fmin(dot, 30000);
			dot = fmax(dot, -30000);

			dot *= abs(bestupdown+2)/200.0f;

			//if(dot < 0 && area > 5000)
			//	dot *= 1000;

			//if(dot > 0)
			//	continue;
	#if 1
			if(dot > 0)
			{
				//continue;
				//dot = 1.0f / dot / 100.0f;

		//		dot = fmin(dot, 3);
			}
	#endif
			//if(dot < 0)
			{
	#if 0
				int minv = -1;
				float minvlen = 0;
				if(dot < 0)
				{
	#if 0
					for(int v=0; v<3; v++)
					{
						float vlen = Magnitude(wrappos[v] - wrappos[(v+1)%3]);

						if(vlen < minvlen || minv < 0)
						{
							minvlen = vlen;
							minv = v;
						}
					}
	#endif
					//check if there's an edge with another tet that it is correctly facing forward with
					for(int v=0; v<3; v++)
					{
						bool hasfair = false;

						for(std::list<Tet*>::iterator htit1=tet->neib[v]->holder.begin();
							htit1!=tet->neib[v]->holder.end();
							htit1++)
						{
							Tet *htet1 = *htit1;

							if(htet1 == tet)
								continue;

							for(std::list<Tet*>::iterator htit2=tet->neib[(v+1)%3]->holder.begin();
												htit2!=tet->neib[(v+1)%3]->holder.end();
												htit2++)
							{
								Tet *htet2 = *htit2;

								if(htet2 == tet)
									continue;
								if(htet2 != htet1)
									continue;

								//got 2 pts with htet2/htet1(same)
								//check winding to be clockwise

								//should it go p1,p2 or p2,p1?
								//and is it clockwise?
								//given these two answers, should the edge to be flipped be set to v?
								//bool edgeiscw = false;	//gives clockwise winding?
								//bool givesposdot = false;	//gives positive dot product using normal and direction from origin?

								//if(edgeiscw && !givesposdot

								//just care if this tet needs to be flipped too

								Vec3f tet2tri[3];
								tet2tri[0] = htet2->neib[0]->wrappos;
								tet2tri[1] = htet2->neib[1]->wrappos;
								tet2tri[2] = htet2->neib[2]->wrappos;

								float vlen = Dot( (tet2tri[0]+tet2tri[1]+tet2tri[2])/3.0f, Normal(tet2tri) );

								//if( vlen <= 0 && ( vlen < minvlen || minv < 0 ) )
								if(vlen < 0)
								{
								//	minv = v;
								//	minvlen = vlen;
								}
								else
								{
									hasfair = true;
									continue;
								}
							}
						}

						if(!hasfair)
						{
							minv = v;
							break;
						}
					}
				}
	#endif

				//if(dot < 0)
				//	minv = rand()%3;

				for(int v=0; v<3; v++)
				{
	#if 001
					///*
					Vec3f away = wrappos[v] - cen;

					if(Magnitude(away) <= 0)
						away = Vec3f(rand()%300-150, rand()%300-150, rand()%300-150);

					Vec3f dir = Normalize(away);
					//*/
	#endif
					/*
					Vec3f dir = Vec3f(0,0,0);

					if(v == minv)
					{
						dir = Normalize(wrappos[(v+1)%3] - wrappos[v]);
					}
					else if(v == (minv+1)%3 && minv >= 0)
					{
						dir = Normalize(wrappos[v] - wrappos[(v+3-1)%3]);
					}
					else if(minv < 0)
					{
						dir = wrappos[v] - cen;
						dir = Normalize(dir);
					}


					if(Magnitude(dir) <= 0)
						dir = Vec3f(rand()%300-150, rand()%300-150, rand()%300-150);

					dir = Normalize(dir);
	*/
					//while(abs(dot) < 500)
					//dot *= 30;

					tet->neib[v]->pressure = 
						tet->neib[v]->pressure + 
						dir * dot / 10;
					tet->neib[v]->pressure =
						tet->neib[v]->pressure +
						Normal(wrappos) * 1000;
					//tet->neib[v]->pressure = 
					//	tet->neib[v]->pressure +
					//	Vec3f(rand()%300-150, rand()%300-150, rand()%300-150)/100.0f;
					
					if(ISNAN(tet->neib[v]->pressure.x))
						ErrMess("sdsfg","pressnanx");
					if(ISNAN(tet->neib[v]->pressure.y))
						ErrMess("sdsfg","pressnany");
					if(ISNAN(tet->neib[v]->pressure.z))
						ErrMess("sdsfg","pressnanz");
			//	fprintf(g_applog, "1dot%f press%f,%f,%f dir%f,%f,%f\r\n", dot, 
			//		tet->neib[v]->pressure.x,
			//		tet->neib[v]->pressure.y,
			//		tet->neib[v]->pressure.z,
			//		dir.x, dir.y, dir.z);
				}
			}
		}
	
		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
				tit!=surf->tets2.end();
				++tit)
		{
			Tet * tet = *tit;
		
			for(int v=0; v<3; v++)
			{
				float mag = Magnitude( tet->neib[v]->wrappos );
				if( mag == 0 )
					mag = 1;
				tet->neib[v]->wrappos = 
					tet->neib[v]->wrappos + tet->neib[v]->pressure;
				tet->neib[v]->wrappos = 
					(tet->neib[v]->wrappos * 1000*1 / mag );
				tet->neib[v]->pressure = Vec3f(0,0,0);
			}
		}
	}
#endif
}


//arrange the orc's on a globe and then balance it so everything is seen
bool MapGlobe(Surf *surf)
{
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
		for(int v=0; v<3; v++)
		{
			SurfPt *pt = tet->neib[v];
			if(ISNAN(pt->pos.x))
				ErrMess("yhjgjf","wpnnx");
			if(ISNAN(pt->pos.y))
				ErrMess("yhjgjf","wpnny");
			if(ISNAN(pt->pos.z))
				ErrMess("yhjgjf","wpnnz");
			Vec3f wrappos = pt->pos;
			//float yaw = atan2(wrappos.x, wrappos.z);
			float yaw = 0.5f + atan2(wrappos.z, wrappos.x) / (2.0f*M_PI);
			if(ISNAN(yaw))
				ErrMess("asdsdg","nanyaw");
			//tan(0)=op/adj=0/1
			//fprintf(g_applog, "prepos1 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			//wrappos = Rotate(wrappos, yaw, 0, 1, 0);
			//fprintf(g_applog, "prepos2 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			//float lat = atan2(wrappos.y, wrappos.x);
			float lat = 0.5f - asin(wrappos.y)/M_PI;
			if(ISNAN(lat))
				ErrMess("asdsdg","nanlat");
			pt->orc.x = yaw / (2.0f * M_PI);
			pt->orc.y = lat / (1.0f * M_PI);

			wrappos = Rotate(Vec3f(1000*1,0,0), lat, 0, 0, 1);
			//fprintf(g_applog, "prepos3 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			wrappos = Rotate(wrappos, -yaw, 0, 1, 0);
			pt->wrappos = wrappos;
			if(ISNAN(wrappos.x))
				ErrMess("yhjgjf","wpnnxww");
			if(ISNAN(wrappos.y))
				ErrMess("yhjgjf","wpnnyww");
			if(ISNAN(wrappos.z))
				ErrMess("yhjgjf","wpnnzww");
			//fprintf(g_applog, "1pos %f,%f yaw,lat,%f,%f\r\n", pt->orc.x, pt->orc.y, yaw, lat);
			//fprintf(g_applog, "prepos4 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
		}

#if 0
		for(int v=0; v<3; v++)
		{
			SurfPt *pt = tet->neib[v];
			SurfPt *pt2 = tet->neib[(v+1)%3];

			ClassifyEdge(pt->orc, pt2->orc,
				&tet->edgeposx[v],
				&tet->edgeplaced[v],
				&tet->edgeposy[v]);
		}
#endif
#if 1
		for(int v=0; v<3; v++)
		{
			
			SurfPt *pt = tet->neib[v];
			while(pt->orc.x < 0)
				pt->orc.x += 1;
			while(pt->orc.y < 0)
				pt->orc.y += 1;
			while(pt->orc.x >= 1)
				pt->orc.x -= 1;
			while(pt->orc.y >= 1)
				pt->orc.y -= 1;
		}
#endif
	}

	//return true;

	bool haveupdown = false;

	int bestupdown = -1;

	
	bool progress = true;

again:
	haveupdown = false;

	//Test3(surf);

	int currupdown = 0;

	//progress = false;
	progress = true;
#if 011
	if(progress)
	{
			for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet * tet = *tit;
		
			for(int v=0; v<3; v++)
			{
				tet->neib[v]->pressure = Vec3f(0,0,0);
			}
		}

		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet * tet = *tit;

			SurfPt *fatt[3];

			fatt[0] = tet->neib[0];
			fatt[1] = tet->neib[1];
			fatt[2] = tet->neib[2];

			Vec3f wrappos[3];
			wrappos[0] = fatt[0]->wrappos;
			wrappos[1] = fatt[1]->wrappos;
			wrappos[2] = fatt[2]->wrappos;

			//for(int v=0; v<3; v++)
			//	fprintf(g_applog, "wrappos[%d]%f,%f,%f\r\n",
			//	v,
			//	wrappos[v].x,
			//	wrappos[v].y,
			//	wrappos[v].z);

			Vec3f cen = (wrappos[0] + wrappos[1] + wrappos[2])/3.0f;
			Vec3f norm = Normal(wrappos);

				if(ISNAN(norm.x))
					ErrMess("yhjgjf","nannormx");
				if(ISNAN(norm.y))
					ErrMess("yhjgjf","nannormy");
				if(ISNAN(norm.z))
					ErrMess("yhjgjf","nannormz");

			Vec3f correctnorm = Normalize(cen);

			
				if(ISNAN(correctnorm.x))
					ErrMess("yhjgjf","nannormxcc");
				if(ISNAN(correctnorm.y))
					ErrMess("yhjgjf","nannormycc");
				if(ISNAN(correctnorm.z))
					ErrMess("yhjgjf","nannormzcc");

			//fprintf(g_applog, "corr%f,%f,%f nor%f,%f,%f\r\n", 
			//	correctnorm.x,
			//	correctnorm.y,
			//	correctnorm.z,
			//	norm.x,
			//	norm.y,
			//	norm.z);

			float dot = Dot(correctnorm, norm);

			//fprintf(g_applog, "dot%f\r\n", 
			//	dot);

			if(ISNAN(dot))
			{
				ErrMess("asfdgsdfg","dotnan");
			}

			if(dot < 0)
			{
				currupdown++;
				haveupdown = true;
				//dot *= 2;
				float dot2 = 1000.0f / (dot - 0.1f);
				//dot = fmax(dot, -10);

				if(ISNAN(dot2))
				{
					dot = -1000.0f / (0.001f);
				}
				else
					dot = dot2;
			}
			else
			{
				//float dot2 = 1.0 / dot;

				//if(ISNAN(dot2))
				//	dot = 1.0 / 0.001f;
				float dot2 = 10.0f / (0.1f + dot);
				//dot = fmax(dot, -10);

				if(ISNAN(dot2))
				{
					dot = 1.0f / (0.01f);
				}
				else
					dot = dot2;
			}
			//if(dot == 0)
			//	dot += 1;


			//if(dot >= 0.5f)
			//	;
			//else if(dot > 0 && dot < 0.5f)
			//	dot *= 2.6f;

			
			float a,b,c,s,area;

			a = Magnitude(wrappos[0]-wrappos[1]);
			b = Magnitude(wrappos[1]-wrappos[2]);
			c = Magnitude(wrappos[2]-wrappos[0]);

			s = (a+b+c)/2.0f;

			area = sqrt(s*(s-a)*(s-b)*(s-c));

			if(area == 0)
				area = 1;

			float strength = 10000.0f / area;

			if(!ISNAN(strength))
				dot *= strength;

			dot = fmin(dot, 30000);
			dot = fmax(dot, -30000);

			dot *= abs(bestupdown+2)/200.0f;

			//if(dot < 0 && area > 5000)
			//	dot *= 1000;

			//if(dot > 0)
			//	continue;
	#if 1
			if(dot > 0)
			{
				//continue;
				//dot = 1.0f / dot / 100.0f;

		//		dot = fmin(dot, 3);
			}
	#endif
			//if(dot < 0)
			{
	#if 0
				int minv = -1;
				float minvlen = 0;
				if(dot < 0)
				{
	#if 0
					for(int v=0; v<3; v++)
					{
						float vlen = Magnitude(wrappos[v] - wrappos[(v+1)%3]);

						if(vlen < minvlen || minv < 0)
						{
							minvlen = vlen;
							minv = v;
						}
					}
	#endif
					//check if there's an edge with another tet that it is correctly facing forward with
					for(int v=0; v<3; v++)
					{
						bool hasfair = false;

						for(std::list<Tet*>::iterator htit1=tet->neib[v]->holder.begin();
							htit1!=tet->neib[v]->holder.end();
							htit1++)
						{
							Tet *htet1 = *htit1;

							if(htet1 == tet)
								continue;

							for(std::list<Tet*>::iterator htit2=tet->neib[(v+1)%3]->holder.begin();
												htit2!=tet->neib[(v+1)%3]->holder.end();
												htit2++)
							{
								Tet *htet2 = *htit2;

								if(htet2 == tet)
									continue;
								if(htet2 != htet1)
									continue;

								//got 2 pts with htet2/htet1(same)
								//check winding to be clockwise

								//should it go p1,p2 or p2,p1?
								//and is it clockwise?
								//given these two answers, should the edge to be flipped be set to v?
								//bool edgeiscw = false;	//gives clockwise winding?
								//bool givesposdot = false;	//gives positive dot product using normal and direction from origin?

								//if(edgeiscw && !givesposdot

								//just care if this tet needs to be flipped too

								Vec3f tet2tri[3];
								tet2tri[0] = htet2->neib[0]->wrappos;
								tet2tri[1] = htet2->neib[1]->wrappos;
								tet2tri[2] = htet2->neib[2]->wrappos;

								float vlen = Dot( (tet2tri[0]+tet2tri[1]+tet2tri[2])/3.0f, Normal(tet2tri) );

								//if( vlen <= 0 && ( vlen < minvlen || minv < 0 ) )
								if(vlen < 0)
								{
								//	minv = v;
								//	minvlen = vlen;
								}
								else
								{
									hasfair = true;
									continue;
								}
							}
						}

						if(!hasfair)
						{
							minv = v;
							break;
						}
					}
				}
	#endif

				//if(dot < 0)
				//	minv = rand()%3;

				for(int v=0; v<3; v++)
				{
	#if 001
					///*
					Vec3f away = wrappos[v] - cen;

					if(Magnitude(away) <= 0)
						away = Vec3f(rand()%300-150, rand()%300-150, rand()%300-150);

					Vec3f dir = Normalize(away);
					//*/
	#endif
					/*
					Vec3f dir = Vec3f(0,0,0);

					if(v == minv)
					{
						dir = Normalize(wrappos[(v+1)%3] - wrappos[v]);
					}
					else if(v == (minv+1)%3 && minv >= 0)
					{
						dir = Normalize(wrappos[v] - wrappos[(v+3-1)%3]);
					}
					else if(minv < 0)
					{
						dir = wrappos[v] - cen;
						dir = Normalize(dir);
					}


					if(Magnitude(dir) <= 0)
						dir = Vec3f(rand()%300-150, rand()%300-150, rand()%300-150);

					dir = Normalize(dir);
	*/
					//while(abs(dot) < 500)
					//dot *= 30;

					tet->neib[v]->pressure = 
						tet->neib[v]->pressure + 
						dir * dot / 10;
					tet->neib[v]->pressure =
						tet->neib[v]->pressure +
						Normal(wrappos) * 1000;
					//tet->neib[v]->pressure = 
					//	tet->neib[v]->pressure +
					//	Vec3f(rand()%300-150, rand()%300-150, rand()%300-150)/100.0f;
					
					if(ISNAN(tet->neib[v]->pressure.x))
						ErrMess("sdsfg","pressnanx");
					if(ISNAN(tet->neib[v]->pressure.y))
						ErrMess("sdsfg","pressnany");
					if(ISNAN(tet->neib[v]->pressure.z))
						ErrMess("sdsfg","pressnanz");
			//	fprintf(g_applog, "1dot%f press%f,%f,%f dir%f,%f,%f\r\n", dot, 
			//		tet->neib[v]->pressure.x,
			//		tet->neib[v]->pressure.y,
			//		tet->neib[v]->pressure.z,
			//		dir.x, dir.y, dir.z);
				}
			}
		}
	}
#endif
#if 01

	else
	{

		for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
		{
			Tet * tet = *tit;

			SurfPt *fatt[3];

			fatt[0] = tet->neib[0];
			fatt[1] = tet->neib[1];
			fatt[2] = tet->neib[2];

			Vec3f wrappos[3];
			wrappos[0] = fatt[0]->wrappos;
			wrappos[1] = fatt[1]->wrappos;
			wrappos[2] = fatt[2]->wrappos;

			//for(int v=0; v<3; v++)
			//	fprintf(g_applog, "wrappos[%d]%f,%f,%f\r\n",
			//	v,
			//	wrappos[v].x,
			//	wrappos[v].y,
			//	wrappos[v].z);

			Vec3f cen = (wrappos[0] + wrappos[1] + wrappos[2])/3.0f;
			Vec3f norm = Normal(wrappos);

				if(ISNAN(norm.x))
					ErrMess("yhjgjf","nannormx");
				if(ISNAN(norm.y))
					ErrMess("yhjgjf","nannormy");
				if(ISNAN(norm.z))
					ErrMess("yhjgjf","nannormz");

			Vec3f correctnorm = Normalize(cen);

			
				if(ISNAN(correctnorm.x))
					ErrMess("yhjgjf","nannormxcc");
				if(ISNAN(correctnorm.y))
					ErrMess("yhjgjf","nannormycc");
				if(ISNAN(correctnorm.z))
					ErrMess("yhjgjf","nannormzcc");

			//fprintf(g_applog, "corr%f,%f,%f nor%f,%f,%f\r\n", 
			//	correctnorm.x,
			//	correctnorm.y,
			//	correctnorm.z,
			//	norm.x,
			//	norm.y,
			//	norm.z);

			float dot = Dot(correctnorm, norm);

			//fprintf(g_applog, "dot%f\r\n", 
			//	dot);

			if(ISNAN(dot))
			{
				ErrMess("asfdgsdfg","dotnan");
			}

			if(dot < 0)
			{
				currupdown++;
				haveupdown = true;
			}
		}
	}
#endif

#if 0
void GetToEmerge(Tet *etet,
				 Vec3f *emline,
				 SurfPt **esp);
void Emerge(Surf *surf,
			SurfPt *esp,
			Vec3f emline[2]);
void CheckEmerged(Surf *surf, Tet** halfemerged);
#endif

#if 0
	Tet *emtet = NULL;
	CheckEmerged(surf, &emtet);
	if(emtet)
	{
		Vec3f emline[2];
		SurfPt *esp = NULL;
		GetToEmerge(surf, emtet, emline, &esp);
		if(esp)
		{
			Emerge(surf, esp, emline);
		}
	}
#endif

	if(currupdown < bestupdown ||
		bestupdown < 0)
	{
		std::string dt = DateTime();
		fprintf(g_applog, "curr updown: %d (%s)\r\n", currupdown, dt.c_str());
		fflush(g_applog);

		if(!currupdown)
			return true;

		//if(bestupdown < 0 ||
		//	currupdown < bestupdown)
		bestupdown = currupdown;
		if(rand()%199==1)
			progress = true;
	}
	else
	{
		if(rand()%1000==1)
		{
			std::string dt = DateTime();
			fprintf(g_applog, "curr bad updown: %d (%s)\r\n", currupdown, dt.c_str());
			fflush(g_applog);
		}

		if(rand()%19==1 || currupdown > bestupdown * 1.1f)
			progress = false;
	}

	static float avgupdown = currupdown;
	static __int64 lasttick = GetTicks();

	avgupdown = avgupdown * 29/30.0 + currupdown * 1/30.0;

	if(GetTicks() - lasttick > 1000*60*3)
	{
		//avgupdown = avgupdown * 2/3.0 + currupdown * 1/3.0;
		std::string dt2 = DateTime();
		fprintf(g_applog, "avgupdown: %f (%s)\r\n", avgupdown, dt2.c_str());
		fflush(g_applog);
		lasttick = GetTicks();
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
	{
		Tet * tet = *tit;
	
		for(int v=0; v<3; v++)
		{
			float mag = Magnitude( tet->neib[v]->wrappos );
			if( mag == 0 )
				mag = 1;
			tet->neib[v]->wrappos = 
				tet->neib[v]->wrappos + tet->neib[v]->pressure;
			tet->neib[v]->wrappos = 
				(tet->neib[v]->wrappos * 1000*1 / mag );
			tet->neib[v]->pressure = Vec3f(0,0,0);

			
				if(ISNAN(tet->neib[v]->wrappos.x))
					ErrMess("sdsfg","wrappos2nanx");
				if(ISNAN(tet->neib[v]->wrappos.y))
					ErrMess("sdsfg","wrappos2nany");
				if(ISNAN(tet->neib[v]->wrappos.z))
					ErrMess("sdsfg","wrappos2nanz");
		}
	}

	static int times = 0;
	times++;

#if 0001
	if(/* times < 3000 || */ haveupdown)
	{
	//	if(rand()%190==1)
	//		SplitEdges(surf, NULL, NULL, NULL, 2);

		goto again;
	}
#endif
	//return true;
again2:

	
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
	
		for(int v=0; v<3; v++)
		{
			tet->neib[v]->pressure = Vec3f(0,0,0);
		}
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;

		SurfPt *fatt[3];

		fatt[0] = tet->neib[0];
		fatt[1] = tet->neib[1];
		fatt[2] = tet->neib[2];

		Vec3f wrappos[3];
		wrappos[0] = fatt[0]->wrappos;
		wrappos[1] = fatt[1]->wrappos;
		wrappos[2] = fatt[2]->wrappos;

		//for(int v=0; v<3; v++)
		//	fprintf(g_applog, "wrappos[%d]%f,%f,%f\r\n",
		//	v,
		//	wrappos[v].x,
		//	wrappos[v].y,
		//	wrappos[v].z);

		Vec3f cen = (wrappos[0] + wrappos[1] + wrappos[2])/3.0f;
		Vec3f norm = Normal(wrappos);

		if(ISNAN(norm.x))
		{
			ErrMess("sfgsfg","nbannormx");
		}
		if(ISNAN(norm.y))
		{
			ErrMess("sfgsfg","nbannormy");
		}
		if(ISNAN(norm.z))
		{
			ErrMess("sfgsfg","nbannormz");
		}

		Vec3f correctnorm = Normalize(cen);

		if(ISNAN(correctnorm.x))
		{
			ErrMess("sfgsfg","nbannormxcorrectnorm");
		}
		if(ISNAN(correctnorm.y))
		{
			ErrMess("sfgsfg","nbannormycorrectnorm");
		}
		if(ISNAN(correctnorm.z))
		{
			ErrMess("sfgsfg","nbannormzcorrectnorm");
		}

		//fprintf(g_applog, "corr%f,%f,%f nor%f,%f,%f\r\n", 
		//	correctnorm.x,
		//	correctnorm.y,
		//	correctnorm.z,
		//	norm.x,
		//	norm.y,
		//	norm.z);

		float dot = Dot(correctnorm, norm);

		if(ISNAN(dot))
		{
			ErrMess("sfgdsfg","dotnan2");
		}
		//fprintf(g_applog, "dot%f\r\n", 
		//	dot);

		if(dot < 0)
		{
			haveupdown = true;
			//continue;
			//goto again;
			//dot *= 2;
			//dot = 1.0f / dot;
			//dot = fmax(dot, -10);
		}
		//if(dot == 0)
		//	dot += 1;

		//if(dot > 0)
		//	dot *= 2.0f;

		//if(dot > 0)
		//	continue;

		float a,b,c,s,area;

		a = Magnitude(wrappos[0]-wrappos[1]);
		b = Magnitude(wrappos[1]-wrappos[2]);
		c = Magnitude(wrappos[2]-wrappos[0]);

		s = (a+b+c)/2.0f;

		area = sqrt(s*(s-a)*(s-b)*(s-c));

		if(area == 0)
			area = 1;

		float strength = 10.0f / area;

		if(!ISNAN(strength))
		{

			strength = fmin(strength, 30);
			strength = fmax(strength, -30);
		}
		else
			strength = 0.1f;

#if 1
		//if(dot > 0)
		{
			//continue;
			//dot = 1.0f / dot;

	//		dot = fmin(dot, 3);
		}
#endif
		//if(dot < 0)
		{
			for(int v=0; v<3; v++)
			{
				Vec3f dir = Normalize(wrappos[v] - cen);
				tet->neib[v]->pressure = 
					tet->neib[v]->pressure + 
					dir * strength;
				
			//fprintf(g_applog, "1dot%f press%f,%f,%f dir%f,%f,%f\r\n", dot, 
			///	tet->neib[v]->pressure.x,
			///	tet->neib[v]->pressure.y,
			//	tet->neib[v]->pressure.z,
			//	dir.x, dir.y, dir.z);
			}
		}
	}

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
			tit!=surf->tets2.end();
			++tit)
	{
		Tet * tet = *tit;
	
		for(int v=0; v<3; v++)
		{
			float mag = Magnitude( tet->neib[v]->wrappos );
			if( mag == 0 )
				mag = 1;
			tet->neib[v]->wrappos = 
				tet->neib[v]->wrappos + tet->neib[v]->pressure;
			tet->neib[v]->wrappos = 
				(tet->neib[v]->wrappos * 1000*1 / mag );
			tet->neib[v]->pressure = Vec3f(0,0,0);
		}
	}

	static int times2 = 0;
	times2++;

	if(haveupdown)
	{
		times2=0;
		goto again;
	}

	if(times2 < 3000)
		goto again2;

again3:

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		Tet * tet = *tit;
			tet->placed = true;
		for(int v=0; v<3; v++)
		{
			SurfPt *pt = tet->neib[v];
			pt->placed = true;
			Vec3f wrappos = pt->wrappos;
			//float yaw = atan2(wrappos.x, wrappos.z);
			//wrappos = Rotate(wrappos, yaw, 0, 1, 0);
			//float lat = atan2(wrappos.y, wrappos.x);
			
			float yaw = 0.5f + atan2(wrappos.z, wrappos.x) / (2.0f*M_PI);
			if(ISNAN(yaw))
				ErrMess("asdsdg","nanyaw");
			//tan(0)=op/adj=0/1
			//fprintf(g_applog, "prepos1 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			//wrappos = Rotate(wrappos, yaw, 0, 1, 0);
			//fprintf(g_applog, "prepos2 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
			//float lat = atan2(wrappos.y, wrappos.x);
			float lat = 0.5f - asin(wrappos.y)/M_PI;

			pt->orc.x = yaw / (2.0f * M_PI);
			pt->orc.y = lat / (1.0f * M_PI);

			if(ISNAN(pt->orc.x))
				ErrMess("sfgsdf","forcxnan");

			if(ISNAN(pt->orc.y))
				ErrMess("sfgsdf","forcynan");

			//wrappos = Rotate(Vec3f(100,0,0), lat, 0, 0, 1);
			//wrappos = Rotate(wrappos, lat, 0, 1, 0);
			//pt->wrappos = wrappos;
			//fprintf(g_applog, "pos %f,%f yaw,lat,%f,%f\r\n", pt->orc.x, pt->orc.y, yaw, lat);
		}	
		
#if 0
		for(int v=0; v<3; v++)
		{
			SurfPt *pt = tet->neib[v];
			SurfPt *pt2 = tet->neib[(v+1)%3];

			ClassifyEdge(pt->orc, pt2->orc,
				&tet->edgeposx[v],
				&tet->edgeplaced[v],
				&tet->edgeposy[v]);
		}
#endif
		for(int v=0; v<3; v++)
		{
			SurfPt *pt = tet->neib[v];
			
			while(pt->orc.x < 0)
				pt->orc.x += 1;
			while(pt->orc.y < 0)
				pt->orc.y += 1;
			while(pt->orc.x >= 1)
				pt->orc.x -= 1;
			while(pt->orc.y >= 1)
				pt->orc.y -= 1;
		}
	}


	return true;
}

void OutMesh(Surf *surf)
{
	return;

	fprintf(g_applog, "\r\n \r\n =========== OUT MESH: ========= \r\n \r\n");

	fprintf(g_applog, "\r\n tets: %d, pt's: %d \r\n ", (int)surf->tets2.size(), (int)surf->pts2.size());
	fflush(g_applog);

	int tin = 0;

	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit, ++tin)
	{
		
		fprintf(g_applog, "\r\n \r\n------------- tet %d: ----------------- \r\n \r\n", tin);
		Tet *tet = *tit;

		int erpn = 0;

		for(int v=0; v<3; v++)
		{
			SurfPt *sp = tet->neib[v];

			fprintf(g_applog, "\r\n neib[%d] = rp:%f,%f,%f \r\n",
				v,
				sp->pos.x,
				sp->pos.y,
				sp->pos.z);
			fflush(g_applog);

			if(sp->pos.x >= -150 &&
				sp->pos.x <= 150 &&
				sp->pos.y >= 0 &&
				sp->pos.y <= 100 &&
				sp->pos.z >= -550 &&
				sp->pos.z <= 250)
			{
				erpn++;
				fprintf(g_applog, "erp^^^%d\r\n", erpn);
				fflush(g_applog);
			}
		}

		fflush(g_applog);
	}
	
	fprintf(g_applog, "\r\n \r\n =========== OUT MESH^ ========= \r\n \r\n");
	fflush(g_applog);
}

void OrRender(int rendstage, Vec3f offset)
{
	for(std::list<Tet*>::iterator etit=g_surf.tets2.begin();
		etit!=g_surf.tets2.end();
		++etit)
		delete *etit;
	
	for(std::list<SurfPt*>::iterator epit=g_surf.pts2.begin();
		epit!=g_surf.pts2.end();
		++epit)
		delete *epit;

	for(std::list<Tet*>::iterator etit=g_fullsurf.tets2.begin();
		etit!=g_fullsurf.tets2.end();
		++etit)
		delete *etit;
	
	for(std::list<SurfPt*>::iterator epit=g_fullsurf.pts2.begin();
		epit!=g_fullsurf.pts2.end();
		++epit)
		delete *epit;

	g_surf.pts2.clear();
	g_surf.tets2.clear();
	g_fullsurf.pts2.clear();
	g_fullsurf.tets2.clear();
	//Surf surf, fullsurf;
	//SurfPt *p1;
	//StartRay(&surf, tet, Vec3f(0,30000,0), &p1);

	//fprintf(g_applog, "\r\n111\r\n");
	//fflush(g_applog);

	if(!AddClipMesh(&g_surf, &g_fullsurf))
		return;

	Test(&g_surf);
	
	//fprintf(g_applog, "\r\n222\r\n");
	//fflush(g_applog);

	/*
problem: non-continuous/non-connected "real" triangles
are connected continuously, giving uneven jumps in the posxyz maps

also, if a triangle is clipped into two that used to share an edge
with another triangle, there will be free floating vertices!

	*/
	do
	{
	TestC(&g_surf, __FILE__, __LINE__);
		if(!ClipTris(&g_surf, &g_fullsurf))
			break;
	TestC(&g_surf, __FILE__, __LINE__);
		Test(&g_surf);
	fprintf(g_applog, "\r\n333\r\n");
	fflush(g_applog);
		if(!JoinPts(&g_surf, &g_fullsurf))
			break;
	fprintf(g_applog, "\r\n444\r\n");
	fflush(g_applog);
	TestC(&g_surf, __FILE__, __LINE__);
	fprintf(g_applog, "\r\n444111\r\n");
	fflush(g_applog);
	TestC(&g_surf, __FILE__, __LINE__);
		if(!RemFloaters(&g_surf, &g_fullsurf))
			break;
	fprintf(g_applog, "\r\n444222\r\n");
	fflush(g_applog);
	TestC(&g_surf, __FILE__, __LINE__);
		if(!RemHid2(&g_surf, false))
			break;
	fprintf(g_applog, "\r\n444333\r\n");
	fflush(g_applog);
		if(!RemOverEdges(&g_surf, &g_fullsurf))
			break;
	fprintf(g_applog, "\r\n555\r\n");
	fflush(g_applog);
		if(MarkVis(&g_surf, &g_fullsurf))
			break;
	fprintf(g_applog, "\r\n666\r\n");
	fflush(g_applog);
		if(!RemHidden(&g_surf, &g_fullsurf))
			break;
	fprintf(g_applog, "\r\n777\r\n");
	fflush(g_applog);
		Vec2f vmin(0.5f,0.5f), vmax(0.5,0.5);
		//if(!SplitEdges(&g_surf, &g_fullsurf, &vmin, &vmax, BIGTEX*BIGTEX))
		//	break;
		//Test2(&surf);
		OutMesh(&g_surf);
	fprintf(g_applog, "\r\n888111\r\n");
	fflush(g_applog);
	//	if(!GrowMapMesh(&surf, &fullsurf, &vmin, &vmax))
	//		break;
	//	if(!JoinPts2(&g_surf, &g_fullsurf))
	//		break;
	if(!MapGlobe4(&g_surf))
		break;

#if 0
	LoadedTex outtex[8];
	for(int i=0; i<8; i++)
		AllocTex(&outtex[i], BIGTEX, BIGTEX, 3);
	//OutTex(&surf, outtex);
	OutTex2(&g_surf, outtex);
	for(int t=0; t<8; t++)
	{
		char outpath[SPE_MAX_PATH+1];
		char file[32];
		sprintf(file, "renders/outa%d.png", t);
		FullPath(file, outpath);

		SavePNG2(outpath, &outtex[t]);
	}

#endif

	fprintf(g_applog, "\r\n888\r\n");
	fflush(g_applog);
		//if(!BalanceMesh(&surf, &fullsurf, &vmin, &vmax))
		//	break;
	fprintf(g_applog, "\r\n999\r\n");
	fflush(g_applog);
		break;
	}while(0);

	fprintf(g_applog, "\r\n000\r\n");
	fflush(g_applog);

	LoadedTex outtex[8];
	//for(int i=0; i<8; i++)
	//	AllocTex(&outtex[i], BIGTEX, BIGTEX, 3);

	AllocTex(&outtex[0], g_bigtex, g_bigtex, 3);	//diffuse
	AllocTex(&outtex[1], g_orwpx * g_orlons, g_orhpx * g_orlats, 3);	//jump/islands map
	AllocTex(&outtex[2], g_bigtex, g_bigtex, 3);	//posx
	AllocTex(&outtex[3], g_bigtex, g_bigtex, 3);	//posy
	AllocTex(&outtex[4], g_bigtex, g_bigtex, 3);	//posz

	//OutTex(&surf, outtex);
	OutTex2(&g_surf, outtex);

#if 0
	for(int t=0; t<8; t++)
	{
		char outpath[SPE_MAX_PATH+1];
		char file[32];
		sprintf(file, "renders/out%d.png", t);
		FullPath(file, outpath);

		//SavePNG2(outpath, &outtex[t]);
	}
#else
	{
		char outpath[SPE_MAX_PATH+1];
		NameRender(outpath, -1);
		strcat(outpath, "_diff.png");
		//sprintf(outpath, "%s_diff.png", g_renderbasename);
		SavePNG2(outpath, &outtex[0]);
	}
	{
		char outpath[SPE_MAX_PATH+1];
		NameRender(outpath, -1);
		strcat(outpath, "_isle.png");
		//sprintf(outpath, "%s_isle.png", g_renderbasename);
		SavePNG2(outpath, &outtex[1]);
	}
	{
		char outpath[SPE_MAX_PATH+1];
		NameRender(outpath, -1);
		strcat(outpath, "_posx.png");
		//sprintf(outpath, "%s_posx.png", g_renderbasename);
		SavePNG2(outpath, &outtex[2]);
	}
	{
		char outpath[SPE_MAX_PATH+1];
		NameRender(outpath, -1);
		strcat(outpath, "_posy.png");
		//sprintf(outpath, "%s_posy.png", g_renderbasename);
		SavePNG2(outpath, &outtex[3]);
	}
	{
		char outpath[SPE_MAX_PATH+1];
		NameRender(outpath, -1);
		strcat(outpath, "_posz.png");
		//sprintf(outpath, "%s_posz.png", g_renderbasename);
		SavePNG2(outpath, &outtex[4]);
	}
#endif
	//InfoMess("Done", "Done rendering orientability map");

	GUI* gui = &g_gui;
	g_mode = EDITOR;
	gui->hideall();
	gui->show("editor");
}

#if 0
void OrRender2(int rendstage, Vec3f offset)
{
	Surf surf;

	surf.tets.push_back(Tet());
	Tet* tet = &*surf.tets.rbegin();
	tet->level = 0;
	
	
//	StartRay(&surf, tet, Vec3f(0,-30000,30000));	//bottom far

#if 1
	SurfPt *p1, *p2, *p3, *p4;
	StartRay(&surf, tet, Vec3f(-30000,30000,0), &p1);	//top left
	StartRay(&surf, tet, Vec3f(0,-30000,30000), &p2);	//bottom far
	StartRay(&surf, tet, Vec3f(0,-30000,-30000), &p3);	//bottom near
	StartRay(&surf, tet, Vec3f(30000,30000,0), &p4);	//top right
	
	tet->level=0;
	tet->neib[0] = p1;
	tet->neib[1] = p2;
	tet->neib[2] = p3;
	tet->neib[3] = NULL;

	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level=0;
	tet->neib[0] = p1;
	tet->neib[1] = p2;
	tet->neib[2] = p4;
	tet->neib[3] = NULL;
	
	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level=0;
	tet->neib[0] = p1;
	tet->neib[1] = p3;
	tet->neib[2] = p4;
	tet->neib[3] = NULL;

	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level=0;
	tet->neib[0] = p2;
	tet->neib[1] = p3;
	tet->neib[2] = p4;
	tet->neib[3] = NULL;

#if 0
	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level = 0;
	StartRay(&surf, tet, Vec3f(-30000,30000,0));	//top left
	StartRay(&surf, tet, Vec3f(0,-30000,30000));	//bottom far
	//StartRay(&surf, tet, Vec3f(0,-30000,-30000));	//bottom near
	StartRay(&surf, tet, Vec3f(30000,30000,0));	//top right


	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level = 0;
	StartRay(&surf, tet, Vec3f(-30000,30000,0));	//top left
	//StartRay(&surf, tet, Vec3f(0,-30000,30000));	//bottom far
	StartRay(&surf, tet, Vec3f(0,-30000,-30000));	//bottom near
	StartRay(&surf, tet, Vec3f(30000,30000,0));	//top right

	surf.tets.push_back(Tet());
	tet = &*surf.tets.rbegin();
	tet->level = 0;
	//StartRay(&surf, tet, Vec3f(-30000,30000,0));	//top left
	StartRay(&surf, tet, Vec3f(30000,30000,0));	//top right
	StartRay(&surf, tet, Vec3f(0,-30000,30000));	//bottom far
	StartRay(&surf, tet, Vec3f(0,-30000,-30000));	//bottom near
#endif

	float fatlinew = 0;
	float fattriw = 0, secfattriw = 0;

	do
	{
		SurfPt* fatt[3] = {NULL,NULL,NULL};
		SurfPt* secfatt[3] = {NULL,NULL,NULL};
		fprintf(g_applog, "\r\fa0[0,1,2]=%d,%d,%d\r\n", (int)fatt[0], (int)fatt[1], (int)fatt[2]);
		fflush(g_applog);
		tet=NULL;
		fattriw = 0;
		secfattriw = 0;
		if(!GetFatTri(&surf, &tet, fatt, secfatt, &fattriw, &secfattriw))
		{
			
		fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
			return;
		}
		//SurfPt *fatl[2] = {NULL};
		//GetFatLine(tet, &fatl, &fatlinew);

		//if(!fatt[0])
		//	ErrMess("!","!");
		fprintf(g_applog, "\r\nr%f\r\nn%d t%d\r\n", fattriw, (int)surf.pts.size(), (int)surf.tets.size());
		fprintf(g_applog, "\r\fa[0,1,2]=%d,%d,%d\r\n", (int)fatt[0], (int)fatt[1], (int)fatt[2]);
		fflush(g_applog);

		if(!SplitTri(&surf, tet, fatt))
		{
			
		fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
			return;
		}

		fprintf(g_applog, "\r\nr%f\r\nn%d\r\nt%d\r\n", fattriw, (int)surf.pts.size(), (int)surf.tets.size());
		fflush(g_applog);

		if(fattriw <= 3)
		{
		fprintf(g_applog, "!%s %d", __FILE__, __LINE__);
		fflush(g_applog);
			return;
		}

	}while(1);
#endif
}
#endif

void OrRender1(int rendstage, Vec3f offset)
{
	LoadedTex lookuptex;//isles
	LoadedTex rendtex;//rgb
	LoadedTex spectex;//rgb
	LoadedTex normtex;//rgb
	LoadedTex localsouth;	//16b lat ., 8b lon
	LoadedTex surfnorm, //16,8 
		surfdepth;	
	LoadedTex xtex,ytex,ztex;

	//AllocTex(&lookuptex, g_orwpx * g_orlats, g_orhpx * g_orlons, 3);
	//AllocTex(&rendtex, g_orwpx * g_orlats, g_orhpx * g_orlons, 3);
	//AllocTex(&localsouth, g_orwpx * g_orlats, g_orhpx * g_orlons, 3);

/////////////

	Vec2i vmin, vmax;
	AllScreenMinMax(&vmin, &vmax, g_width, g_height);

	int wx = vmax.x - vmin.x;
	int wy = vmax.y - vmin.y;

#if 001
	wx = BIGTEX;
	wy = BIGTEX;

	//AllocTex(&lookuptex, g_orwpx * g_orlats, g_orhpx * g_orlons, 3);
	AllocTex(&rendtex, wx, wy, 3);
	AllocTex(&xtex, wx, wy, 3);
	AllocTex(&ytex, wx, wy, 3);
	AllocTex(&ztex, wx, wy, 3);
	AllocTex(&spectex, wx, wy, 3);
	AllocTex(&normtex, wx, wy, 3);
	AllocTex(&surfnorm, wx, wy, 3);
	AllocTex(&surfdepth, wx, wy, 3);
	//AllocTex(&localsouth, g_orwpx * g_orlats, g_orhpx * g_orlons, 3);

	//for(int px=vmin.x; px<=vmax.x; ++px)
	for(int px=0; px<BIGTEX; ++px)
	{
		//for(int py=vmin.y; py<=vmax.y; ++py)
		for(int py=0; py<BIGTEX; ++py)
		{
			unsigned char pr = 0;
			unsigned char pg = 0;
			unsigned char pb = 0;
			unsigned char pa = 0;
			
			unsigned char pr2 = 0;
			unsigned char pg2 = 0;
			unsigned char pb2 = 0;
			unsigned char pa2 = 0;

			
			unsigned char spr = 0;
			unsigned char spg = 0;
			unsigned char spb = 0;
			unsigned char spa = 0;
			
			unsigned char spr2 = 0;
			unsigned char spg2 = 0;
			unsigned char spb2 = 0;
			unsigned char spa2 = 0;


			
			unsigned char npr = 0;
			unsigned char npg = 0;
			unsigned char npb = 0;
			unsigned char npa = 0;
			
			unsigned char npr2 = 0;
			unsigned char npg2 = 0;
			unsigned char npb2 = 0;
			unsigned char npa2 = 0;

			//Vec3f onnear = OnNear(g_width/2, g_height/2, g_width, g_height, 
			//	g_cam.lookpos(), g_cam.m_strafe, g_cam.up2());

			//Vec3f dirvec = Normalize(g_cam.m_view - g_cam.m_pos);

			Vec3f line[2];
			line[1] = Vec3f(0,0,0);
			line[0] = Vec3f(0,100000,0);

			line[0] = Rotate(line[0], DEGTORAD((90 * (float)py/(float)wy)), 1, 0, 0);
			line[0] = Rotate(line[0], DEGTORAD((180 - 360.0f * (float)px/(float)wx)), 0, 1, 0);

			//line[0] = onnear;
			//line[1] = onnear + dirvec * 1000000.0;

			//line[1] = Vec3f(0,0,0) - line[0];

			int nearesti = -1;
			float nearestd = Magnitude(line[1]-line[0]);
			char neartype = 0;
			Vec3f nearestv;
			Vec3f nearnorm;

			int ci = 0;

			Vec2f retexc;
			Texture *retex, *retexs, *retexn;

			for(std::list<ModelHolder>::iterator mit=g_modelholder.begin();
				mit!=g_modelholder.end(); 
				++mit, ++ci)
			{
				Vec3f p = mit->traceray(line, &pr2, &pg2, &pb2, &pa2, 
					&spr2, &spg2, &spb2, &spa2,
					&npr2, &npg2, &npb2, &npa2,
					&retex, &retexs, &retexn,
					&retexc,
					&nearnorm, true);

				if(p != line[1])
				{
					//fprintf(g_applog, "p1 %f,%f,%f \r\n", p.x, p.y, p.z);
					neartype = 1;

					float d = Magnitude(p - line[0]);

					if(d < nearestd || nearesti < 0)
					{
						//fprintf(g_applog, "p3 %f,%f,%f \r\n", p.x, p.y, p.z);

						nearestd = d;
						nearesti = ci;
						pr = pr2;
						pg = pg2;
						pb = pb2;
						pa = pa2;
						npr = npr2;
						npg = npg2;
						npb = npb2;
						npa = npa2;
						spr = spr2;
						spg = spg2;
						spb = spb2;
						spa = spa2;
						nearestv = p;
						line[0] = nearestv;
					}
				}
			}
			ci = 0;
			for(std::list<Brush>::iterator bit=g_edmap.m_brush.begin();
				bit!=g_edmap.m_brush.end();
				bit++, ci++)
			{
				Vec3f p = bit->traceray(line, &pr2, &pg2, &pb2, &pa2, 
					&spr2, &spg2, &spb2, &spa2,
					&npr2, &npg2, &npb2, &npa2,
					&retex, &retexs, &retexn,
					&retexc,
					&nearnorm);
				
				if(p != line[1])
				{
					//fprintf(g_applog, "p2 %f,%f,%f \r\n", p.x, p.y, p.z);

					float d = Magnitude(p - line[0]);

					if(d < nearestd || nearesti < 0)
					{
						//fprintf(g_applog, "p4 %f,%f,%f \r\n", p.x, p.y, p.z);

						neartype = 2;
						nearestd = d;
						nearesti = ci;
						pr = pr2;
						pg = pg2;
						pb = pb2;
						pa = pa2;
						npr = npr2;
						npg = npg2;
						npb = npb2;
						npa = npa2;
						spr = spr2;
						spg = spg2;
						spb = spb2;
						spa = spa2;
						nearestv = p;
						line[0] = nearestv;
					}
				}
			}

			rendtex.data[3 * (px + py * wx) + 0] = pr;
			rendtex.data[3 * (px + py * wx) + 1] = pg;
			rendtex.data[3 * (px + py * wx) + 2] = pb;
		//	rendtex.data[3 * (px + py * wx) + 3] = pa;

			spectex.data[3 * (px + py * wx) + 0] = spr;
			spectex.data[3 * (px + py * wx) + 1] = spg;
			spectex.data[3 * (px + py * wx) + 2] = spb;
			
			normtex.data[3 * (px + py * wx) + 0] = npr;
			normtex.data[3 * (px + py * wx) + 1] = npg;
			normtex.data[3 * (px + py * wx) + 2] = npb;

			//unsigned int xc = (unsigned int)(50000+nearestv.x);
			//unsigned int yc = (unsigned int)(50000+nearestv.y);
			//unsigned int zc = (unsigned int)(50000+nearestv.z);

			xtex.data[3 * (px + py * wx) + 2] = 0;
			ytex.data[3 * (px + py * wx) + 2] = 0;
			ztex.data[3 * (px + py * wx) + 2] = 0;

			*(unsigned short*)&(xtex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.x);
			*(unsigned short*)&(ytex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.y);
			*(unsigned short*)&(ztex.data[3 * (px + py * wx) + 0]) = (unsigned short)(30000+nearestv.z);

			unsigned short neard = nearestd;
			*(unsigned short*)&(surfdepth.data[3 * (px + py * wx) + 0]) = neard;

			nearnorm = Normalize(nearnorm);

			surfnorm.data[3 * (px + py * wx) + 0] = (signed short)(nearnorm.x * 127);
			surfnorm.data[3 * (px + py * wx) + 1] = (signed short)(nearnorm.y * 127);
			surfnorm.data[3 * (px + py * wx) + 2] = (signed short)(nearnorm.z * 127);
		}
	}
	////////////////////
#endif

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
	SavePNG2(fullpath, &rendtex);

	stage = "_posx";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &xtex);

	stage = "_posy";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &ytex);

	stage = "_posz";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &ztex);
	
	stage = "_surfnorm";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &surfnorm);

	stage = "_surfdepth";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &surfdepth);
	
	stage = "_norm";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &normtex);
	
	stage = "_spec";
	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG2(fullpath, &spectex);

	rendtex.destroy();
	xtex.destroy();
	ytex.destroy();
	ztex.destroy();
	spectex.destroy();
	normtex.destroy();
	surfnorm.destroy();
	surfdepth.destroy();
}