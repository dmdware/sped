//
// weviewport.cpp
//
//

#include "seviewport.h"
#include "../render/shader.h"
#include "../gui/gui.h"
#include "../math/math3d.h"
#include "../window.h"
#include "../platform.h"
#include "../gui/font.h"
#include "../math/camera.h"
#include "../math/matrix.h"
#include "../render/shadow.h"
#include "../sim/map.h"
#include "sesim.h"
#include "../math/vec4f.h"
#include "segui.h"
#include "../math/brush.h"
#include "../render/sortb.h"
#include "undo.h"
#include "../math/frustum.h"
#include "appmain.h"
#include "../sim/explocrater.h"
#include "../tool/rendersprite.h"
#include "../debug.h"

VpType g_vptype[VIEWPORT_TYPES];
VpWrap g_viewport[4];
//Vec3f g_focus;
static Vec3f accum(0,0,0);
bool g_changed = false;
UndoH g_beforechange;

VpType::VpType(Vec3f offset, Vec3f up, const char* label)
{
	m_offset = offset;
	m_up = up;
	strcpy(m_label, label);
}

VpWrap::VpWrap()
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
}

VpWrap::VpWrap(int type)
{
	m_drag = false;
	m_ldown = false;
	m_rdown = false;
	m_mdown = false;
	m_type = type;
}

Vec3f VpWrap::up()
{
	Vec3f upvec = g_cam.m_up;
	VpType* t = &g_vptype[m_type];

	if(m_type != VIEWPORT_ANGLE45O)
		upvec = t->m_up;

	return upvec;
}

Vec3f VpWrap::up2()
{
	Vec3f upvec = g_cam.up2();
	VpType* t = &g_vptype[m_type];

	if(m_type != VIEWPORT_ANGLE45O)
		upvec = t->m_up;

	return upvec;
}

Vec3f VpWrap::strafe()
{
	Vec3f upvec = up();
	VpType* t = &g_vptype[m_type];
	Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));

	if(m_type == VIEWPORT_ANGLE45O)
		sidevec = g_cam.m_strafe;

	return sidevec;
}

Vec3f VpWrap::focus()
{
	Vec3f viewvec = g_cam.m_view;
	return viewvec;
}

Vec3f VpWrap::viewdir()
{
	Vec3f focusvec = focus();
	Vec3f posvec = pos();
	//Vec3f viewvec = posvec + Normalize(focusvec-posvec);
	//return viewvec;
	return focusvec-posvec;
}

Vec3f VpWrap::pos()
{
	Vec3f posvec = g_cam.m_pos;

	if(g_projtype == PROJ_PERSP && m_type == VIEWPORT_ANGLE45O)
	{
		Vec3f dir = Normalize( g_cam.m_view - g_cam.m_pos );
		//posvec = g_cam.m_view - dir * 1000.0f / g_zoom;
		posvec = g_cam.m_view - dir * 100000.0f / g_zoom;
	}

	VpType* t = &g_vptype[m_type];

	if(m_type != VIEWPORT_ANGLE45O)
		posvec = g_cam.m_view + t->m_offset;

	return posvec;
}

// draw grid of dots and crosses
// for ease of placement/alignment
void DrawGrid(Vec3f vmin, Vec3f vmax)
{
	Shader* s = &g_shader[g_curS];
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 0.5f, 0.5f, 1.0f, 1.0f);

	// draw x axis
	float xaxisverts[] = {vmin.x, 0.0f, 0.0f, vmax.x, 0.0f, 0.0f};
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, xaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, xaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	// draw y axis
	float yaxisverts[] = {0.0f, vmin.y, 0.0f, 0.0f, vmax.y, 0.0f};
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, yaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, yaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	// draw z axis
	float zaxisverts[] = {0.0f, 0.0f, vmin.z, 0.0f, 0.0f, vmax.z};
	//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, zaxisverts);
    glVertexPointer(3, GL_FLOAT, 0, zaxisverts);
#ifdef DEBUG
	CHECKGLERROR();
#endif
	glDrawArrays(GL_LINES, 0, 2);

	//float interval = (10.0f/g_zoom);
	//float interval = log10(g_zoom * 100.0f) * 10.0f;
	//float interval = (int)(1.0f/g_zoom/10.0f)*10.0f;

	//float base = PROJ_RIGHT/20*2;
	//float invzoom = base/g_zoom;
	//int power = log(invzoom)/log(base);
	//float interval = pow(base, power);

	//STOREY_HEIGHT=250
	//float baseinterval = STOREY_HEIGHT / 5.0f;
	//float screenfit = PROJ_RIGHT*2/g_zoom;
	//int scale =

	//-  float invzoom = 2.0f/g_zoom;
	//-  int tenpower = log(invzoom)/log(2);
	//-  float interval = pow(2, tenpower);
	//+
	//+  float base = PROJ_RIGHT/15*2;
	//+  float invzoom = base/g_zoom;
	//+  int power = log(invzoom)/log(base);
	//+  float interval = pow(base, power);

	float base = 50.0f;
	//float invzoom = base/g_zoom;
	//int power = log(invzoom)/log(base);
	//float interval = pow(base, power);

	float interval;

	// zoom 1 -> interval 50
	// zoom 0.5 -> interval = 100
	// zoom 0.25 -> interval = 200
	// zoom 0.125 -> interval = 400
	// zoom 0.0625 -> interval 800

	//if(power == 0)
	//if(g_zoom > 1.0f)
	{
		//	interval = invzoom;
		//interval = base / pow(2, (int)g_zoom-1);
		int power2 = log(g_zoom) / log(2.0f);
		interval = base / pow(2.0f, (float)power2);
	}

	// zoom 1 -> interval 50 = 50 / 1 = 50 / 2^0
	// zoom 2 -> interval 25 = 50 / 2 = 50 / 2^1
	// zoom 3 -> interval 12.5 = 50 / 4 = 50 / 2^2
	// zoom 4 -> interval 6.25 = 50 / 8 = 50 / 2^3
	// zoom 5 -> interval 3.125 = 50 / 16 = 50 / 2^4
	// wrong


	Vec3f start = Vec3f( (int)(vmin.x/interval)*interval, (int)(vmin.y/interval)*interval, (int)(vmin.z/interval)*interval );
	Vec3f end = Vec3f( (int)(vmax.x/interval)*interval, (int)(vmax.y/interval)*interval, (int)(vmax.z/interval)*interval );

	//dots
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 0.3f, 0.3f, 0.3f, 1.0f);
	for(float x=start.x; x<=end.x; x+=interval)
		for(float y=start.y; y<=end.y; y+=interval)
			for(float z=start.z; z<=end.z; z+=interval)
			{
				float point[] = {x, y, z};
				//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, point);
                glVertexPointer(3, GL_FLOAT, 0, point);
				glDrawArrays(GL_POINTS, 0, 1);
			}

			// crosses more spaced out
			float interval2 = interval * 5.0f;

			start = Vec3f( (int)(vmin.x/interval2)*interval2, (int)(vmin.y/interval2)*interval2, (int)(vmin.z/interval2)*interval2 );
			end = Vec3f( (int)(vmax.x/interval2)*interval2, (int)(vmax.y/interval2)*interval2, (int)(vmax.z/interval2)*interval2 );

			for(float x=start.x; x<=end.x; x+=interval2)
				for(float y=start.y; y<=end.y; y+=interval2)
					for(float z=start.z; z<=end.z; z+=interval2)
					{
						float xline[] = {x-interval/2.0f, y, z, x+interval/2.0f, y, z};
						//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, xline);
                        glVertexPointer(3, GL_FLOAT, 0, xline);
						glDrawArrays(GL_LINES, 0, 2);

						float yline[] = {x, y-interval/2.0f, z, x, y+interval/2.0f, z};
						//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, yline);
                        glVertexPointer(3, GL_FLOAT, 0, yline);
						glDrawArrays(GL_LINES, 0, 2);

						float zline[] = {x, y, z-interval/2.0f, x, y, z+interval/2.0f};
						//glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, zline);
                        glVertexPointer(3, GL_FLOAT, 0, zline);
						glDrawArrays(GL_LINES, 0, 2);
					}
}

void DrawViewport(int which, int x, int y, int width, int height)
{
#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	EndS();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_projtype == PROJ_PERSP && v->m_type == VIEWPORT_ANGLE45O)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else if(g_projtype == PROJ_ORTHO || v->m_type != VIEWPORT_ANGLE45O)
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	g_camproj = projection;

	//Vec3f viewvec = g_focus;	//g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view;
	Vec3f focusvec = v->focus();
	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	posvec = g_cam.m_view + t->m_offset;

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = g_cam.m_up;
	Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

#if 0
	if(g_mode == PREREND_ADJFRAME || g_mode == RENDERING)
	{
		focusvec = g_cam.m_view;
		posvec = g_cam.m_pos;
		Vec3f sidevec = Cross(Normalize(focusvec-posvec), Vec3f(0,1,0));
		upvec = Cross(sidevec, Normalize(focusvec-posvec));
	}
#endif

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

	g_camview = viewmat;

	Matrix modelview;
	Matrix modelmat;
	float translation[] = {0, 0, 0};
	modelview.setTranslation(translation);
	modelmat.setTranslation(translation);
	//modelview.postmult(viewmat);
	modelview.postmult(viewmat);

	g_cammodelview = modelview;

	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);
	g_cammvp = mvpmat;

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;
	//Vec3f vmin = g_focus - Vec3f(extentx, extenty, extentx);
	//Vec3f vmax = g_focus + Vec3f(extentx, extenty, extentx);
	Vec3f vmin = g_cam.m_view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.m_view + Vec3f(extentx, extenty, extentx);

	//draw only one layer of grid dots ...
	if(v->m_type == VIEWPORT_FRONT)	// ... on the x and y axises
	{
		vmin.z = 0;
		vmax.z = 0;
	}
	else if(v->m_type == VIEWPORT_LEFT)	// ... on the y and z axises
	{
		vmin.x = 0;
		vmax.x = 0;
	}
	else if(v->m_type == VIEWPORT_TOP)	// ... on the x and z axises
	{
		vmin.y = 0;
		vmax.y = 0;
	}

#if 1
	if(v->m_type == VIEWPORT_ANGLE45O)
	{
		EndS();
		//RenderToShadowMap(projection, viewmat, modelmat, g_focus);
#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif
		RenderToShadowMap(projection, viewmat, modelmat, g_cam.m_view, DrawSceneDepth);
#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif
		RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif
	//EndS();
	//Log("sh at t p:"<<g_curS<<std::endl;
	//

#if 1
	if(v->m_type == VIEWPORT_FRONT || v->m_type == VIEWPORT_LEFT || v->m_type == VIEWPORT_TOP)
	{
		UseS(SHADER_COLOR3D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		Shader* s = &g_shader[g_curS];

		//if(s->m_slot[SSLOT_PROJECTION] == -1)	ErrMess("po", "po");
		//if(s->m_slot[SSLOT_VIEWMAT] == -1)	ErrMess("v", "v");
		//if(s->m_slot[SSLOT_MODELMAT] == -1)	ErrMess("m", "m");

#if 0
		for(int i=0; i<16; i+=4)
		{
			float* m = projection.m_matrix;
			Log("pr");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}

		for(int i=0; i<16; i+=4)
		{
			float* m = viewmat.m_matrix;
			Log("vm");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}

		for(int i=0; i<16; i+=4)
		{
			float* m = modelmat.m_matrix;
			Log("mm");
			Log("\t"<<m[i+0]<<","<<m[i+1]<<","<<m[i+2]<<","<<m[i+3]<<std::endl;
		}
#endif

		glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m_matrix);
		glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m_matrix);
		glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m_matrix);
		glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m_matrix);
		//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
		DrawGrid(vmin, vmax);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		EndS();
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	//EndS();

#if 1
	//if(v->m_type == VIEWPORT_FRONT || v->m_type == VIEWPORT_LEFT || v->m_type == VIEWPORT_TOP)
	{
		Shader* s = &g_shader[g_curS];

#if 1
		if(persp)
		//if(v->m_type == VIEWPORT_ANGLE45O)
		{
			UseS(SHADER_COLOR3DPERSP);
			s = &g_shader[g_curS];
			glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m_matrix);
			//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
		}
		else
#endif
		{
			UseS(SHADER_COLOR3D);
			s = &g_shader[g_curS];
			glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, GL_FALSE, projection.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, GL_FALSE, viewmat.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, GL_FALSE, modelmat.m_matrix);
			glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, GL_FALSE, mvpmat.m_matrix);
		}

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		DrawFilled(&g_edmap, g_modelholder);
#ifdef DEBUG
		CHECKGLERROR();
		LastNum(__FILE__, __LINE__);
#endif

		if(v->m_type != VIEWPORT_ANGLE45O)
			DrawOutlines(&g_edmap, g_modelholder);
		else
			DrawSelOutlines(&g_edmap, g_modelholder);
		glEnable(GL_DEPTH_TEST);
		EndS();

		UseS(SHADER_COLOR2D);
		s = &g_shader[g_curS];
		glUniform1f(s->m_slot[SSLOT_WIDTH], (float)width);
		glUniform1f(s->m_slot[SSLOT_HEIGHT], (float)height);
		glUniform4f(s->m_slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
		//glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
		DrawDrag(&g_edmap, &mvpmat, width, height, persp);
#ifdef GLDEBUG
		CHECKGLERROR();
#endif
		EndS();

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		if(g_edtool == EDTOOL_CUT && v->m_ldown && !g_keys[SDL_SCANCODE_LCTRL] && !g_keys[SDL_SCANCODE_RCTRL])
		{
			UseS(SHADER_COLOR2D);
			glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)width);
			glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)height);
			glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 1, 0, 0, 1);
			//glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION]);
			//glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_TEXCOORD0]);
			/*
			Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, t->m_up));
			float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;
			Vec3f last = t->m_up*(float)v->m_lastmouse.y*screenratio + strafe*(float)-v->m_lastmouse.x*screenratio;
			Vec3f cur = t->m_up*(float)v->m_curmouse.y*screenratio + strafe*(float)-v->m_curmouse.x*screenratio;
			*/
			Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));
			//last and current cursor positions relative to viewport top,left corner
			Vec3f last = OnNear(v->m_lastmouse.x, v->m_lastmouse.y, width, height, posvec, sidevec, upvec);
			Vec3f cur = OnNear(v->m_curmouse.x, v->m_curmouse.y, width, height, posvec, sidevec, upvec);
			//snap the last and current cursor positions to the nearest grid point (grid size is variable)
			last.x = SnapNearest(g_snapgrid, last.x);
			last.y = SnapNearest(g_snapgrid, last.y);
			last.z = SnapNearest(g_snapgrid, last.z);
			cur.x = SnapNearest(g_snapgrid, cur.x);
			cur.y = SnapNearest(g_snapgrid, cur.y);
			cur.z = SnapNearest(g_snapgrid, cur.z);
			//Vec4f last4 = Vec4f(last, 1);
			//Vec4f cur4 = Vec4f(cur, 1);
			//last4.transform(mvpmat);
			//cur4.transform(mvpmat);

			// get xyzw vector (vec4f) for pixel coordinates of cursor pos's
			Vec4f last4 = ScreenPos(&mvpmat, last, width, height, persp);
			Vec4f cur4 = ScreenPos(&mvpmat, cur, width, height, persp);

			float line[] = {last4.x, last4.y, 0, cur4.x, cur4.y, 0};
			//glVertexAttribPointer(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, line);
            glVertexPointer(3, GL_FLOAT, 0, line);
#ifdef GLDEBUG
			//CHECKGLERROR();
			CHECKGLERROR();
#endif
			glDrawArrays(GL_LINES, 0, 2);
			//CHECKGLERROR();

#ifdef GLDEBUG
			//CHECKGLERROR();
			CHECKGLERROR();
#endif
			//Log("cut draw "<<v->m_lastmouse.x<<","<<v->m_lastmouse.y<<"->"<<v->m_curmouse.x<<","<<v->m_curmouse.y<<std::endl;
			//Log("cut draw2 "<<last4.x<<","<<last4.y<<"->"<<cur4.x<<","<<cur4.y<<std::endl;
			//Log("cut draw3 "<<last.x<<","<<last.y<<","<<last.z<<"->"<<cur.x<<","<<cur.y<<","<<cur.z<<std::endl;
			//
			EndS();
		}
#ifdef GLDEBUG
		CHECKGLERROR();
#endif
	}
#endif

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	Ortho(width, height, 1, 0, 0, 1);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif
	//glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	if(g_mode != RENDERING)
	{
		RichText rlabel(t->m_label);
		DrawShadowedText(MAINFONT8, 0, 0, &rlabel, NULL, -1);
	}

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	glFlush();

#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
	CHECKGLERROR();
#endif

	//EndS();
}

bool ViewportLDown(int which, int relx, int rely, int width, int height)
{
	//return false;

	accum = Vec3f(0,0,0);
	g_changed = false;
	WriteH(&g_beforechange);	//write undo history
	VpWrap* v = &g_viewport[which];
	v->m_ldown = true;
	v->m_lastmouse = Vec2i(relx, rely);
	v->m_curmouse = Vec2i(relx, rely);

	if(g_edtool != EDTOOL_NONE)
	{
		g_dragS = -1;
		g_dragV = -1;
		g_dragW = false;
		g_dragD = -1;
		return true;
	}

	VpType* t = &g_vptype[v->m_type];

	//Log("vp["<<which<<"] l down");
	//

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(v->m_type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	//Vec3f viewvec = g_focus; //g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view;
	Vec3f focusvec = v->focus();
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

	Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	//mvpmat.postmult(viewmat);
	mvpmat.postmult(viewmat);

	SelectDrag(&g_edmap, &mvpmat, width, height, relx, rely, posvec, persp);

	return true;
}


bool ViewportRDown(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	v->m_rdown = true;
	v->m_lastmouse = Vec2i(relx, rely);
	v->m_curmouse = Vec2i(relx, rely);

	return true;
}

void CutBrushes(Plane cuttingp)
{
	EdMap* map = &g_edmap;

	Vec3f pop = PointOnPlane(cuttingp);
	BrushSide news0(cuttingp.m_normal, pop);
	BrushSide news1(Vec3f(0,0,0)-cuttingp.m_normal, pop);

	std::list<Brush*> newsel;

	for(std::list<Brush*>::iterator i=g_selB.begin(); i!=g_selB.end(); )
	{
		Brush* b = *i;

		Brush newb0 = *b;
		Brush newb1 = *b;

		newb0.add(news0);
		newb1.add(news1);

		newb0.collapse();
		newb1.collapse();

		newb0.remaptex();
		newb1.remaptex();

		g_edmap.m_brush.push_back(newb0);
		std::list<Brush>::reverse_iterator j = map->m_brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}

		g_edmap.m_brush.push_back(newb1);
		j = map->m_brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}

		for(std::list<Brush>::iterator k=map->m_brush.begin(); k!=map->m_brush.end(); k++)
		{
			if(&*k == b)
			{
				map->m_brush.erase(k);
				break;
			}
		}

		i = g_selB.erase( i );
	}

	g_selB.clear();
	g_selB = newsel;
	g_sel1b = NULL;

	//g_selB.push_back( &*map->m_brush.rbegin() );
}

void ViewportLUp_CutBrush(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	//Vec3f viewvec = g_focus;	//g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view - g_cam.m_pos;
	//Vec3f viewdir = v->viewdir();
	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
		//posvec = g_cam.m_view + t->m_offset;
		//viewvec = Normalize(Vec3f(0,0,0)-t->m_offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewdir, upv2ec));
	Vec3f sidevec = v->strafe();

	Vec3f last = OnNear(v->m_lastmouse.x, v->m_lastmouse.y, width, height, posvec, sidevec, up2vec);
	Vec3f cur = OnNear(v->m_curmouse.x, v->m_curmouse.y, width, height, posvec, sidevec, up2vec);
	last.x = SnapNearest(g_snapgrid, last.x);
	last.y = SnapNearest(g_snapgrid, last.y);
	last.z = SnapNearest(g_snapgrid, last.z);
	cur.x = SnapNearest(g_snapgrid, cur.x);
	cur.y = SnapNearest(g_snapgrid, cur.y);
	cur.z = SnapNearest(g_snapgrid, cur.z);

	if(last != cur)
	{
		LinkPrevUndo();
		Vec3f crossaxis[2];
		crossaxis[0] = Normalize(cur - last);
		crossaxis[1] = Normalize(t->m_offset);
		Plane cuttingp;
		cuttingp.m_normal = Normalize( Cross(crossaxis[0], crossaxis[1]) );
		cuttingp.m_d = PlaneDistance(cuttingp.m_normal, last);
		CutBrushes(cuttingp);
		//LinkLatestUndo();
	}

	g_edtool = EDTOOL_NONE;
}

void ViewportLUp_Explosion(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	//Vec3f viewvec = g_focus;	//g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view - g_cam.m_pos;
	//Vec3f viewdir = v->viewdir();
	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
		//posvec = g_cam.m_view + t->m_offset;
		//viewvec = Normalize(Vec3f(0,0,0)-t->m_offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewdir, upv2ec));
	Vec3f sidevec = v->strafe();

	float aspect = fabsf((float)width / (float)height);

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;

	Vec3f vmin = g_cam.m_view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.m_view + Vec3f(extentx, extenty, extentx);

	Vec3f line[2];
	//Log("===========");
	//Log("t->m_offset = "<<t->m_offset.x<<","<<t->m_offset.y<<","<<t->m_offset.z<<std::endl;
	line[0] = OnNear(relx, rely, width, height, posvec, sidevec, up2vec);
	line[1] = line[0] - t->m_offset*2.0f;

	if(v->m_type == VIEWPORT_ANGLE45O)
	{
		line[1] = line[0] + Normalize(v->viewdir()) * (MAX_DISTANCE / 2.0f);
	}

	if(v->m_type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		line[0] = posvec;
		line[1] = posvec + ScreenPerspRay(relx, rely, width, height, posvec, sidevec, up2vec, v->viewdir(), FIELD_OF_VIEW) * (MAX_DISTANCE / 2.0f);
	}

	ExplodeCrater(&g_edmap, line, vmin, vmax);

	g_edtool = EDTOOL_NONE;
}

void ViewportLUp_SelectBrush(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	float aspect = fabsf((float)width / (float)height);

	float extentx = PROJ_RIGHT*aspect/g_zoom;
	float extenty = PROJ_RIGHT/g_zoom;
	//Vec3f vmin = g_focus - Vec3f(extentx, extenty, extentx);
	//Vec3f vmax = g_focus + Vec3f(extentx, extenty, extentx);
	Vec3f vmin = g_cam.m_view - Vec3f(extentx, extenty, extentx);
	Vec3f vmax = g_cam.m_view + Vec3f(extentx, extenty, extentx);

	//Vec3f viewvec = g_focus;	//g_cam.m_view;
	//Vec3f viewvec = g_cam.m_view - g_cam.m_pos;

	//Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_cam.m_pos;
	Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
		//	posvec = g_cam.m_view + t->m_offset;
		//	viewvec = Normalize(Vec3f(0,0,0)-t->m_offset);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
	//Vec3f posvec2 = g_cam.lookpos() + t->m_offset;
	//Vec3f upvec = t->m_up;
	//Vec3f upvec = g_cam.up2();
	Vec3f up2vec = v->up2();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	//Vec3f vCross = Cross(m_view - m_pos, m_up);
	//Vec3f sidevec = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, upvec));
	//Vec3f sidevec = Normalize(Cross(viewvec, upvec));
	Vec3f sidevec = v->strafe();

	//Log("viewvec "<<viewvec.x<<","<<viewvec.y<<","<<viewvec.z<<std::endl;
	//Log("upvec "<<upvec.x<<","<<upvec.y<<","<<upvec.z<<std::endl;
	//Log("sidevec "<<sidevec.x<<","<<sidevec.y<<","<<sidevec.z<<std::endl;
	//

#if 0
	// pass frustum to SelectBrush to cull possible selection?
	if(v->m_type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSEP)
	{
		Frustum frust;
		frust.CalculateFrustum(proj, modl);
	}
#endif

	Vec3f line[2];
	//Log("===========");
	//Log("t->m_offset = "<<t->m_offset.x<<","<<t->m_offset.y<<","<<t->m_offset.z<<std::endl;
	line[0] = OnNear(relx, rely, width, height, posvec, sidevec, up2vec);
	line[1] = line[0] - t->m_offset*2.0f;

	if(v->m_type == VIEWPORT_ANGLE45O)
	{
		line[1] = line[0] + Normalize(v->viewdir()) * (MAX_DISTANCE / 2.0f);
	}

	if(v->m_type == VIEWPORT_ANGLE45O && g_projtype == PROJ_PERSP)
	{
		line[0] = posvec;
		line[1] = posvec + ScreenPerspRay(relx, rely, width, height, posvec, sidevec, up2vec, v->viewdir(), FIELD_OF_VIEW) * (MAX_DISTANCE / 2.0f);
	}

	SelectBrush(&g_edmap, line, vmin, vmax);
	//Log("============");
}

bool ViewportLUp(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	if(v->m_ldown)
	{
		//return true;
		v->m_ldown = false;

		if(g_changed)
		{
			LinkPrevUndo(&g_beforechange);
			g_changed = false;
			//LinkLatestUndo();
		}

		//Log("vp["<<which<<"] l up = false");
		//

		if(!g_keys[SDL_SCANCODE_LCTRL] && !g_keys[SDL_SCANCODE_RCTRL])
		{
			if(g_edtool == EDTOOL_CUT)
			{
				ViewportLUp_CutBrush(which, relx, rely, width, height);
			}
			else if(g_edtool == EDTOOL_EXPLOSION)
			{
				ViewportLUp_Explosion(which, relx, rely, width, height);
			}
			else if(g_sel1b == NULL && g_sel1m == NULL && g_dragV < 0 && g_dragS < 0)
			{
				ViewportLUp_SelectBrush(which, relx, rely, width, height);
			}
		}
	}

	//g_sel1b = NULL;
	//g_dragV = -1;
	//g_dragS = -1;

	return false;
}

bool ViewportRUp(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	v->m_rdown = false;

	return false;
}

bool ViewportMousewheel(int which, int delta)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	//if(v->m_type == VIEWPORT_ANGLE45O)
	{
		g_zoom *= 1.0f + (float)delta / 10.0f;
		return true;
	}

	return false;
}

void ViewportTranslate(int which, int dx, int dy, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	//Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, t->m_up));

	float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;

	//Vec3f move = t->m_up*(float)dy*screenratio + strafe*(float)-dx*screenratio;

	//Vec3f up = Normalize(Cross(strafe, Vec3f(0,0,0)-t->m_offset));
	Vec3f up2 = v->up2();

	//if(v->m_type == VIEWPORT_ANGLE45O)
	//{
	//	strafe = g_cam.m_strafe;
	//	up = g_cam.up2();
	//}

	Vec3f strafe = v->strafe();

	Vec3f move = up2*(float)dy*screenratio + strafe*(float)-dx*screenratio;
	//g_cam.move(move);
	//g_focus = g_focus + move;
	g_cam.move(move);
}


void ViewportRotate(int which, int dx, int dy)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];
	/*
	Vec3f view = Normalize( Vec3f(0,0,0)-t->m_offset );

	Vec3f strafe = Normalize(Cross(view, t->m_up));

	if(Magnitude(view - t->m_up) <= EPSILON || Magnitude(Vec3f(0,0,0) - view - t->m_up) <= EPSILON)
	{
	strafe = Vec3f(1,0,0);
	t->m_offset = Vec3f(1000.0f/3, 1000.0f/3, 1000.0f/3);
	}*/

	//t->m_offset = RotateAround(t->m_offset, g_focus, dy / 100.0f, strafe.x, strafe.y, strafe.z);
	//t->m_offset = RotateAround(t->m_offset, g_focus, dx / 100.0f, t->m_up.x, t->m_up.y, t->m_up.z);

	g_cam.rotateabout(g_cam.m_view, dy / 100.0f, g_cam.m_strafe.x, g_cam.m_strafe.y, g_cam.m_strafe.z);
	g_cam.rotateabout(g_cam.m_view, dx / 100.0f, g_cam.m_up.x, g_cam.m_up.y, g_cam.m_up.z);

	//Log("rotate "<<dx/10.0f<<","<<dy/10.0f<<std::endl;
	//
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
}

//#define DRAGV_DEBUG

void DragV(Brush* b, BrushSide* s, int j, Vec3f& newv, bool& mergedv, bool* invalidv, bool& remove)
{
	remove = false;
	Vec3f movev = b->m_sharedv[ s->m_vindices[j] ];

	if(!mergedv)
	{
		for(int i=0; i<b->m_nsharedv; i++)
		{
			if(i == s->m_vindices[j])
				continue;

			Vec3f thisv = b->m_sharedv[i];
			float mag = Magnitude( newv - thisv );

			if(mag <= MERGEV_D)
			{
				newv = thisv;
				break;
			}
		}

		mergedv = true;
	}

	//if(s->m_outline.m_edv.size() % 2 == 0)
	//if(true)
	if(!g_keys[SDL_SCANCODE_LSHIFT] && !g_keys[SDL_SCANCODE_RSHIFT])
	{
		Vec3f farthestv = movev;
		float farthestD = 0;

		for(int i=0; i<s->m_ntris+2; i++)
		{
			Vec3f thisv = b->m_sharedv[ s->m_vindices[i] ];
			float mag = Magnitude( thisv - movev );

			if(mag > farthestD)
			{
				farthestD = mag;
				farthestv = thisv;
			}
		}

		//return Normalize( Cross( m_strafe, m_view - m_pos ) );

		Vec3f newline = Normalize(newv - farthestv);
		Vec3f crossaxis = Normalize(Cross(newline, s->m_plane.m_normal));
		s->m_plane.m_normal = Normalize(Cross(crossaxis, newline));
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, newv);

#ifdef DRAGV_DEBUG
		Log("crossaxis = "<<crossaxis.x<<","<<crossaxis.y<<","<<crossaxis.z<<std::endl;
		//Log("midv = "<<midv.x<<","<<midv.y<<","<<midv.z<<std::endl;
		//Log("crossaxis2 = "<<crossaxis2.x<<","<<crossaxis2.y<<","<<crossaxis2.z<<std::endl;
		
#endif
	}
	else
	{
		float farthestd[] = {0, 0};
		Vec3f farthestv[] = {movev, movev};

#ifdef DRAGV_DEBUG
		Log("--------------move v side"<<(s-b->m_sides)<<"------------------");
		
#endif

		for(int i=0; i<s->m_ntris+2; i++)
		{
			//if(i == j)
			//	continue;

			//if(invalidv[i])
			//	continue;

			Vec3f thisv = b->m_sharedv[ s->m_vindices[i] ];

			if(thisv == movev)
				continue;

			//if(thisv == newv)
			//	continue;

			float mag = Magnitude( thisv - movev );

			if(mag <= EPSILON)
				continue;

#ifdef DRAGV_DEBUG
			Log("thisv="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			Log("nearestd[0]="<<nearestd[0]<<std::endl;
			
#endif

			if(mag >= farthestd[0] || farthestd[0] <= 0)
			{
#ifdef DRAGV_DEBUG
				Log("closer vert0="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
				
#endif

				farthestd[0] = mag;
				farthestv[0] = thisv;
			}
		}

		for(int i=0; i<s->m_ntris+2; i++)
		{
			//if(i == j)
			//	continue;

			//if(invalidv[i])
			//	continue;

			Vec3f thisv = b->m_sharedv[ s->m_vindices[i] ];

			if(thisv == movev)
				continue;

			//if(thisv == newv)
			//	continue;

			if(thisv == farthestv[0])
				continue;

			float mag = Magnitude( thisv - movev );

			if(mag <= EPSILON)
				continue;

#ifdef DRAGV_DEBUG
			Log("thisv="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
			Log("nearestd[]="<<nearestd[1]<<std::endl;
			
#endif

			if(mag >= farthestd[1] || farthestd[1] <= 0)
			{
#ifdef DRAGV_DEBUG
				Log("closer vert1="<<thisv.x<<","<<thisv.y<<","<<thisv.z<<std::endl;
				
#endif

				farthestd[1] = mag;
				farthestv[1] = thisv;
			}
		}

		Vec3f tri[3];
		tri[0] = movev;
		//tri[0] = newv;
		tri[1] = farthestv[0];
		tri[2] = farthestv[1];

		if(Close(tri[0], tri[1]) || Close(tri[0], tri[2]) || Close(tri[1], tri[2]))
		{
			remove = true;
			return;	//invalid side should be discarded
		}

		Vec3f norm = Normal(tri);

		//Log("tri = ("<<tri[0].x<<","<<tri[0].y<<","<<tri[0].z<<"),("<<tri[1].x<<","<<tri[1].y<<","<<tri[1].z<<"),("<<tri[2].x<<","<<tri[2].y<<","<<tri[2].z<<")");
		//Log("tri norm="<<norm.x<<","<<norm.y<<","<<norm.z<<"    plane norm="<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<std::endl;

		if(Magnitude( norm - s->m_plane.m_normal ) > Magnitude( Vec3f(0,0,0) - norm - s->m_plane.m_normal ))
		{
			//Log("flip vertex order YES");
			//
			Vec3f tempv = farthestv[0];
			farthestv[0] = farthestv[1];
			farthestv[1] = tempv;
		}
		else
		{
#ifdef DRAGV_DEBUG
			Log("flip vertex order NO");
			
#endif
		}

		//return Normalize( Cross( m_strafe, m_view - m_pos ) );

		Vec3f crossaxis = Normalize(farthestv[0] - farthestv[1]);
		Vec3f midv = (farthestv[0] + farthestv[1])/2.0f;
		Vec3f crossaxis2 = Normalize(newv - midv);

		//Vec3f crossaxis2 = Normalize(newv - s->m_centroid);
		//Vec3f crossaxis = Normalize(Cross( crossaxis2, s->m_plane.m_normal ));

		s->m_plane.m_normal = Normalize(Cross(crossaxis, crossaxis2));
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, newv);

#ifdef DRAGV_DEBUG
		Log("crossaxis = "<<crossaxis.x<<","<<crossaxis.y<<","<<crossaxis.z<<std::endl;
		Log("midv = "<<midv.x<<","<<midv.y<<","<<midv.z<<std::endl;
		Log("crossaxis2 = "<<crossaxis2.x<<","<<crossaxis2.y<<","<<crossaxis2.z<<std::endl;
		
#endif
	}
}

void Drag_Brush(Brush* b, Vec3f newmove)
{
	std::list<float> oldus;
	std::list<float> oldvs;

	for(int i=0; i<b->m_nsides; i++)
	{
		BrushSide* s = &b->m_sides[i];

		//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
		float oldu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		float oldv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
		//Vec3f axis = s->m_plane.m_normal;
		//float radians = DEGTORAD(degrees);
		//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
		//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);

		oldus.push_back(oldu);
		oldvs.push_back(oldv);

		Vec3f pop = PointOnPlane(s->m_plane);
		pop = pop - newmove;
		s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);

	}

	b->collapse();

	std::list<float>::iterator oldu = oldus.begin();
	std::list<float>::iterator oldv = oldvs.begin();

	for(int i=0; i<b->m_nsides; i++, oldu++, oldv++)
	{
		BrushSide* s = &b->m_sides[i];

		//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
		float newu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
		float newv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
		float changeu = newu - *oldu;
		float changev = newv - *oldv;
		s->m_tceq[0].m_d -= changeu;
		s->m_tceq[1].m_d -= changev;
	}

	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
}

void Drag_BrushVert(Brush* b, Vec3f newmove)
{
	bool* invalidv = new bool[b->m_nsharedv];
	for(int i=0; i<b->m_nsharedv; i++)
		invalidv[i] = false;

	Vec3f movev = b->m_sharedv[ g_dragV ];
	Vec3f newv = movev - newmove;
	bool mergedv = false;

	for(int i=0; i<b->m_nsides; i++)
	{
		BrushSide* s = &b->m_sides[i];
		for(int j=0; j<s->m_ntris+2; j++)
		{
			if(s->m_vindices[j] == g_dragV)
			{
				bool remove;
				DragV(b, s, j, newv, mergedv, invalidv, remove);

				//s->gentexeq();
				s->remaptex();
				b->prunev(invalidv);

				//if(invalidv[ s->m_vindices[j] ])
				//	g_dragV = -1;

				//if(remove)
				{
					//	b->removeside(i);
					//	i--;
					//	break;
				}
			}
		}
	}
	delete [] invalidv;
	b->collapse();
	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
}

void Drag_BrushSide(Brush* b, Vec3f newmove)
{
	BrushSide* s = &b->m_sides[g_dragS];
	Vec3f pop = PointOnPlane(s->m_plane);
	pop = pop - newmove;
	s->m_plane.m_d = PlaneDistance(s->m_plane.m_normal, pop);
	b->collapse();
	b->remaptex();
	PruneB(&g_edmap, g_sel1b);

	VpType* t = &g_vptype[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
}

void Drag_BrushDoor(Brush* b, Vec3f newmove)
{
	EdDoor* door = b->m_door;

	if(g_dragD == DRAG_DOOR_POINT)
	{
		door->point = door->point - newmove;
	}
	else if(g_dragD == DRAG_DOOR_AXIS)
	{
		door->axis = door->axis - newmove;
	}
}

void Drag_Model(ModelHolder* mh, Vec3f newmove)
{
	mh->translation = mh->translation + newmove;
	mh->absmin = mh->absmin + newmove;
	mh->absmax = mh->absmax + newmove;
}

void Drag_ModelSide(ModelHolder* mh, Vec3f newmove)
{
	int side = g_dragS;

	if(side < 0)
		return;

	Vec3f scalechange(1,1,1);
	Vec3f span = mh->absmax - mh->absmin;

	if(span.x <= 0.0f)
		span.x = 1;
	if(span.y <= 0.0f)
		span.y = 1;
	if(span.z <= 0.0f)
		span.z = 1;

	if(side == DRAG_TOP)
	{
		scalechange.y = 1 + (newmove.y / span.y);
		mh->translation.y = mh->translation.y + newmove.y/2;
	}
	else if(side == DRAG_BOTTOM)
	{
		scalechange.y = 1 - (newmove.y / span.y);
		mh->translation.y = mh->translation.y + newmove.y/2;
	}
	else if(side == DRAG_LEFT)
	{
		scalechange.x = 1 - (newmove.x / span.x);
		mh->translation.x = mh->translation.x + newmove.x/2;
	}
	else if(side == DRAG_RIGHT)
	{
		scalechange.x = 1 + (newmove.x / span.x);
		mh->translation.x = mh->translation.x + newmove.x/2;
	}
	else if(side == DRAG_NEAR)
	{
		scalechange.z = 1 + (newmove.z / span.z);
		mh->translation.z = mh->translation.z + newmove.z/2;
	}
	else if(side == DRAG_FAR)
	{
		scalechange.z = 1 - (newmove.z / span.z);
		mh->translation.z = mh->translation.z + newmove.z/2;
	}

	//char msg[1024];
	//sprintf(msg, "newmove %f,%f,%f \n span %f,%f,%f \n scale cahge %f,%f,%f", newmove.x, newmove.y, newmove.z, span.x, span.y, span.z, scalechange.x, scalechange.y, scalechange.z);
	//MessageBox(g_hWnd, msg, "asd", NULL);

	mh->scale = mh->scale * scalechange;

	if(mh->scale.x <= 0.0f)
		mh->scale.x = 1;
	if(mh->scale.y <= 0.0f)
		mh->scale.y = 1;
	if(mh->scale.z <= 0.0f)
		mh->scale.z = 1;

	mh->retransform();
}

void Drag(int which, int dx, int dy, int width, int height)
{
	VpWrap* v = &g_viewport[which];
	VpType* t = &g_vptype[v->m_type];

	//Vec3f strafe = Normalize(Cross(Vec3f(0,0,0)-t->m_offset, t->m_up));
	Vec3f strafe = v->strafe();
	Vec3f up2 = v->up2();

	float screenratio = (2.0f*PROJ_RIGHT)/(float)height/g_zoom;

	Vec3f move = up2*(float)dy*screenratio + strafe*(float)-dx*screenratio;
	Vec3f newmove;
	newmove.x = Snap(g_snapgrid, move.x + accum.x);
	newmove.y = Snap(g_snapgrid, move.y + accum.y);
	newmove.z = Snap(g_snapgrid, move.z + accum.z);
	accum = accum + move - newmove;

	//Log("move = "<<move.x<<","<<move.y<<","<<move.z<<std::endl;
	//Log("newmove = "<<newmove.x<<","<<newmove.y<<","<<newmove.z<<std::endl;
	//Log("accum = "<<accum.x<<","<<accum.y<<","<<accum.z<<std::endl;
	//

	if(newmove != Vec3f(0,0,0))
		g_changed = true;

	if(g_sel1b)
	{
		Brush* b = g_sel1b;

		if(g_dragW)
		{
			Drag_Brush(b, newmove);
		}
		else if(g_dragV >= 0)
		{
			Drag_BrushVert(b, newmove);
		}
		else if(g_dragS >= 0)
		{
			Drag_BrushSide(b, newmove);
		}
		else if(g_dragD >= 0)
		{
			Drag_BrushDoor(b, newmove);
		}
	}
	else if(g_sel1m)
	{
		ModelHolder* mh = g_sel1m;

		//MessageBox(g_hWnd, "drag m", "asd", NULL);

		Vec3f modelnewmove = Vec3f(0,0,0) - newmove;

		if(g_dragW)
		{
			//MessageBox(g_hWnd, "drag m w", "asd", NULL);
			Drag_Model(mh, modelnewmove);
		}
		else if(g_dragS >= 0)
		{
			Drag_ModelSide(mh, modelnewmove);
		}
	}
}

bool ViewportMousemove(int which, int relx, int rely, int width, int height)
{
	VpWrap* v = &g_viewport[which];

	if(v->m_ldown)
	{
		//Log("vp["<<which<<"] down mouse move l");
		//

		if(g_sel1b || g_sel1m)
		{
			Drag(which, relx - v->m_lastmouse.x, rely - v->m_lastmouse.y, width, height);
		}
		else if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		{
			ViewportTranslate(which, relx - v->m_lastmouse.x, rely - v->m_lastmouse.y, width, height);
		}

		if((g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL]) || g_edtool != EDTOOL_CUT || !v->m_ldown)
			v->m_lastmouse = Vec2i(relx, rely);
		v->m_curmouse = Vec2i(relx, rely);
		return true;
	}
	else if(v->m_rdown)
	{
		if(v->m_type == VIEWPORT_ANGLE45O)
		{
			ViewportRotate(which, relx - v->m_lastmouse.x, rely - v->m_lastmouse.y);
		}
		v->m_lastmouse = Vec2i(relx, rely);
		v->m_curmouse = Vec2i(relx, rely);
		return true;
	}

	if((g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL]) || g_edtool != EDTOOL_CUT || (!v->m_ldown && !v->m_rdown))
		v->m_lastmouse = Vec2i(relx, rely);

	v->m_curmouse = Vec2i(relx, rely);

	return false;
}
