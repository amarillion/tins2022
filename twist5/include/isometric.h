#pragma once

#include <allegro5/allegro.h>
#include <assert.h>
#include <math.h>
#include "graphicscontext.h"
#include <algorithm>

extern float LIGHTX;
extern float LIGHTY;
extern float LIGHTZ;

/**
 * The return value is between -1 and 1.
 */
float surfaceLighting(float x1, float y1, float z1, float x2, float y2, float z2);
ALLEGRO_COLOR litColor (ALLEGRO_COLOR color, float light);

/**
	draw wire model
	to help testing

	the wireFrame fills the cube from iso (0,0,0) to iso (sx, sy, sz),
	with iso (0, 0, 0) positioned at screen (rx, ry)
*/

// assume z == 0
void screenToIso (int rx, int ry, float &x, float &y);

/* transform x, y and z from isometric to screen coordinates and put them in rx, ry */
void isoToScreen (float x, float y, float z, int &rx, int &ry);

/* same as isoToScreen but with floats */
void isoToScreen_f (float x, float y, float z, float &rx, float &ry);
