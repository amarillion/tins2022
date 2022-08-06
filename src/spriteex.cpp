#include "spriteex.h"
#include "game.h"
#include "tegel5.h"

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
		dy += GRAVITY_ACC;
		if (dy > MAX_Y) dy = MAX_Y;
		if (dy < -MAX_Y) dy = -MAX_Y;
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
