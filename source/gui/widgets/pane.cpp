







#include "../gui.h"
#include "../../texture.h"

#include "pane.h"
#include "../../debug.h"

Pane::Pane() : Widget()
{
	m_type = WIDGET_WINDOW;
	m_parent = NULL;
	m_opened = false;
}

void Pane::chcall(Widget* ch, int type, void* data)
{
	if(ch == &m_vscroll)
	{

	}
	//else if(ch == &hscroll)
	{
	}
}

Pane::Pane(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : Widget()
{
	m_type = WIDGET_WINDOW;
	reframefunc = reframef;
	m_parent = parent;
	m_name = n;
	m_opened = false;
	m_scroll[0] = 0;
	m_scroll[1] = 0;

	const float alpha = 0.9f;

	CHECKGLERROR();

	m_vscroll = VScroll(this, "m_vscroll");

	// only call this when window is created
	if(reframefunc)
		reframefunc(this);
	else //default size and position
	{
		// to do
	}
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}

void Pane::reframe()
{
	//if(reframefunc)
	//	reframefunc(this);

	m_vscroll.m_pos[0] = m_pos[2] - 3 - 10;
	m_vscroll.m_pos[1] = m_pos[1] + 27;
	m_vscroll.m_pos[2] = m_pos[2] - 3;
	m_vscroll.m_pos[3] = m_pos[3] - 27 - 10;
	m_vscroll.reframe();

	for(std::list<Widget*>::iterator w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->reframe();
}

void Pane::draw()
{
	//m_vscroll.draw();

	Widget::draw();
}

void Pane::drawover()
{
	//m_vscroll.drawover();

	Widget::drawover();
}

void Pane::inev(InEv* ie)
{
	Widget::inev(ie);

	//m_vscroll.inev(ie);

	if(m_ldown)
	{
		if(ie->type == INEV_MOUSEMOVE ||
		                ( (ie->type == INEV_MOUSEDOWN || ie->type == INEV_MOUSEUP) && ie->key == MOUSE_LEFT) )
			ie->intercepted = true;

		if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
			m_ldown = false;

		if(ie->type == INEV_MOUSEMOVE)
		{
			int dx = g_mouse.x - m_mousedown[0];
			int dy = g_mouse.y - m_mousedown[1];
			m_mousedown[0] = g_mouse.x;
			m_mousedown[1] = g_mouse.y;
		}
	}

	if(m_over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			m_mousedown[0] = g_mouse.x;
			m_mousedown[1] = g_mouse.y;
			m_ldown = true;
			ie->intercepted = true;
			tofront();	//can't change list order, breaks iterators
		}
	}

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(m_ldown)
		{
			ie->intercepted = true;
			ie->curst = g_curst;
			return;
		}

		if(!ie->intercepted &&
		                g_mouse.x >= m_pos[0]-64 &&
		                g_mouse.y >= m_pos[1]-64 &&
		                g_mouse.x <= m_pos[2]+64 &&
		                g_mouse.y <= m_pos[3]+64)
		{
			m_over = true;

			if(g_mousekeys[MOUSE_MIDDLE])
				return;

			//ie->intercepted = true;
		}
		else
		{
			//cursor out of window area?
			if(!ie->intercepted)
			{
				if(m_over)
				{
					//ie->intercepted = true;
					//ie->curst = CU_DEFAULT;
				}
			}
			//event intercepted but cursor in window rectangle (maybe covered up by something else)?
			else
			{
				// to do: this will be replaced by code in other
				//widgets that will set the cursor
				//g_curst = CU_DEFAULT;
			}

			m_over = false;
		}
	}
}

void Pane::subframe(float* fr)
{
	memcpy(fr, m_pos, sizeof(float)*4);
}
