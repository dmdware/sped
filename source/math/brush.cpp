
/*

Convex hulls or "brushes" are how first person shooters that derive from
Quake/Doom define geometry for use in collision detection and response.
Using a set of planes, 6 for all sides of a cube, we can define any convex
shape. This simplifies collision detection against axis-aligned bounding boxes
and spheres because all we have to do is check each plane/side of the brush
against the farthest and nearest point of the AA box or sphere, and check if
both points are on both sides of the plane or inside. If there's a point on
the inside side of each plane there's an overlap with the sphere or AA box
and its not hard to get the exact distance we have to move to just be touching
the surface using the dot product.

To be drawn, brushes must be broken down into triangles. To do this I loop
through each plane/side of the brush "i". And for each side "i" I get another,
different side "j". I get the line intersection between them. This is the code
I used.

http://devmaster.net/forums/topic/8676-2-plane-intersection/

Then we need another side (each is a different side) "k" that I then get the
point intersection of the line with, and another side "l" that I get another point
intersection with. I use a for-loop to go through all the sides and for "l" I
started counting at "k+1" so we don't get any repeats (this becomes important
later when building a polygon for the brush side). The two point intersections
form a side edge for a polygon for the brush side. I store it in an array of STL
lists of lines. Each brush side has a list of lines. I store the line for side "i"
because that is the brush side that the side edge belongs to and is along.

Then I loop the side edges for each brush side, making a "polygon" - basically an
outline, with a point for each vertex. I use an epsilon value to check if two points
are within a certain distance, and use the side edge's other vertex as the next point
to check for proximity, starting over from the first side edge and making sure to
exclude checking the last connecting edge.

Then I check the polygon to be clockwise order (because that is how I cull my
polygons) by checking the normal of a triangle formed by the first three vertices
of the polygon and checking if its closer to the plane normal or if the opposite
normal is closer. If the opposite is closer I reverse the list of vertices.

Oh before I make the polygon I discard any side edges with at least one point that
is not inside or on any one plane of the brush. This is necessary to cull away
bounding planes that are outside the brush, resulting from moving the other planes.
Later I remove these planes that have less than 3 side edges, the minimum to form
a triangle.

Next I allocate (v-2) triangles where "v" is the number of vertices in the side's
polygon. I construct the triangles in a fan pattern.

There's probably some improvements that can be made like storing shared edges and
not having to reconnect them by checking distance, which I will probably learn as
I follow in the footsteps of q3map and other Quake/Doom games' source code.

[edit2] By "nearest point to the plane" I mean nearest to the "inside" side of the
plane, according to the normal. For an AA box we just check the signedness of each
axis of the normal and use the min or max on each axis to get the innermost or
outermost point of the 8 points of the AA box.

[edit3] And actually, the farthest point has to be the one from before the AA box
moved and the "nearest" point has to be from the moved position.

 */

#include "../platform.h"
#include "brush.h"
#include "plane.h"
#include "math3d.h"
#include "line.h"
#include "polygon.h"
#include "../utils.h"
#include "math3d.h"
#include "../sim/map.h"
#include "../save/edmap.h"
#include "../texture.h"
#include "../tool/rendertopo.h"

Brush& Brush::operator=(const Brush& original)
{
	/*
	int m_nsides;
	BrushSide* m_sides;
	*/

	//Log("copy edbrush nsides="<<original.m_nsides<<std::endl;
	//

	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
	}

	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}

	m_nsides = original.m_nsides;
	m_sides = new BrushSide[m_nsides];
	for(int i=0; i<m_nsides; i++)
		m_sides[i] = original.m_sides[i];
	m_nsharedv = original.m_nsharedv;
	m_sharedv = new Vec3f[m_nsharedv];
	for(int i=0; i<m_nsharedv; i++)
		m_sharedv[i] = original.m_sharedv[i];

	m_texture = original.m_texture;

	if(m_door)
	{
		delete m_door;
		m_door = NULL;
	}

	if(original.m_door)
	{
		m_door = new EdDoor();
		*m_door = *original.m_door;
	}

	return *this;
}

Brush::Brush(const Brush& original)
{
	m_sides = NULL;
	m_nsides = 0;
	m_sharedv = NULL;
	m_nsharedv = 0;
	m_texture = 0;
	m_door = NULL;
	*this = original;
}

Brush::Brush()
{
	m_sides = NULL;
	m_nsides = 0;
	m_sharedv = NULL;
	m_nsharedv = 0;
	m_texture = 0;
	m_door = NULL;

	//MessageBox(g_hWnd, "ed b constr", "asd", NULL);
}

Brush::~Brush()
{
#if 0
	Log("~Brush");
#endif

	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
	}

	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}

	m_nsharedv = 0;
	m_nsides = 0;

	if(m_door)
	{
		delete m_door;
		m_door = NULL;
	}
}

bool Brush::addclipmesh(Surf *surf)
{
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		for(int ti=0; ti<s->m_ntris; ++ti)
		{
			Triangle2 *t = &s->m_tris[ti];

			Vec3f tr[3];
			Vec2f txc[3];
			Vec3f vnorm[3];
			
			tr[0] = t->m_vertex[0];
			tr[1] = t->m_vertex[1];
			tr[2] = t->m_vertex[2];
			
			txc[0] = t->m_texcoord[0];
			txc[1] = t->m_texcoord[1];
			txc[2] = t->m_texcoord[2];

			int vin[3];
			vin[0] = s->m_vindices[ti*3+0];
			vin[1] = s->m_vindices[ti*3+1];
			vin[2] = s->m_vindices[ti*3+2];

			vnorm[0] = s->m_drawva.normals[vin[0]];
			vnorm[1] = s->m_drawva.normals[vin[1]];
			vnorm[2] = s->m_drawva.normals[vin[2]];

			Texture* diff = &g_texture[s->m_diffusem];
			Texture* spec = &g_texture[s->m_specularm];
			Texture* norm = &g_texture[s->m_normalm];

			SurfPt* lastvs[3];
			lastvs[0] = new SurfPt;
			lastvs[1] = new SurfPt;
			lastvs[2] = new SurfPt;
			Tet* tet = new Tet();
			
			GenTexEq(tet, tr, txc);

			tet->approved = false;
			tet->hidden = false;

			surf->pts2.push_back(lastvs[0]);
			surf->pts2.push_back(lastvs[1]);
			surf->pts2.push_back(lastvs[2]);
			surf->tets2.push_back(tet);

			tet->neib[0] = lastvs[0];
			tet->neib[1] = lastvs[1];
			tet->neib[2] = lastvs[2];
			tet->neib[3] = NULL;

			lastvs[0]->pos = tr[0];
			lastvs[1]->pos = tr[1];
			lastvs[2]->pos = tr[2];

			lastvs[0]->texc = txc[0];
			lastvs[1]->texc = txc[1];
			lastvs[2]->texc = txc[2];

			lastvs[0]->norm = vnorm[0];
			lastvs[1]->norm = vnorm[1];
			lastvs[2]->norm = vnorm[2];

			lastvs[0]->holder.push_back(tet);
			lastvs[1]->holder.push_back(tet);
			lastvs[2]->holder.push_back(tet);

			tet->tex = diff;
			tet->stex = spec;
			tet->ntex = norm;
		}
	}

	return true;
}

//#define SELECT_DEBUG

#ifdef SELECT_DEBUG
Brush* g_debugb = NULL;
#endif

Vec3f Brush::traceray(Vec3f line[], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a,
		unsigned char *sr, unsigned char *sg, unsigned char *sb, unsigned char *sa,
		unsigned char *nr, unsigned char *ng, unsigned char *nb, unsigned char *na,
		Texture **retex, Texture **retexs, Texture **retexn,
		Vec2f* retexc,
					  Vec3f *nearnorm)
{
	float startRatio = -1.0f;
	float endRatio = 1.0f;
	Vec3f intersection = line[1];

	BrushSide *is = m_sides;

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		float startD = Dot(line[0], s->m_plane.m_normal) + s->m_plane.m_d;
		float endD = Dot(line[1], s->m_plane.m_normal) + s->m_plane.m_d;

#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			Log("side "<<i<<std::endl;
			Log("startD="<<startD<<", endD="<<endD<<std::endl;
			
		}

		if(g_debugb == this)
		{
			Log("traceray plane=("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<"d="<<s->m_plane.m_d<<") startD="<<startD<<" endD="<<endD<<std::endl;
			
		}
#endif

		if(startD > 0 && endD > 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("startD > 0 && endD > 0");
				
			}
#endif

			return line[1];
		}

		if(startD <= 0 && endD <= 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("startD <= 0 && endD <= 0");
				
			}
#endif

			continue;
		}

		if(startD > endD)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("startD > endD");
				
			}
#endif

			// This gets a ratio from our starting point to the approximate collision spot
			float ratio1 = (startD - EPSILON) / (startD - endD);

#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("ratio1 ="<<ratio1<<std::endl;
				
			}
#endif

			if(ratio1 > startRatio)
			{
				startRatio = ratio1;

				is = s;

#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					Log("ratio1 > startRatio == "<<startRatio<<std::endl;
					
				}
#endif
			}
		}
		else
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("else startD <= endD");
				
			}
#endif

			float ratio = (startD + EPSILON) / (startD - endD);

#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				Log("ratio ="<<ratio<<std::endl;
				
			}
#endif

			if(ratio < endRatio)
			{
				endRatio = ratio;

				//is = s;

#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					Log("ratio < endRatio == "<<endRatio<<std::endl;
					
				}
#endif
			}
		}
	}

	if(startRatio < endRatio)
	{
#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			Log("startRatio ("<<startRatio<<") < endRatio ("<<endRatio<<")");
			
		}
#endif

		//if(startRatio > -1)
		{
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					Log("startRatio > -1");
					
				}
#endif

			if(startRatio < 0)
				startRatio = 0;

#ifdef SELECT_DEBUG
			if(g_debugb == NULL)
				g_debugb = this;
#endif

			Vec3f ir = line[0] + (line[1]-line[0]) * startRatio;

			Vec3f tr[3];

			//for(int tin=0; tin<s->m_ntries; ++tin)
			{
				//tr[0] = m_

				Texture* diff = &g_texture[is->m_diffusem];
				Texture* spec = &g_texture[is->m_specularm];
				Texture* norm = &g_texture[is->m_normalm];
				LoadedTex* difpx = diff->pixels;
				LoadedTex* specpx = spec->pixels;
				LoadedTex* normpx = norm->pixels;

				// TODO check transparency pass

				*retex = diff;
				*retexs = spec;
				*retexn = norm;

				float uc = is->m_tceq[0].m_normal.x * ir.x +
					is->m_tceq[0].m_normal.y * ir.y +
					is->m_tceq[0].m_normal.z * ir.z +
					is->m_tceq[0].m_d;
				
				float vc = is->m_tceq[1].m_normal.x * ir.x +
					is->m_tceq[1].m_normal.y * ir.y +
					is->m_tceq[1].m_normal.z * ir.z +
					is->m_tceq[1].m_d;

				retexc->x = uc;
				retexc->y = vc;

				int tx = uc * difpx->sizex;
				int ty = vc * difpx->sizey;
				
				int stx = uc * specpx->sizex;
				int sty = vc * specpx->sizey;
				
				int ntx = uc * normpx->sizex;
				int nty = vc * normpx->sizey;

				while(tx < 0)
					tx += difpx->sizex;
				while(tx >= difpx->sizex)
					tx %= difpx->sizex;
				while(ty < 0)
					ty += difpx->sizey;
				while(ty >= difpx->sizey)
					ty %= difpx->sizey;

				while(stx < 0)
					stx += specpx->sizex;
				while(stx >= specpx->sizex)
					stx %= specpx->sizex;
				while(sty < 0)
					sty += specpx->sizey;
				while(sty >= specpx->sizey)
					sty %= specpx->sizey;

				while(ntx < 0)
					ntx += normpx->sizex;
				while(ntx >= normpx->sizex)
					ntx %= normpx->sizex;
				while(nty < 0)
					nty += normpx->sizey;
				while(nty >= normpx->sizey)
					nty %= normpx->sizey;

				int pxin = (tx + difpx->sizex * ty);
				int spxin = (stx + specpx->sizex * sty);
				int npxin = (ntx + normpx->sizex * nty);

				Vec4f rgba;
				Vec4f srgba;
				Vec4f nrgba;

				rgba.x = difpx->data[ pxin * difpx->channels + 0 ];
				rgba.y = difpx->data[ pxin * difpx->channels + 1 ];
				rgba.z = difpx->data[ pxin * difpx->channels + 2 ];
				rgba.w = 255;

				srgba.x = specpx->data[ spxin * specpx->channels + 0 ];
				srgba.y = specpx->data[ spxin * specpx->channels + 1 ];
				srgba.z = specpx->data[ spxin * specpx->channels + 2 ];
				srgba.w = 255;

				nrgba.x = normpx->data[ npxin * normpx->channels + 0 ];
				nrgba.y = normpx->data[ npxin * normpx->channels + 1 ];
				nrgba.z = normpx->data[ npxin * normpx->channels + 2 ];
				nrgba.w = 255;

				*r = (int)(rgba.x);
				*g = (int)(rgba.y);
				*b = (int)(rgba.z);
				*a = (int)(rgba.w);

				*sr = (int)(srgba.x);
				*sg = (int)(srgba.y);
				*sb = (int)(srgba.z);
				*sa = (int)(srgba.w);

				*nr = (int)(nrgba.x);
				*ng = (int)(nrgba.y);
				*nb = (int)(nrgba.z);
				*na = (int)(nrgba.w);
				
				*nearnorm = is->m_plane.m_normal;
			}

			return line[0] + (line[1]-line[0]) * startRatio;
		}
	}

	return line[1];
}

void Brush::prunev(bool* invalidv)
{
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		for(int j=0; j<m_nsharedv; j++)
		{
			if(!PointOnOrBehindPlane(m_sharedv[j], s->m_plane))
				invalidv[j] = true;
		}
	}
}

void Brush::moveto(Vec3f newp)
{
	Vec3f currp;

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		currp = currp + s->m_centroid;
	}

	currp = currp / (float)m_nsides;

	Vec3f delta = newp - currp;

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		Vec3f pop = PointOnPlane(s->m_plane);
		pop = pop + delta;
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);

		for(int j=0; j<2; j++)
		{
			pop = PointOnPlane(s->m_tceq[j]);
			pop = pop + delta;
			s->m_tceq[j].m_d = PlaneDistance(s->m_tceq[j].m_normal, pop);
		}
	}

	collapse();
	remaptex();
}

void Brush::add(BrushSide b)
{
#if 0
	Log("addside before: ");
	for(int vertindex = 0; vertindex < b.m_drawva.numverts; vertindex++)
	{
		Vec3f vert = b.m_drawva.vertices[vertindex];
		Log("\taddvert: "<<vert.x<<","<<vert.y<<","<<vert.z<<std::endl;
	}
#endif

	BrushSide* newsides = new BrushSide[m_nsides + 1];
	if(!newsides) OutOfMem(__FILE__, __LINE__);

	if(m_nsides > 0)
	{
		//memcpy(newsides, m_sides, sizeof(BrushSide)*m_nsides);
		for(int i=0; i<m_nsides; i++)
			newsides[i] = m_sides[i];
		delete [] m_sides;
	}

	newsides[m_nsides] = b;
	m_sides = newsides;
	m_nsides ++;

#if 0
	Log("addside after: ");
	for(int vertindex = 0; vertindex < b.m_drawva.numverts; vertindex++)
	{
		Vec3f vert = b.m_drawva.vertices[vertindex];
		Log("\taddvert: "<<vert.x<<","<<vert.y<<","<<vert.z<<std::endl;
	}
#endif
}


void Brush::setsides(int nsides, BrushSide* sides)
{
	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
		m_nsides = 0;
	}

	for(int i=0; i<nsides; i++)
		add(sides[i]);
}

void Brush::getsides(int* nsides, BrushSide** sides)
{
	*nsides = m_nsides;

	if(*sides)
	{
		delete [] *sides;
		*sides = NULL;
	}

	if(m_nsides > 0)
	{
		*sides = new BrushSide[m_nsides];

		for(int i=0; i<m_nsides; i++)
		{
			(*sides)[i] = m_sides[i];
		}
	}
}

//#define REMOVESIDE_DEBUG

void Brush::removeside(int i)
{
#ifdef REMOVESIDE_DEBUG
	Log("remove side 1 "<<i<<endl;
	
#endif

	BrushSide* newsides = new BrushSide[m_nsides-1];

#if 0
	memcpy(&newsides[0], &m_sides[0], sizeof(BrushSide)*i);
	memcpy(&newsides[i], &m_sides[i+1], sizeof(BrushSide)*(m_nsides-i-1));

	m_nsides --;
	m_sides = newsides;
#endif
	
#ifdef REMOVESIDE_DEBUG
	Log("remove side 2 "<<i<<endl;
	
#endif

	for(int j=0; j<i; j++)
		newsides[j] = m_sides[j];
	
#ifdef REMOVESIDE_DEBUG
	Log("remove side 3 "<<i<<endl;
	
#endif
		
	for(int j=i+1; j<m_nsides; j++)
		newsides[j-1] = m_sides[j];
	
#ifdef REMOVESIDE_DEBUG
	Log("remove side 4 "<<i<<endl;
	
#endif

	m_nsides --;

	delete [] m_sides;

	m_sides = newsides;

#ifdef REMOVESIDE_DEBUG
	Log("removed side "<<i<<endl;
	
#endif
}

//#define DEBUG_COLLAPSE

//collapse geometry into triangles
void Brush::collapse()
{
	int oldnsharedv = m_nsharedv;

#ifdef DEBUG_COLLAPSE
	Log("==================collapse this====================");
	

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		Vec3f n = s->m_plane.m_normal;
		float d = s->m_plane.m_d;
		Log("\t side["<<i<<"] plane=("<<n.x<<","<<n.y<<","<<n.z<<"),"<<d<<std::endl;
	}
#endif

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		if(s->m_tris)
		{
			delete [] s->m_tris;
			s->m_tris = NULL;
		}
		s->m_ntris = 0;

		if(s->m_vindices)
		{
			delete [] s->m_vindices;
			s->m_vindices = NULL;
		}
	}

	if(m_sharedv)
	{
		delete [] m_sharedv;
		m_sharedv = NULL;
	}
	m_nsharedv = 0;

	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* a = &m_sides[i];
		for(int j=i+1; j<m_nsides; j++)
		{
			BrushSide* b = &m_sides[j];
			if(Close(a->m_plane, b->m_plane))
			{

#ifdef DEBUG_COLLAPSE
				Log("planes["<<i<<","<<j<<"] ("<<a->m_plane.m_normal.x<<","<<a->m_plane.m_normal.y<<","<<a->m_plane.m_normal.z<<"),"<<a->m_plane.m_d<<" and ("<<b->m_plane.m_normal.x<<","<<b->m_plane.m_normal.y<<","<<b->m_plane.m_normal.z<<"),"<<b->m_plane.m_d<<std::endl;
				
#endif
				removeside(j);
				j--;
			}
		}
	}


#ifdef DEBUG_COLLAPSE
	Log("tag6. m_nsides="<<m_nsides<<std::endl;
	
#endif

	std::list<Line>* sideedges = new std::list<Line>[m_nsides];	// a line along a plane intersecting two other planes. both vertices form the edge of a polygon.
	if(!sideedges) OutOfMem(__FILE__, __LINE__);

	for(int i=0; i<m_nsides; i++)
	{
		//for(int j=i+1; j<m_nsides; j++)
		for(int j=0; j<m_nsides; j++)
		{
			if(j == i)
				continue;

			for(int k=0; k<m_nsides; k++)
			{
				if(k == i || k == j)
					continue;

				//for(int l=0; l<m_nsides; l++)
				for(int l=k+1; l<m_nsides; l++)
				{
					if(l == i || l == j || l == k)
						continue;

					BrushSide* a = &m_sides[i];
					BrushSide* b = &m_sides[j];
					BrushSide* c = &m_sides[k];
					BrushSide* d = &m_sides[l];

					//if(a->m_plane == b->m_plane)
					//if(Close(a->m_plane, b->m_plane))
					//{
					//	Log("hide "<<i<<","<<j<<std::endl;
					//	
					//	continue;
					//}

					//if(Close(a->m_plane, c->m_plane))
					//{
					//	Log("hide "<<i<<","<<k<<std::endl;
					//	
					//	continue;
					//}

#ifdef DEBUG_COLLAPSE
					Log("--------new side"<<i<<" edge--------");
					
#endif

					// http://devmaster.net/forums/topic/8676-2-plane-intersection/page__view__findpost__p__47568
					Vec3f linedir = Cross(a->m_plane.m_normal, b->m_plane.m_normal);
					Vec3f pointonplanea = PointOnPlane(a->m_plane);	// arbitrary point on plane A
					Vec3f v = Cross(linedir, a->m_plane.m_normal); // direction toward plane B, parallel to plane A
					Vec3f l0;

#ifdef DEBUG_COLLAPSE
					Log("collapse ("<

					Log("linedir="<<linedir.x<<","<<linedir.y<<","<<linedir.z<<std::endl;
					Log("pointonplanea="<<pointonplanea.x<<","<<pointonplanea.y<<","<<pointonplanea.z<<std::endl;
					
#endif

					if(!Intersection(pointonplanea, v, b->m_plane, l0))
						continue;

#ifdef DEBUG_COLLAPSE
					Log("l0="<<l0.x<<","<<l0.y<<","<<l0.z<<std::endl;
					

					Log("\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 2");
					
#endif
					Vec3f lineorigin;

					if(!Intersection(l0, linedir, c->m_plane, lineorigin))
						continue;

#ifdef DEBUG_COLLAPSE
					Log("\t\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 3");
					
#endif
					Vec3f lineend;

					if(!Intersection(l0, linedir, d->m_plane, lineend))
						continue;

#ifdef DEBUG_COLLAPSE
					Log("\t\t\tcollapse ("<<i<<","<<j<<","<<k<<","<<l<<") 4");
					
#endif

					if(Close(lineorigin, lineend))
					//if(lineorigin == lineend)
					{
#ifdef DEBUG_COLLAPSE
						Log("hide side["<<i<<"] newedge ("<<lineorigin.x<<","<<lineorigin.y<<","<<lineorigin.z<<")->("<<lineend.x<<","<<lineend.y<<","<<lineend.z<<")");
#endif
						continue;
					}

					Line edge(lineorigin, lineend);
					sideedges[i].push_back(edge);

#ifdef DEBUG_COLLAPSE
					Log("\t\t\t\tfinal=("<<lineorigin.x<<","<<lineorigin.y<<","<<lineorigin.z<<") final=("<<lineend.x<<","<<lineend.y<<","<<lineend.z<<")");
					
#endif
				}
			}
		}

#ifdef DEBUG_COLLAPSE
		Log("side"<<i<<" remaining edges");
		for(std::list<Brush>::iterator k=sideedges[i].begin(); k!=sideedges[i].end(); k++)
		{
			Line l = *k;
			Log("("<<l.m_vertex[0].x<<","<<l.m_vertex[0].y<<","<<l.m_vertex[0].z<<")->("<<l.m_vertex[1].x<<","<<l.m_vertex[1].y<<","<<l.m_vertex[1].z<<")");
		}
#endif
	}


#ifdef DEBUG_COLLAPSE
	Log("tag7. m_nsides="<<m_nsides<<std::endl;
	
#endif

	//remove side edges that have any vertex outside (and not on) of at least one other plane
	for(int i=0; i<m_nsides; i++)
	{
		for(int j=0; j<m_nsides; j++)
		{
			if(i == j)
				continue;

			BrushSide* s = &m_sides[j];

			std::list<Line>::iterator k=sideedges[i].begin();
			while(k != sideedges[i].end())
			{
				if(!PointOnOrBehindPlane(k->m_vertex[0], s->m_plane, CLOSE_EPSILON*2) || !PointOnOrBehindPlane(k->m_vertex[1], s->m_plane, CLOSE_EPSILON*2))
				{
#ifdef DEBUG_COLLAPSE
					Log("-----------remove side["<<i<<"] edge ("<<k->m_vertex[0].x<<","<<k->m_vertex[0].y<<","<<k->m_vertex[0].z<<")->("<<k->m_vertex[1].x<<","<<k->m_vertex[1].y<<","<<k->m_vertex[1].z<<") out of plane["<<j<<"]=("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<"d="<<s->m_plane.m_d<<")--------------");
					
#endif
					k = sideedges[i].erase(k);
					continue;
				}

				k++;
			}
		}

#ifdef DEBUG_COLLAPSE
		Log("side"<<i<<" 2remaining edges");
		for(std::list<Brush>::iterator k=sideedges[i].begin(); k!=sideedges[i].end(); k++)
		{
			Line l = *k;
			Log("("<<l.m_vertex[0].x<<","<<l.m_vertex[0].y<<","<<l.m_vertex[0].z<<")->("<<l.m_vertex[1].x<<","<<l.m_vertex[1].y<<","<<l.m_vertex[1].z<<")");
		}
#endif
	}

#ifdef DEBUG_COLLAPSE
	Log("tag8. m_nsides="<<m_nsides<<std::endl;
	
#endif

	//construct outlines of polygons from side edges
	//Polyg* sidepolys = new Polyg[m_nsides];
	for(int i=0; i<m_nsides; i++)
	{
#ifdef DEBUG_COLLAPSE
		for(std::list<Brush>::iterator j=sideedges[i].begin(); j!=sideedges[i].end(); j++)
		{
			Line* edge = &*j;
			if(Close(edge->m_vertex[0], edge->m_vertex[1]))
			{
				//j = sideedges[i].erase(j);
				Log("hide side["<<i<<"] edge ("<<edge->m_vertex[0].x<<","<<edge->m_vertex[0].y<<","<<edge->m_vertex[0].z<<")->("<<edge->m_vertex[1].x<<","<<edge->m_vertex[1].y<<","<<edge->m_vertex[1].z<<")");
			}
		}
#endif

		if(sideedges[i].size() < 3)
		{
#ifdef DEBUG_COLLAPSE
			Log("sideedges["<<i<<"] < 3");
			
#endif
			continue;
		}

		std::list<Line>::iterator j=sideedges[i].begin();
		//std::list<Brush>::iterator lastconnection = j;
		std::set<Line*> connected;
		//set<Vec3f> connectedv;
		std::list<Vec3f> connectedv;
		//set<Vec3f> connected;

		Vec3f first = j->m_vertex[0];
		//connectedv.insert(first);
		connectedv.push_back(first);
		//sidepolys[i].m_vertex.push_back(first);
		BrushSide* s = &m_sides[i];
		s->m_outline.m_edv.clear();
		s->m_outline.m_edv.push_back(first);

		Vec3f curr = j->m_vertex[1];
		for(j++; j!=sideedges[i].end(); j++)
		{
#ifdef DEBUG_COLLAPSE
			Log("sideedges["<<i<<"] first="<<first.x<<","<<first.y<<","<<first.z<<std::endl;
			
#endif

			//if(j == lastconnection)
			//	continue;

			if(connected.count(&*j) > 0)
				continue;

			int whichclose = -1;

#ifdef DEBUG_COLLAPSE
			Log("\tclose? "<<j->m_vertex[0].x<<","<<j->m_vertex[0].y<<","<<j->m_vertex[0].z<<" == "<<curr.x<<","<<curr.y<<","<<curr.z<<std::endl;
			
#endif

			if(Close(j->m_vertex[0], curr))
			{
#ifdef DEBUG_COLLAPSE
				Log("\t\tyes");
				
#endif
				whichclose = 0;
				goto foundnext;
			}

#ifdef DEBUG_COLLAPSE
			Log("\tclose? "<<j->m_vertex[1].x<<","<<j->m_vertex[1].y<<","<<j->m_vertex[1].z<<" == "<<curr.x<<","<<curr.y<<","<<curr.z<<std::endl;
			
#endif

			if(Close(j->m_vertex[1], curr))
			{
#ifdef DEBUG_COLLAPSE
				Log("\t\tyes");
				
#endif
				whichclose = 1;
				goto foundnext;
			}

			foundnext:

			if(whichclose >= 0)
			{
				//lastconnection = j;
				connected.insert(&*j);

				Vec3f next = j->m_vertex[1-whichclose];

				bool found = false;
				for(std::list<Vec3f>::iterator k=connectedv.begin(); k!=connectedv.end(); k++)
				{
					if(Close(*k, next))
					{
						found = true;
						break;
					}
				}

				//if(connectedv.count( next ) > 0)
				if(found)
				{
#ifdef DEBUG_COLLAPSE
					Log("\t\t found");
#endif
					if(Close(next, first))
					{
#ifdef DEBUG_COLLAPSE
						Log("\t\t\t next hide to first");
#endif
						if(s->m_outline.m_edv.size() < 2)
						{
#ifdef DEBUG_COLLAPSE
							Log("found hide(next,first) outline verts < 3");
#endif
							continue;
						}
					}
					else
						continue;
				}
#ifdef DEBUG_COLLAPSE
				else
					Log("\t\t unique");
#endif

				//connectedv.insert(next);
				connectedv.push_back(next);

				//sidepolys[i].m_vertex.push_back(j->m_vertex[whichclose]);
				s->m_outline.m_edv.push_back(curr);

				if(Close(curr, next))
					break;	//avoid infinite loop in degenerate sides

				curr = next;

#ifdef DEBUG_COLLAPSE
				Log("\t\tcurr = "<<curr.x<<","<<curr.y<<","<<curr.z<<std::endl;
				
#endif

				//if(Close(j->m_vertex[0], first) || Close(j->m_vertex[1], first))
				if(Close(curr, first))
				{
#ifdef DEBUG_COLLAPSE
					Log("\t\t\tpolygon loop complete");

					for(std::list<Brush>::iterator k=s->m_outline.m_edv.begin(); k!=s->m_outline.m_edv.end(); k++)
					{
						Log("("<<(*k).x<<","<<(*k).y<<","<<(*k).z<<")->";
					}

					Log(std::endl;

					
#endif
					//sidepolys[i].m_vertex.push_back(curr);
					break;	//loop completed
				}

				j = sideedges[i].begin();
			}
		}
	}


#ifdef DEBUG_COLLAPSE
	Log("tag9. m_nsides="<<m_nsides<<std::endl;
	
#endif

	//make sure all polys are clockwise
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		Vec3f tri[3];
		//std::list<Brush>::iterator j = sidepolys[i].m_vertex.begin();
		std::list<Vec3f>::iterator j = s->m_outline.m_edv.begin();
		tri[0] = *j;	j++;
		tri[1] = *j;	j++;
		tri[2] = *j;

		Vec3f norm = Normal(tri);
		Vec3f revnorm = Normal2(tri);

		if(Magnitude(s->m_plane.m_normal - revnorm) < Magnitude(s->m_plane.m_normal - norm))
		{
#ifdef DEBUG_COLLAPSE
			Log("reverse polygon loop order (revnorm("<<revnorm.x<<","<<revnorm.y<<","<<revnorm.z<<") is closer to planenorm("<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<")d="<<s->m_plane.m_d<<")");
#endif
			//sidepolys[i].m_vertex.reverse();
			s->m_outline.m_edv.reverse();
		}
	}

#ifdef DEBUG_COLLAPSE
	Log("tag10. m_nsides="<<m_nsides<<std::endl;
	
#endif

	//construct triangles from polygons
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];
		//s->m_ntris = (int)sidepolys[i].m_vertex.size() - 2;
		s->m_ntris = (int)s->m_outline.m_edv.size() - 2;

		//Log("sidepolys["<<i<<"].m_vertex.size() = "<<(int)sidepolys[i].m_vertex.size()<<std::endl;
		//Log("sidepolys["<<i<<"].m_vertex.size() = "<<(int)s->m_outline.m_edv.size()<<std::endl;

		if(s->m_ntris <= 0)
		{
#ifdef DEBUG_COLLAPSE
			Log("tris["<<i<<"] = "<<0<<std::endl;
#endif
			continue;
		}

#ifdef DEBUG_COLLAPSE
			Log("tris["<<i<<"] = "<<s->m_ntris<<std::endl;
#endif
			

		s->m_tris = new Triangle2[s->m_ntris];

		//std::list<Brush>::iterator j = sidepolys[i].m_vertex.begin();
		std::list<Vec3f>::iterator j = s->m_outline.m_edv.begin();
		Vec3f first = *j;
		j++;
		Vec3f prev = *j;
		j++;
		//for(int tri=0; j!=sidepolys[i].m_vertex.end(); j++, tri++)
		for(int tri=0; j!=s->m_outline.m_edv.end(); j++, tri++)
		{
			Vec3f curr = *j;
			s->m_tris[tri].m_vertex[0] = first;
			s->m_tris[tri].m_vertex[1] = prev;
			s->m_tris[tri].m_vertex[2] = curr;
			prev = curr;
		}

		s->makeva();
	}


#ifdef DEBUG_COLLAPSE
	Log("tag11. m_nsides="<<m_nsides<<std::endl;
	
#endif

	//delete [] sidepolys;
	bool* removes = new bool[m_nsides];	//degenerate sides to remove
	if(!removes) OutOfMem(__FILE__, __LINE__);

	//remove bounding planes outside of the brush.
	//if brush side has no side edges, remove it.
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		//if(sideedges[i].size() <= 0)
		if(sideedges[i].size() < 3 ||
			s->m_outline.m_edv.size() < 3 ||	//3 is the minimum number of edges to form a polygon
			s->m_ntris <= 0)
		{
#ifdef DEBUG_COLLAPSE
			Log("remove side. "<<i<<" sideedges[i].size()="<<sideedges[i].size()<<" s->m_outline.m_edv.size()="<<s->m_outline.m_edv.size()<<" s->m_ntris="<<s->m_ntris<<std::endl;
			
#endif
			//removeside(i);
			//i--;
			removes[i] = true;
		}
		else
			removes[i] = false;
	}

	for(int i=m_nsides-1; i>=0; i--)
		if(removes[i])
			removeside(i);

	delete [] removes;

	delete [] sideedges;

#ifdef DEBUG_COLLAPSE
	Log("1a");
	
#endif

	//get shared vertices and centroids
	std::vector<Vec3f> sharedv;
	for(int i=0; i<m_nsides; i++)
	{
		BrushSide* s = &m_sides[i];

		if(s->m_ntris <= 0)
			continue;

		//s->m_vindices = new int[s->m_outline.m_edv.size()];
		s->m_vindices = new int[s->m_ntris+2];

#ifdef DEBUG_COLLAPSE
	Log("1b side"<<i<<" vindices="<<s->m_outline.m_edv.size()<<std::endl;
	
#endif

		Vec3f centroid(0,0,0);
		float count = 0;

		int jj=0;
		for(std::list<Vec3f>::iterator j=s->m_outline.m_edv.begin(); j!=s->m_outline.m_edv.end(); j++, jj++)
		{
			centroid = centroid * (count/(count+1)) + (*j) * (1.0f/(count+1));
#ifdef DEBUG_COLLAPSE
			Log("centroid "<<count<<" "<<centroid.x<<","<<centroid.y<<","<<centroid.z<<" *j "<<(*j).x<<","<<(*j).y<<","<<(*j).z<<std::endl;
			
#endif

			count += 1;

			bool found = false;
			for(unsigned int k=0; k<sharedv.size(); k++)
			{
				if(Close(sharedv[k], *j))
				{
					found = true;
					s->m_vindices[jj] = k;
				}
			}

			if(!found)
			{
				sharedv.push_back(*j);
				s->m_vindices[jj] = sharedv.size()-1;
			}
		}

		s->m_centroid = centroid;

#ifdef DEBUG_COLLAPSE
		Log("1c side"<<i<<std::endl;
		
#endif
	}

#ifdef DEBUG_COLLAPSE
	Log("1d");
	
#endif

	m_nsharedv = sharedv.size();
	m_sharedv = new Vec3f[m_nsharedv];
	for(int i=0; i<m_nsharedv; i++)
		m_sharedv[i] = sharedv[i];

#ifdef DEBUG_COLLAPSE
	Log("shared vertices num = "<<m_nsharedv<<std::endl;
	
#endif

	if(g_sel1b == this && oldnsharedv != m_nsharedv)
		g_dragV = -1;
}

void Brush::remaptex()
{
	for(int i=0; i<m_nsides; i++)
		m_sides[i].remaptex();
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-radius, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(radius, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -radius), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, radius), Vec3f(0, 0, 1));	//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-hwx, 0, 0), Vec3f(-1, 0, 0));		//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(hwx, 0, 0), Vec3f(1, 0, 0));		//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -hwz), Vec3f(0, 0, -1));		//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, hwz), Vec3f(0, 0, 1));		//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const Vec3f vmin, const Vec3f vmax)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, vmax.y, 0), Vec3f(0, 1, 0));		//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, vmin.y, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(vmin.x, 0, 0), Vec3f(-1, 0, 0));		//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(vmax.x, 0, 0), Vec3f(1, 0, 0));		//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, vmin.z), Vec3f(0, 0, -1));		//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, vmax.z), Vec3f(0, 0, 1));		//back
}

bool HullsIntersect(Vec3f* hull1norms, float* hull1dist, int hull1planes, Vec3f* hull2norms, float* hull2dist, int hull2planes)
{
	return false;
}

// line intersects convex hull?
bool LineInterHull(const Vec3f* line, Plane3f* planes, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
    {
		Vec3f inter;
        if(LineInterPlane(line, planes[i].m_normal, -planes[i].m_d, &inter))
        {
			bool allin = true;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(inter, planes[j]))
				{
					allin = false;
					break;
				}
			}
			if(allin)
			{
				return true;
			}
        }
    }

    return false;
}

// line intersects convex hull?
bool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
    {
		Vec3f inter;
        if(LineInterPlane(line, norms[i], -ds[i], &inter))
        {
#if 0
			Log("inter"<<i<<" at "<<inter.x<<","<<inter.y<<","<<inter.z<<std::endl;
#endif

			bool allin = true;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

#if 0
				float result = inter.x*norms[j].x + inter.y*norms[j].y + inter.z*norms[j].z + ds[j];
				Log("pldot"<<j<<" = "<<result<<std::endl;
#endif

				if(!PointOnOrBehindPlane(inter, norms[j], ds[j]))
				{
					allin = false;
					break;
				}
			}
			if(allin)
			{
				return true;
			}
        }
    }

    return false;
}
