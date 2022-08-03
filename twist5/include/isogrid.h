#pragma once

#include "color.h"
#include "component.h"
#include "isometric.h"
#include <iostream>

const int DEFAULT_TILEX = 32;
const int DEFAULT_TILEY = 32;
const int DEFAULT_TILEZ = 16;

// default tile size in texture space
const int DEFAULT_TILEU = 16;
const int DEFAULT_TILEV = 16;

// default maximum height of a map. Affects origin and clipping rectangle
const int DEFAULT_DIM_MZ = 20;

/** Used only for drawing polygons atm */
struct Coord3D {
	float x;
	float y;
	float z;
};

/*
 * implements coordinate system for a isometric grid system
 *
 * does not implement draw method, does not contain map.
 * but has handy transformation methods and drawing primitives
 */
class IsoGrid
{
protected:
	int dim_mx;
	int dim_my;
	int dim_mz;

	int tilex;
	int tiley;
	int tilez;

	/* orthogonal transform used throughout */
	ALLEGRO_TRANSFORM t;

	int rx0; // location of origin
	int ry0;

	bool autoOrigin = true;
	bool use30deg = true;

	/*
	 *
	 * coordinate systems:
	 *
	 *    rx, ry     = screen 2D coordinates
	 *    ox, oy     = rx, ry shifted to origin
	 *    ix, iy, iz = isometric pixel coordinates
	 *    cx, cy, cz / mx, my, mz = isometric grid cell coordinates
	 *    px, py, pz = isometric pixel delta against grid
	 *
	 *
	 *    xs, ys, zs = x-size, y-size, z-size
	 *
	 */

	void calculateOrigin()
	{
		if (autoOrigin)
		{
			rx0 = getw() / 2;
			ry0 = dim_mz * tilez;
			t.m[3][0] = rx0;
			t.m[3][1] = ry0;
		}
	}

public:
	IsoGrid(int sizex, int sizey, int sizez, int tilesizexy, int tilesizez) : dim_mx(sizex), dim_my(sizey), dim_mz(sizez), tilex(tilesizexy), tiley(tilesizexy), tilez(tilesizez)
	{
		// initialize with default 30 degree orthogonal projection.
		set30Deg();
		calculateOrigin();
	}

	void setOrigin (int rx0Val, int ry0Val)
	{
		rx0 = rx0Val;
		ry0 = ry0Val;
		t.m[3][0] = rx0;
		t.m[3][1] = ry0;
		autoOrigin = false;
	}

	IsoGrid() : dim_mx(0), dim_my(0), dim_mz(0), tilex(DEFAULT_TILEX), tiley(DEFAULT_TILEY), tilez(DEFAULT_TILEZ), rx0(0), ry0(0)
	{
		set30Deg();
	}

	int getSize_ix() const { return dim_mx * tilex; }
	int getSize_iy() const { return dim_my * tiley; }
	int getSize_iz() const { return dim_mz * tilez; }

	virtual ~IsoGrid() {}

//	void isoDrawWireFrame (int rx, int ry, int ixs, int iys, int izs, ALLEGRO_COLOR color)
//	{
//		drawWireFrame (rx + rx0, ry + ry0, ixs, iys, izs, color);
//	}

	/**
	 * Check that a given grid coordinate is within bounds
	 */
	bool cellInBounds(int cx, int cy, int cz) const
	{
		return
				cx >= 0 && cx < dim_mx &&
				cy >= 0 && cy < dim_my &&
				cz >= 0 && cz < dim_mz;
	}


	int getw() const { return (dim_mx + tilex + dim_my * tiley) * 2; }
	int geth() const { return (dim_mx * tilex + dim_my * tiley) / 2 + dim_mz * tilez; }


	/** distance from the cell at 0,0 to the edge of the virtual screen */
	int getXorig () const { return rx0; }

	/** distance from the cell at 0,0 to the edge of the virtual screen */
	int getYorig () const { return ry0; }

	void set30Deg() {
		use30deg = true;
		al_identity_transform(&t);

		// rx = rx0 + (x - y);
		t.m[0][0] = 1;
		t.m[1][0] = -1;
		t.m[2][0] = 0;
		t.m[3][0] = rx0;

		// ry = ry0 + (x * 0.5 + y * 0.5 - z);
		t.m[0][1] = 0.5;
		t.m[1][1] = 0.5;
		t.m[2][1] = -1;
		t.m[3][1] = ry0;

		// rz = (x + y + z);
		// TODO: not sure about accuracy here.
		t.m[0][2] = 1;
		t.m[1][2] = 1;
		t.m[2][2] = 1;
	}

	void set45Deg() {
		use30deg = false;
		al_identity_transform(&t);

		// rx = rx0 + (x - (0.5 * y));
		t.m[0][0] = 1;
		t.m[1][0] = -0.5;
		t.m[2][0] = 0;
		t.m[3][0] = rx0;

		// ry = ry0 + ((0.5 * y) - z);
		t.m[0][1] = 0;
		t.m[1][1] = 0.5;
		t.m[2][1] = -1;
		t.m[3][1] = ry0;

		// rz = (0.5 * y);
		t.m[0][2] = 0;
		t.m[1][2] = 0.5;
		t.m[2][2] = 0;
	}

	void setTransformationMatrix(ALLEGRO_TRANSFORM *val) {
		al_copy_transform(&t, val);
	}

	void canvasFromIso_f (float *x, float *y, float *z) const {
		al_transform_coordinates_3d(&t, x, y, z);
	}

	void canvasFromIso_f (float x, float y, float z, float &rx, float &ry) const
	{
		float xx = x, yy = y, zz = z;
		al_transform_coordinates_3d(&t, &xx, &yy, &zz);
		rx = xx;
		ry = yy;
	}

	// rz is useful for z-ordering sprites.
	void canvasFromIso_f (float x, float y, float z, float &rx, float &ry, float &rz) const
	{
		float xx = x, yy = y, zz = z;
		al_transform_coordinates_3d(&t, &xx, &yy, &zz);
		rx = xx;
		ry = yy;
		rz = zz;
	}

	void drawMapSurfaceWire (const GraphicsContext &gc, int mx, int my, int mz, int mxs, int mys, ALLEGRO_COLOR color) const;
	void drawMapLeftWire (const GraphicsContext &gc, int mx, int my, int mz, int mxs, int mys, ALLEGRO_COLOR color) const;
	void drawMapRightWire (const GraphicsContext &gc, int mx, int my, int mz, int mxs, int mys, ALLEGRO_COLOR color) const;

	// using map units
	void drawMapWireFrame (const GraphicsContext &gc, int mx, int my, int mz, int sx, int sy, int sz, ALLEGRO_COLOR color) const;
	// using pixel units
	void drawWireFrame (const GraphicsContext &gc, int ix, int iy, int iz, int sx, int sy, int sz, ALLEGRO_COLOR color) const;

	int getTilex() const { return tilex; }
	int getTiley() const { return tiley; }
	int getTilez() const { return tilez; }

	void setDimension(int mxVal, int myVal, int mzVal)
	{
		dim_mx = mxVal;
		dim_my = myVal;
		dim_mz = mzVal;
		calculateOrigin();
	}

	void setTileSize(int x, int y, int z)
	{
		tilex = x;
		tiley = y;
		tilez = z;
		calculateOrigin();
	}

	/** get dimension of map in isometric pixel units */
	int getDimIX() const { return dim_my * tiley; }
	int getDimIY() const { return dim_mx * tilex; }

	void canvasFromMap(float mx, float my, int *rx, int *ry) const {
		float xx = mx * tilex, yy = my * tiley, zz = 0;
		al_transform_coordinates_3d(&t, &xx, &yy, &zz);
		*rx = (int)xx;
		*ry = (int)yy;
	}

	void canvasFromMap(float mx, float my, float *rx, float *ry) const {
		float xx = mx * tilex, yy = my * tiley, zz = 0;
		al_transform_coordinates_3d(&t, &xx, &yy, &zz);
		*rx = xx;
		*ry = yy;
	}

	// assuming iz = 0...
	// used by Dr. F.
	void isoFromCanvas (float rx, float ry, float &ix, float &iy)
	{
		isoFromCanvas(0, rx, ry, ix, iy);
	}

	/**
	 * given a certain screen coordinate and a isometric z-level,
	 * calculate the isometric x, y
	 *
	 * Because a given point on the screen corresponds to many possible x,y,z points in isometric space,
	 * the caller has to choose the z coordinate (usually, in a loop trying possibilities until a good fit comes up)
	 */
	// used by Usagi
	void isoFromCanvas (float iz, int rx, int ry, float &ix, float &iy)
	{
		int ox = rx - rx0;
		int oy = ry - ry0;
		if (use30deg) {
			ix = oy + (ox / 2) + (iz / 2);
			iy = oy - (ox / 2) + (iz / 2);
		}
		else {
			//TODO: formula not yet 100% tested
			ix = iz + ox + oy;
			iy = 2 * (iz + oy);
		}
	}

	//TODO: currently only works for standard deg30 transform
	int mapFromCanvasX (int x, int y) const
	{
		return ((x - rx0) / 2 + (y - ry0)) / tilex;
	}

	//TODO: currently only works for standard deg30 transform
	int mapFromCanvasY (int x, int y) const
	{
		return  (y - ry0 - (x - rx0) / 2) / tiley;
	}

	/*
	 * draw a polygon consisting of num lines, between all consecutive points in coords, closing the path back to the beginning.
	 * The coordinates WILL BE CHANGED IN PLACE by this function
	 */
	void drawIsoPoly (const GraphicsContext &gc, int num, Coord3D *coords, ALLEGRO_COLOR color) const;
};
