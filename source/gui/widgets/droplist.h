







#ifndef DROPDOWNS_H
#define DROPDOWNS_H

#include "../widget.h"

class DropList : public Widget
{
public:

	int m_mousedown[2];

	DropList();
	DropList(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)());

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);

	int rowsshown();
	int square();
	void erase(int which);

	float topratio()
	{
		return m_scroll[1] / (float)(m_options.size());
	}

	float bottomratio()
	{
		return (m_scroll[1]+rowsshown()) / (float)(m_options.size());
	}

	float scrollspace();
};

#endif
