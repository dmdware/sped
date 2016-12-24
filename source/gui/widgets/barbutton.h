


/*******************************************************
 * Copyright (C) 2015 DMD 'Ware <dmdware@gmail.com>
 * 
 * This file is part of 3D Sprite Renderer and Convex Hull Editor.
 * 
 * You are entitled to use this source code to learn.
 *
 * You are not entitled to duplicate or copy this source code 
 * into your own projects, commercial or personal, UNLESS you 
 * give credit.
 *
 *******************************************************/



#ifndef BARBUTTON_H
#define BARBUTTON_H

#include "../widget.h"
#include "button.h"

class BarButton : public Button
{
public:
	float m_healthbar;

	BarButton(Widget* parent, unsigned int sprite, float bar, void (*reframef)(Widget* w), void (*click)(), void (*overf)(), void (*out)());

	void draw();
};

#endif
