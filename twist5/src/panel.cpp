#include "panel.h"

void Panel::doDraw(const GraphicsContext &gc)
{
	if (border)
	{
		Bitmap dst = Bitmap(gc.buffer);
		border.TiledBlit(dst, 0, 0, getx() + gc.xofst, gety() + gc.yofst, getw(), geth(), 1, 1);
	}
}
