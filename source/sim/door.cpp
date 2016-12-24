
#include "../save/edmap.h"
#include "door.h"

EdDoor::EdDoor()
{
	m_nsides = 0;
	m_sides = NULL;
}

EdDoor::~EdDoor()
{
	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
		m_nsides = 0;
	}
}

EdDoor& EdDoor::operator=(const EdDoor& original)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	bool startopen;
	*/

	axis = original.axis;
	point = original.point;
	opendeg = original.opendeg;
	startopen = original.startopen;

	m_nsides = original.m_nsides;

	if(m_sides)
	{
		delete [] m_sides;
		m_sides = NULL;
	}

	m_sides = new BrushSide[m_nsides];
	for(int i=0; i<m_nsides; i++)
		m_sides[i] = original.m_sides[i];

	return *this;
}

EdDoor::EdDoor(const EdDoor& original)
{
	m_nsides = 0;
	m_sides = NULL;
	*this = original;
}