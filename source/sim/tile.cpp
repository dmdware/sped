
#include "tile.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../texture.h"
#include "../render/shadow.h"

VertexArray g_tileva[INCLINES];
//int g_currincline = INC_1011;
int g_currincline = 0;
unsigned int g_tiletexs[TEX_TYPES] = {0,0,0,0};
int g_tilesize = 10 * 100;	//10 meters
bool g_cornerinc[INCLINES][4];	//corners in order of digits displayed on name, not in clock-wise corner order
float g_tilerisecm = (tan(DEGTORAD(30))*TILE_DIAG/4);

void MakeTiles()
{
	for(int i=0; i<INCLINES; i++)
	{
		g_tileva[i].alloc(6);
	}

#if 1
	Vec3f low_n = Vec3f(-g_tilesize/2, 0, -g_tilesize/2);
	Vec3f low_s = Vec3f(g_tilesize/2, 0, g_tilesize/2);
	Vec3f low_e = Vec3f(g_tilesize/2, 0, -g_tilesize/2);
	Vec3f low_w = Vec3f(-g_tilesize/2, 0, g_tilesize/2);
	Vec3f high_n = Vec3f(-g_tilesize/2, TILE_RISE, -g_tilesize/2);
	Vec3f high_s = Vec3f(g_tilesize/2, TILE_RISE, g_tilesize/2);
	Vec3f high_e = Vec3f(g_tilesize/2, TILE_RISE, -g_tilesize/2);
	Vec3f high_w = Vec3f(-g_tilesize/2, TILE_RISE, g_tilesize/2);
#else
	Vec3f low_n = Vec3f(-g_tilesize/1.99, 0, -g_tilesize/1.99);
	Vec3f low_s = Vec3f(g_tilesize/1.99, 0, g_tilesize/1.99);
	Vec3f low_e = Vec3f(g_tilesize/1.99, 0, -g_tilesize/1.99);
	Vec3f low_w = Vec3f(-g_tilesize/1.99, 0, g_tilesize/1.99);
	Vec3f high_n = Vec3f(-g_tilesize/1.99, TILE_RISE, -g_tilesize/1.99);
	Vec3f high_s = Vec3f(g_tilesize/1.99, TILE_RISE, g_tilesize/1.99);
	Vec3f high_e = Vec3f(g_tilesize/1.99, TILE_RISE, -g_tilesize/1.99);
	Vec3f high_w = Vec3f(-g_tilesize/1.99, TILE_RISE, g_tilesize/1.99);
#endif

	bool cornerinc[4] = {0,0,0,0};

	for(int i=0; i<INCLINES; i++)
	{
		/*
		Try two triangle arrangements
		and choose the one where both
		points that meet between the two
		triangles are on the same elevation
		level.
		This is necessary to create smooth terrain slopes.
		In the two cases that either arrangement
		results in the two pairs of joining points
		being on the same level, choose the one where
		the joining points are on the low elevation.
		*/

		Vec3f arrange[2][6];

		arrange[0][0] = cornerinc[0] ? high_n : low_n;
		arrange[0][1] = cornerinc[1] ? high_e : low_e;
		arrange[0][2] = cornerinc[3] ? high_w : low_w;
		arrange[0][3] = cornerinc[1] ? high_e : low_e;
		arrange[0][4] = cornerinc[2] ? high_s : low_s;
		arrange[0][5] = cornerinc[3] ? high_w : low_w;

		arrange[1][0] = cornerinc[0] ? high_n : low_n;
		arrange[1][1] = cornerinc[1] ? high_e : low_e;
		arrange[1][2] = cornerinc[2] ? high_s : low_s;
		arrange[1][3] = cornerinc[0] ? high_n : low_n;
		arrange[1][4] = cornerinc[2] ? high_s : low_s;
		arrange[1][5] = cornerinc[3] ? high_w : low_w;

		// the two pairs of joining points
		// indices: arragement, pair
		Vec3f join[2][2];

		join[0][0] = arrange[0][1];
		join[0][1] = arrange[0][2];

		join[1][0] = arrange[1][0];
		join[1][1] = arrange[1][2];

		//now check if both pairs are on the same elevation in both arrangements
		bool samelevel[2] = {false,false};

		for(int j=0; j<2; j++)
			if(join[j][0].y == join[j][1].y)
			//if(fabs(join[j][0].y - join[j][1].y) < TILE_RISE/10)
				samelevel[j] = true;

		int usearr = 0;

		//if both arrangements have joining pairs on same level,
		//use the arrangement with the pair on the low elevation
		if(samelevel[0] && samelevel[1])
		{
			if(join[0][0].y < join[1][0].y)
				usearr = 0;
			else
				usearr = 1;
		}
		else if(samelevel[0])
			usearr = 0;
		else if(samelevel[1])
			usearr = 1;

		//USE ONLY ONE TRIANGLE CONFIGURATION
		//This must match that of Heightmap::accheight, or else it will have different depths and models and tile terrain will interpenetrate
		usearr = 1;

		VertexArray* va = &g_tileva[i];
		for(int j=0; j<6; j++)
			va->vertices[j] = arrange[usearr][j];

		//Record for use in bilerping vertex heights between the tile corners
		for(int corneri=0; corneri<4; corneri++)
		{
			g_cornerinc[i][corneri] = cornerinc[corneri];
		}

		//increment to the next incline combo,
		//counting the digits up from the right side to the left
		//(0000 -> 0001 -> 0010 -> ...).
		int corner = 3;
		while(corner >= 0)
		{
			if(!cornerinc[corner])
			{
				cornerinc[corner] = 1;
				break;
			}

			cornerinc[corner] = 0;
			corner--;
		}
	}

	for(int i=0; i<INCLINES; i++)
	{
		VertexArray* va = &g_tileva[i];
		for(int j=0; j<6; j++)
		{
			Vec3f v = va->vertices[j];
			va->texcoords[j] = Vec2f(v.x / (float)g_tilesize + 0.5f, v.z / (float)g_tilesize + 0.5f);
		}

		Vec3f tri1[3];
		Vec3f tri2[3];

		tri1[0] = va->vertices[0];
		tri1[1] = va->vertices[1];
		tri1[2] = va->vertices[2];
		tri2[0] = va->vertices[3];
		tri2[1] = va->vertices[4];
		tri2[2] = va->vertices[5];

		Vec3f norm1 = Normal(tri1);
		Vec3f norm2 = Normal(tri2);

		va->normals[0] = norm1;
		va->normals[1] = norm1;
		va->normals[2] = norm1;
		va->normals[3] = norm2;
		va->normals[4] = norm2;
		va->normals[5] = norm2;
	}
}

void DrawTile()
{
	if(g_tiletexs[TEX_DIFF] == 0)
		return;

	Shader* s = &g_shader[g_curS];

	if(g_curS != SHADER_DEPTH)
	{
		Matrix modelmat;
		Matrix mvp;
		mvp.set(g_camproj.m_matrix);
		mvp.postmult(g_camview);
		//mvp.postmult(modelmat);
		glUniformMatrix4fv(s->m_slot[SSLOT_MVP], 1, 0, mvp.m_matrix);
	}

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_tiletexs[TEX_DIFF]].texname);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);

	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_tiletexs[TEX_SPEC]].texname);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);

	glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_tiletexs[TEX_NORM]].texname);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);

	glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, m_diffusem);
	glBindTexture(GL_TEXTURE_2D, g_texture[g_tiletexs[TEX_TEAM]].texname);
	glUniform1i(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);

	VertexArray* va = &g_tileva[g_currincline];

	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);

	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	//glDrawArrays(GL_LINE_LOOP, 0, va->numverts);

	glVertexPointer(3, GL_FLOAT, 0, va->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
	glNormalPointer(GL_FLOAT, 0, va->normals);
	//glDrawArrays(GL_POINTS, 0, va->numverts);
}
