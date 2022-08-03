#pragma once

#include "motionimpl.h"
#include "container.h"

template <typename W>
ComponentBuilder<W> & ComponentBuilder<W>::xywh(double x, double y, double w, double h)
{
	component->setx(x);
	component->sety(y);
	component->setDimension(w, h);
	component->setLayout(Layout::DISABLED, 0, 0, 0, 0);
	return *this;
}

template <typename W>
ComponentBuilder<W> &ComponentBuilder<W>::motion(IMotionPtr motion)
{
	component->setMotion(motion);
	return *this;
}

template <typename W>
ComponentBuilder<W> &ComponentBuilder<W>::linear(int x, int y)
{
	IMotionPtr temp = IMotionPtr(new Linear (x, y));
	component->setMotion(temp);
	return *this;
}

template <typename W>
ComponentBuilder<W> & ComponentBuilder<W>::center(int prefw, int prefh)
{
	//TODO: automatically fill in preferred width & height
	component->setLayout(Layout::CENTER_MIDDLE_W_H, 0, 0, prefw, prefh);
	return *this;
}

template <typename W>
ComponentBuilder<W> & ComponentBuilder<W>::xy(double x, double y)
{
	component->setxy(x, y);
	return *this;
}

template <typename W>
ComponentBuilder<W> & ComponentBuilder<W>::layout(int flags, int x1, int y1, int x2, int y2)
{
	component->setLayout(flags, x1, y1, x2, y2);
	return *this;
}

template <typename W>
ComponentBuilder<W> & ComponentBuilder<W>::font(ALLEGRO_FONT *font)
{
	component->setFont(font);
	return *this;
}
