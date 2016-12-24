







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
#include "frame.h"
#include "../../platform.h"

Frame::Frame(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_FRAME;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	reframe();
}

void Frame::draw()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->draw();
}

void Frame::drawover()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->drawover();
}

void Frame::frameupd()
{
	for(std::list<Widget*>::reverse_iterator i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->frameupd();
}

void Frame::inev(InEv* ie)
{
	for(std::list<Widget*>::reverse_iterator i=m_subwidg.rbegin(); i!=m_subwidg.rend(); i++)
		(*i)->inev(ie);
}
