







#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include "widget.h"

class ViewLayer : public Widget
{
public:

	ViewLayer() : Widget()
	{
		m_name = "";
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = NULL;
		m_hidden = true;
	}

	ViewLayer(Widget* parent, const char* n) : Widget()
	{
		m_name = n;
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = parent;
		m_hidden = true;
		
		reframe();
	}

	virtual void reframe();
	virtual void show();
};

#endif
