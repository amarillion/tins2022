#include "objectbase.h"
#include "color.h"
#include "anim.h"
#include <allegro5/allegro_primitives.h>

using namespace std;

bool *ObjectBase::debugFlag = NULL;
ITimer *ObjectBase::timer = NULL;

void ObjectBase::draw (const GraphicsContext &gc)
{
	if (current)
	{
		int counter = timer->getMsecCounter();
		current->drawFrame(animstate, dir, counter - animstart, x - gc.xofst, y - gc.yofst);
	}
	if (debugFlag && (*debugFlag))
	{
		al_set_target_bitmap (gc.buffer);
		al_draw_rectangle (
			x - gc.xofst,
			y - gc.yofst,
			x - gc.xofst + w,
			y - gc.yofst + h,
			GREEN, 1.0);
	}

}

void ObjectBase::setAnim (Anim *a, int state)
{
	assert (timer);
	current = a;
	animstart = timer->getMsecCounter();
	animstate = state;
	w = a->sizex;
	h = a->sizey;
}

void ObjectBase::setLocation (double nx, double ny)
{
	x = nx;
	y = ny;
}
