

#include "ordraw.h"
#include "../app/appmain.h"
#include "../app/segui.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/tile.h"
#include "../render/sprite.h"
#include "../tool/rendersprite.h"
#include "../save/compilemap.h"
#include "../gui/gui.h"
#include "../render/shadow.h"
#include "../save/saveedm.h"
#include "../app/appmain.h"
#include "../render/sortb.h"
#include "../math/camera.h"
#include "../app/seviewport.h"

OrList g_orlist;

bool g_rolllock = false;

//project vector onto plane, ie draw a line from the vector parallal to plane normal, up to the plane
Vec3f ProjVecOntoPl(Vec3f v, Plane3f p)
{
	//http://www.maplesoft.com/support/help/Maple/view.aspx?path=MathApps/ProjectionOfVectorOntoPlane
	//plane assumed to have d=0 (on origin)
	//subtract "cen" (object center) from v first before following equation, then add it back,
	//if object is not centered on origin.
	return (v - p.m_normal * Dot(v, p.m_normal) / (pow(Magnitude(p.m_normal),2.0f)) );
	
}

void DrawOr(OrList *ol, int frame, Vec3f pos, 
		float pitchrad, 
		float yawrad)
{
	//return;
	Shader* s = &g_shader[g_curS];

	//Vec3f pos(0,0,0);

	int ci = SpriteRef(
		ol->rotations,
		ol->sides,
		ol->nsides,
		ol->frames,
		ol->nframes,
		ol->inclines,
		0,
		g_renderframe,
		g_currincline,
		0,
		0,
		0,
		0,0);

	if(ci >= ol->nors)
	{
		char mm[1234];
		sprintf(mm,
			">ors \r\n"\
			"f%d on%d"\
			"currinc%d \r\n"\
			"ci%d olnors%d",
			g_renderframe,
			(int)ol->on,
			(int)g_currincline,
			ci, (int)ol->nors);
		ErrMess("asdasd",mm);
	}

	Or* o = &ol->ors[ci];

	float maxrad = o->maxrad;

#if 0
	Matrix modelmat;
	modelmat.setTranslation((const float*)&pos);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	Matrix mvp;
	mvp.set(g_camproj.m_matrix);
	mvp.postmult(g_camview);
	mvp.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix modelview;
#ifdef SPECBUMPSHADOW
	modelview.set(g_camview.m_matrix);
#endif
	modelview.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELVIEW], 1, 0, modelview.m_matrix);

	//modelview.set(g_camview.m_matrix);
	//modelview.postmult(modelmat);
	Matrix modelviewinv;
	Transpose(modelview, modelview);
	Inverse2(modelview, modelviewinv);
	//Transpose(modelviewinv, modelviewinv);
	glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);
#endif
	//no model translation/rotation matrix needed
	
	Matrix mvp;
	//if(g_curS != SHADER_ORPERSP)
	{
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(g_camview);
	}
	//else
	{
	//	mvp.set(g_camproj.m_matrix);
	//	mvp.postmult2(g_camview);
	}
	//mvp.postmult(modelmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);

	Matrix imvp;
	InvertMatrix(mvp.m_matrix, imvp.m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_IMVP], 1, 0, imvp.m_matrix);

	Vec3f viewdir = Normalize(g_cam.m_view - g_cam.m_pos);
	Vec3f updir = g_cam.up2();
	Vec3f sidedir = Normalize(g_cam.m_strafe);

	
	VpWrap* vp = &g_viewport[3];
	Vec3f viewpos = vp->pos();

	Vec3f v[6];
	Vec2f t[6];
	///////

	if(g_projtype == PROJ_ORTHO)
		maxrad *= 1.1;
	else if(g_projtype == PROJ_PERSP)
		maxrad * 1.9;

	Vec3f a, b, c, d;
	a = pos + updir * maxrad - sidedir * maxrad;
	b = pos + updir * maxrad + sidedir * maxrad;
	c = pos - updir * maxrad + sidedir * maxrad;
	d = pos - updir * maxrad - sidedir * maxrad;

	Vec4f a4 = Vec4f(a.x, a.y, a.z, 1);
	Vec4f b4 = Vec4f(b.x, b.y, b.z, 1);
	Vec4f c4 = Vec4f(c.x, c.y, c.z, 1);
	Vec4f d4 = Vec4f(d.x, d.y, d.z, 1);

	glUniform4fv(s->m_slot[SSLOT_CORNERA], 1, (float*)&a4);
	glUniform4fv(s->m_slot[SSLOT_CORNERB], 1, (float*)&b4);
	glUniform4fv(s->m_slot[SSLOT_CORNERC], 1, (float*)&c4);
	glUniform4fv(s->m_slot[SSLOT_CORNERD], 1, (float*)&d4);

	//glUniform3fv(s->m_slot[SSLOT_CAMCEN], 1, (float*)&g_cam.m_pos);
	glUniform3fv(s->m_slot[SSLOT_CAMCEN], 1, (float*)&viewpos);

	glUniform1i(s->m_slot[SSLOT_ORJPLWPX], (int)g_orwpx);
	glUniform1i(s->m_slot[SSLOT_ORJPLHPX], (int)g_orhpx);
	glUniform1i(s->m_slot[SSLOT_ORJLONS], (int)g_orlons);
	glUniform1i(s->m_slot[SSLOT_ORJLATS], (int)g_orlats);
	glUniform1i(s->m_slot[SSLOT_ORJROLLS], (int)g_orrolls);
	
	glUniform1i(s->m_slot[SSLOT_ORMAPSZ], (int)g_bigtex);

	//angle of view around the object will be opposite
	//like if you view it from back with your view vector = (0,0,1) looking forward,
	//object's view vector of rotation should be (0,0,-1) (with it's backside front, and looking backward)
	//Vec3f objview = Vec3f(0,0,0) - viewdir;
	//Vec3f objview = viewdir;
	//orientation of object forward in absolute space
	Vec3f objview = Normalize( Vec3f(0,0,1) );
	Vec3f objside = Normalize( Vec3f(1,0,0) );

#if 0
	//tests
	{
		//test1
		Vec3f viewang1 = Vec3f(0,0,1);
		Vec3f viewside1 = Vec3f(1,0,0);
		Vec3f viewang2 = Vec3f(0,0,1);
		Vec3f viewside2 = Vec3f(1,0,0);

#if 0
		//Vec3f up = Vec3f(0,1,0);
		//Vec3f side = Vec3f(1,0,0);
		//Vec3f view = Vec3f(0,0,-1);

		up = Rotate(up, M_PI*(float)orlat/(float)g_orlats, 1, 0, 0);
		side = Rotate(side, M_PI*(float)orlat/(float)g_orlats, 1, 0, 0);
		view = Rotate(view, M_PI*(float)orlat/(float)g_orlats, 1, 0, 0);

		up = Rotate(up, 2.0*M_PI*(float)orlon/(float)g_orlons, 0, 1, 0);
		side = Rotate(side, 2.0*M_PI*(float)orlon/(float)g_orlons, 0, 1, 0);
		view = Rotate(view, 2.0*M_PI*(float)orlon/(float)g_orlons, 0, 1, 0);
#endif

		float orlats = 16;
		float orlons = 16;
		float orrolls = 16;

		float orlat1 = 3+12;
		float orlon1 = 12+1-1-12;
		float orroll1 = 4+3;

		float orlat2 = 8-6;
		float orlon2 = 11-1+1+4;
		float orroll2 = 14;

		orroll1 = orroll2-7;

		float latrad1 = M_PI*(float)orlat1/(float)orlats-M_PI/2.0f;
		float lonrad1 = 2.0*M_PI*(float)orlon1/(float)orlons-M_PI/2.0f;
		float rollrad1 = 1.0*M_PI*(float)orroll1/(float)orrolls;

		float latrad2 = M_PI*(float)orlat2/(float)orlats-M_PI/2.0f;
		float lonrad2 = 2.0*M_PI*(float)orlon2/(float)orlons-M_PI/2.0f;
		float rollrad2 = 1.0*M_PI*(float)orroll2/(float)orrolls;

		//lonrad1 = 0;
		//lonrad2 = 0;

		//viewang1 = Rotate(viewang1, latrad1, 1, 0, 0);
		//viewang2 = Rotate(viewang2, latrad2, 1, 0, 0);

		//viewang1 = Rotate(viewang1, lonrad1, 0, 1, 0);
		//viewang2 = Rotate(viewang2, lonrad2, 0, 1, 0);

		viewang1 = SetLatLonRoll(viewang1, orlat1/orlats, orlon1/orlons, orroll1/orrolls);
		viewside1 = SetLatLonRoll(viewside1, orlat1/orlats, orlon1/orlons, orroll1/orrolls);
		viewang2 = SetLatLonRoll(viewang2, orlat2/orlats, orlon2/orlons, orroll2/orrolls);
		viewside2 = SetLatLonRoll(viewside2, orlat2/orlats, orlon2/orlons, orroll2/orrolls);

		
		//viewang1 = SetLatLon(viewang1, orlat1/orlats, orlon1/orlons);
		//viewside1 = SetLatLon(viewside1, orlat1/orlats, orlon1/orlons);
		//viewang2 = SetLatLon(viewang2, orlat2/orlats, orlon2/orlons);
		//viewside2 = SetLatLon(viewside2, orlat2/orlats, orlon2/orlons);

		//float getorlon1 = atan2(viewang1.x, viewang1.z) / (2.0 * M_PI);
		//float getorlon2 = atan2(viewang2.x, viewang2.z) / (2.0 * M_PI);
		
		float getorlon1 = GetLon(viewang1.x, viewang1.z);
		float getorlat1 = GetLat(viewang1.y);
		float getorroll1 = GetRoll(viewang1, viewside1);
		
		float getorlon2 = GetLon(viewang2.x, viewang2.z);
		float getorlat2 = GetLat(viewang2.y);
		float getorroll2 = GetRoll(viewang2, viewside2);

		//getyawreturn RADTODEG( atan2(dx, dz) );
		//float getorlon1 = 0.0 + (0.25 + atan2(viewang1.x, viewang1.z) / (2.0 * M_PI));
		//float getorlon1 = 0.5 + atan2(viewang1.x, viewang1.z) / (1.0 * M_PI);
		//float getorlon1 = 0.25 + atan2(viewang1.x, viewang1.z) / (2.0 * M_PI);
		//float getorlat1 = 0.5 - asin(viewang1.y)/M_PI;
		//viewang1 = Rotate(viewang1, - M_PI * 1.0 * getorlat1, 1, 0, 0);
		//float getorlon1 = atan2(viewang1.x, viewang1.z) / (1.0 * M_PI);
		
		//float getorlon2 = 0.0 + (0.25 + atan2(viewang2.x, viewang2.z) / (2.0 * M_PI));
		//float getorlon2 = 0.5 + atan2(viewang2.x, viewang2.z) / (1.0 * M_PI);
		//float getorlon2 = 0.25 + atan2(viewang2.x, viewang2.z) / (2.0 * M_PI);
		//float getorlat2 = 0.5 - asin(viewang2.y)/M_PI;
		//viewang2 = Rotate(viewang2, - M_PI * 1.0 * getorlat1, 1, 0, 0);
		//float getorlon2 = atan2(viewang2.x, viewang2.z) / (1.0 * M_PI);

//		if(getorlon1 < 0)
//			getorlon1 = getorlon1 + 1.0;
//		if(getorlon2 < 0)
//			getorlon2 = getorlon2 + 1.0;

		//float yaw = 0.5f + atan2(wrappos.z, wrappos.x) / (2.0f*M_PI);
		//if(ISNAN(orlon))
		//	ErrMess("asdsdg","nanyaw");
		//tan(0)=op/adj=0/1
		//fprintf(g_applog, "prepos1 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
		//viewang1 = Rotate(viewang1, -orlon1, 0, 1, 0);
		//viewang2 = Rotate(viewang2, -orlon2, 0, 1, 0);
		//fprintf(g_applog, "prepos2 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
		//float getorlat1 = atan2(viewang1.y, viewang1.x) / M_PI;
		//float getorlat2 = atan2(viewang2.y, viewang2.x) / M_PI;

		fprintf(g_applog, "\n\r test1: \r\n");
		fprintf(g_applog, "\r\n");

#if 01
		fprintf(g_applog, "start lon #1: ratio:%f (denom:%f) rad:%f \r\n",
			orlon1/orlons, orlons, lonrad1);
		fprintf(g_applog, "get lon #1: ratio:%f \r\n",
			getorlon1);

#endif

		fprintf(g_applog, "\r\n");

#if 01
		fprintf(g_applog, "start lat #1: ratio:%f (denom:%f) rad:%f \r\n",
			orlat1/orlats, orlats, latrad1);
		fprintf(g_applog, "get lat #1: ratio:%f \r\n",
			getorlat1);
#endif

		fprintf(g_applog, "\r\n");

#if 01//////////////////////////////////
		fprintf(g_applog, "start roll #1: ratio:%f (denom:%f) rad:%f \r\n",
			orroll1/orrolls, orrolls, rollrad1);
		fprintf(g_applog, "get roll #1: ratio:%f \r\n",
			getorroll1);
#endif////////////////////////////////////

		fprintf(g_applog, "\r\n");

#if 10
		fprintf(g_applog, "start lon #2: ratio:%f (denom:%f) rad:%f \r\n",
			orlon2/orlons, orlons, lonrad2);
		fprintf(g_applog, "get lon #2: ratio:%f \r\n",
			getorlon2);
#endif	
		fprintf(g_applog, "\r\n");

#if 01
		fprintf(g_applog, "start lat #2: ratio:%f (denom:%f) rad:%f \r\n",
			orlat2/orlats, orlats, latrad2);
		fprintf(g_applog, "get lat #2: ratio:%f \r\n",
			getorlat2);
#endif
		fprintf(g_applog, "\r\n");

#if 01
		fprintf(g_applog, "start roll #2: ratio:%f (denom:%f) rad:%f \r\n",
			orroll2/orrolls, orrolls, rollrad2);
		fprintf(g_applog, "get roll #2: ratio:%f \r\n",
			getorroll2);
#endif

		fprintf(g_applog, "\r\n");

		fflush(g_applog);
	}
	exit(0);
#endif

	//ratios
	///float orlon = 0.5 + atan2(objview.z, objview.x) / (2.0 * M_PI);
	///float orlat = 0.5 - asin(objview.y)/M_PI;
	//float orlon = atan2(objview.x, objview.z) / (2.0 * M_PI);
	
	//float orlon = atan2(objview.z, objview.x) / (2.0 * M_PI);
	//float orlat = - asin(objview.y)/M_PI;

	//if(orlon < 0)
	//	orlon = orlon + 1.0;

	//float yaw = 0.5f + atan2(wrappos.z, wrappos.x) / (2.0f*M_PI);
	//if(ISNAN(orlon))
	//	ErrMess("asdsdg","nanyaw");
	//tan(0)=op/adj=0/1
	//fprintf(g_applog, "prepos1 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
	//objview = Rotate(objview, -orlon, 0, 1, 0);
	//fprintf(g_applog, "prepos2 %f,%f,%f\r\n", wrappos.x, wrappos.y, wrappos.z);
	//float orlat = atan2(objview.y, objview.x) / M_PI;
	//float lat = 0.5f - asin(wrappos.y)/M_PI;

	//if(orlat < 0)
	//	orlat = orlat + 1;

	//float orlon = 1.0 - (0.25 - atan2(objview.x, objview.z) / (2.0 * M_PI));
	//float orlat = 0.5 + asin(objview.y)/M_PI;
	
	//float orlon = 1.0 - ( - atan2(objview.x, objview.z) / (2.0 * M_PI));
	//float orlat = asin(objview.y)/M_PI;

	//if(orlon < 0)
	//	orlon = orlon + 1;

	//if(orlat < 0)
	//	orlat = orlat + 1;

	float orlon = GetLon(objview.x, objview.z);
	float orlat = GetLat(objview.y);
	float orroll = GetRoll(objview, objside);

	//orlon = orlat = 0;
	
	glUniform1f(s->m_slot[SSLOT_ORJLON], (float)orlon);
	glUniform1f(s->m_slot[SSLOT_ORJLAT], (float)orlat);
	glUniform1f(s->m_slot[SSLOT_ORJROLL], (float)orroll);


	//in image, top-left is origin.
	//in SpEd, space coords going up is increasing y coord value,
	//which means updir in space is reverse of texture y coord increase.
	//so use -1 for movement up along texture
	updir = ( a - d );
	sidedir = ( c - d );
	float uplen = Magnitude( updir );
	float sidelen = Magnitude( sidedir );
	updir = ( updir ) / uplen;
	sidedir = ( sidedir ) / sidelen;
	viewdir = Cross(  updir, sidedir );
	viewdir = Normalize( viewdir );

	
	//jump table index coords

	//the jump depends on 1.) object orientation and 2.) viewing angle.
	//so the overall viewed angle is what's needed for the jump,
	//but only the object's orientation angles are passed (orjlon,orjlat).

	//incident angles
	//the combined angles of the camera and object rotations.
	//set up a view and side vector,
	//rotate by the object's rotations,
	//then rotate that view and side vector by the reverse of the cam's rotations.
	Vec3f incview = Vec3f(0,0,1);
	Vec3f incside = Vec3f(1,0,0);

	incview = SetLatLonRoll(incview, orlat, orlon, orroll);
	incside = SetLatLonRoll(incside, orlat, orlon, orroll);

	float camlat = GetLat(viewdir.y);
	float camlon = GetLon(viewdir.x, viewdir.z);
	float camroll = GetRoll(viewdir, sidedir);

	//RevSet...?
	incview = SetLatLonRoll(incview, 1-camlat, 1-camlon, 1-camroll);
	incside = SetLatLonRoll(incside, 1-camlat, 1-camlon, 1-camroll);
	//incview = RevSetLatLonRoll(incview, camlat, camlon, camroll);
	//incside = RevSetLatLonRoll(incside, camlat, camlon, camroll);

	float inclat = GetLat(incview.y);
	float inclon = GetLon(incview.x, incview.z);
	float incroll = GetRoll(incview, incside);

	glUniform3fv(s->m_slot[SSLOT_UPDIR], 1, (float*)&updir);
	glUniform3fv(s->m_slot[SSLOT_SIDEDIR], 1, (float*)&sidedir);
	glUniform3fv(s->m_slot[SSLOT_VIEWDIR], 1, (float*)&viewdir);
	glUniform1f(s->m_slot[SSLOT_UPLEN], (float)uplen);
	glUniform1f(s->m_slot[SSLOT_SIDELEN], (float)sidelen);
	glUniform1f(s->m_slot[SSLOT_CAMLAT], (float)camlat);
	glUniform1f(s->m_slot[SSLOT_CAMLON], (float)camlon);
	glUniform1f(s->m_slot[SSLOT_CAMROLL], (float)camroll);
	glUniform1f(s->m_slot[SSLOT_INCIDLAT], (float)inclat);
	glUniform1f(s->m_slot[SSLOT_INCIDLON], (float)inclon);
	glUniform1f(s->m_slot[SSLOT_INCIDROLL], (float)incroll);

	Vec2f ta, tb, tc, td;
	ta = Vec2f(0,1);
	tb = Vec2f(1,1);
	tc = Vec2f(1,0);
	td = Vec2f(0,0);

	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = a;
	v[4] = c;
	v[5] = d;

	t[0] = ta;
	t[1] = tb;
	t[2] = tc;
	t[3] = ta;
	t[4] = tc;
	t[5] = td;


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ o->difftexi ].texname);
	//glBindTexture(GL_TEXTURE_2D, g_texture[ 0 ].texname);
	glUniform1i(s->m_slot[SSLOT_TEXTURE0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ o->jumptexi ].texname);
	glUniform1i(s->m_slot[SSLOT_JUMPTEX], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ o->postexi[0] ].texname);
	glUniform1i(s->m_slot[SSLOT_POSXTEX], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ o->postexi[1] ].texname);
	glUniform1i(s->m_slot[SSLOT_POSYTEX], 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_texture[ o->postexi[2] ].texname);
	glUniform1i(s->m_slot[SSLOT_POSZTEX], 4);

	glVertexPointer(3, GL_FLOAT, 0, v);
	glTexCoordPointer(2, GL_FLOAT, 0, t);

	glDrawArrays(GL_TRIANGLES, 0, 6);

#if 0
	
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_diffusem ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_specularm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_normalm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, g_texture[ mat->m_ownerm ].texname);
			glUniform1i(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);

		//glVertexPointer(3, GL_FLOAT, 0, &Positions[basevertex]);
		glVertexPointer(3, GL_FLOAT, 0, &TransformedPos[basevertex]);
		glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[basevertex]);
		//glNormalPointer(GL_FLOAT, 0, &Normals[basevertex]);
		glNormalPointer(GL_FLOAT, 0, &TransformedNorm[basevertex]);
		//glIndexPointer(GL_UNSIGNED_INT, 0, &Indices[baseindex]);
		//glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
		CHECKGLERROR();
#endif
		//glDrawArrays(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, 0, numindices);
		//glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
		glDrawRangeElements(GL_TRIANGLES, 0, numunique, numindices, GL_UNSIGNED_INT, &Indices[baseindex]);
#endif


}

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
	char diffpath2[SPE_MAX_PATH+1];
	NameRender(diffpath, -1);
	strcat(diffpath, "_diff.png");
	MakeRel(diffpath, diffpath2);
	CreateTex(or->difftexi, diffpath2, false, false);
	
#if 0
	unsigned int difftexi;
	unsigned int postexi[3];
	unsigned int jumptexi;
	float maxrad;
#endif

	char posxpath[SPE_MAX_PATH+1];
	char posxpath2[SPE_MAX_PATH+1];
	NameRender(posxpath, -1);
	strcat(posxpath, "_posx.png");
	MakeRel(posxpath, posxpath2);
	CreateTex(or->postexi[0], posxpath2, false, false);

	char posypath[SPE_MAX_PATH+1];
	char posypath2[SPE_MAX_PATH+1];
	NameRender(posypath, -1);
	strcat(posypath, "_posy.png");
	MakeRel(posypath, posypath2);
	CreateTex(or->postexi[1], posypath2, false, false);

	char poszpath[SPE_MAX_PATH+1];
	char poszpath2[SPE_MAX_PATH+1];
	NameRender(poszpath, -1);
	strcat(poszpath, "_posz.png");
	MakeRel(poszpath, poszpath2);
	CreateTex(or->postexi[2], poszpath2, false, false);

	char jumppath[SPE_MAX_PATH+1];
	char jumppath2[SPE_MAX_PATH+1];
	NameRender(jumppath, -1);
	strcat(jumppath, "_isle.png");
	MakeRel(jumppath, jumppath2);
	CreateTex(or->jumptexi, jumppath2, false, false);

	return true;
}

void ResetOrRender()
{
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
	g_orlons = 16;	//orientability map longitude slices (rotations)
	g_orlats = 8;	//orientability map latitude slices (rotations)
	g_orrolls = 8;	//orientability map roll slices (rotations)
	g_bigtex = 4096;	//orientability diffuse colors and surface positions map size
}

void EndCombos()
{
	g_mode = EDITOR;
	g_gui.hideall();
	//g_gui.show();
	g_gui.show("editor");
	g_renderframe = 0;
	g_lightPos = g_origlightpos;
	//g_cam = g_origcam;
	//CallResize(g_origwidth, g_origheight);
	//ResetView(false);
	SortEdB(&g_edmap, g_cam.m_view, g_cam.m_pos);
	//g_gui.reframe();
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

	ResetOrRender();

	strcpy(g_renderbasename, fullpath);
	//StripExt(g_renderbasename);
	char *pos = strstr(g_renderbasename, "_list2.txt");
	*pos = 0;	//strip end

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
	g_orlats = 8;	//orientability map latitude slices
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
			else if(strcmp(a, "orjrolls") == 0)
			{
				sscanf(b, "%d", &g_orrolls);
			}
			else if(strcmp(a, "ormapsz") == 0)
			{
				sscanf(b, "%d", &g_bigtex);
			}
		}
	}

	fclose(infofp);

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
		imax(0,g_renderframes-1), imax(0,INCLINES-1), 
		imax(0,g_nrendsides-1), imax(0,g_nrendsides-1), imax(0,g_nrendsides-1),
		0, 0)+1;
	//max combos ci

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
	g_mode = ORVIEW;	//necessary so message "Done rendering" doesn't show up at EndRender();

	do
	{
		LoadOr1();
	}while(AdvRender());
	//EndRender();	//can't use because calls resize with g_origwidth/height
	EndCombos();
	//return true;
	ResetOrRender();	//reset frame number etc.
	LoadConfig();	//reload settings

	GUI* gui = &g_gui;
	g_mode = EDITOR;
	gui->hideall();
	gui->show("editor");
	return true;
}

void ViewTopo(const char* fullpath)
{
	FreeEdMap(&g_edmap);

	LoadOr(fullpath);

	Vec3f up = Vec3f(0,1,0);
	Vec3f side = Vec3f(1,0,0);
	Vec3f view = Vec3f(0,0,-1);
#if 0

	up = Rotate(up, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);
	side = Rotate(side, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);
	view = Rotate(view, M_PI*(float)orlat/(float)g_orlats-M_PI/2.0f, 1, 0, 0);

	up = Rotate(up, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);
	side = Rotate(side, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);
	view = Rotate(view, M_PI*(float)orlon/(float)g_orlons-M_PI/2.0f, 0, 1, 0);
#endif

	Vec3f pos = Vec3f(0,0,0) - view * 30000;

	g_cam.position( pos.x, pos.y, pos.z, view.x, view.y, view.z, up.x, up.y, up.z);

	g_rolllock = true;

	return;
	//g_mode = ORVIEW;

	//SkipLogo();
	//GUI* gui = &g_gui;
	//gui->hideall();
	//gui->show("render");
	GUI* gui = &g_gui;
	g_mode = EDITOR;
	gui->hideall();
	gui->show("editor");
}

void FreeOrList(OrList *ol)
{
	ol->free();
}