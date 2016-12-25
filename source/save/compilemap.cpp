

#include "compilemap.h"
#include "../utils.h"
#include "savemap.h"
#include "../math/camera.h"
#include "../app/segui.h"
#include "../window.h"
#include "../gui/gui.h"
#include "../sim/tile.h"
#include "../app/appmain.h"
#include "../app/seviewport.h"
#include "../math/vec4f.h"
#include "../tool/rendersprite.h"

float g_defrenderpitch = 30;
float g_defrenderyaw = 45;
int g_1tilewidth = 64;
int g_renderframe = 0;

void ToCutSide(CutBrushSide* cuts, BrushSide* eds)
{
	/*
class BrushSide
{
public:
	Plane3f m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;

class BrushSide : public BrushSide
{
public:
	int m_ntris;
	Triangle2* m_tris;
	Plane3f m_tceq[2];	//tex coord uv equations
	Polyg m_outline;
	int* m_vindices;	//indices into parent brush's shared vertex array; only stores unique vertices as defined by polygon outline
	Vec3f m_centroid;
	*/

	/*
class CutBrushSide
{
public:
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	Plane3f m_tceq[2];
	std::list<Triangle> m_frag;
	*/

	cuts->m_diffusem = eds->m_diffusem;
	cuts->m_specularm = eds->m_specularm;
	cuts->m_normalm = eds->m_normalm;
	cuts->m_tceq[0] = eds->m_tceq[0];
	cuts->m_tceq[1] = eds->m_tceq[1];

	for(int i=0; i<eds->m_ntris; i++)
		cuts->m_frag.push_back(eds->m_tris[i]);
}

void ToCutBrush(CutBrush* cutb, Brush* edb)
{
	for(int i=0; i<edb->m_nsides; i++)
	{
		CutBrushSide cuts;
		ToCutSide(&cuts, &edb->m_sides[i]);
		cutb->m_side.push_back(cuts);
	}
}

bool BrushTouch(Brush* b1, Brush* b2)
{
	for(int i=0; i<b1->m_nsides; i++)
	{
		BrushSide* s = &b1->m_sides[i];
		Plane3f p = s->m_plane;
		p.m_d += EPSILON;
		bool found = false;

		for(int j=0; j<b2->m_nsharedv; j++)
		{
			Vec3f v = b2->m_sharedv[j];

			if(PointOnOrBehindPlane(v, p))
			{
				found = true;
				break;
			}
		}

		if(!found)
			return false;
	}

	return true;
}

void FragBrush(CutBrush* cutb, Brush* edb)
{
	for(std::list<CutBrushSide>::iterator i=cutb->m_side.begin(); i!=cutb->m_side.end(); i++)
	{
		for(std::list<Triangle>::iterator j=i->m_frag.begin(); j!=i->m_frag.end(); j++)
		{

		}
	}
}

static int g_fragerased;

// Remove hidden triangles (triangles that are completely
// covered up by convex hulls)
void RemoveHiddenFrags(CutBrush* cutb, Brush* edb)
{
	for(std::list<CutBrushSide>::iterator i=cutb->m_side.begin(); i!=cutb->m_side.end(); i++)
	{
		std::list<Triangle>::iterator j = i->m_frag.begin();

		while(j != i->m_frag.end())
		{
			bool inall = true;

			for(int k=0; k<edb->m_nsides; k++)
			{
				BrushSide* s = &edb->m_sides[k];
				Plane3f* p = &s->m_plane;

				if(!PointOnOrBehindPlane(j->m_vertex[0], *p))
				{
					inall = false;
					break;
				}

				if(!PointOnOrBehindPlane(j->m_vertex[1], *p))
				{
					inall = false;
					break;
				}

				if(!PointOnOrBehindPlane(j->m_vertex[2], *p))
				{
					inall = false;
					break;
				}
			}

			if(inall)
			{
				j = i->m_frag.erase( j );
				g_fragerased ++;
				continue;
			}

			j++;
		}
	}
}

static std::list<CutBrush> cutbs;	//the cut brush sides
static int ntouch = 0;	//the number of touching brushes
static EdMap* cmap;	//the map to compile
static std::list<Brush> finalbs; //This will hold the final brushes

void CutBrushes()
{
	cutbs.clear();

	for(std::list<Brush>::iterator i=cmap->m_brush.begin(); i!=cmap->m_brush.end(); i++)
	{
		CutBrush cutb;
		ToCutBrush(&cutb, &*i);
		cutbs.push_back(cutb);
	}

	ntouch = 0;

	std::list<Brush>::iterator a = cmap->m_brush.begin();
	std::list<CutBrush>::iterator cuta = cutbs.begin();
	for(; a!=cmap->m_brush.end(); a++, cuta++)
	{
		std::list<Brush>::iterator b = cmap->m_brush.begin();
		std::list<CutBrush>::iterator cutb = cutbs.begin();

		while(&*b != &*a)
		{
			b++;
			cutb++;
		}

		b++;
		cutb++;

		for(; b!=cmap->m_brush.end(); b++)
		{
			if(!BrushTouch(&*a, &*b) && !BrushTouch(&*b, &*a))
				continue;

			ntouch++;

			//FragBrush(&*cuta, &*b);
			//FragBrush(&*cutb, &*a);
		}
	}

//	Log("num touches: "<<ntouch<<std::endl;

	// For each brush, see which other brush
	//it touches, and remove any hidden fragments
	//covered up by the other brush.
	g_fragerased = 0;
	a = cmap->m_brush.begin();
	cuta = cutbs.begin();
	for(; a!=cmap->m_brush.end(); a++, cuta++)
	{
		std::list<Brush>::iterator b = cmap->m_brush.begin();
		std::list<CutBrush>::iterator cutb = cutbs.begin();

		while(&*b != &*a)
		{
			b++;
			cutb++;
		}

		b++;
		cutb++;

		for(; b!=cmap->m_brush.end(); b++)
		{
			if(!BrushTouch(&*a, &*b) && !BrushTouch(&*b, &*a))
				continue;

			RemoveHiddenFrags(&*cuta, &*b);
			RemoveHiddenFrags(&*cutb, &*a);
		}
	}
}

void MakeFinalBrushes()
{
	finalbs.clear();

	//Copy the map brushes to final brushes
	//and substitute its drawable vertex array
	//of triangles to the cut-down list.
	std::list<Brush>::iterator brushitr = cmap->m_brush.begin();
	std::list<CutBrush>::iterator cutbrushitr = cutbs.begin();
	for(; brushitr!=cmap->m_brush.end(); brushitr++, cutbrushitr++)
	{
		Brush finalb = *brushitr;
		std::list<CutBrushSide>::iterator cutsideitr = cutbrushitr->m_side.begin();

		for(int sideindex = 0; sideindex < finalb.m_nsides; cutsideitr++, sideindex++)
		{
			BrushSide* s = &finalb.m_sides[sideindex];
			CutBrushSide* cuts = &*cutsideitr;
			s->vafromcut(cuts);
		}

		finalbs.push_back(finalb);
	}
}

void CleanUpMapCompile()
{
	cutbs.clear();
	finalbs.clear();
}

void CompileMap(const char* full, EdMap* map)
{
	cmap = map;

//	Log("Compiling map "<<full<<std::endl;
//	Log("num brushes: "<<map->m_brush.size()<<std::endl;

	CutBrushes();

//	Log("frags removed: "<<g_fragerased<<std::endl;

	MakeFinalBrushes();

	SaveMap(full, finalbs);

	CleanUpMapCompile();
}

void ResetView(bool checkupscale)
{
	//g_cam.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_projtype = PROJ_ORTHO;
	g_cam.position(0, 0, 10000.0f, 0, 0, 0, 0, 1, 0);
	g_cam.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_defrenderpitch), 1, 0, 0);
	g_cam.rotateabout(Vec3f(0,0,0), DEGTORAD(g_defrenderyaw), 0, 1, 0);

	g_zoom = 1;

	Vec3f topleft(-g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomleft(-g_tilesize/2, 0, g_tilesize/2);
	Vec3f topright(g_tilesize/2, 0, -g_tilesize/2);
	Vec3f bottomright(g_tilesize/2, 0, g_tilesize/2);

	int width;
	int height;

	if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
	{
		width = g_width;
		height = g_height;
	}
	//if(g_mode == EDITOR)
	else
	{
		ViewLayer* edview = (ViewLayer*)g_gui.get("editor");
		Widget* viewportsframe = edview->get("viewports frame");
		Widget* toprightviewport = viewportsframe->get("top right viewport");
		width = toprightviewport->m_pos[2] - toprightviewport->m_pos[0];
		height = toprightviewport->m_pos[3] - toprightviewport->m_pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

#ifdef DEBUG
	{
		Log("rv"<<aspect<<","<<width<<","<<height<<" r-l:"<<(PROJ_RIGHT*aspect/g_zoom)<<","<<(-PROJ_RIGHT*aspect/g_zoom)<<" gwh:"<<g_width<<","<<g_height<<std::endl;
	}
#endif

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" rv pmat0:"<<projection.m_matrix[0]<<","<<projection.m_matrix[1]<<","<<projection.m_matrix[2]<<","<<projection.m_matrix[3]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat1:"<<projection.m_matrix[4]<<","<<projection.m_matrix[5]<<","<<projection.m_matrix[6]<<","<<projection.m_matrix[7]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat2:"<<projection.m_matrix[8]<<","<<projection.m_matrix[9]<<","<<projection.m_matrix[10]<<","<<projection.m_matrix[11]<<std::endl;
		Log("rf"<<g_renderframe<<" rv pmat3:"<<projection.m_matrix[12]<<","<<projection.m_matrix[13]<<","<<projection.m_matrix[14]<<","<<projection.m_matrix[15]<<std::endl;
	}
#endif

	VpWrap* v = &g_viewport[3];
	//Vec3f viewvec = g_focus; //g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view;
	Vec3f viewvec = v->focus();
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_cam.m_view + t->m_offset;
		//viewvec = posvec + Normalize(g_cam.m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
    //Vec3f upvec = t->m_up;
    //Vec3f upvec = g_cam.m_up;
	Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

#ifdef DEBUG
	{
		Log("rf"<<g_renderframe<<" rv vmat0:"<<viewmat.m_matrix[0]<<","<<viewmat.m_matrix[1]<<","<<viewmat.m_matrix[2]<<","<<viewmat.m_matrix[3]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat1:"<<viewmat.m_matrix[4]<<","<<viewmat.m_matrix[5]<<","<<viewmat.m_matrix[6]<<","<<viewmat.m_matrix[7]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat2:"<<viewmat.m_matrix[8]<<","<<viewmat.m_matrix[9]<<","<<viewmat.m_matrix[10]<<","<<viewmat.m_matrix[11]<<std::endl;
		Log("rf"<<g_renderframe<<" rv vmat3:"<<viewmat.m_matrix[12]<<","<<viewmat.m_matrix[13]<<","<<viewmat.m_matrix[14]<<","<<viewmat.m_matrix[15]<<std::endl;
	}
#endif

	persp = false;

	Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	float minx = fmin(topleft4.x, fmin(topright4.x, fmin(bottomleft4.x, bottomright4.x)));
	float maxx = fmax(topleft4.x, fmax(topright4.x, fmax(bottomleft4.x, bottomright4.x)));
	//float miny = min(topleft4.y, min(topright4.y, min(bottomleft4.y, bottomright4.y)));
	//float maxy = max(topleft4.y, max(topright4.y, max(bottomleft4.y, bottomright4.y)));

	float xrange = (float)maxx - (float)minx;

	if(xrange <= 0.0f)
		xrange = g_1tilewidth;

	float zoomscale = (float)g_1tilewidth / xrange;

	g_zoom *= zoomscale;

	if(checkupscale && g_antialias)
		g_zoom *= (float)ANTIALIAS_UPSCALE;

#ifdef DEBUG
	Log("zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<std::endl;
#endif
}
