







#ifndef VSCROLLBAR_H
#define VSCROLLBAR_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"

class VScroll : public Widget
{
public:

	struct ScrollEv
	{
		float delta;
		float newpos;
	};

	//m_scroll[1] is the ratio of the current position of the top of the scroll bar. mustn't go up to 1 because area below is covered by m_domain (subtract it from total 1.0).
	float m_domain;	//ratio of how much of the scroll space the current view covers
	float m_barpos[4];
	float m_uppos[4];
	float m_downpos[4];
	int m_mousedown[2];
	bool m_ldownbar;
	bool m_ldownup;
	bool m_ldowndown;

	VScroll();
	VScroll(Widget* parent, const char* n);

	void inev(InEv* ie);
	virtual void frameupd();
	void draw();
	void reframe();
};

#endif
