
/*
#include "water.h"
#include "heightmap.h"
#include "../shader.h"
#include "../math/math3d.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../platform.h"
#include "../shadow.h"
#include "../texture.h"
#include "../utils.h"

unsigned int g_watertex[WATER_TEXS];

void LoadWater()
{
	CreateTextureI(g_watertex[WATER_TEX_GRADIENT], "terrain/default/water.gradient.png", false);
	CreateTextureI(g_watertex[WATER_TEX_DETAIL], "terrain/default/water.detail.jpg", false);
	CreateTextureI(g_watertex[WATER_TEX_SPECULAR], "terrain/default/water.spec.jpg", false);
	CreateTextureI(g_watertex[WATER_TEX_NORMAL], "terrain/default/water.norm.jpg", false);
}

void DrawWater()
{			
	Shader* s = &g_shader[g_curS];
	
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_GRADIENT] ].tex);
	glUniform1iARB(s->m_slot[SSLOT_GRADIENTTEX], 0);

	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_DETAIL] ].tex);
	glUniform1iARB(s->m_slot[SSLOT_DETAILTEX], 1);

	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_SPECULAR] ].tex);
	glUniform1iARB(s->m_slot[SSLOT_SPECULARMAP], 2);

	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_watertex[WATER_TEX_NORMAL] ].tex);
	glUniform1iARB(s->m_slot[SSLOT_NORMALMAP], 3);
	
	Vec3f a, b, c, d;

	int wx = g_hmap.m_widthX;
	int wz = g_hmap.m_widthZ;

	a = Vec3f(wx * g_tilesize, WATER_LEVEL, wz * g_tilesize);
	b = Vec3f(0, WATER_LEVEL, wz * g_tilesize);
	c = Vec3f(0, WATER_LEVEL, 0);
	d = Vec3f(wx * g_tilesize, WATER_LEVEL, 0);

	float vertices[] =
	{
		a.x, a.y, a.z,
		b.x, b.y, b.z,
		c.x, c.y, c.z,
		d.x, d.y, d.z,
		a.x, a.y, a.z,
		c.x, c.y, c.z
	};

	float texcoords0[] =
	{
		wx+1, wz+1,
		0, wz+1,
		0, 0,
		wx+1, 0,
		wx+1, wz+1,
		0, 0
	};

	const float normals[] =
	{
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0,
		0, 1, 0
	};

	glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, texcoords0);
	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, normals);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}*/