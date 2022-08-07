#include <assert.h>
#include "engine.h"
#include "color.h"
#include <math.h>
#include <list>
#include <algorithm>
#include "game.h"
#include "engine.h"
#include "color.h"
#include "anim.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "player.h"
#include "componentbuilder.h"
#include "util.h"
#include <map>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "textstyle.h"
#include "DrawStrategy.h"
#include "mainloop.h"
#include "resources.h"
#include "tilemap.h"
#include "sprite.h"

using namespace std;

map<string, Anim*> Sprite::anims;

Sprite::Sprite (Game *game, int amx, int amy) : awake(true), alive(true), visible(true), counter(0),
		x(0), y(0), w(0), h(0), motion(), anim(NULL), state(0), dir(0), animStart(0)
{
	setx (amx);
	sety (amy);
	alive = true;
	visible = true;
	blockedByTiles = true;
	parent = game;
}

void Sprite::update()
{
	assert (parent);
}

void Sprite::draw(const GraphicsContext &gc)
{
	int x = getx() + gc.xofst;
	int y = gety() + gc.yofst;
	int time = 0;
	if (anim != NULL)
		anim->drawFrame (state, dir, time, x, y);
}

bool Sprite::hasXOverlap (Sprite *s2)
{
	int x1 = x;
	int dx1 = w;
	int x2 = s2->getx();
	int dx2 = s2->getw();
	return !(
			(x1 >= x2+dx2) || (x2 >= x1+dx1)
		);
}

bool Sprite::hasOverlap (Sprite *s2)
{
	int y1 = y;
	int dy1 = h;
	int y2 = s2->gety();
	int dy2 = s2->geth();
	return hasXOverlap(s2) &&
		!(
			(y1 >= y2+dy2) || (y2 >= y1+dy1)
		);
}

int Sprite::getTileStackFlags(int mx, int my)
{
	int result = 0;
	TEG_MAP *map = parent->getMap();
	if (!map) return 0; // no map found !!!

	if (mx < 0 || my < 0 || mx >= map->w || my >= map->h)
	{
		return 0;
	}
	else
	{
		int i1, i2, f1, f2;
		i1 = teg_mapget (map, 0, mx, my);
		i2 = teg_mapget (map, 1, mx, my);
		if (i1 >= 0) f1 = map->tilelist->tiles[i1].flags; else f1 = 0;
		if (i2 >= 0) f2 = map->tilelist->tiles[i2].flags; else f2 = 0;

		// check for solids
		if (f1 == 1 || f2 == 1) result |= TS_SOLID;

		return result;
	}
}

/*
 * Attempt to move dx, dy.
 * We divide the movement (dx, dy) in increments of one pixel,
 * taking turns moving horizontal and vertical until
 * the entire movement has been filled up.
 *
 * If any sprite or player blocks, the movement is stopped and a non-zero value is returned.
 *
 * Collisions with sprites are stored in a list, and later processed, calling onCol() twice for each collision.
 *
 * The push_force parameter is initially set at the pushForce value of the sprite doing the movement.
 * When a move is blocked by another sprite, we attempt to move it as well, using the push_force of the primary mover.
 */
int Sprite::try_move (double dx, double dy, int push_force)
{
	double dxleft = dx, dyleft = dy;
	int ddx = dx > 0 ? 1 : -1;
	int hdir = dx > 0 ? SpriteEx::DIR_RIGHT : SpriteEx::DIR_LEFT;
	int ddy = dy > 0 ? 1 : -1;
	int vdir = dy > 0 ? SpriteEx::DIR_DOWN: SpriteEx::DIR_UP;
	double trydx, trydy;
	int result = 0;

	// we keep processing until dxleft & dyleft are 0.
	while ((fabs(dxleft) > 0 || fabs (dyleft) > 0))
	{
		bool valid = true;
		int dir = 0;

		// we only move (0,+/-1) or (+/-1,0), nothing in between.
		// if there is more horizontal movement left than vertical, we move horizontal.
		if (fabs(dxleft) > fabs(dyleft))
		{
			trydy = 0;
			dir = hdir;
			if (fabs(dxleft) >= 1)
				trydx = ddx;
			else
				trydx = dxleft;
		}
		else
		{
			dir = vdir;
			trydx = 0;
			if (fabs(dyleft) >= 1)
				trydy = ddy;
			else
				trydy = dyleft;
		}

		// check with tilemap background, but only if object is solid.
		if (blockedByTiles)
		{
			// check if (x +  |trydx, y + trydy) is valid
			int mx1, my1, mx2, my2;
			int ix, iy;
			TEG_MAP *map = parent->getMap();
			mx1 = ((int)x + trydx) / map->tilelist->tilew;
			my1 = ((int)y + trydy) / map->tilelist->tileh;
			mx2 = ((int)x + trydx + w - 1) / map->tilelist->tilew;
			my2 = ((int)y + trydy + h - 1) / map->tilelist->tileh;

			// loop through all map positions we touch with the solid region
			for (ix = mx1; ix <= mx2; ++ix)
			{
				for (iy = my1; iy <= my2; ++iy)
				{
					// see if there is a solid tile at this position
					if (getTileStackFlags (ix, iy) & TS_SOLID)
					{
						valid = false;
						result |= dir; // multiply by 16 to turn DIR* into TILE_DIR*
						result |= SpriteEx::BLOCKED_BY_TILE;
					}
				}
			}
		}

		// loop through all sprites and see if we touch a solid sprite
		// detect collisions
		list<Sprite*>::iterator i;

		// temporarily move for testing. Need to adjust x, y temporarily for hasOverlap()
		double oldx = x;
		double oldy = y;

		x += trydx;
		y += trydy;

		for (i = parent->getSprites().begin(); i != parent->getSprites().end(); i++)
		{
			if (!(*i)->isAlive()) continue;
			if (*i == this) continue;
			SpriteEx * sei = dynamic_cast<SpriteEx*>(*i);
			SpriteEx * sej = dynamic_cast<SpriteEx*>(this);

			if (sei == NULL || sej == NULL) continue; // not a SpriteEx (must be explosion)

			if (sej->hasOverlap(sei))
			{
				// platforms players and enemies are unpassable.
				if (sei->isUnpassable() && sej->isUnpassable())
				{
					// attempt to push
					if (sei->getPushForce() < push_force)
					{
						//TODO: make sure we're not in an infinite loop somehow...
						int result2 = sei->try_move(trydx, trydy, push_force);
						if (result2 > 0)
						{
							// couldn't push.
							valid = false;
							result |= dir;
						}
						else
						{
							// pushing worked, we're free to continue moving.
						}
					}
					else
					{
						valid = false;
						result |= dir;
					}
				}
				// record the collision for later playback.
				parent->addCollision(sej, sei, dir);
			}
		}

		// restore x & y values
		x = oldx;
		y = oldy;

		// if we have been blocked
		if (!valid)
		{
			if (trydx == 0)
			{
				dyleft = 0;
			}
			else
			{
				dxleft = 0;
			}
		}
		else
		{
			x += trydx;
			dxleft -= trydx;
			y += trydy;
			dyleft -= trydy;
		}
	}
	return result;
}

void Sprite::setAnim(Anim *_anim) { 
	anim = _anim;
	if (anim != nullptr)
	{
		int oldh = h;
		w = anim->sizex;
		h = anim->sizey;
		
		// adjust y position so the _bottom_ stays in place
		y += (oldh - h);
	}
}
