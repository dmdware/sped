
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


//#define BIGTEX	4096
#define BIGTEX	256

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
	winding_t	*neww;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = DotProduct (in->points[i], split->normal);
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
	neww = NewWinding (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];
		
		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
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
			
		VectorCopy (mid, neww->points[neww->numpoints]);
		neww->numpoints++;
	}
	
	if (neww->numpoints > maxpts)
		Error ("ClipWinding: points exceeded estimate");
		
// free the original winding
	FreeWinding (in);
	
	return neww;
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

SurfPt* CopyPt(SurfPt *sp, Tet *totet)
{
	for(int vi=0; vi<4; ++vi)
	{
		if(!totet->neib[vi])
		{
			SurfPt* sp2 = new SurfPt;
			totet->neib[vi] = sp2;
			sp2->gen = sp->gen+1;
			sp2->holder = totet;
			sp2->norm = sp->norm;
			sp2->pos = sp->pos;
			sp2->tex = sp->tex;
			sp2->stex = sp->stex;
			sp2->ntex = sp->ntex;

			for(std::list<Tet*>::iterator hit=sp->holder.begin();
				hit!=sp->holder.end();
				++hit)
				sp2->holder.push_back(*hit);

			return sp2;
		}
	}
	return NULL;
}
void CopyTet(Surf *surf, Tet *fromtet)	//just copies tet, not pt's, which will be handled differently
{
	Tet *newtet = new Tet;
	surf->tets2.push_back(newtet);

	newtet->level = fromtet->level+1;
	newtet->neib[0] = NULL;
	newtet->neib[1] = NULL;
	newtet->neib[2] = NULL;
	newtet->neib[3] = NULL;
	newtet->hidden = false;
	newtet->approved = false;
	newtet->tex = fromtex->tex;
	newtet->stex = fromtex->stex;
	newtet->ntex = fromtex->ntex;
	newtet->texceq[0] = fromtex->texceq[0];
	newtet->texceq[1] = fromtex->texceq[1];
}
void FreeTet(Tet *tet, bool freepts)	//entry must be freed from surf also
{
	if(freepts)
	{
		//if no other holder owns these pt's
		for(int vi=0; vi<4; ++vi)
		{
			delete tet->neib[4];
		}
	}
	delete tet;
}
/*
If there's 4 verts in this tet, break it into two 3-vert tets
The original tet should then be removed from the list because it is freed
*/
void SplitTris(Surf *surf, Tet *fourtet)
{
	for(int vi=0; vi<4; ++vi)
	{
		if(!fourtet->neib[vi])
			return;
	}
	CopyTet(surf, fourtet);
	Tet* tet1 = *surf->tets2.rbegin();
	CopyTet(surf, fourtet);
	Tet* tet2 = *surf->tets2.rbegin();
	for(int vi=0; vi<4; ++vi)
	{
		if(vi)
		{
			//tet2->neib[vi-1] = new SurfPt;
			CopyPt(fourtet->neib[vi], tet2);
		}
		if(vi!=3)
		{
			//tet1->neib[vi] = new SurfPt;
			CopyPt(fourtet->neib[vi], tet1);
		}
	}
	FreeTet(fourtet);
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
		for(std::list<Tet*>::iterator fulltit=fullsurf->tets2.begin();
			fulltit!=fullsurf->tets2.end();
			++fulltit)
		{
			Tet *in = *tit;
			Vec3f fulltri[3];
			fulltri[0] = (*fulltit)->neib[0]->pos;
			fulltri[1] = (*fulltit)->neib[1]->pos;
			fulltri[2] = (*fulltit)->neib[2]->pos;
			Vec3f fullnorm = Normal(fulltri);
			Plane3f split;
			MakePlane(&split.m_normal, &split.m_d, (fulltri[0]+fulltri[1]+fulltri[2])/3.0f,
				fullnorm);
			bool keepon = false;

#define MAX_POINTS_ON_WINDING 7

			float	dists[MAX_POINTS_ON_WINDING];
			int		sides[MAX_POINTS_ON_WINDING];
			int		counts[3];
			float	dot;
			int		i, j;
			SurfPt	*p1, *p2;
			Vec3f	mid;
			Tet		*neww, *backw;
			int		maxpts;

			counts[0] = counts[1] = counts[2] = 0;

#define SIDE_BACK	0
#define SIDE_ON		1
#define SIDE_FRONT	2

			// determine sides for each point
			for (i=0 ; i<3 ; i++)
			{
				dot = DotProduct (in->neib[i]->pos, split.m_normal);
				dot -= split.m_d;
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

			if (//keepon && 
				!counts[0] && !counts[1])
				continue;

			if (!counts[0])
			{
				//FreeWinding (in);
				//return NULL;
				continue;	//don't know enough to say that it is truly hidden or if just random tri in front
				//goto again;
			}
			//if (!counts[1])
			//	continue;	//don't know enough to say that it is truly hidden or if just random tri in front

			maxpts = 3+4;	// can't use counts[0]+2 because
			// of fp grouping errors
			//neww = NewWinding (maxpts);
			//std::list<SurfPt*> neww2;
			CopyTet(surf, in);
			neww = *surf->tets2.rbegin();
			CopyTet(surf, in);
			backw = *surf->tets2.rbegin();

			int newvi = 0;
			int backvi = 0;

			for (i=0 ; i<3 ; i++)
			{
				//p1 = in->points[i];
				p1 = in->neib[i];

				if (sides[i] == SIDE_ON)
				{
					//VectorCopy (p1, neww->points[neww->numpoints]);
					//neww->numpoints++;

					CopyPt(p1, neww);
					CopyPt(p1, backw);

					newvi++;
					backvi++;

					continue;
				}

				if (sides[i] == SIDE_FRONT)
				{
					//VectorCopy (p1, neww->points[neww->numpoints]);
					//neww->numpoints++;
					CopyPt(p1, neww);
					newvi++;
				}

				if (sides[i] == SIDE_BACK)
				{
					//VectorCopy (p1, neww->points[neww->numpoints]);
					//neww->numpoints++;
					CopyPt(p1, backw);
					backvi++;
				}

				if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
					continue;

				// generate a split point
				p2 = in->neib[(i+1)%in->numpoints];

				dot = dists[i] / (dists[i]-dists[i+1]);
				for (j=0 ; j<3 ; j++)
				{	// avoid round off error when possible
					if (split->m_normal[j] == 1)
						mid[j] = split->m_d;
					else if (split->m_normal[j] == -1)
						mid[j] = -split->m_d;
					else
						mid[j] = p1->pos[j] + dot*(p2->pos[j]-p1->pos[j]);
				}

				//VectorCopy (mid, neww->points[neww->numpoints]);
				//neww->numpoints++;
				SurfPt *news1 = CopyPt(p2, neww);
				SurfPt *news2 = CopyPt(p2, backw);

				news1->pos = mid;
				news2->pos = mid;
			}

			//if (neww->numpoints > maxpts)
			//	Error ("ClipWinding: points exceeded estimate");

			// free the original winding
			//FreeWinding (in);
			FreeTet(in);
			surf->tets2.erase(tit);

			//return neww;
			goto again;
		}
	}

	return true;
}

void MergePt(SurfPt *topt, SurfPt *frompt)
{
	topt->gen++;

	for(std::list<Tet*>::iterator hit=frompt->holder.begin();
		hit!=frompt->holder.end();
		++hit)
	{
		for(std::list<Tet*>::iterator tohit=frompt->topt.begin();
			tohit!=topt->holder.end();
			++tohit)
		{
			if(*hit == *tohit)
				goto next;
		}
		topt->holder.push_back(*hit);
next:
		;
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
{
again:
	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{
		std::list<Tet*>::iterator tit2=tit1;
		tit2++;
		for(;
			tit2!=surf->tets2.end();
			++tit2)
		{
			for(int vin1=0; vin1<3; ++vin1)
			{
				for(int vin2=0; vin2<3; vin2++)
				{
					if(Magnitude( (*tit1)->neib[vin1]->pos - (*tit2)->neib[vin2]->pos ) <= 1.0f &&
						(*tit1)->neib[vin1] != (*tit2)->neib[vin2] )
					{
						MergePt((*tit1)->neib[vin1],
							(*tit2)->neib[vin2]);
						delete (*tit2)->neib[vin2];
						(*tit2)->neib[vin2] = (*tit1)->neib[vin1];
						goto again;
					}
				}
			}
		}
	}

	return true;
}

/*
internal func called by RemTet
removes the holder tet from the list in the pt "sp".
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
	std::list<Tet*> checkremove;

	for(int vin=0; vin<3; ++vin)
	{
		//for(std::list<Tet*>::iterator hit=(*tit)->neib[vin]->holder.begin();
		//	hit!=(*tit)->neib[vin]->holder.end();
		//	++hit)
		//{
		//	if(*hit == *tit)
		//		continue;	//already going to remove this
			//remove pt from other holders
			SepPt(*tit, (*tit)->neib[vin], &checkremove);
		//}
				
		if((*tit)->neib[vin]->holder.size() == 0)
		{
			delete (*tit)->neib[vin];
		}

		(*tit)->neib[vin] = NULL;
	}

	FreeTet(*tit, true);
	surf->tets.erase(tit);
}

/*
this fun musn't use NULL neibs in the tet.
it is assumed that neib[vin] is NULL.
*/
void RemTet2(Surf *surf,
		   std::list<Tet*>::iterator tit)
{
	//check these tets to see if they have less than 3 verts, and remove them
	std::list<Tet*> checkremove;

	for(int vin=0; vin<3; ++vin)
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
			SepPt(*tit, (*tit)->neib[vin], &checkremove);
		//}

		if((*tit)->neib[vin]->holder.size() == 0)
		{
			delete (*tit)->neib[vin];
		}

		(*tit)->neib[vin] = NULL;
	}

	FreeTet(*tit, true);
	surf->tets.erase(tit);
}

/*
Remove free-floating triangles
that are not attached to the main body mesh.
After having connected all the points together/welded,
we can see what is disattached.
*/
bool RemFloaters(Surf *surf, Surf *fullsurf)
{
	//if a surfpt has less than 3 holders/owners,
	//then it is a floater. test to confirm. TODO.

again:

	for(std::list<Tet*>::iterator tit1=surf->tets2.begin();
		tit1!=surf->tets2.end();
		++tit1)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if((*tit1)->neib[vin]->holder.size() < 3)
			{
				RemTet(surf, tit1, vin);
				goto again;
			}
		}
	}

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
				goto again;
			}
		}
	}

	//if three tets share an edge (a set of two pt's)
	//one of them must be covered by the others

	return true;
}

bool TetRayCollides(std::list<Tet*>::iterator tit,
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
				return true;
				//collides with a front side of tet
			}
		}
	}

	return false;
}

bool InfectVis(Surf *surf, Surf *fullsurf)
{
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
			for(int vin1=0; vin1<9; ++vin1)
			{
				int samec = 0;
				for(int vin2=0; vin2<9; ++vin2)
				{
					if((*tit)->neib[vin1%3] == (*tit2)->neib[vin2%3])
						samec++;
					else
						samec=0;
					
					if(samec >= 2)
					{
						//success
						(*tit)->approved = true;
						(*tit2)->approved = true;
						goto nexttit2;
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
again:
	for(std::list<Tet*>::iterator tit=surf->tets2.begin();
		tit!=surf->tets2.end();
		++tit)
	{
		for(int vin=0; vin<3; ++vin)
		{
			if(!(*tit)->approved)
			{
				RemTet2(surf, tit);
				goto again;
			}
		}
	}

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
			line[0] = tri[attempt%3] + tnorm * 0.5f;
			line[1] = line[0] + tnorm * 3000;

			if(TetRayCollides(tit, line))
			{
				(*tit)->approved = false;
				(*tit)->hidden = false;
				continue;	//inconclusive
			}
			else
			{
				(*tit)->approved = true;
				(*tit)->hidden = false;
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
							tet[0] = *tit;
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
							MakePlane(&tpl[0].m_normal, *tpl[0].m_d, (tri[0][0]+tri[0][1]+tri[0][2])/3.0f,
								tnorm[0]);
							MakePlane(&tpl[1].m_normal, *tpl[1].m_d, (tri[1][0]+tri[1][1]+tri[1][2])/3.0f,
								tnorm[1]);
							MakePlane(&tpl[2].m_normal, *tpl[1].m_d, (tri[2][0]+tri[2][1]+tri[2][2])/3.0f,
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
								ret = tit;
							else if(largest ==1)
								ret = hitv1;
							else if(largest==2)
								ret = hitv2;

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

	return true;
}


void PlaceTet(Surf *surf, Tet *tet, std::list<Tet*>* toplace)
{
	if(!tet->placed)
	{
		int placec = 0;
		int fate = -1;

		SurfPt* placept[2] = {NULL,NULL};

		for(int vin=0; vin<3; ++vin)
		{
			if(tet->neib[vin]->placed)
			{
				placept[placec] = tet->neib
				placec++;
			}
			else
			{
				placec=0;
				fate = vin;
			}

			if(placec>=2)
			{
				if(vin==2 || !tet->neib[2]->placed)
				{
					int e1 = (fate+1)%3;
					int e2 = (fate+2)%3;

					Vec3f p1 = Vec3f(tet->neib[e1]->orc.x, tet->neib[e1]->orc.y, 0);
					Vec3f p2 = Vec3f(tet->neib[e2]->orc.x, tet->neib[e2]->orc.y, 0);
					Vec3f up = Vec3f(0,0,-1);

					Vec3f axis = Cross(up,p1-p2);
					axis = Normalize(axis);

					Vec3f midp = (p1+p2)/2.0f;
					float d = Magnitude(p1-p2);

					Vec3f p3 = midp + axis * d;

					tet->neib[fate]->orc = Vec2f(p3.x,p3.y);
					tet->neib[fate]->placed = true;
					tet->placed = true;
/////////////////
					tet->edgedrawarea = Magnitude( tet->neib[fate]->pos - tet->neib[1]->pos );
					tet->edgeorarea = Magnitude( tet->neib[0]->orc - tet->neib[1]->orc );

					for(std::list<Tet*>::iterator hit=tet->neib[fate]->holder.begin();
						hit!=tet->neib[fate]->holder.end();
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

/*
Go from triangle to triangle, mapping to the orientability map.
Try moves that require adding only one vertex, or then two.
All the identical points must be single without repeats,
so join any <1 distance points before hand.
*/
bool GrowMapMesh(Surf *surf, Surf *fullsurf)
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

	Tet *tet = *surf->tets2.begin();

	tet->neib[0]->orc.x = 0.5f;
	tet->neib[0]->orc.y = 0.52f;
	tet->neib[0]->placed = true;
	tet->neib[1]->orc.x = 0.52f;
	tet->neib[1]->orc.y = 0.52f;
	tet->neib[1]->placed = true;
	//tet->neib[2]->orc.x = 0.52f;
	//tet->neib[2]->orc.y = 0.5f;
	tet->edgedrawarea = Magnitude( tet->neib[0]->pos - tet->neib[1]->pos );
	tet->edgeorarea = Magnitude( tet->neib[0]->orc - tet->neib[1]->orc );

	//maybe split some tri's?
	std::list<Tet*> toplace;
	PlaceTet(surf, tet, &toplace);

	while(toplace.size())
	{
		PlaceTet(surf, *toplace.begin(), &toplace);
		toplace.erase(toplace.begin());
	}

	return true;
}

bool BalanceMesh(Surf *surf, Surf *fullsurf)
{

	return true;
}

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

void OrRender(int rendstage, Vec3f offset)
{
	Surf surf, fullsurf;
	//SurfPt *p1;
	//StartRay(&surf, tet, Vec3f(0,30000,0), &p1);

	if(!AddClipMesh(&surf, &fullsurf))
		return;

	do
	{
		if(!ClipTris(&surf, &fullsurf))
			break;
		if(!JoinPts(&surf, &fullsurf))
			break;
		if(!RemFloaters(&surf, &fullsurf))
			break;
		if(MarkVis(&surf, &fullsurf))
			break;
		if(!RemHidden(&surf, &fullsurf))
			break;
		if(!GrowMapMesh(&surf, &fullsurf))
			break;
		if(!BalanceMesh(&surf, &fullsurf))
			break;
		break;
	}while(true);
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