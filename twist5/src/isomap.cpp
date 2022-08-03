#include "isomap.h"
#include <limits>

void drawMap(const GraphicsContext &gc, IsoView *isoview)
{
	for (size_t mx = 0; mx < isoview->map.getDimMX(); ++mx)
		for (size_t my = 0; my < isoview->map.getDimMY(); ++my)
		{
			Cell &c = isoview->map.get(mx, my);

			isoview->grid.drawSurface (gc, mx, my, c);
			isoview->grid.drawLeftWall (gc, mx, my, c);
			isoview->grid.drawRightWall (gc, mx, my, c);
		}

}

void IsoTexturedGrid::drawSurface(const GraphicsContext &gc, int mx, int my, const Cell &c)
{
	ALLEGRO_VERTEX coord[6]; // hold memory for coordinates

	memset(&coord, 0, sizeof(ALLEGRO_VERTEX) * 6);

	// ALLEGRO_COLOR baseColor = al_map_rgb (192, 255, 192);
	ALLEGRO_COLOR baseColor = al_map_rgb (255, 255, 255);

	// TODO hard coded tile indices for use in krampushack for now...
	int tilei = (c.z + c.dzbot > 2 || c.z > 2 || c.z + c.dzright > 2 || c.z + c.dzleft > 2) ? 2 : 1;

	int ubase = tileu * tilei;
	int vbase = 0;

	canvasFromIso_f (tilex * mx,
					tiley * my,
					tilez * c.z,
					coord[0].x, coord[0].y);
	coord[0].u = ubase;
	coord[0].v = vbase;

	canvasFromIso_f (	tilex * (mx + 1),
					tiley * my,
					tilez * (c.z + c.dzright),
					coord[1].x, coord[1].y);
	coord[1].u = ubase + tileu;
	coord[1].v = vbase;

	canvasFromIso_f (	tilex * mx,
					tiley * (my + 1),
					tilez * (c.z + c.dzleft),
					coord[5].x, coord[5].y);
	coord[5].u = ubase;
	coord[5].v = vbase + tilev;

	canvasFromIso_f (	tilex * (mx + 1),
					tiley * (my + 1),
					tilez * (c.z + c.dzbot),
					coord[3].x, coord[3].y);

	coord[3].u = ubase + tileu;
	coord[3].v = vbase + tilev;

	ALLEGRO_COLOR color1, color2;

/*
*
*
*    y   A
*   /   / \   x
*  +   /   \   \
*     C     B   +
*      \   /
*       \ /
*        D
*
*
*   Coordinate array
*
*   0 1 2   3 4 5
*   A B -   D - C
* - A B D   D A C   ->  vertical split
* - A B C   D B C   ->  horizontal split
*/
	if (c.isVerticalSplit())
	{
		memcpy (&coord[4], &coord[0], sizeof(ALLEGRO_VERTEX));
		memcpy (&coord[2], &coord[3], sizeof(ALLEGRO_VERTEX));

		/*
		 *
		 *
		 *    y   A
		 *   /   /|\   x
		 *  +   / | \   \
		 *     C  |  B   +
		 *      \ | /
		 *       \|/
		 *        D
		 *
		*/
		// lighting for A-B-D
		color1 = litColor (baseColor,
					surfaceLighting (-1, 0, -c.dzright, 0, 1, c.dzbot - c.dzright) );
		// lighting for A-D-C
		color2 = litColor (baseColor,
						surfaceLighting (0, -1, -c.dzleft, 1, 0, c.dzbot - c.dzleft) );
	}
	else
	{
		/*
		 *
		 *
		 *    y   A
		 *   /   / \   x
		 *  +   /   \   \
		 *     C-----B   +
		 *      \   /
		 *       \ /
		 *        D
		 *
		*/

		memcpy (&coord[4], &coord[1], sizeof(ALLEGRO_VERTEX));
		memcpy (&coord[2], &coord[5], sizeof(ALLEGRO_VERTEX));
		// lighting for A-B-C
		color1 = litColor (baseColor,
						surfaceLighting (1, 0, c.dzright, 0, 1, c.dzleft) );

		// lighting for C-B-D
		color2 = litColor (baseColor,
					surfaceLighting (0, -1, c.dzright - c.dzbot, -1, 0, c.dzleft - c.dzbot) );
	}

	for (int i = 0; i < 6; ++i)
	{
		coord[i].x += gc.xofst;
		coord[i].y += gc.yofst;
	}


	for (int i = 0; i < 3; ++i)
	{
		coord[i].color = color1;
	}


	for (int i = 3; i < 6; ++i)
	{
		coord[i].color = color2;
	}

	al_draw_prim(coord, NULL, tiles, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);

	/*
	// debugging help for interpolation
	for (int xx = 0; xx < 8; xx ++)
	{
		for (int yy = 0; yy < 8; yy ++)
		{
			float jx = (mx * tilex) + (xx * tilex / 8);
			float jy = (my * tiley) + (yy * tiley / 8);
			float jz = getSurfaceIsoz(jx, jy);
			float rx, ry;
			canvasFromIso_f(jx, jy, jz, rx, ry);

			al_draw_filled_circle(rx + gc.xofst, ry + gc.yofst, 2.0, MAGENTA);
		}
	}
	*/

}

// get the iso z coordinate in pixels, at a given isometric x, y coordinate
double IsoView::getSurfaceIsoz(double ix, double iy)
{
	// cell
	int mx = ix / grid.getTilex();
	int remx = ix - (mx * grid.getTilex());

	int my = iy / grid.getTiley();
	int remy = iy - (my * grid.getTiley());

	if (!map.inBounds(mx, my)) return -std::numeric_limits<double>::infinity();

	Cell &c = map.get(mx, my);

	double result = 0;
	// interpolate

	if (c.isVerticalSplit())
	{
		// NOTE: this comparison assumes grid.getTilex() == grid.getTiley()
		if (remx < remy)
		{
			// left half
			result = c.z * grid.getTilez();
			result += remx * (- c.dzleft + c.dzbot) * grid.getTilez() / grid.getTilex();
			result += remy * c.dzleft * grid.getTilez() / grid.getTiley();
		}
		else
		{
			// right half
			result = c.z * grid.getTilez();
			result += remx * c.dzright * grid.getTilez() / grid.getTilex();
			result += remy * (- c.dzright + c.dzbot) * grid.getTilez() / grid.getTiley();
		}
	}
	else
	{
		// NOTE: this comparison assumes grid.getTilex() == grid.getTiley()
		if (remx + remy < grid.getTilex())
		{
			// top half
			result = c.z * grid.getTilez();
			result += remx * c.dzright * grid.getTilez() / grid.getTilex();
			result += remy * c.dzleft * grid.getTilez() / grid.getTiley();
		}
		else
		{
			// bottom half
			result = (c.z + c.dzbot) * grid.getTilez();
			result += (grid.getTilex() - remx) * (- c.dzbot + c.dzleft) * grid.getTilez() / grid.getTilex();
			result += (grid.getTiley() - remy) * (- c.dzbot + c.dzright) * grid.getTilez() / grid.getTiley();
		}
	}

	return result;
}

void IsoTexturedGrid::drawSurfaceTile(const GraphicsContext &gc, int mx, int my, const Cell &c, int tilei, ALLEGRO_VERTEX *coord)
{
	// ALLEGRO_COLOR baseColor = al_map_rgb (192, 255, 192);
	ALLEGRO_COLOR baseColor = al_map_rgb (255, 255, 255);

	int ubase = tileu * (tilei % tilesPerRow);
	int vbase = tilev * (tilei / tilesPerRow);

	canvasFromIso_f (tilex * mx,
					tiley * my,
					tilez * c.z,
					coord[0].x, coord[0].y);
	coord[0].u = ubase;
	coord[0].v = vbase;

	canvasFromIso_f (	tilex * (mx + 1),
					tiley * my,
					tilez * (c.z + c.dzright),
					coord[1].x, coord[1].y);
	coord[1].u = ubase + tileu;
	coord[1].v = vbase;

	canvasFromIso_f (	tilex * mx,
					tiley * (my + 1),
					tilez * (c.z + c.dzleft),
					coord[5].x, coord[5].y);
	coord[5].u = ubase;
	coord[5].v = vbase + tilev;

	canvasFromIso_f (	tilex * (mx + 1),
					tiley * (my + 1),
					tilez * (c.z + c.dzbot),
					coord[3].x, coord[3].y);

	coord[3].u = ubase + tileu;
	coord[3].v = vbase + tilev;

	ALLEGRO_COLOR color1, color2;

/*
*
*
*    y   A
*   /   / \   x
*  +   /   \   \
*     C     B   +
*      \   /
*       \ /
*        D
*
*
*   Coordinate array
*
*   0 1 2   3 4 5
*   A B -   D - C
* - A B D   D A C   ->  vertical split
* - A B C   D B C   ->  horizontal split
*/
	if (c.isVerticalSplit())
	{
		memcpy (&coord[4], &coord[0], sizeof(ALLEGRO_VERTEX));
		memcpy (&coord[2], &coord[3], sizeof(ALLEGRO_VERTEX));

		/*
		 *
		 *
		 *    y   A
		 *   /   /|\   x
		 *  +   / | \   \
		 *     C  |  B   +
		 *      \ | /
		 *       \|/
		 *        D
		 *
		*/
		// lighting for A-B-D
		color1 = litColor (baseColor,
					surfaceLighting (-1, 0, -c.dzright, 0, 1, c.dzbot - c.dzright) );
		// lighting for A-D-C
		color2 = litColor (baseColor,
						surfaceLighting (0, -1, -c.dzleft, 1, 0, c.dzbot - c.dzleft) );
	}
	else
	{
		/*
		 *
		 *
		 *    y   A
		 *   /   / \   x
		 *  +   /   \   \
		 *     C-----B   +
		 *      \   /
		 *       \ /
		 *        D
		 *
		*/

		memcpy (&coord[4], &coord[1], sizeof(ALLEGRO_VERTEX));
		memcpy (&coord[2], &coord[5], sizeof(ALLEGRO_VERTEX));
		// lighting for A-B-C
		color1 = litColor (baseColor,
						surfaceLighting (1, 0, c.dzright, 0, 1, c.dzleft) );

		// lighting for C-B-D
		color2 = litColor (baseColor,
					surfaceLighting (0, -1, c.dzright - c.dzbot, -1, 0, c.dzleft - c.dzbot) );
	}

	for (int i = 0; i < 6; ++i)
	{
		coord[i].x += gc.xofst;
		coord[i].y += gc.yofst;
	}


	for (int i = 0; i < 3; ++i)
	{
		coord[i].color = color1;
	}


	for (int i = 3; i < 6; ++i)
	{
		coord[i].color = color2;
	}

	/*
	// debugging help for interpolation
	for (int xx = 0; xx < 8; xx ++)
	{
		for (int yy = 0; yy < 8; yy ++)
		{
			float jx = (mx * tilex) + (xx * tilex / 8);
			float jy = (my * tiley) + (yy * tiley / 8);
			float jz = getSurfaceIsoz(jx, jy);
			float rx, ry;
			canvasFromIso_f(jx, jy, jz, rx, ry);

			al_draw_filled_circle(rx + gc.xofst, ry + gc.yofst, 2.0, MAGENTA);
		}
	}
	*/
}

void IsoTexturedGrid::drawLeftWall(const GraphicsContext &gc, int mx, int my, const Cell &c)
{
	int x[4];
	int y[4];
	int z[4];

	x[0] = tilex * (mx + 1);
	y[0] = tiley * (my + 1);
	z[0] = 0;

	x[1] = tilex * mx;
	y[1] = tiley * (my + 1);
	z[1] = 0;

	x[2] = tilex * mx;
	y[2] = tiley * (my + 1);
	z[2] = tilez * (c.z + c.dzleft);

	x[3] = tilex * (mx + 1);
	y[3] = tiley * (my + 1);
	z[3] = tilez * (c.z + c.dzbot);

	ALLEGRO_COLOR color = litColor (al_map_rgb (192, 192, 192),
				surfaceLighting (0, 1, 0, 0, 0, 1 ));

	drawIsoPoly(gc, 4, x, y, z, color);
}

void IsoTexturedGrid::drawRightWall(const GraphicsContext &gc, int mx, int my, const Cell &c)
{
	int x[4];
	int y[4];
	int z[4];

	x[0] = tilex * (mx + 1);
	y[0] = tiley * my;
	z[0] = 0;

	x[1] = tilex * (mx + 1);
	y[1] = tiley * (my + 1);
	z[1] = 0;

	x[2] = tilex * (mx + 1);
	y[2] = tiley * (my + 1);
	z[2] = tilez * (c.z + c.dzbot);

	x[3] = tilex * (mx + 1);
	y[3] = tiley * my;
	z[3] = tilez * (c.z + c.dzright);

	ALLEGRO_COLOR color = litColor (al_map_rgb (192, 192, 192),
				surfaceLighting (0, 0, 1, -1, 0, 0) );

	drawIsoPoly(gc, 4, x, y, z, color);
}


void IsoTexturedGrid::drawIsoPoly (const GraphicsContext &gc, int num, int x[], int y[], int z[], ALLEGRO_COLOR color)
{
	const int BUF_SIZE = 20; // max 20 points
	assert (num <= BUF_SIZE);

	ALLEGRO_VERTEX coord[BUF_SIZE]; // hold actual objects
	ALLEGRO_VERTEX *pcoord[BUF_SIZE]; // hold array of pointers

	// initialize pointers to point to objects
	for (int i = 0; i < BUF_SIZE; ++i) { pcoord[i] = &coord[i]; }

	for (int i = 0; i < num; ++i)
	{
		canvasFromIso_f (x[i], y[i], z[i], coord[i].x, coord[i].y);
		coord[i].x += gc.xofst;
		coord[i].y += gc.yofst;
		coord[i].color = color;
	}

	al_set_target_bitmap (gc.buffer);
	al_draw_prim (coord, NULL, NULL, 0, num, ALLEGRO_PRIM_TRIANGLE_FAN);
}
