







#ifndef LINK_H
#define LINK_H

#include "../widget.h"

class Link : public Widget
{
public:
	Link(Widget* parent, const char* name, const RichText t, int f, void (*reframef)(Widget* w), void (*click)()) : Widget()
	{
		m_parent = parent;
		m_type = WIDGET_LINK;
		m_name = name;
		m_over = false;
		m_ldown = false;
		m_text = t;
		m_font = f;
		reframefunc = reframef;
		clickfunc = click;
		reframe();
	}

	void draw();
	void inev(InEv* ie);
};

#endif
