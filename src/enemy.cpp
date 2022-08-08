#include "enemy.h"
#include "game.h"
#include "player.h"
#include <math.h>
#include "tegel5.h"
#include "textstyle.h"
#include "engine.h"
#include "mainloop.h"
#include <coroutine>
#include "constants.h"

using namespace std;

const int MAX_ENEMY_TYPE = 8;

Enemy::Enemy(Game *game, int x, int y, int _type) : SpriteEx(game, ST_ENEMY, x, y, _type)
{
	unpassable = true;
	enemyType = _type;
	assert (enemyType < MAX_ENEMY_TYPE);
	switch (enemyType)
	{
		case ELECTRICAT: setAnim(anims["Electricat"]); break;
		case SLINKYCAT: setAnim(anims["Slinkycat"]); break;
		case SPIDERCAT: setAnim(anims["Spidercat"]); break;
		case DRAGONCAT: setAnim(anims["Dragoncat"]); break;
		case GENERATOR: /* TODO */ break;
		case TELECAT: setAnim(anims["Telecat"]); break;
		case ROLLINGCAT: setAnim(anims["Rollingcat"]); break;
		case SHARKCAT: 
			setAnim(anims["Shark"]); 
			break;
	}

	hittimer = 0;
	estate = 0;
	period = 1;
	vsign = 1.0;
	hsign = (rand() % 100 > 50) ? 1.0 : -1.0;

	destx = 0;
	desty = 0;
	bulletTimer = 0;
	jumpTimer = 0;
	state = 0;

	switch (enemyType)
	{
	case ELECTRICAT: /* ElectriCat */
		gravity = true;
		hp = 3; /* PRIME */
		damage = 4;
		dx = 4;
		dy = 0;
		break;
	case SLINKYCAT:
		gravity = true;
		hp = 3; /* PRIME */
		damage = 4;
		dx = 3;
		dy = 0;
		break;
	case SPIDERCAT:
		gravity = false;
		blockedByTiles = false;
		damage = 4;
		hp = 5; /* PRIME */
		dx = 4;
		dy = 0;
		period = 101; /* PRIME */
		break;
	case DRAGONCAT: /* DragonCat */
		hp = 31; /* PRIME */
		gravity = false;
		damage = 6;
		blockedByTiles = false;
		dx = 3;
		dy = 0;
		period = 301; /* PRIME */
		break;
	case GENERATOR:
		gravity = false;
		blockedByTiles = false;
		period = 31; /* PRIME */
		break;
	case TELECAT:
		hp = 7; /* PRIME */
		gravity = true;
		damage = 4;
		blockedByTiles = true;
		dx = 4;
		dy = 0;
		break;
	case ROLLINGCAT:
		hp = 3; /* PRIME */
		gravity = false;
		damage = 2;
		// blockedByTiles = false;
		break;
	case SHARKCAT:
		hp = 30;
		gravity = false;
		damage = 7;
		blockedByTiles = true;
		break;
	}

	dx *= hsign;
}

void Enemy::generatorSpawn()
{
	Enemy *e = new Enemy(parent, x, y, SLINKYCAT);
	e->dx = -3;
	parent->addSprite(e);
}

//TODO: swap name with generatorSpawn.
void Enemy::spawn(int val)
{
	Player *player = parent->player;
	if (player == NULL) return; // no sense in shooting when player is already dead.

	double dx = player->getx() - x;
	double dy = player->gety() - y;

	if (fabs (dx) > 400) return; // too far away.

	double len = sqrt (dx * dx + dy * dy);

	if (val == EBullet::ENERGY)
	{
		EBullet *b = new EBullet(parent, val, x, y, dx / len, dy / len);
		parent->addSprite (b);
	}
	else
	{
		EBullet *b = new EBullet(parent, val, x, y, dx / len, 0);
		parent->addSprite (b);
	}
}

void Enemy::update1()
{
	if (!air)
	{
		// check if we are moving onto empty space
		TEG_MAP *map = parent->getMap();
		int xx = x + ((dx > 0) ? getw() : 0);
		int yy = y + geth() + 1;
		int mx = xx / map->tilelist->tilew;
		int my = yy / map->tilelist->tileh;

		int tileflags = getTileStackFlags(mx, my);
		if ((tileflags & TS_SOLID) == 0)
		{
			// turn around
			dx = -dx;
		}
	}

	if (bulletTimer >=  301)
	{
		spawn(EBullet::ENERGY);
		bulletTimer = 0;
	}
	else
	{
		bulletTimer++;
	}
}

void Enemy::update2()
{
	if (bulletTimer >=  200)
	{
		spawn(EBullet::ENERGY);
		bulletTimer = 0;
	}
	else
	{
		bulletTimer++;
	}

	// jump
	if (!air)
	{
		dy = -13;
	}
}

void Enemy::update3()
{
	if (bulletTimer >=  91) /* PRIME */
	{
		spawn(EBullet::ENERGY);
		bulletTimer = 0;
	}
	else
	{
		bulletTimer++;
	}

	if (state != 2)
	{
		dx = 5 * cos(float(phase * 2) * 6.282f / float(period));
		dy = -5 * sin(float(phase) * 6.282f / float(period));
	}
}

void Enemy::moveTo (double _x, double _y, double speed)
{
	destx = _x;
	desty = _y;
	double xx = destx - x;
	double yy = desty - y;
	double len = sqrt(xx * xx + yy * yy);
	dx = xx / len * speed;
	dy = yy / len * speed;
}

void Enemy::draw (const GraphicsContext &gc)
{
	SpriteEx::draw (gc);
	if (subtype == DRAGONCAT || subtype == SHARKCAT)
	{
		draw_textf_with_background(parent->gamefont, RED, BLACK, MAIN_WIDTH / 2, 0, ALLEGRO_ALIGN_CENTER,
				"ENEMY: %i", hp);
#ifdef DEBUG
		if (parent->getParent()->isDebug())
		{
			draw_textf_with_background (parent->smallfont, RED, BLACK, 0, 0, ALLEGRO_ALIGN_LEFT,
					"estate %i, dx %3.2f, dy %3.2f, destx %3.2f, desty %3.2f", estate, dx, dy, destx, desty);
			int xofst = gc.xofst;
			int yofst = gc.yofst;
			al_draw_line (x - xofst, y - yofst, destx - xofst, desty - yofst, GREEN, 1.0);
		}
#endif
	}
}

bool Enemy::nearDest ()
{
	return ((fabs (x - destx) < 8) && (fabs (y - desty) < 8));
}

void Enemy::update5()
{
	if (bulletTimer >=  150)
	{
		generatorSpawn();
		bulletTimer = 0;
	}
	else
	{
		bulletTimer++;
	}
}

void Enemy::update6() {
	// using estate as dir...
	// using jumpTimer for turning

	int w = getw();
	int h = geth();

	TEG_MAP *map = parent->getMap();
	int tx = map->tilelist->tilew;
	int ty = map->tilelist->tileh;

	int zdx, zdy;
	switch (estate) {
		case 0: zdx = 0; zdy = h + 1; break;
		case 1: zdx = -1; zdy = 0; break;
		case 2: zdx = w; zdy = -1; break;
		case 3: zdx = w + 1; zdy = h; break;
		default: assert(false);
	}
	
	int mx = (getx() + zdx) / tx;
	int my = (gety() + zdy) / ty;

	if (jumpTimer == 0) {
		if ((getTileStackFlags(mx, my) & TS_SOLID) == 0) {
			estate = (estate + 1) % 4;
			jumpTimer = 15; // comes very precise.
		}
	}
	else {
		jumpTimer--;
	}

	int speed = 4;
	int ddx, ddy;
	switch (estate) {
		case 0: ddx =  1; ddy = 0; break;
		case 1: ddx =  0; ddy = 1; break;
		case 2: ddx = -1; ddy = 0; break;
		case 3: ddx = 0; ddy = -1; break;
		default: assert(false);
	}

	dx = ddx * speed;
	dy = ddy * speed;
}

void Enemy::update4()
{
	if (state == 2)
	{
		if (bulletTimer >=  20)
		{
			Explosion *e = new Explosion(parent, getx(), gety(), 30);
			parent->addSprite (e);
			bulletTimer = 0;
		}
		else
		{
			bulletTimer++;
		}

		return; // no further update
	}

	switch (estate)
	{
	case 0: /* Make large circles */
		dx = 5 * cos(float(phase) * 6.282f / float(period));
		dy = -5 * sin(float(phase) * 6.282f / float(period));

		jumpTimer++;
		if (jumpTimer > 500)
		{
			destx = teg_pixelw(parent->getMap()) / 2;
			desty = 32;
			jumpTimer = 0;
			estate = 1;
		}
		if (bulletTimer >=  100)
		{
			spawn(EBullet::FIRE);
			bulletTimer = 0;
		}
		else
		{
			bulletTimer++;
		}
		break;
	case 1: /* Move slowly to top-center of map */
		moveTo (destx, desty, 4);
		if (nearDest())
		{
			if (parent->player == nullptr)
			{
				destx = teg_pixelw(parent->getMap()) / 2;
				desty = teg_pixelh(parent->getMap()) / 2;
				estate = 3;
			}
			else
			{
				destx = parent->player->getx();
				desty = parent->player->gety();
				estate = 2;
			}
		}
		break;
	case 2: /* Dive towards player */
		moveTo (destx, desty, 12);
		if (nearDest())
		{
			destx = getx();
			desty = gety() - 100;
			estate = 3;
		}
		break;
	case 3: /* Move up 100 */
		moveTo (destx, desty, 2);
		if (nearDest())
		{
			estate = 0;
		}
		break;
	}
}

void Enemy::update7() {
	switch(estate) {
	case 0:
		// move back to center of map
		destx = teg_pixelw(parent->getMap()) / 2;
		desty = teg_pixelh(parent->getMap()) / 2;
		estate = 1;
		moveTo (destx, desty, 4);
		if (dx < 0) dir = 0; else dir = 1;
		break;
	case 1:
		if(!nearDest()) {
			moveTo (destx, desty, 4);
		}
		else {
			jumpTimer = 200 + rand() % 400;
			bulletTimer = 100;
			estate = 2;
			dx = -1; dy = 0;
		}
		break;
	case 2:
		dir = dx < 0 ? 0 : 1;
		// swim slowly back and forth...
		if (--jumpTimer > 0) {
			if (--bulletTimer < 0) {
				// turn around
				dx = -dx;
				bulletTimer = 100;
			}
		}
		else {
			// signal attack...
			estate = 3;
			state = 3; // set animation
			dx = 0; dy = 0;
			jumpTimer = 40;
		}
		break;
	case 3: // anticipation
		if (--jumpTimer > 0) {
			// turn towards player...
			if (parent->player->getx() - getx() > 0) {
				dir = 1;
			}
			else {
				dir = 0;
			}
		}
		else {
			state = 0; // back to regular animation
			estate = 4;
			jumpTimer = 35;
			if (parent->player == nullptr)
			{
				destx = teg_pixelw(parent->getMap()) / 2;
				desty = teg_pixelh(parent->getMap()) / 2;
			}
			else
			{
				destx = parent->player->getx();
				desty = parent->player->gety();
			}
			moveTo (destx, desty, 12); // set attack vector, but don't update it
		}
		break;
	case 4: 
		// dive towards player with overshoot...
		// set vector one time only...
		if (--jumpTimer > 0) {
			if (hittimer > 0) jumpTimer = 0; // bail immediately
		}
		else {
			estate = 0;
		} 
		break;
	}
}

void Enemy::update()
{
	if (parent->player != NULL)
	{
		double delta = x - parent->player->getx();
		if (fabs (delta) > 1000)
			return; // out of range, don't update.
	}

	phase = (phase + 1) % period;

	SpriteEx::update();

	if (state == 2) return;

	if (hittimer > 0)
	{
		hittimer--;
		if (hittimer == 0)
		{
			state = 0;
		}
	}

	switch (enemyType)
	{
	case Enemy::ELECTRICAT: update1(); break;
	case Enemy::SLINKYCAT: update2(); break;
	case Enemy::SPIDERCAT: update3(); break;
	case Enemy::DRAGONCAT: update4(); break;
	case Enemy::GENERATOR: update5(); break;
	case Enemy::TELECAT: update1(); /* TODO same as electricat for now */ break;
	case Enemy::ROLLINGCAT:	update6(); break;
	case Enemy::SHARKCAT: {
			int mx = (getx() + getw() / 2) / TILE_SIZE;
			int my = (gety() + geth() / 2) / TILE_SIZE;
			int flags = getTileStackFlags(mx, my);
			if (flags & TS_SPIKE) {
				hit(10);
				hittimer = 10; // override default value
			}
			update7();
			break;
		}
	}

	return;
}

void Enemy::kill()
{
	SpriteEx::kill();
	if (subtype == DRAGONCAT || subtype == SHARKCAT)
	{
		parent->setTimer(50, Game::MSG_PLAYER_WIN);
	}
}

void Enemy::hit(int damage) {
	hp -= damage;
	if (hp <= 0)
	{
		blockedByTiles = false;
		gravity = true;
		state = 2;
	}
	else
	{
		state = 1;
		hittimer = 5;
	}
}

void Enemy::onCol (SpriteType st, Sprite *s, int dir)
{
	if (st == ST_FLOOR && gravity)
	{
		kill();
	}
	if (state == 2) return; // no need to handle collission
	if (st == ST_BULLET)
	{
		SpriteEx *se = dynamic_cast<SpriteEx *>(s);
		assert (se != NULL);
		switch (se->getSubType())
		{
		case Bullet::WPN_ROCK: hit(2); break;
		case Bullet::WPN_ICE: hit(3); break;
		case Bullet::WPN_LASER: hit(1); break;
		case Bullet::WPN_BAZOOKA: hit(5); break;
		}
	}
	else if (st == ST_TILE || st == ST_BOUNDS || st == ST_ENEMY || st == ST_PLATFORM)
	{
		if (subtype == ROLLINGCAT) {
			estate = (estate + 3) % 4; // turn the other direction
		}
		else if (subtype != DRAGONCAT)
		{
			if ((dir & (DIR_LEFT | DIR_RIGHT)) > 0)
			{
				// reverse direction
				dx = -dx;
			}
			if ((dir & (DIR_UP | DIR_DOWN)) > 0)
			{
				// reverse direction
				dy = -dy;
				vsign = -vsign;
			}
		}
	}
}

