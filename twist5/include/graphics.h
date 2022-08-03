#pragma once

#include "component.h"

/**
 * Turns a series of drawing commands into a persistent object
 */
class Graphics : public Component {
public:
	virtual void filledRectangle(int x, int y, int w, int h, const ALLEGRO_COLOR &col) = 0;
	static std::shared_ptr<Graphics> create();
};
