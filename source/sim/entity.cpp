

#include "entity.h"
#include "entitytype.h"

Entity* g_entity[ENTITIES];

void DestroyEntities()
{
	for(int i=0; i<ENTITIES; i++)
	{
		if(g_entity[i])
		{
			delete g_entity[i];
			g_entity[i] = NULL;
		}
	}
}

int NewEntity()
{
	for(int i=0; i<ENTITIES; i++)
	{
		if(!g_entity[i])
			return i;
	}

	return -1;
}

bool PlaceEntity(int type, int controller, float amount, float clip, Vec3f pos, float yaw, int* ID)
{
	int i = NewEntity();
	if(i < 0)
		return i;

	g_entity[i] = new Entity();
	Entity* e = g_entity[i];

	e->frame[BODY_LOWER] = 0;
	e->frame[BODY_UPPER] = 0;
	e->type = type;
	e->controller = controller;
	e->amount = amount;
	e->clip = clip;
#if 0
	e->state = STATE_NONE;
	e->nolightvol = nolightvol;
	e->script = script;
#endif

	Camera* c = &e->camera;

	c->position(pos.x, pos.y, pos.z, pos.x+1, pos.y, pos.z, 0, 1, 0);
	c->rotateview(DEGTORAD(yaw), 0, 1, 0);

#if 0
	EntityT* t = &g_entityT[type];
	e->cluster = g_map.FindCluster(c->Position() + t->centerOff);

	Player* p = NULL;
	if(controller >= 0)
	{
		g_player[controller].entity = i;
		p = &g_player[controller];
	}

	if(ID)
		(*ID) = i;

	if(controller >= 0)
		p->goal = pos;
#endif

	return true;
}

void DrawEntities()
{
	Entity* e = NULL;
	Camera* c = NULL;
	EntityT* t = NULL;
	Model2* m = NULL;

	//int localEntityIndex = -1;

	for(int i=0; i<ENTITIES; i++)
	{
		e = g_entity[i];

		if(!e)
			continue;

		c = &e->camera;
		t = &g_entityT[e->type];

#if 0
		//Don't draw if we're in first person view and this is the player
		if(e->controller >= 0)
			p = &g_player[e->controller];

		if(i == localE && g_viewmode == FIRSTPERSON)
			continue;
#endif

#if 0
		foundt = false;

		if(t->model[BODY_LOWER] >= 0)
		{
			m = &g_model[t->model[BODY_LOWER]];
			if(m->transp)
				foundt = true;
		}

		if(t->model[BODY_UPPER] >= 0)
		{
			m = &g_model[t->model[BODY_UPPER]];
			if(m->transp)
				foundt = true;
		}

		if(foundt != transp)
			continue;

		if(e->nolightvol)
			colorv = CVector3(1, 1, 1);
		else
			colorv = g_map.LightVol(c->Position() + t->vCenterOff);

		colorf[0] = colorv.x;
		colorf[1] = precolor[1] * colorv.y;
		colorf[2] = precolor[2] * colorv.z;

		glUniform4f(g_slots[MODEL][COLOR], colorf[0], colorf[1], colorf[2], colorf[3]);

		bounds[0] = c->Position() + t->vMin;
		bounds[1] = c->Position() + t->vMax;

		if(!g_frustum.BoxInFrustum(bounds[0].x, bounds[0].y, bounds[0].z, bounds[1].x, bounds[1].y, bounds[1].z))
			continue;
#endif

		if(t->model >= 0)
		{
			m = &g_model2[t->model];
			//m->draw(e->frame[BODY_LOWER], c->m_pos, c->m_orientv.y);
		}
#if 0
		if(t->model[BODY_UPPER] >= 0)
		{
			m = &g_model[t->model[BODY_UPPER]];
			m->draw2(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
		}
#endif
#if 0
		if(e->controller >= 0 && p->equipped >= 0)
		{
			h = &p->items[p->equipped];
			item = h->type;
			iT = &g_itemType[item];
			iM = &g_model[iT->model];
			iM->Draw2(e->frame[BODY_UPPER], c->Position(), -c->Pitch(), c->Yaw());
		}
#endif
	}
}
