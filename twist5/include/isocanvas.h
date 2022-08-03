#pragma once

#include "isogrid.h"
#include "dialog.h"
#include "color.h"
#include "scrollbox.h"
#include "mainloop.h"
#include "isomap.h"

template <class T> class IsoCanvas : public Scrollable
{
protected:
	T *map;
	IsoTexturedGrid grid;

	// std::shared_ptr<Timer> scrollTimer;
	int cursor_x;
	int cursor_y;
public:
	IsoCanvas() : map(nullptr), grid(), cursor_x(-1), cursor_y(-1)
	{
	}

	virtual ~IsoCanvas() {}

	void setMap (T *value)
	{
		map = value;
		if (map)
		{
			grid.setDimension(map->getDimMX(), map->getDimMY(), 0);
			setDimension(grid.getw(), grid.geth());
			center();
		}
	}

	void drawCursor (const GraphicsContext &gc)
	{
		if (!map) return;
		if (cursor_x >= 0 && cursor_y >= 0 &&
			cursor_x < (int)map->getDimMX() && cursor_y < (int)map->getDimMY())
		{
			int rx, ry;
			grid.canvasFromMap(cursor_x, cursor_y, &rx, &ry);
			int x = rx + gc.xofst;
			int y = ry + gc.yofst;

			al_draw_line (x, y,           x + 32, y + 15, YELLOW, 1.0);
			al_draw_line (x + 32, y + 15, x, y + 31,      YELLOW, 1.0);
			al_draw_line (x, y + 31,      x - 31, y + 15, YELLOW, 1.0);
			al_draw_line (x - 32, y + 15, x, y,           YELLOW, 1.0);
		}
	}

	virtual void MsgMousemove(const Point &d) override
	{
		int qx = d.x();
		int qy = d.y();

		cursor_x = grid.mapFromCanvasX (qx, qy);
		cursor_y = grid.mapFromCanvasY (qx, qy);
	}

};
