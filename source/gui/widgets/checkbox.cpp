







#include "../widget.h"
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


CheckBox::CheckBox() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_CHECKBOX;
	m_name = "";
	m_text = "";
	m_font = MAINFONT16;
	reframefunc = NULL;
	m_ldown = false;
	m_rgba[0] = 1;
	m_rgba[1] = 1;
	m_rgba[2] = 1;
	m_rgba[3] = 1;
	m_selected = 0;
	changefunc = NULL;
	CreateTex(m_frametex, "gui/frame.jpg", true, false);
	CreateTex(m_filledtex, "gui/accept.png", true, false);
	reframe();
}

CheckBox::CheckBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), int sel, float r, float g, float b, float a, void (*change)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_CHECKBOX;
	m_name = n;
	m_text = t;
	m_font = f;
	reframefunc = reframef;
	m_ldown = false;
	m_rgba[0] = r;
	m_rgba[1] = g;
	m_rgba[2] = b;
	m_rgba[3] = a;
	m_selected = sel;
	changefunc = change;
	CreateTex(m_frametex, "gui/frame.jpg", true, false);
	CreateTex(m_filledtex, "gui/accept.png", true, false);
	reframe();
}

int CheckBox::square()
{
	return (int)g_font[m_font].gheight;
}

void CheckBox::draw()
{
	DrawImage(g_texture[m_frametex].texname,  m_pos[0], m_pos[1], m_pos[0]+square(), m_pos[1]+square(), 0,0,1,1, m_crop);

	if(m_selected > 0)
		DrawImage(g_texture[m_filledtex].texname, m_pos[0], m_pos[1], m_pos[0]+square(), m_pos[1]+square(), 0,0,1,1, m_crop);

	DrawShadowedText(m_font, m_pos[0]+square()+5, m_pos[1], &m_text);
}

void CheckBox::inev(InEv* ie)
{
	

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] &&
		                g_mouse.x <= m_pos[2] &&
		                g_mouse.y <= m_pos[3])
		{
			m_over = true;
			ie->intercepted = true;
			return;
		}
		else
		{
			m_over = false;
			return;
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(m_over)
		{
			m_ldown = true;
			ie->intercepted = true;
			return;	// intercept mouse event
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(m_over && m_ldown)
		{
			if(m_selected <= 0)
				m_selected = 1;
			else
				m_selected  = 0;

			if(changefunc != NULL)
				changefunc();

			m_ldown = false;

			ie->intercepted = true;
			return;
		}

		m_ldown = false;
	}
}

