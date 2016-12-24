







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


TouchListener::TouchListener() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_TOUCHLISTENER;
	m_over = false;
	m_ldown = false;
	reframefunc = NULL;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = NULL;
	m_param = -1;
	m_name = "";
}

TouchListener::TouchListener(Widget* parent, const char* name, void (*reframef)(Widget* w), void (*click2)(int p), void (*overf2)(int p), void (*out)(), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_TOUCHLISTENER;
	m_over = false;
	m_ldown = false;
	reframefunc = reframef;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = overf2;
	outfunc = out;
	m_param = parm;
	m_name = name;
	reframe();
}

void TouchListener::inev(InEv* ie)
{
	

	if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(m_over && m_ldown)
		{
			if(clickfunc != NULL)
				clickfunc();

			if(clickfunc2 != NULL)
				clickfunc2(m_param);

			m_over = false;
			m_ldown = false;

			ie->intercepted = true;
			return;	// intercept mouse event
		}

		if(m_ldown)
		{
			m_ldown = false;
			ie->intercepted = true;
			return;
		}

		m_over = false;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(m_over)
		{
			m_ldown = true;
			ie->intercepted = true;
			return;	// intercept mouse event
		}
	}
	else if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
		{
			if(overfunc != NULL)
				overfunc();
			if(overfunc2 != NULL)
				overfunc2(m_param);

			m_over = true;

			ie->intercepted = true;
			return;
		}
		else
		{
			if(m_over && outfunc != NULL)
				outfunc();

			m_over = false;
		}
	}
}
