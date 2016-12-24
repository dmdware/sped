







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

#include "../../sys/unicode.h"

TextArea::TextArea(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), float r, float g, float b, float a, void (*change)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_TEXTAREA;
	m_name = n;
	m_value = t;
	m_font = f;
	reframefunc = reframef;
	m_ldown = false;
	m_rgba[0] = r;
	m_rgba[1] = g;
	m_rgba[2] = b;
	m_rgba[3] = a;
	m_highl[0] = 0;
	m_highl[1] = 0;
	changefunc = change;
	m_scroll[1] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	m_caret = m_value.texlen();
	CreateTex(m_frametex, "gui/frame.jpg", true, false);
	CreateTex(m_filledtex, "gui/filled.jpg", true, false);
	CreateTex(m_uptex, "gui/up.jpg", true, false);
	//CreateTex(m_downtex, "gui/down.jpg", true, false);
	reframe();
	m_lines = CountLines(&m_value, f, m_pos[0], m_pos[1], m_pos[2]-m_pos[0]-square(), m_pos[3]-m_pos[1]);
}

void TextArea::draw()
{
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(m_frametex, m_pos[0], m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);

	DrawImage(g_texture[m_frametex].texname, m_pos[2]-square(), m_pos[1], m_pos[2], m_pos[3], 0,0,1,1, m_crop);
	DrawImage(g_texture[m_uptex].texname, m_pos[2]-square(), m_pos[1], m_pos[2], m_pos[1]+square(), 0,0,1,1, m_crop);
	DrawImage(g_texture[m_downtex].texname, m_pos[2]-square(), m_pos[3]-square(), m_pos[2], m_pos[3], 0,0,1,1, m_crop);
	DrawImage(g_texture[m_filledtex].texname, m_pos[2]-square(), m_pos[1]+square()+scrollspace()*topratio(), m_pos[2], m_pos[1]+square()+scrollspace()*bottomratio(), 0,0,1,1, m_crop);

	float width = m_pos[2] - m_pos[0] - square();
	float height = m_pos[3] - m_pos[1];

	//DrawBoxShadText(m_font, m_pos[0], m_pos[1], width, height, m_value.c_str(), m_rgba, m_scroll[1], m_opened ? m_caret : -1);

	DrawShadowedTextF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &m_value, NULL, m_opened ? m_caret : -1);

	HighlightF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], &m_value, m_highl[0], m_highl[1]);
}

int TextArea::rowsshown()
{
	int rows = (int)( (m_pos[3]-m_pos[1])/g_font[m_font].gheight );

	return rows;
}

int TextArea::square()
{
	return (int)g_font[m_font].gheight;
}

float TextArea::scrollspace()
{
	return (m_pos[3]-m_pos[1]-square()*2);
}

void TextArea::inev(InEv* ie)
{
	

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(m_ldown)
		{
			int newcaret = MatchGlyphF(&m_value, m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

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
			return;
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(m_over && m_ldown)
		{
			m_ldown = false;
			ie->intercepted = true;
			gainfocus();
			return;
		}

		m_ldown = false;

		if(m_opened)
		{
			m_opened = false;
			return;
		}
	}
	else if(ie->type == INEV_KEYDOWN && !ie->intercepted)
	{
		if(!m_opened)
			return;

		//int len = m_value.length();
		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		if(ie->key == SDLK_LEFT)
		{
			if(m_caret <= 0)
			{
				ie->intercepted = true;
				return;
			}

			m_caret --;
		}
		else if(ie->key == SDLK_RIGHT)
		{
			int len = m_value.texlen();

			if(m_caret >= len)
			{
				ie->intercepted = true;
				return;
			}

			m_caret ++;
		}
		else if(ie->key == SDLK_DELETE)
		{
			len = m_value.texlen();

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			{
				ie->intercepted = true;
				return;
			}

			delnext();

			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
		}
#if 0
		else if(ie->key == 190 && !g_keys[SDLK_SHIFT])
		{
			//placechar('.');
		}
#endif

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(m_param);

		ie->intercepted = true;
	}
	else if(ie->type == INEV_KEYUP && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;
	}
	else if(ie->type == INEV_CHARIN && !ie->intercepted)
	{
		if(!m_opened)
			return;

		int len = m_value.texlen();

		if(m_caret > len)
			m_caret = len;

		if(ie->key == SDLK_BACKSPACE)
		{
			len = m_value.texlen();

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			{
				ie->intercepted = true;
				return;
			}

			delprev();

			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
		}/*
		 else if(k == SDLK_DELETE)
		 {
		 len = m_value.texlen();

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
		else if(ie->key == SDLK_SPACE)
		{
			placechar(' ');
		}
		else if(ie->key == SDLK_RETURN)
		{
			placechar('\n');
		}
		//else if(k >= 'A' && k <= 'Z' || k >= 'a' && k <= 'z')
		//else if(k == 190 || k == 188)
		//else if((k >= '!' && k <= '@') || (k >= '[' && k <= '`') || (k >= '{' && k <= '~') || (k >= '0' || k <= '9'))
		else
		{

#ifdef PASTE_DEBUG
			Log("charin "<<(char)k<<" ("<<k<<")");
			
#endif
#if 0
			//if(k == 'C' && g_keys[SDLK_CONTROL])
			if(ie->key == 3)	//copy
			{
				copyval();
			}
			//else if(k == 'V' && g_keys[SDLK_CONTROL])
			else if(ie->key == 22)	//paste
			{
				pasteval();
			}
			//else if(k == 'A' && g_keys[SDLK_CONTROL])
			else if(ie->key == 1)	//select all
			{
				selectall();
			}
			else
#endif
				placechar(ie->key);
		}

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(m_param);

		ie->intercepted = true;
	}
	else if(ie->type == INEV_TEXTIN && !ie->intercepted)
	{
		if(!m_opened)
			return;

		ie->intercepted = true;

		int len = m_value.texlen();

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

		//if(changefunc3 != NULL)
		//	changefunc3(first, 0, true, m_param);

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

void TextArea::changevalue(const char* newv)
{
	m_value = newv;
	if(m_caret > strlen(newv))
		m_caret = strlen(newv);
	m_lines = CountLines(&m_value, MAINFONT8, m_pos[0], m_pos[1], m_pos[2]-m_pos[0]-square(), m_pos[3]-m_pos[1]);
}

#if 0
void TextArea::placestr(const char* str)
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		len -= m_highl[1] - m_highl[0];
	}

	int addlen = strlen(str);
	if(addlen + len >= m_maxlen)
		addlen = m_maxlen - len;

	unsigned int* addstr = ToUTF32((unsigned char*)str);

	if(!addstr)
		OutOfMem(__FILE__, __LINE__);

	if(addlen > 0)
	{
		addstr[addlen] = 0;
	}
	else
	{
		delete [] addstr;
		return;
	}

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText before = m_value.substr(0, m_highl[0]);
		//RichText after = m_value.substr(m_highl[1]-1, m_value.texlen()-m_highl[1]);
		RichText after = m_value.substr(m_highl[1], m_value.texlen()-m_highl[1]);
		m_value = before + RichText(UStr(addstr)) + after;

		m_caret = m_highl[0] + addlen;
		m_highl[0] = m_highl[1] = 0;
	}
	else
	{
		if(len >= m_maxlen)
		{
			delete [] addstr;
			return;
		}

		RichText before = m_value.substr(0, m_caret);
		RichText after = m_value.substr(m_caret, m_value.texlen()-m_caret);
		m_value = before + addstr + after;
		m_caret += addlen;
	}

	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	delete [] addstr;
}
#else
void TextArea::placestr(const RichText* str)
{
	int len = m_value.texlen();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		len -= m_highl[1] - m_highl[0];
	}

	int addlen = str->texlen();
	if(addlen + len >= m_maxlen)
		addlen = m_maxlen - len;

	RichText addstr = str->substr(0, addlen);

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

	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;
}
#endif

bool TextArea::delnext()
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

	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

bool TextArea::delprev()
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

		m_caret--;
	}

	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

//#define PASTE_DEBUG

void TextArea::copyval()
{
#ifdef PASTE_DEBUG
	Log("copy vkc");
	
#endif

#if 0
	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText highl = m_value.substr(m_highl[0], m_highl[1]-m_highl[0]);
		std::string rawhighl = highl.rawstr();
		const size_t len = strlen(rawhighl.c_str())+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), rawhighl.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	else
	{
		const char* output = "";
		const size_t len = strlen(output)+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), output, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}

	//return true;
#else
	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		RichText highl = m_value.substr(m_highl[0], m_highl[1]-m_highl[0]);
		SDL_SetClipboardText( highl.rawstr().c_str() );
	}
	else
	{
		SDL_SetClipboardText( "" );
	}
#endif
}

void TextArea::pasteval()
{
#if 0
	placestr( SDL_GetClipboardText() );
#else
	unsigned int* ustr = ToUTF32( (unsigned char*)SDL_GetClipboardText() ); 
	RichText rstr = RichText(UStr(ustr));
	delete [] ustr;
	placestr( &rstr );
#endif

	if(!m_passw)
		m_value = ParseTags(m_value, &m_caret);
}

void TextArea::placechar(unsigned int k)
{
	//int len = m_value.length();

	//if(m_type == WIDGET_EDITBOX && len >= m_maxlen)
	//	return;

	//char addchar = k;

	//std::string before = m_value.substr(0, m_caret);
	//std::string after = m_value.substr(m_caret, len-m_caret);
	//m_value = before + addchar + after;

	RichText newval;

	int currplace = 0;
	bool changed = false;
	for(std::list<RichPart>::iterator i=m_value.m_part.begin(); i!=m_value.m_part.end(); i++)
	{
		if(currplace + i->texlen() >= m_caret && !changed)
		{
			changed = true;

			if(i->m_type == RICH_TEXT)
			{
				if(i->m_text.m_length <= 1)
					continue;

				RichPart chpart;

				chpart.m_type = RICH_TEXT;

				int subplace = m_caret - currplace;

				if(subplace > 0)
				{
					chpart.m_text = chpart.m_text + i->m_text.substr(0, subplace);
				}

				chpart.m_text = chpart.m_text + UStr(k);

				if(i->m_text.m_length - subplace > 0)
				{
					chpart.m_text = chpart.m_text + i->m_text.substr(subplace, i->m_text.m_length-subplace);
				}

				chpart.m_text = i->m_text.substr(0, i->m_text.m_length-1);

				newval = newval + RichText(chpart);
			}
			else if(i->m_type == RICH_ICON)
			{
				Icon* icon = &g_icon[i->m_icon];

				int subplace = m_caret - currplace;

				if(subplace <= 0)
				{
					newval = newval + RichText(RichPart(UStr(k)));
					newval = newval + RichText(*i);
				}
				else
				{
					newval = newval + RichText(*i);
					newval = newval + RichText(RichPart(UStr(k)));
				}
			}

		}
		else
		{
			newval = newval + RichText(*i);
			currplace += i->texlen();
		}
	}

	m_value = newval;

	m_caret ++;
}

void TextArea::selectall()
{
	m_highl[0] = 0;
	m_highl[1] = m_value.texlen()+1;
	m_caret = -1;

	//RichText val = drawvalue();
	int endx = EndX(&m_value, m_value.texlen(), m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[2])
		m_scroll[0] += m_pos[2] - endx - 1;

	if(m_scroll[0] >= 0)
		m_scroll[0] = 0;

	//return true;
}

void TextArea::hide()
{
	losefocus();

	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->hide();
}


void TextArea::gainfocus()
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
		r.x = (int)m_pos[0];
		r.y = (int)m_pos[3];
		r.w = g_width - (int)m_pos[0];
		r.h = g_height - (int)m_pos[3];
		SDL_SetTextInputRect(&r);
		g_kbfocus++;
	}
}

void TextArea::losefocus()
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
