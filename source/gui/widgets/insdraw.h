







#ifndef INSDRAW_H
#define INSDRAW_H

#include "../widget.h"

class InsDraw : public Widget
{
public:
	InsDraw();
	InsDraw(Widget* parent, void (*inst)());

	void draw();
};

#endif
