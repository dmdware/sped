







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

#include "../gui.h"
#include "../../sys/unicode.h"
#include "../../debug.h"

EditBox::EditBox() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_EDITBOX;
	m_name = "";
	m_font = MAINFONT8;
	m_value = "";
	m_caret = m_value.texlen();
	m_opened = false;
	m_passw = false;
	m_maxlen = 0;
	reframefunc = NULL;
	submitfunc = NULL;
	changefunc = NULL;
	changefunc2 = NULL;
	changefunc3 = NULL;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTex(m_frametex, "gui/frame.jpg", true, false);
	m_param = -1;
	changefunc2 = NULL;
	//reframe();
}

EditBox::EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), bool pw, int maxl, void (*change3)(unsigned int key, unsigned int scancode, bool down, int parm), void (*submitf)(), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_EDITBOX;
	m_name = n;
	m_font = f;
	m_value = t;
	m_caret = m_value.texlen();
	m_opened = false;
	m_passw = pw;
	m_maxlen = maxl;
	reframefunc = reframef;
	submitfunc = submitf;
	changefunc = NULL;
	changefunc2 = NULL;
	changefunc3 = change3;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTex(m_frametex, "gui/frame.jpg", true, false);
	m_param = parm;
	changefunc2 = NULL;
	reframe();
}

RichText EditBox::drawvalue()
{
	/*
	std::string val = m_value;

	if(m_passw)
	{
		val = "";
		for(int i=0; i<m_value.length(); i++)
			val.append("*");
	}

	return val;*/

	if(!m_passw)
		return m_value;

	return m_value.pwver();
}

void EditBox::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[m_frametex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

	if(m_over)
		//glColor4f(1, 1, 1, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	else
		//glColor4f(0.8f, 0.8f, 0.8f, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1);

	RichText val = drawvalue();

	//if(m_opened)
	//	Log("op m_caret="<<m_caret);

	DrawShadowedTextF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &val, NULL, m_opened ? m_caret : -1);

	//glColor4f(1, 1, 1, 1);
	//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	HighlightF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &val, m_highl[0], m_highl[1]);
}

//#define MOUSESC_DEBUG

void EditBox::frameupd()
{
#ifdef MOUSESC_DEBUG
	Log("editbox frameup");
	
#endif

	

	if(m_ldown)
	{
		bool movedcar = false;

#ifdef MOUSESC_DEBUG
		Log("ldown frameup");
		
#endif

		if(g_mouse.x >= m_pos[2]-5)
		{
			m_scroll[0] -= fmax(1, g_font[m_font].gheight/4.0f);

			RichText val = drawvalue();
			int vallen = val.texlen();

			int endx = EndX(&val, vallen, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			if(endx < m_pos[2])
				m_scroll[0] += m_pos[2] - endx;

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}
		else if(g_mouse.x <= m_pos[0]+5)
		{
			m_scroll[0] += fmax(1, g_font[m_font].gheight/4.0f);

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}

		if(movedcar)
		{
			RichText val = drawvalue();
			int newcaret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

			if(newcaret > m_caret)
			{
				m_highl[0] = m_caret;
				m_highl[1] = newcaret;
			}
			else if(newcaret < m_caret)
			{
				m_highl[0] = newcaret;
				m_highl[1] = m_caret;
			}
		}
	}
}

void EditBox::inev(InEv* ie)
{
//#ifdef MOUSESC_DEBUG
	//Log("editbox mousemove");
	//
//#endif

	

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(!ie->intercepted)
		{
			if(m_ldown)
			{
				RichText val = drawvalue();
				int newcaret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

				if(newcaret > m_caret)
				{
					m_highl[0] = m_caret;
					m_highl[1] = newcaret;
					//Log("hihgl "<<m_highl[0]<<"->"<<m_highl[1]);
					//
				}
				else
				{
					m_highl[0] = newcaret;
					m_highl[1] = m_caret;
					//Log("hihgl "<<m_highl[0]<<"->"<<m_highl[1]);
					//
				}

				ie->intercepted = true;
				return;
			}

			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
			{
				m_over = true;

				g_mouseoveraction = true;

				ie->intercepted = true;
				return;
			}
			else
			{
				m_over = false;

				return;
			}
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
	{
		if(m_opened)
		{
			m_opened = false;
			m_highl[0] = m_highl[1] = 0;
		}

		if(!ie->intercepted)
		{
			if(m_over)
			{
				m_ldown = true;

				RichText val = drawvalue();

				//m_highl[1] = MatchGlyphF(m_value.c_str(), m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
				//m_highl[0] = m_highl[1];
				//m_caret = m_highl[1];
				m_caret = MatchGlyphF(&val, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

				m_highl[0] = 0;
				m_highl[1] = 0;

				ie->intercepted = true;
				return;
			}
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//if(m_over && m_ldown)
		if(m_ldown)
		{
			m_ldown = false;

			if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = -1;
			}

			ie->intercepted = true;
			gainfocus();

			return;
		}

		m_ldown = false;

		if(m_opened)
		{
			ie->intercepted = true;
			return;
		}
	}
	else if(ie->type == INEV_KEYDOWN && !ie->intercepted)
	{
		if(!m_opened)
			return;

		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		if(ie->key == SDLK_F1)
			return;

		if(ie->key == SDLK_LEFT)
		{
			if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = m_highl[0];
				m_highl[0] = m_highl[1] = 0;
			}
			else if(m_caret <= 0)
			{
				ie->intercepted = true;
				return;
			}
			else
				m_caret --;

			RichText val = drawvalue();
			int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			//Log("left endx = "<<endx<<"/"<<m_pos[0]);
			//

			if(endx <= m_pos[0])
				m_scroll[0] += m_pos[0] - endx + 1;
		}
		else if(ie->key == SDLK_RIGHT)
		{
			int len = m_value.texlen();

			if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = m_highl[1];
				m_highl[0] = m_highl[1] = 0;
			}
			else if(m_caret >= len)
			{
				ie->intercepted = true;
				return;
			}
			else
				m_caret ++;

			RichText val = drawvalue();
			int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			if(endx >= m_pos[2])
				m_scroll[0] -= endx - m_pos[2] + 1;
		}
		else if(ie->key == SDLK_DELETE)
		{
			len = m_value.texlen();

			//Log("vk del");
			//

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			{
				ie->intercepted = true;
				return;
			}

			delnext();

			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
		}
		if(ie->key == SDLK_BACKSPACE)
		{
			len = m_value.texlen();
			//len = m_value.rawstr().length();

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && len <= 0)
			{
				ie->intercepted = true;
				return;
			}

			delprev();

			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
		}/*
		 else if(ie->key == SDLK_DELETE)
		 {
		 len = m_value.texlen();

		 Log("vk del");
		 

		 if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
		 return true;

		 delnext();

		 if(!m_passw)
		 m_value = ParseTags(m_value, &m_caret);
		 }*/
		else if(ie->key == SDLK_LSHIFT || ie->key == SDLK_RSHIFT)
		{
			ie->intercepted = true;
			return;
		}
		else if(ie->key == SDLK_CAPSLOCK)
		{
			ie->intercepted = true;
			return;
		}
		else if(ie->key == SDLK_RETURN || ie->key == SDLK_RETURN2)
		{
			ie->intercepted = true;
			if(submitfunc)
				submitfunc();
			return;
		}
#if 0
		else if(ie->key == 190 && !g_keys[SDLK_SHIFT])
		{
			//placechar('.');
		}
#endif

		if(changefunc2 != NULL)
			changefunc2(m_param);

		if(changefunc3 != NULL)
			changefunc3(ie->key, ie->scancode, true, m_param);

		ie->intercepted = true;
	}
	else if(ie->type == INEV_KEYUP && !ie->intercepted)
	{
		if(!m_opened)
			return;

		if(changefunc3 != NULL)
			changefunc3(ie->key, ie->scancode, false, m_param);

		ie->intercepted = true;
	}
	else if(ie->type == INEV_TEXTIN && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;

		int len = m_value.texlen();
		//len = m_value.rawstr().length();

		if(m_caret > len)
			m_caret = len;

		//Log("vk "<<ie->key);
		//


#if 0
		if(ie->key == SDLK_SPACE)
		{
			placechar(' ');
		}
		else
#endif

#ifdef PASTE_DEBUG
			Log("charin "<<(char)ie->key<<" ("<<ie->key<<")");
		
#endif

#if 0
		//if(ie->key == 'C' && g_keys[SDLK_CONTROL])
		if(ie->key == 3)	//copy
		{
			copyval();
		}
		//else if(ie->key == 'V' && g_keys[SDLK_CONTROL])
		else if(ie->key == 22)	//paste
		{
			pasteval();
		}
		//else if(ie->key == 'A' && g_keys[SDLK_CONTROL])
		else if(ie->key == 1)	//select all
		{
			selectall();
		}
		else
#endif
		//unsigned int* ustr = ToUTF32((const unsigned char*)ie->text.c_str(), ie->text.length());
		unsigned int* ustr = ToUTF32((const unsigned char*)ie->text.c_str());
		//RichText addstr(RichPart(UStr(ustr)));	//Why does MSVS2012 not accept this?
		RichText addstr = RichText(RichPart(UStr(ustr)));
		unsigned int first = ustr[0];
		delete [] ustr;

		placestr(&addstr);

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(m_param);

		if(changefunc3 != NULL)
			changefunc3(first, 0, true, m_param);

		ie->intercepted = true;
	}
	else if(ie->type == INEV_PASTE && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;

		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		pasteval();
	}
	else if(ie->type == INEV_COPY && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;

		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		copyval();
	}
	else if(ie->type == INEV_SELALL && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;

		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		selectall();
	}
}

void EditBox::placestr(const RichText* str)
{
	int len = m_value.texlen();
	int rawlen = m_value.rawstr().length();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		len -= m_highl[1] - m_highl[0];
	}

	//corpc fix's all around texlen();

	int addlen = str->texlen();
	int rawaddlen = str->rawstr().length();
	if(rawaddlen + rawlen >= m_maxlen)
		//addlen = m_maxlen - len;
		rawaddlen = m_maxlen - rawlen;

	/*
	we want to make sure that the UTF8 string
	will be below m_maxlen, with the icon tags
	shown, NOT the final resulting RichText string.
	*/
	//RichText addstr = str->substr(0, addlen);
	std::string rawaddstr = str->rawstr().substr(0, rawaddlen);
	unsigned int* rawaddustr = ToUTF32((unsigned char*)rawaddstr.c_str());
	RichText addstr = RichText(UStr(rawaddustr));
	delete [] rawaddustr;

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		//RichText after = m_value.substr(m_highl[1]-1, m_value.texlen()-m_highl[1]);
		RichText after = m_value.substr(m_highl[1], m_value.texlen()-m_highl[1]);
		m_value = before + addstr + after;

		m_caret = m_highl[0] + addlen;
		m_highl[0] = m_highl[1] = 0;
	}
	else
	{
		if(len >= m_maxlen)
		{
			return;
		}

		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret, m_value.texlen()-m_caret);
		m_value = before + addstr + after;
		m_caret += addlen;

		//LogRich(&m_value);
	}

	if(!m_passw)
		m_value = ParseTags(m_value, &m_caret);

	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;
}

#if 0
void EditBox::changevalue(const char* str)
{
	int len = m_value.texlen();

	if(len >= m_maxlen)
		return;

	int setlen = strlen(str);
	if(setlen >= m_maxlen)
		setlen = m_maxlen;

	char* setstr = new char[setlen+1];

	if(!setstr)
		OutOfMem(__FILE__, __LINE__);

	for(int i=0; i<setlen; i++)
		setstr[i] = str[i];
	setstr[setlen] = '\0';

	m_value = setstr;
	m_highl[0] = m_highl[1] = 0;
	m_caret = 0;

	delete [] setstr;
}
#else

void EditBox::changevalue(const RichText* str)
{
#if 0
	int setlen = str->texlen();
	if(setlen >= m_maxlen)
		setlen = m_maxlen;

	m_value = str->substr(0, setlen);
	m_highl[0] = m_highl[1] = 0;
	m_caret = 0;
#else
	m_caret = 0;
	m_highl[0] = m_highl[1] = 0;
	m_value = RichText();
	placestr(str);	//does rawlen checks here
#endif
}
#endif

bool EditBox::delnext()
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret >= len || len <= 0)
		return true;
	else
	{
		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret+1, len-m_caret);
		m_value = before + after;
	}

	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

bool EditBox::delprev()
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		RichText after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret <= 0 || len <= 0)
		return true;
	else
	{
		RichText before = m_value.substr(0, m_caret-1);
		RichText after = m_value.substr(m_caret, len-m_caret);
		m_value = before + after;

		//Log("before newval="<<before.rawstr()<<" texlen="<<before.texlen());
		//Log("after="<<after.rawstr()<<" texlen="<<after.texlen());
		//Log("ba newval="<<m_value.rawstr()<<" texlen="<<(before + after).texlen());
		//Log("newval="<<m_value.rawstr()<<" texlen="<<m_value.texlen());

		m_caret--;
	}

	RichText val = drawvalue();
	int endx = EndX(&val, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

//#define PASTE_DEBUG

void EditBox::copyval()
{
#ifdef PASTE_DEBUG
	Log("copy vkc");
	
#endif

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText highl = m_value.substr(m_highl[0], m_highl[1]-m_highl[0]);
		SDL_SetClipboardText( highl.rawstr().c_str() );
	}
	else
	{
		SDL_SetClipboardText( "" );
	}

	//return true;
}

void EditBox::pasteval()
{
	unsigned int* ustr = ToUTF32( (unsigned char*)SDL_GetClipboardText() ); 
	RichText rstr = RichText(UStr(ustr));
	delete [] ustr;
	placestr( &rstr );

	if(!m_passw)
		m_value = ParseTags(m_value, &m_caret);
}

void EditBox::selectall()
{
	m_highl[0] = 0;
	m_highl[1] = m_value.texlen()+1;
	m_caret = -1;

	RichText val = drawvalue();
	int endx = EndX(&val, m_value.texlen(), m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[2])
		m_scroll[0] += m_pos[2] - endx - 1;

	if(m_scroll[0] >= 0)
		m_scroll[0] = 0;

	//return true;
}

void EditBox::hide()
{
	losefocus();

	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->hide();
}


void EditBox::gainfocus()
{
	if(!m_opened)
	{
		

		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		m_opened = true;
		SDL_StartTextInput();
		SDL_Rect r;
#if 0
		r.x = (int)m_pos[0];
		r.y = (int)m_pos[3];
		r.w = (int)(g_width - m_pos[0]);
		r.h = (int)(g_height - m_pos[3]);
#else
		r.x = (int)g_width/2;
		r.y = (int)0;
		r.w = (int)g_width/2;
		r.h = (int)g_height;
#endif
		SDL_SetTextInputRect(&r);
		g_kbfocus++;
	}
}

void EditBox::losefocus()
{
	if(m_opened)
	{
		

		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		m_opened = false;
	}
}
