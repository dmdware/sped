







#include "vscrollbar.h"
#include "../../render/shader.h"
#include "../../debug.h"
#include "../gui.h"

VScroll::VScroll() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_VSCROLLBAR;
	m_name = "";
	m_opened = false;
	m_selected = 0;
	m_scroll[1] = 0;
	m_mousescroll = false;
	m_ldown = false;
	m_ldownbar = false;
	m_ldownup = false;
	m_ldowndown = false;
	m_domain = 1;
	CreateTex(m_uptex, "gui/up.jpg", true, false);
	changefunc = NULL;
	reframefunc = NULL;
}

VScroll::VScroll(Widget* parent, const char* n) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_VSCROLLBAR;
	m_name = n;
	m_opened = false;
	m_selected = 0;
	m_scroll[1] = 0;
	m_mousescroll = false;
	m_ldown = false;
	m_ldownbar = false;
	m_ldownup = false;
	m_ldowndown = false;
	m_domain = 0.5f;
	CreateTex(m_uptex, "gui/up.jpg", true, false);
	changefunc = NULL;
	reframefunc = NULL;
	//reframe();
}

void VScroll::reframe()
{
	Widget::reframe();

	if(m_parent)
	{
		float* parp = m_parent->m_pos;

		//must be bounded by the parent's frame

		m_pos[0] = fmax(parp[0], m_pos[0]);
		m_pos[0] = fmin(parp[2], m_pos[0]);
		m_pos[2] = fmax(parp[0], m_pos[2]);
		m_pos[2] = fmin(parp[2], m_pos[2]);
		m_pos[1] = fmax(parp[1], m_pos[1]);
		m_pos[1] = fmin(parp[3], m_pos[1]);
		m_pos[3] = fmax(parp[1], m_pos[3]);
		m_pos[3] = fmin(parp[3], m_pos[3]);

		m_pos[1] = fmin(m_pos[1], m_pos[3]);
		m_pos[0] = fmin(m_pos[0], m_pos[2]);
	}

	int w = (int)( m_pos[2]-m_pos[0] );

	m_uppos[0] = m_pos[0];
	m_uppos[1] = m_pos[1];
	m_uppos[2] = m_pos[2];
	m_uppos[3] = m_pos[1]+w;

	m_downpos[0] = m_pos[0];
	m_downpos[1] = m_pos[3]-w;
	m_downpos[2] = m_pos[2];
	m_downpos[3] = m_pos[3];

	m_barpos[0] = m_pos[0];
	m_barpos[1] = m_uppos[3] + (m_downpos[1]-m_uppos[3])*m_scroll[1];
	m_barpos[2] = m_pos[2];
	//m_barpos[3] = m_barpos[1] + fmax(w, m_domain*(m_downpos[1]-m_uppos[3]));
	m_barpos[3] = m_barpos[1] + m_domain*(m_downpos[1]-m_uppos[3]);

	if(m_parent)
	{
		float* parp = m_parent->m_pos;

		//must be bounded by the parent's frame

		m_uppos[0] = fmax(parp[0], m_uppos[0]);
		m_uppos[0] = fmin(parp[2], m_uppos[0]);
		m_uppos[2] = fmax(parp[0], m_uppos[2]);
		m_uppos[2] = fmin(parp[2], m_uppos[2]);
		m_uppos[1] = fmax(parp[1], m_uppos[1]);
		m_uppos[1] = fmin(parp[3], m_uppos[1]);
		m_uppos[3] = fmax(parp[1], m_uppos[3]);
		m_uppos[3] = fmin(parp[3], m_uppos[3]);

		m_downpos[0] = fmax(parp[0], m_downpos[0]);
		m_downpos[0] = fmin(parp[2], m_downpos[0]);
		m_downpos[2] = fmax(parp[0], m_downpos[2]);
		m_downpos[2] = fmin(parp[2], m_downpos[2]);
		m_downpos[1] = fmax(parp[1], m_downpos[1]);
		m_downpos[1] = fmin(parp[3], m_downpos[1]);
		m_downpos[3] = fmax(parp[1], m_downpos[3]);
		m_downpos[3] = fmin(parp[3], m_downpos[3]);

		//bar must be vertically between the two arrow buttons
		m_barpos[0] = fmax(parp[0], m_barpos[0]);
		m_barpos[0] = fmin(parp[2], m_barpos[0]);
		m_barpos[2] = fmax(parp[0], m_barpos[2]);
		m_barpos[2] = fmin(parp[2], m_barpos[2]);

		m_barpos[1] = fmax(parp[1], m_barpos[1]);
		m_barpos[1] = fmin(parp[3], m_barpos[1]);
		m_barpos[3] = fmax(m_pos[1], m_barpos[3]);
		m_barpos[3] = fmin(m_pos[3], m_barpos[3]);

		m_barpos[1] = fmax(m_barpos[1], m_uppos[3]);
		m_barpos[1] = fmin(m_barpos[1], m_downpos[1]);
		m_barpos[3] = fmax(m_barpos[3], m_uppos[3]);
		m_barpos[3] = fmin(m_barpos[3], m_downpos[1]);
	}
}

void VScroll::draw()
{
	glUniform4f(g_shader[g_curS].m_slot[SSLOT_COLOR], 1, 1, 1, 0.8f);
	DrawImage(g_texture[ m_uptex ].texname, m_uppos[0], m_uppos[1], m_uppos[2], m_uppos[3], 0, 0, 1, 1, m_crop);
	DrawImage(g_texture[ m_uptex ].texname, m_downpos[0], m_downpos[1], m_downpos[2], m_downpos[3], 0, 1, 1, 0, m_crop);

	EndS();

	UseS(SHADER_COLOR2D);
	glUniform1f(g_shader[g_curS].m_slot[SSLOT_WIDTH], (float)g_currw);
	glUniform1f(g_shader[g_curS].m_slot[SSLOT_HEIGHT], (float)g_currh);

	float midcolor[] = {0.7f,0.7f,0.7f,0.8f};
	float lightcolor[] = {0.8f,0.8f,0.8f,0.8f};
	float darkcolor[] = {0.5f,0.5f,0.5f,0.8f};

	if(m_over)
	{
		for(int i=0; i<3; i++)
		{
			midcolor[i] = 0.8f;
			lightcolor[i] = 0.9f;
			darkcolor[i] = 0.6f;
		}
	}

	DrawSquare(midcolor[0], midcolor[1], midcolor[2], midcolor[3], m_barpos[0], m_barpos[1], m_barpos[2], m_barpos[3], m_crop);

	DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_barpos[2], m_barpos[1], m_barpos[2], m_barpos[3]-1, m_crop);
	DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_barpos[0], m_barpos[1], m_barpos[2]-1, m_barpos[1], m_crop);

	DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_barpos[0]+1, m_barpos[3], m_barpos[2], m_barpos[3], m_crop);
	DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_barpos[2], m_barpos[1]+1, m_barpos[2], m_barpos[3], m_crop);

	EndS();
	CHECKGLERROR();
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

void VScroll::frameupd()
{
	if(m_ldownup)
	{
		float dy = -(float)g_drawfrinterval * m_domain;

		float origscroll = m_scroll[1];

		m_scroll[1] += (float)dy;

		int w = (int)( m_pos[2]-m_pos[0] );

		m_barpos[1] = m_uppos[3] + (m_downpos[1]-m_uppos[3])*m_scroll[1];
		//m_barpos[3] = m_barpos[1] + fmax(w, m_domain*(m_downpos[1]-m_uppos[3]));
		m_barpos[3] = m_barpos[1] + m_domain*(m_downpos[1]-m_uppos[3]);

		float overy = m_barpos[3] - m_downpos[1];

		if(overy > 0)
		{
			m_barpos[1] -= overy;
			m_barpos[3] -= overy;
			m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
		}

		float undery = m_uppos[3] - m_barpos[1];

		if(undery > 0)
		{
			m_barpos[1] += undery;
			m_barpos[3] += undery;
			m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
		}
	}

	else if(m_ldowndown)
	{
		float dy = (float)g_drawfrinterval * m_domain;

		float origscroll = m_scroll[1];

		m_scroll[1] += (float)dy;

		int w = (int)( m_pos[2]-m_pos[0] );

		m_barpos[1] = m_uppos[3] + (m_downpos[1]-m_uppos[3])*m_scroll[1];
		//m_barpos[3] = m_barpos[1] + fmax(w, m_domain*(m_downpos[1]-m_uppos[3]));
		m_barpos[3] = m_barpos[1] + m_domain*(m_downpos[1]-m_uppos[3]);

		float overy = m_barpos[3] - m_downpos[1];

		if(overy > 0)
		{
			m_barpos[1] -= overy;
			m_barpos[3] -= overy;
			m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
		}

		float undery = m_uppos[3] - m_barpos[1];

		if(undery > 0)
		{
			m_barpos[1] += undery;
			m_barpos[3] += undery;
			m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
		}
	}
}

void VScroll::inev(InEv* ie)
{
	//for(std::list<Brush>::iterator w=m_subwidg.rbegin(); w!=m_subwidg.rend(); w++)
	//	(*w)->inev(ie);

	//return;

#if 1
	if(m_ldown)
	{
		if(ie->type == INEV_MOUSEMOVE ||
			( (ie->type == INEV_MOUSEDOWN || ie->type == INEV_MOUSEUP) && ie->key == MOUSE_LEFT) )
			ie->intercepted = true;

		if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
		{
			m_ldown = false;
			m_ldownbar = false;
			m_ldownup = false;
			m_ldowndown = false;
		}

		if(ie->type == INEV_MOUSEMOVE && m_ldownbar)
		{
			if(g_mouse.y < m_barpos[1] || g_mouse.y > m_barpos[3])
				return;

			int dx = g_mouse.x - m_mousedown[0];
			int dy = g_mouse.y - m_mousedown[1];
			m_mousedown[0] = g_mouse.x;
			m_mousedown[1] = g_mouse.y;

			float origscroll = m_scroll[1];

			m_scroll[1] += (float)dy / (m_downpos[1] - m_uppos[3]);

			int w = (int)( m_pos[2]-m_pos[0] );

			m_barpos[1] = m_uppos[3] + (m_downpos[1]-m_uppos[3])*m_scroll[1];
			//m_barpos[3] = m_barpos[1] + fmax(w, m_domain*(m_downpos[1]-m_uppos[3]));
			m_barpos[3] = m_barpos[1] + m_domain*(m_downpos[1]-m_uppos[3]);

			float overy = m_barpos[3] - m_downpos[1];

			if(overy > 0)
			{
				m_barpos[1] -= overy;
				m_barpos[3] -= overy;
				m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
			}

			float undery = m_uppos[3] - m_barpos[1];

			if(undery > 0)
			{
				m_barpos[1] += undery;
				m_barpos[3] += undery;
				m_scroll[1] = (m_barpos[1] - m_uppos[3]) / (m_downpos[1] - m_uppos[3]);
			}

			if(m_parent)
			{
				VScroll::ScrollEv sev;
				sev.delta = m_scroll[1] - origscroll;
				sev.newpos = m_scroll[1];

				//m_parent->chcall(this, CHCALL_VSCROLL, (void*)&sev);
			}
		}
	}
#endif

	//return;

	if(m_over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			m_ldown = true;

			if(g_mouse.x >= m_barpos[0] &&
				g_mouse.y >= m_barpos[1] &&
				g_mouse.x <= m_barpos[2] &&
				g_mouse.y <= m_barpos[3])
			{
				m_ldownbar = true;
				m_mousedown[0] = g_mouse.x;
				m_mousedown[1] = g_mouse.y;
				ie->intercepted = true;

				//ie->curst = CU_RESZT;
			}

			if(g_mouse.x >= m_uppos[0] &&
				g_mouse.y >= m_uppos[1] &&
				g_mouse.x <= m_uppos[2] &&
				g_mouse.y <= m_uppos[3])
			{
				m_ldownup = true;
				ie->intercepted = true;

				//ie->curst = CU_RESZT;
			}

			if(g_mouse.x >= m_downpos[0] &&
				g_mouse.y >= m_downpos[1] &&
				g_mouse.x <= m_downpos[2] &&
				g_mouse.y <= m_downpos[3])
			{
				m_ldowndown = true;
				ie->intercepted = true;

				//ie->curst = CU_RESZT;
			}
		}
	}

	//return;

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(m_ldown)
		{
			ie->intercepted = true;
			return;
		}

		//return;

#if 1
		//corpe fix
		if(/* !ie->intercepted && */
			g_mouse.x >= m_pos[0] &&
			g_mouse.y >= m_pos[1] &&
			g_mouse.x <= m_pos[2] &&
			g_mouse.y <= m_pos[3])
		{
			m_over = true;

			//if(g_mousekeys[MOUSE_MIDDLE])
			//	return;

			//return;
			ie->intercepted = true;

			//InfoMess("in", "i");

#if 0
			//if(g_curst == CU_RESZT)
			//	return;

			if(g_mouse.x >= m_barpos[0] &&
				g_mouse.y >= m_barpos[1] &&
				g_mouse.x <= m_barpos[2] &&
				g_mouse.y <= m_barpos[3])
				ie->curst = CU_RESZT;
#endif
		}
		else
		{
			if(!ie->intercepted)
			{
				if(m_over)
				{
					//g_curst = CU_DEFAULT;
				}
			}
			else
			{
				// to do: this will be replaced by code in other
				//widgets that will set the cursor
				//ie->curst = CU_DEFAULT;
			}

			m_over = false;
		}
	}
#endif
}
