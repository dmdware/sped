

#include "entitytype.h"

EntityT g_entityT[ENTITY_TYPES];

EntityT::EntityT()
{
	model = -1;
	collider = -1;
	item = -1;
}

EntityT::EntityT(int collider, const char* model, Vec3f modelScale, Vec3f modelOffset, Vec3f vMin, Vec3f vMax, float maxStep, float speed, float jump, float crouch, float animRate, int item)
{
	this->collider = collider;
	//QueueModel2(&this->model, model, modelScale, modelOffset);
	this->vMin = vMin;
	this->vMax = vMax;
	this->maxStep = maxStep;
	this->speed = speed;
	this->jump = jump;
	this->crouch = crouch;
	this->animRate = animRate;
	this->item = item;
}
