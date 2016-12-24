







#include "widget.h"
#include "gui.h"
#include "font.h"
#include "../window.h"
#include "icon.h"

void Widget::hideall()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->hide();
}

void Widget::hide(const char* name)
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
		{
			(*i)->hide();
		}
}

void Widget::show(const char* name)
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
		{
			(*i)->show();
			break;	//important - list may shift after show() and tofront() call
		}
}

void Widget::frameupd()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();
}

void Widget::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

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

#if 0	//only use when add windows widgets, and then fix parent bounds of "zoom text" and "max elev" labels
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
#endif

	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();
}

void Widget::draw()
{
	for(std::list<Widget*>::iterator wit=m_subwidg.begin(); wit!=m_subwidg.end(); wit++)
	{
		Widget* w = *wit;

		if(w->m_hidden)
			continue;

		w->draw();
	}
}

void Widget::drawover()
{
	for(std::list<Widget*>::iterator wit=m_subwidg.begin(); wit!=m_subwidg.end(); wit++)
	{
		Widget* w = *wit;

		if(w->m_hidden)
			continue;

		w->drawover();
	}
}

void Widget::inev(InEv* ie)
{
	bool intercepted = ie->intercepted;
	
	//safe, may shift during call
	for(std::list<Widget*>::reverse_iterator wit=m_subwidg.rbegin(); wit!=m_subwidg.rend();)
	{
		Widget* w = *wit;
		wit++;	//safe, may shift during call

		if(w->m_hidden)
			continue;
		
		w->inev(ie);
		
		if(ie->intercepted != intercepted)
		{
			intercepted = ie->intercepted;
			wit = m_subwidg.rbegin();
		}
	}
}

void Widget::tofront()
{
	//return;

	if(!m_parent)
		return;

	//return;

	std::list<Widget*>* subs = &m_parent->m_subwidg;

	for(std::list<Widget*>::iterator wi=subs->begin(); wi!=subs->end(); wi++)
	{
		if(*wi == this)
		{
			subs->erase(wi);
			subs->push_back(this);
			break;
		}
	}
}

void CenterLabel(Widget* w)
{
	BmpFont* f = &g_font[w->m_font];

	int texwidth = TextWidth(w->m_font, &w->m_label);

#if 0
	char msg[128];
	sprintf(msg, "tw %d, tl %d, fn %d, gh %f", texwidth, w->m_label.texlen(), w->m_font, f->gheight);
	if(g_netmode == NETM_CLIENT)
	InfoMess(msg, msg);
#endif

	w->m_tpos[0] = (w->m_pos[2]+w->m_pos[0])/2 - texwidth/2;
	w->m_tpos[1] = (w->m_pos[3]+w->m_pos[1])/2 - f->gheight/2;
}

Widget* Widget::get(const char* name)
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if(stricmp((*i)->m_name.c_str(), name) == 0)
			return *i;

	return NULL;
}

void Widget::add(Widget* neww)
{
	if(!neww)
		OutOfMem(__FILE__, __LINE__);

	m_subwidg.push_back(neww);
}

void Widget::gainfocus()
{
}

//TODO lose focus win blview members edit boxes

void Widget::losefocus()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->losefocus();
}

void Widget::hide()
{
	m_hidden = true;
	losefocus();
	
	//for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
	//	(*i)->hide();
}

void Widget::show()
{
	m_hidden = false;
	//necessary for window widgets:
	//tofront();	//can't break list iterator, might shift

	//for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
	//	(*i)->show();
}

void Widget::chcall(Widget* ch, int type, void* data)
{
}

//free subwidget children
void Widget::freech()
{
	std::list<Widget*>::iterator witer = m_subwidg.begin();
	while(witer != m_subwidg.end())
	{
		delete *witer;
		witer = m_subwidg.erase(witer);
	}
}

void SubCrop(float *src1, float *src2, float *dest)
{
	dest[0] = fmax(src1[0], src2[0]);
	dest[1] = fmax(src1[1], src2[1]);
	dest[2] = fmin(src1[2], src2[2]);
	dest[3] = fmin(src1[3], src2[3]);

	//purposely inverted frame means it's out of view
	
	//if(ndest[0] > ndest[2])
	//	ndest[0] = ndest[2]+1.0f;

	//if(ndest[1] > ndest[3])
	//	ndest[1] = ndest[3]+1.0f;
}