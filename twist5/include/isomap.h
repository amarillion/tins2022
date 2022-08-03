#pragma once

#include "isogrid.h"
#include <allegro5/allegro_primitives.h>
#include "map2d.h"
#include "isogrid.h"

/**
 * Draw a surface.
 *
 * z is the height, in tile units, of the top corner.
 *
 * dzleft, dzright and dzbot are the z-delta, in tile units, of the left,
 * right and bottom corners
 */
struct Cell
{
	int z;
	short dzleft;
	short dzright;
	short dzbot;
	Cell() { z = 0; dzleft= 0; dzright = 0; dzbot = 0; }


	bool isVerticalSplit() const
	{
		return dzbot == 0;
	}

	// lift corner of a single tile
	void liftCorner(int delta, int side)
	{
		switch (side)
		{
		case 0:
			z += delta;
			dzleft -= delta;
			dzright -= delta;
			dzbot -= delta;
			break;
		case 1:
			dzright += delta;
			break;
		case 2:
			dzbot += delta;
			break;
		case 3:
			dzleft += delta;
			break;
		default:
			assert (false);
			break;
		}
	}

	void setCorner(int value, int side)
	{
		int delta = value - z;
		switch (side)
		{
		case 0:
			z += delta;
			dzleft -= delta;
			dzright -= delta;
			dzbot -= delta;
			break;
		case 1:
			dzright = delta;
			break;
		case 2:
			dzbot = delta;
			break;
		case 3:
			dzleft = delta;
			break;
		default:
			assert (false);
			break;
		}
	}

	// return the z of the highest corner.
	int getMaxHeight()
	{
		return std::max(z, std::max (z + dzbot, std::max (z + dzleft, z + dzright)));
	}

	// return the z of the lowest corner
	int getMinHeight()
	{
		return std::min(z, std::min (z + dzbot, std::min (z + dzleft, z + dzright)));
	}

	bool isFlat()
	{
		return (dzbot == 0 && dzleft == 0 && dzright == 0);
	}

};

/**
 * Extension of Map2D<> with some helpful hill-map manipulation functions
 *
 * any T is expected to implement the same functions as Cell.
 */
template <class T>
class IsoMap : public Map2D<T> {

public:
	IsoMap() : Map2D<T>() {}
	IsoMap(int w, int h) : Map2D<T>(w, h) {}

	void setPointAtLeast(int mx, int my, int value)
	{
		if (!this->inBounds(mx, my)) return;

		if (this->get(mx, my).z >= value) return;
		setPoint (mx, my, value);
	}

	void setPoint(int mx, int my, int value)
	{
		if (!this->inBounds(mx, my)) return;

		this->get(mx    ,     my).setCorner(value, 0);

		if (mx >= 1)
		{
			this->get(mx - 1,     my).setCorner(value, 1);
		}

		if (my >= 1 && mx >= 1)
		{
			this->get(mx - 1, my - 1).setCorner(value, 2);
		}

		if (my >= 1)
		{
			this->get(mx    , my - 1).setCorner(value, 3);
		}
	}

	// recursive version of setPointAtLeast
	void setHillAtLeast(int mx, int my, int value)
	{
		if (!this->inBounds(mx, my)) return;

		if (this->get(mx, my).z >= value) return;
		setPointAtLeast (mx, my, value);

		if (value > 0)
		{
			setHillAtLeast (mx + 1, my, value - 1);
			setHillAtLeast (mx, my + 1, value - 1);
			setHillAtLeast (mx - 1, my, value - 1);
			setHillAtLeast (mx, my - 1, value - 1);
		}
	}

	// lift all four corners around a point
	void raisePoint(int mx, int my, int delta)
	{
		this->get(mx    ,     my).liftCorner(delta, 0);
		this->get(mx - 1,     my).liftCorner(delta, 1);
		this->get(mx - 1, my - 1).liftCorner(delta, 2);
		this->get(mx    , my - 1).liftCorner(delta, 3);
	}

};

/**
 * Like IsoGrid but with some extra drawing functions,
 * and deals with a tiled texture.
 */
class IsoTexturedGrid : public IsoGrid {
private:
	// maximum height
	int sizez = 20;

	// dimensions of a texture tile
	int tileu = DEFAULT_TILEU;
	int tilev = DEFAULT_TILEV;

	// number of tiles in a row.
	int tilesPerRow = 1;
	ALLEGRO_BITMAP *tiles = nullptr;
public:

	ALLEGRO_BITMAP *getTiles() const { return tiles; }
	/**
	 * @param u: width of a tile in texture pixels
	 * @param v: height of a tile in texture pixels
	 */
	void setTexture(ALLEGRO_BITMAP *_tiles, int u, int v)
	{
		tileu = u;
		tilev = v;
		tiles = _tiles;
		tilesPerRow = al_get_bitmap_width(tiles) / u;
	}

public:
	IsoTexturedGrid(int sizex, int sizey, int sizez, int tilesizexy, int tilesizez) : IsoGrid(sizex, sizey, sizez, tilesizexy, tilesizez) {
	}

	IsoTexturedGrid() : IsoGrid() {}

	// Lots of drawing helper functions...

	void drawSurface(const GraphicsContext &gc, int mx, int my, const Cell &c);
	void drawLeftWall(const GraphicsContext &gc, int mx, int my, const Cell &c);
	void drawRightWall(const GraphicsContext &gc, int mx, int my, const Cell &c);
	void drawIsoPoly (const GraphicsContext &gc, int num, int x[], int y[], int z[], ALLEGRO_COLOR color);
	void drawSurfaceTile(const GraphicsContext &gc, int mx, int my, const Cell &c, int tilei, ALLEGRO_VERTEX *coord);
};

/**
 * Combines a grid, map and some drawing helper functions
 */
class IsoView
{
protected:

public:
	IsoTexturedGrid grid;
	IsoMap<Cell> map;

	IsoView() : grid(0, 0, 0, DEFAULT_TILEX, DEFAULT_TILEZ), map(0, 0) {}
	IsoView(int w, int h, int tilexVal = DEFAULT_TILEX, int tileyVal = DEFAULT_TILEY, int tilezVal = DEFAULT_TILEZ) :
		grid(w, h, DEFAULT_DIM_MZ, tilexVal, tilezVal), map(w, h) {}

	double getSurfaceIsoz(double ix, double iy);
};

void drawMap(const GraphicsContext &gc, IsoView *view);
