#include "isometric.h"
#include "color.h"
#include <allegro5/allegro_primitives.h>

float LIGHTX = 0.2;
float LIGHTY = -0.8;
float LIGHTZ = -0.6;

// copied from allegro4
float dot_product_f (float x1, float y_1, float z1, float x2, float y2, float z2)
{
   return (x1 * x2) + (y_1 * y2) + (z1 * z2);
}

/* cross_productf:
 *  Floating point version of cross_product().
 */
// copied from allegro4
void cross_product_f(float x1, float y1, float z1, float x2, float y2, float z2, float *xout, float *yout, float *zout)
{
   assert(xout);
   assert(yout);
   assert(zout);

   *xout = (y1 * z2) - (z1 * y2);
   *yout = (z1 * x2) - (x1 * z2);
   *zout = (x1 * y2) - (y1 * x2);
}


/**
 * The return value is between -1 and 1.
 */
float surfaceLighting(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float norm[3]; // normal of the plane defined by the two vectors
	cross_product_f (x1, y1, z1, x2, y2, z2, &norm[0], &norm[1], &norm[2]);
	return cos (dot_product_f (LIGHTX, LIGHTY, LIGHTZ, norm[0], norm[1], norm[2]));
}

ALLEGRO_COLOR litColor (ALLEGRO_COLOR color, float light)
{
	float light2 = light < 0.2 ? 0.2 : light;
	light2 = 0.5 + (light2 / 2);

	unsigned char rr, gg, bb;
	al_unmap_rgb(color, &rr, &gg, &bb);

	float r = light2 * rr;
	float g = light2 * gg;
	float b = light2 * bb;
	return al_map_rgb (r, g, b);
}

void isoToScreen (float x, float y, float z, int &rx, int &ry)
{
	rx = (int)(x - y);
	ry = (int)(x * 0.5 + y * 0.5 - z);
}

void isoToScreen_f (float x, float y, float z, float &rx, float &ry)
{
	rx = (x - y);
	ry = (x * 0.5 + y * 0.5 - z);
}

void screenToIso (int rx, int ry, float &x, float &y)
{
	x = ry + rx / 2;
	y = ry - rx / 2;
}
