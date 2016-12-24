







#include "../gui.h"
#include "../../texture.h"

#include "winw.h"
#include "../../debug.h"

void Click_WinClose(Widget* thisw)
{
	thisw->m_parent->hide();
}

Win::Win() : Widget()
{
	m_type = WIDGET_WINDOW;
	m_parent = NULL;
	m_opened = false;
}

void Win::chcall(Widget* ch, int type, void* data)
{
	if(ch == &m_vscroll)
	{

	}
	//else if(ch == &hscroll)
	{
	}
}

Win::Win(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Widget()
{
	m_type = WIDGET_WINDOW;
	reframefunc = reframef;
	m_parent = parent;
	m_name = n;
	m_scroll[0] = 0;
	m_scroll[1] = 0;
	//m_hidden = true;
	m_hidden = false;

	const float alpha = 0.9f;

	CHECKGLERROR();

	bg_logo_image = Image(this, "", "gui/centerp/pcsgray.png", true, NULL, 1, 1, 1, alpha/8.0f,		0, 0, 1, 1);

	m_vscroll = VScroll(this, "m_vscroll");

	title_text = Text(this, "title", RichText(""), FONT_EUROSTILE16, NULL, true, 1, 1, 1, 1.0f);

	m_trclose = Button(this, "hide", "gui/cancel.png", RichText(), RichText(), MAINFONT16, BUST_LEFTIMAGE, NULL, NULL, NULL, NULL, NULL, NULL, -1, Click_WinClose);

	// only call this when window is created
	if(reframefunc)
		reframefunc(this);
	else //default size and position
	{
		// to do
	}

	reframe();
}

void Win::reframe()
{
#if 1
	if(m_parent)
	{
		SubCrop(m_parent->m_crop, m_pos, m_crop);
	}
	else
	{
		m_crop[0] = 0;
		m_crop[1] = 0;
		m_crop[2] = (float)g_width-1;
		m_crop[3] = (float)g_height-1;
	}
#endif

	m_scar[0] = m_pos[0];
	m_scar[1] = m_pos[1];
	m_scar[2] = m_pos[2];
	m_scar[3] = m_pos[3];

	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();

	//m_crop[0] = 0;
	//m_crop[1] = 0;
	//m_crop[2] = (float)g_width-1;
	//m_crop[3] = (float)g_height-1;
	//fillout(m_crop);

	//m_vscroll.m_pos[0] = innerright - 3 - 10;
	//m_vscroll.m_pos[1] = innertop + 27;
	//m_vscroll.m_pos[2] = innerright - 3;
	//m_vscroll.m_pos[3] = innerbottom - 27 - 10;
	m_vscroll.reframe();

	//title_text.m_pos[0] = inner_top_mid_image.m_pos[0];
	//title_text.m_pos[1] = inner_top_mid_image.m_pos[1];
	//title_text.m_pos[2] = inner_top_mid_image.m_pos[2];
	//title_text.m_pos[3] = inner_top_mid_image.m_pos[1] + 32;
	title_text.reframe();

	float outpos[4];
	fillout(outpos);
	m_trclose.m_pos[0] = outpos[2] - (m_pos[1]-outpos[1]) * 2;
	m_trclose.m_pos[1] = outpos[1];
	m_trclose.m_pos[2] = outpos[2] - (m_pos[1]-outpos[1]) * 1;
	m_trclose.m_pos[3] = m_pos[1];
	m_trclose.reframe();
	m_trclose.m_crop[0] = outpos[0];
	m_trclose.m_crop[1] = outpos[1];
	m_trclose.m_crop[2] = outpos[2];
	m_trclose.m_crop[3] = outpos[3];

	float minsz = fmin((m_pos[2]-m_pos[0]),(m_pos[3]-m_pos[1]));

	bg_logo_image.m_pos[0] = (m_pos[0]+m_pos[2])/2.0f - minsz/2.0f;
	bg_logo_image.m_pos[1] = (m_pos[3]+m_pos[1])/2.0f - minsz/2.0f;
	bg_logo_image.m_pos[2] = (m_pos[0]+m_pos[2])/2.0f + minsz/2.0f;
	bg_logo_image.m_pos[3] = (m_pos[3]+m_pos[1])/2.0f + minsz/2.0f;
	bg_logo_image.reframe();

	m_scar[2] = m_scar[0];
	m_scar[3] = m_scar[1];
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
	{
		Widget* w = *i;
		m_scar[2] = fmax(m_scar[2], w->m_pos[2]);
		m_scar[3] = fmax(m_scar[3], w->m_pos[3]);
	}
}

void Win::fillout(float* outpos)
{
	BmpFont* f = &g_font[MAINFONT16];
	outpos[0] = m_pos[0] - 7;
	outpos[1] = m_pos[1] - 7 - f->gheight;
	outpos[2] = m_pos[2] + 7;
	outpos[3] = m_pos[3] + 7;
}

void Win::draw()
{
	EndS();
	UseS(SHADER_COLOR2D);
	Shader* s = &g_shader[g_curS];
	glUniform1f(s->m_slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->m_slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->m_slot[SSLOT_COLOR], 1,1,1,1);

	float outpos[4];

	fillout(outpos);

	DrawSquare(0.5f, 0.5f, 0.5f, 0.9f, outpos[0], outpos[1], outpos[2], outpos[3], outpos);
	
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, outpos[0], outpos[1], outpos[2], outpos[1], outpos);	//top line outer
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, outpos[0], outpos[1], outpos[0], outpos[3], outpos);	//left line outer
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, outpos[0], outpos[3], outpos[2], outpos[3], outpos);	//bottom line outer
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, outpos[2], outpos[1], outpos[2], outpos[3], outpos);	//right line outer

	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, m_pos[0]-1, m_pos[1]-1, m_pos[2]+1, m_pos[1]-1, outpos);	//top line inner
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, m_pos[0]-1, m_pos[1]-1, m_pos[0]-1, m_pos[3]+1, outpos);	//left line inner
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, m_pos[0]-1, m_pos[3]+1, m_pos[2]+1, m_pos[3]+1, outpos);	//bottom line inner
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, m_pos[2]+1, m_pos[1]-1, m_pos[2]+1, m_pos[3]+1, outpos);	//right line inner

	EndS();
	Ortho(g_width, g_height, 1, 1, 1, 1);

	//bg_logo_image.draw();

	//m_vscroll.draw();
	title_text.draw();
	m_trclose.draw();
	
	Widget::draw();
}

void Win::drawover()
{
	//m_vscroll.drawover();

	Widget::drawover();
}

void Win::show()
{
	Widget::show();
	
	//necessary for window widgets:
	tofront();	//can't break list iterator, might shift
}

void Win::inev(InEv* ie)
{
	Widget::inev(ie);

	//m_vscroll.inev(ie);

	m_trclose.inev(ie);
	
	float outpos[4];
	fillout(outpos);

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

			if(g_curst == CU_MOVE)
			{
				m_pos[0] += dx;
				m_pos[1] += dy;
				m_pos[2] += dx;
				m_pos[3] += dy;

				if(m_pos[0] < 0)
				{
					m_pos[2] -= m_pos[0] - (float)0;
					m_pos[0] = (float)0;
				}
				if(m_pos[2] > (float)g_width)
				{
					m_pos[0] -= m_pos[2] - (float)g_width;
					m_pos[2] = (float)(g_width);
				}
				if(m_pos[1] < 0)
				{
					m_pos[3] -= m_pos[1];
					m_pos[1] = (float)64;
				}
				if(m_pos[3] > g_height)
				{
					m_pos[1] -= m_pos[3] - g_height;
					m_pos[3] = (float)(g_height);
				}

				reframe();
			}
			else if(g_curst == CU_RESZT)
			{
				int newh = (int)( m_pos[3]-m_pos[1]-dy );
				if(newh < 0) newh = 0;
				m_pos[1] = (float)((int)m_pos[3] - newh);
				if(m_pos[1] < 0) m_pos[1] = (float)0;
				if(m_pos[1] > g_height) m_pos[1] = (float)(g_height);
				reframe();
			}
			else if(g_curst == CU_RESZB)
			{
				int newh = (int)( m_pos[3]-m_pos[1]+dy );
				if(newh < 0) newh = 0;
				m_pos[3] = (float)((int)m_pos[1] + newh);
				if(m_pos[3] < 0) m_pos[3] = (float)0;
				if(m_pos[3] > g_height) m_pos[3] = (float)(g_height);
				reframe();
			}
			else if(g_curst == CU_RESZL)
			{
				int neww = (int)( m_pos[2]-m_pos[0]-dx );
				if(neww < 0) neww = 0;
				m_pos[0] = (float)((int)m_pos[2] - neww);
				if(m_pos[0] < 0) m_pos[0] = (float)0;
				if(m_pos[0] > g_width) m_pos[0] = (float)(g_width);
				reframe();
			}
			else if(g_curst == CU_RESZR)
			{
				int neww = (int)( m_pos[2]-m_pos[0]+dx );
				if(neww < 0) neww = 0;
				m_pos[2] = (float)((int)m_pos[0] + neww);
				if(m_pos[2] < 0) m_pos[2] = (float)0;
				if(m_pos[2] > g_width) m_pos[2] = (float)(g_width);
				reframe();
			}
			else if(g_curst == CU_RESZTL)
			{
				int newh = (int)( m_pos[3]-m_pos[1]-dy );
				if(newh < 0) newh = 0;
				m_pos[1] = (float)((int)m_pos[3] - newh);
				if(m_pos[1] < 0) m_pos[1] = (float)0;
				if(m_pos[1] > g_height) m_pos[1] = (float)(g_height);

				int neww = (int)( m_pos[2]-m_pos[0]-dx );
				if(neww < 0) neww = 0;
				m_pos[0] = (float)((int)m_pos[2] - neww);
				if(m_pos[0] < 0) m_pos[0] = (float)0;
				if(m_pos[0] > g_width) m_pos[0] = (float)(g_width);

				reframe();
			}
			else if(g_curst == CU_RESZTR)
			{
				int newh = (int)( m_pos[3]-m_pos[1]-dy );
				if(newh < 0) newh = 0;
				m_pos[1] = m_pos[3] - newh;
				if(m_pos[1] < 0) m_pos[1] = (float)0;
				if(m_pos[1] > g_height) m_pos[1] = (float)(g_height);

				int neww = (int)( m_pos[2]-m_pos[0]+dx );
				if(neww < 0) neww = 0;
				m_pos[2] = (float)((int)m_pos[0] + neww );
				if(m_pos[2] < 0) m_pos[2] = (float)0;
				if(m_pos[2] > g_width) m_pos[2] = (float)(g_width);

				reframe();
			}
			else if(g_curst == CU_RESZBL)
			{
				int newh = (int)( m_pos[3]-m_pos[1]+dy );
				if(newh < 0) newh = 0;
				m_pos[3] = (float)((int)m_pos[1] + newh);
				if(m_pos[3] < 0) m_pos[3] = (float)0;
				if(m_pos[3] > g_height) m_pos[3] = (float)(g_height);

				int neww = (int)( m_pos[2]-m_pos[0]-dx );
				if(neww < 0) neww = 0;
				m_pos[0] = (float)((int)m_pos[2] - neww);
				if(m_pos[0] < 0) m_pos[0] = (float)0;
				if(m_pos[0] > g_width) m_pos[0] = (float)(g_width);

				reframe();
			}
			else if(g_curst == CU_RESZBR)
			{
				int newh = (int)( m_pos[3]-m_pos[1]+dy );
				if(newh < 0) newh = 0;
				m_pos[3] = (float)((int)m_pos[1] + newh);
				if(m_pos[3] < 0) m_pos[3] = (float)0;
				if(m_pos[3] > g_height) m_pos[3] = (float)(g_height);

				int neww = (int)( m_pos[2]-m_pos[0]+dx );
				if(neww < 0) neww = 0;
				m_pos[2] = (float)((int)m_pos[0] + neww);
				if(m_pos[2] < 0) m_pos[2] = (float)0;
				if(m_pos[2] > g_width) m_pos[2] = (float)(g_width);

				reframe();
			}
		}
	}

	if(m_over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			//InfoMess("win ev", "WE");

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

		if(!ie->intercepted)
		{
			if( g_mouse.x >= outpos[0] &&
			   g_mouse.y >= outpos[1] &&
			   g_mouse.x <= outpos[2] &&
			   g_mouse.y <= outpos[3])
			{
				m_over = true;
				
				//InfoMess("win evOVER", "OVER");
				
				if(g_mousekeys[MOUSE_MIDDLE])
					return;
				
				//ie->intercepted = true;
				
				if(g_curst == CU_DRAG)
					return;
				
				if(g_mouse.x <= m_pos[0])
				{
					if(g_mouse.y <= m_pos[1])
						ie->curst = CU_RESZTL;
					else if(g_mouse.y >= m_pos[3])
						ie->curst = CU_RESZBL;
					else
						ie->curst = CU_RESZL;
					
					ie->intercepted = true;
				}
				else if(g_mouse.x >= m_pos[2])
				{
					if(g_mouse.y <= m_pos[1])
						ie->curst = CU_RESZTR;
					else if(g_mouse.y >= m_pos[3])
						ie->curst = CU_RESZBR;
					else
						ie->curst = CU_RESZR;
					
					ie->intercepted = true;
				}
				else if(g_mouse.x >= outpos[0] &&
						g_mouse.x <= outpos[2])
				{
					if(g_mouse.y <= outpos[1]+4)
						ie->curst = CU_RESZT;
					else if(g_mouse.y >= m_pos[3])
						ie->curst = CU_RESZB;
					else if(g_mouse.x >= m_pos[0] &&
							g_mouse.y >= outpos[1]+4 &&
							g_mouse.x <= m_pos[2] &&
							g_mouse.y <= m_pos[1])
						ie->curst = CU_MOVE;
					else
						ie->curst = CU_DEFAULT;
					
					ie->intercepted = true;
				}
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
}

void Win::subframe(float* fr)
{
	memcpy(fr, m_pos, sizeof(float)*4);
}
