#include "spriteex.h"
#include "game.h"
#include "tegel5.h"
#include "textstyle.h"
#include "engine.h"
#include "anim.h"
#include "mainloop.h"

using namespace std;

SpriteEx::SpriteEx(Game *game, SpriteType st, int x, int y, int _subtype) : Sprite (game, x, y)
{
	subtype = _subtype;
	hp = 0;
	dx = 0;
	dy = 0;
	air = true;
	spriteType = st;
	gravity = true;
	unpassable = false;
	pushForce = 0;
}

void SpriteEx::update()
{
	counter++;

	if (motion)
	{
		dx = motion->getdx(counter);
		dy = motion->getdy(counter);
	}

	list<Sprite*>::iterator i;
	list<Sprite*> sprites = parent->getSprites();

	if (gravity)
	{
		if (buoyant && isUnderWater()) {
			dy -= GRAVITY_ACC;
			dy *= 0.8; // friction under water
			if (dy > MAX_Y_UNDERWATER) dy = MAX_Y_UNDERWATER;
			if (dy < -MAX_Y_UNDERWATER) dy = -MAX_Y_UNDERWATER;
		}
		else {
			dy += GRAVITY_ACC;
			if (dy > MAX_Y) dy = MAX_Y;
			if (dy < -MAX_Y) dy = -MAX_Y;
		}
	}

	double oldy = y;
	double oldx = x;

	int block = try_move(dx, dy, pushForce);

	if ((block & DIR_DOWN) > 0)
	{
		air = false;
		dy = 0;
	}
	else
	{
		air = true;
	}

	if (block & BLOCKED_BY_TILE)
	{
		onCol(ST_TILE, NULL, block);
	}

	int floor = teg_pixelh(parent->getMap());
	if (oldy <= floor && y > floor)
	{
		onCol(ST_FLOOR, NULL, DIR_DOWN);
	}

	if (oldy >= CEIL && y < CEIL)
	{
		onCol(ST_BOUNDS, NULL, DIR_UP);
	}

	if (oldx >= 0 && x < 0)
	{
		x = 0;
		onCol(ST_BOUNDS, NULL, DIR_LEFT);
	}

	int mapw = teg_pixelw(parent->getMap());
	if ((oldx + w) < mapw && (x + w) >= mapw)
	{
		x = mapw - w;
		onCol(ST_BOUNDS, NULL, DIR_RIGHT);
	}
}

void SpriteEx::onCol(SpriteType st, Sprite *s, int dir)
{
	SpriteEx *se = dynamic_cast<SpriteEx *>(s);

	if (st == ST_FLOOR)
	{
		// we died
		kill();
	}
}

bool SpriteEx::isUnderWater() {
	return gety() + (geth() / 2) > parent->getLocalWaterLevel();
}


void SpriteEx::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);

	ALLEGRO_COLOR color;
	switch (spriteType)
	{
	case ST_PLAYER: color = YELLOW; break;
	case ST_PLATFORM: color = GREY; break;
	case ST_ENEMY: color = GREEN; break;
	case ST_BULLET: color = GREY; break;
	case ST_ENEMY_BULLET: color = RED; break;
	default: color = BLACK; break;
	}

	int x = getx() + gc.xofst;
	int y = gety() + gc.yofst;

	int msec = MainLoop::getMainLoop()->getMsecCounter();

	if (anim != NULL)
	{
		anim->drawFrame (state, dir, msec, x, y);
	}
	else
	{
		al_draw_filled_rectangle (x, y, x + w, y + h, color);
	}

#ifdef DEBUG
	if (parent->getParent()->isDebug())
	{
		al_draw_rectangle (x, y, x + w, y + h, color, 1.0);
		draw_shaded_textf(parent->smallfont, WHITE, BLACK, x, y - 32, ALLEGRO_ALIGN_LEFT, "%i %i %ix%i", (int)getx(), (int)gety(), w, h);
	}
#endif
}
