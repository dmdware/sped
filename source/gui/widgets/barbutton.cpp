


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



#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "../gui.h"

#include "../../debug.h"

BarButton::BarButton(Widget* parent, unsigned int sprite, float bar, void (*reframef)(Widget* w), void (*click)(), void (*overf)(), void (*out)()) : Button()
{
	m_parent = parent;
	m_type = WIDGET_BARBUTTON;
	m_over = false;
	m_ldown = false;
	m_tex = sprite;
	CreateTex(m_bgtex, "gui/buttonbg.png", true, false);
	CreateTex(m_bgovertex, "gui/buttonbgover.png", true, false);
	reframefunc = reframef;
	m_healthbar = bar;
	clickfunc = click;
	overfunc = overf;
	outfunc = out;
	reframe();
}

void BarButton::draw()
{
	if(m_over)
		DrawImage(g_texture[m_bgovertex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);
	else
		DrawImage(g_texture[m_bgtex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

	DrawImage(g_texture[m_tex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

	

	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_currw);
	glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_currh);
	DrawSquare(1, 0, 0, 1, m_pos[0], m_pos[3]-5, m_pos[2], m_pos[3], m_crop);
	float bar = (m_pos[2] - m_pos[0]) * m_healthbar;
	DrawSquare(0, 1, 0, 1, m_pos[0], m_pos[3]-5, m_pos[0]+bar, m_pos[3], m_crop);

	EndS();
	CHECKGLERROR();
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

