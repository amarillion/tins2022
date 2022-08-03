#include "isogrid.h"
#include <allegro5/allegro_primitives.h>

void IsoGrid::drawMapSurfaceWire (const GraphicsContext &gc, int mx, int my, int mz, int mxs, int mys, ALLEGRO_COLOR color) const {

	Coord3D points[4] {
		{ (float)tilex * mx, (float)tiley * my, (float)tilez * mz },
		{ (float)tilex * (mx + mxs), (float)tiley * my, (float)tilez * mz },
		{ (float)tilex * (mx + mxs), (float)tiley * (my + mys), (float)tilez * mz },
		{ (float)tilex * mx, (float)tiley * (my + mys), (float)tilez * mz }
	};

	drawIsoPoly(gc, 4, points, color);
}

void IsoGrid::drawMapRightWire (const GraphicsContext &gc, int mx, int my, int mz, int mxs, int mzs, ALLEGRO_COLOR color) const {

	Coord3D points[4] {
		{ (float)tilex * mx, (float)tiley * my, (float)tilez * mz },
		{ (float)tilex * (mx + mxs), (float)tiley * my, (float)tilez * mz },
		{ (float)tilex * (mx + mxs), (float)tiley * my, (float)tilez * (mz + mzs) },
		{ (float)tilex * mx, (float)tiley * my, (float)tilez * (mz + mzs) }
	};

	drawIsoPoly(gc, 4, points, color);
}

void IsoGrid::drawMapLeftWire (const GraphicsContext &gc, int mx, int my, int mz, int mys, int mzs, ALLEGRO_COLOR color) const {

	Coord3D points[4] {
		{ (float)tilex * mx, (float)tiley * my, (float)tilez * mz },
		{ (float)tilex * mx, (float)tiley * (my + mys), (float)tilez * mz },
		{ (float)tilex * mx, (float)tiley * (my + mys), (float)tilez * (mz + mzs) },
		{ (float)tilex * mx, (float)tiley * my, (float)tilez * (mz + mzs) }
	};

	drawIsoPoly(gc, 4, points, color);
}

void IsoGrid::drawMapWireFrame (const GraphicsContext &gc, int mx, int my, int mz, int msx, int msy, int msz, ALLEGRO_COLOR color) const
{
	drawWireFrame (gc, mx * tilex, my * tiley, mz * tilez,
			msx * tilex, msy * tiley, msz * tilez, color);
}

void IsoGrid::drawWireFrame (const GraphicsContext &gc, int ix, int iy, int iz, int isx, int isy, int isz, ALLEGRO_COLOR color) const
{
	float drx[7];
	float dry[7];

	int x2 = ix + isx;
	int y2 = iy + isy;
	int z2 = iz + isz;

	canvasFromIso_f(ix, iy, z2, drx[0], dry[0]);
	canvasFromIso_f(x2, iy, z2, drx[1], dry[1]);
	canvasFromIso_f(ix, y2, z2, drx[2], dry[2]);
	canvasFromIso_f(x2, y2, z2, drx[3], dry[3]);
	canvasFromIso_f(x2, iy, iz, drx[4], dry[4]);
	canvasFromIso_f(ix, y2, iz, drx[5], dry[5]);
	canvasFromIso_f(x2, y2, iz, drx[6], dry[6]);

	al_draw_line (gc.xofst + drx[0], gc.yofst + dry[0], gc.xofst + drx[1], gc.yofst + dry[1], color, 1.0);
	al_draw_line (gc.xofst + drx[0], gc.yofst + dry[0], gc.xofst + drx[2], gc.yofst + dry[2], color, 1.0);
	al_draw_line (gc.xofst + drx[3], gc.yofst + dry[3], gc.xofst + drx[1], gc.yofst + dry[1], color, 1.0);
	al_draw_line (gc.xofst + drx[3], gc.yofst + dry[3], gc.xofst + drx[2], gc.yofst + dry[2], color, 1.0);
	al_draw_line (gc.xofst + drx[3], gc.yofst + dry[3], gc.xofst + drx[6], gc.yofst + dry[6], color, 1.0);
	al_draw_line (gc.xofst + drx[1], gc.yofst + dry[1], gc.xofst + drx[4], gc.yofst + dry[4], color, 1.0);
	al_draw_line (gc.xofst + drx[2], gc.yofst + dry[2], gc.xofst + drx[5], gc.yofst + dry[5], color, 1.0);
	al_draw_line (gc.xofst + drx[5], gc.yofst + dry[5], gc.xofst + drx[6], gc.yofst + dry[6], color, 1.0);
	al_draw_line (gc.xofst + drx[4], gc.yofst + dry[4], gc.xofst + drx[6], gc.yofst + dry[6], color, 1.0);
}

void IsoGrid::drawIsoPoly (const GraphicsContext &gc, int num, Coord3D *coords, ALLEGRO_COLOR color) const {
	for (int i = 0; i < num; ++i) {
		canvasFromIso_f(&coords[i].x, &coords[i].y, &coords[i].z);
	}

	int prev = num - 1;
	for (int i = 0; i < num; ++i) {
		al_draw_line (
				gc.xofst + coords[prev].x, gc.yofst + coords[prev].y,
				gc.xofst + coords[i].x,    gc.yofst + coords[i].y,
				color, 1.0);
		prev = i;
	}
}
