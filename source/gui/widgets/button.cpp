







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
#include "../icon.h"
#include "../../ustring.h"
#include "../gui.h"

#include "../../debug.h"

Button::Button() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_BUTTON;
	//m_text = RichText("");
	m_font = MAINFONT8;
	//float length = 0;
	//for(int i=0; i<strlen(t); i++)
	//    length += g_font[f].glyph[t[i]].w;
	//m_tpos[0] = (left+right)/2.0f - length/2.0f;
	//m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_style = BUST_LINEBASED;
	m_over = false;
	m_ldown = false;
	CreateTex(m_tex, "gui/transp.png", true, false);
	CreateTex(m_bgtex, "gui/corrodbutton.png", true, false);
	CreateTex(m_bgovertex, "gui/corrodbuttonover.png", true, false);
	reframefunc = NULL;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = NULL;
	m_param = -1;
	clickfunc3 = NULL;
	//reframe();
}

Button::Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip, int f, int style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm, void (*click3)(Widget* w)) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BUTTON;
	m_name = name;
	m_style = style;
	m_text = tooltip;
	m_label = label;
	m_font = f;
	float length = 0;
	//for(int i=0; i<strlen(t); i++)
	//    length += g_font[f].glyph[t[i]].origsize[0];
	length = EndX(&m_text, m_text.rawlen(), m_font, 0, 0);
	//m_tpos[0] = (left+right)/2.0f - length/2.0f;
	//m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_over = false;
	m_ldown = false;
	CreateTex(m_tex, filepath, true, false);

	if(style == BUST_CORRODE)
	{
		CreateTex(m_bgtex, "gui/corrodbutton.png", true, false);
		CreateTex(m_bgovertex, "gui/corrodbuttonover.png", true, false);
	}

	reframefunc = reframef;
	clickfunc = click;
	clickfunc2 = click2;
	overfunc = overf;
	overfunc2 = overf2;
	outfunc = out;
	m_param = parm;
	clickfunc3 = click3;
	reframe();
}

void Button::inev(InEv* ie)
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

			if(clickfunc3 != NULL)
				clickfunc3(this);

			//m_over = false;
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
	else if(ie->type == INEV_MOUSEMOVE)
	{
		if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
		{
		}
		else
		{
			if(m_over && outfunc != NULL)
				outfunc();

			m_over = false;
		}

		if(!ie->intercepted)
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
		}
	}
}

void Button::draw()
{
	if(m_style == BUST_CORRODE)
	{
		if(m_over)
			DrawImage(g_texture[m_bgovertex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);
		else
			DrawImage(g_texture[m_bgtex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

		DrawImage(g_texture[m_tex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

		DrawShadowedText(m_font, m_tpos[0], m_tpos[1], &m_label);
	}
	else if(m_style == BUST_LEFTIMAGE)
	{
		//InfoMess("li", "li");
		

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

		DrawSquare(midcolor[0], midcolor[1], midcolor[2], midcolor[3], m_pos[0], m_pos[1], m_pos[2], m_pos[3], m_crop);

		DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_pos[2], m_pos[1], m_pos[2], m_pos[3]-1, m_crop);
		DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_pos[0], m_pos[1], m_pos[2]-1, m_pos[1], m_crop);

		DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_pos[0]+1, m_pos[3], m_pos[2], m_pos[3], m_crop);
		DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_pos[2], m_pos[1]+1, m_pos[2], m_pos[3], m_crop);

		EndS();
		CHECKGLERROR();
		Ortho(g_currw, g_currh, 1, 1, 1, 1);

		float w = m_pos[2]-m_pos[0]-2;
		float h = m_pos[3]-m_pos[1]-2;
		float minsz = fmin(w, h);

		DrawImage(g_texture[m_tex].texname, m_pos[0]+1, m_pos[1]+1, m_pos[0]+minsz, m_pos[1]+minsz, 0,0,1,1, m_crop);

		float gheight = g_font[m_font].gheight;
		float texttop = m_pos[1] + h/2.0f - gheight/2.0f;
		float textleft = m_pos[0]+minsz+1;

		DrawShadowedText(m_font, textleft, texttop, &m_label);
	}
	else if(m_style == BUST_LINEBASED)
	{
		//InfoMess("lb", "lb");
		

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

		DrawSquare(midcolor[0], midcolor[1], midcolor[2], midcolor[3], m_pos[0], m_pos[1], m_pos[2], m_pos[3], m_crop);

		DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_pos[2], m_pos[1], m_pos[2], m_pos[3]-1, m_crop);
		DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], m_pos[0], m_pos[1], m_pos[2]-1, m_pos[1], m_crop);

		DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_pos[0]+1, m_pos[3], m_pos[2], m_pos[3], m_crop);
		DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], m_pos[2], m_pos[1]+1, m_pos[2], m_pos[3], m_crop);

		EndS();
		CHECKGLERROR();
		Ortho(g_currw, g_currh, 1, 1, 1, 1);

		float w = m_pos[2]-m_pos[0]-2;
		float h = m_pos[3]-m_pos[1]-2;
		float minsz = fmin(w, h);

		DrawImage(g_texture[m_tex].texname, m_pos[0]+1, m_pos[1]+1, m_pos[0]+minsz, m_pos[1]+minsz, 0,0,1,1, m_crop);
		CenterLabel(this);
		DrawShadowedTextF(m_font, m_tpos[0], m_tpos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &m_label);
	}

	//if(_stricmp(m_name.c_str(), "choose texture") == 0)
	//	Log("draw choose texture");
}

void Button::drawover()
{
	if(m_over)
	{
		
		//DrawShadowedText(m_font, m_tpos[0], m_tpos[1], m_text.c_str());
		DrawShadowedText(m_font, g_mouse.x, g_mouse.y-g_font[m_font].gheight, &m_text);
		//DrawBoxShadText(m_font, g_mouse.x, g_mouse.y-g_font[m_font].gheight, &m_text);
	}
}

