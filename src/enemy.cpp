#include "enemy.h"
#include "game.h"
#include "player.h"
#include <math.h>
#include "tegel5.h"
#include "textstyle.h"
#include "engine.h"
#include "mainloop.h"

const int MAX_ENEMY_TYPE = 5;

Enemy::Enemy(Game *game, int x, int y, int _type) : SpriteEx(game, ST_ENEMY, x, y, _type)
{
	unpassable = true;
	enemyType = (_type % MAX_ENEMY_TYPE);
	switch (enemyType)
	{
		case ELECTRICAT: setAnim(anims["Electricat"]); break;
		case SLINKYCAT: setAnim(anims["Slinkycat"]); break;
		case SPIDERCAT: setAnim(anims["Spidercat"]); break;
		case DRAGONCAT: setAnim(anims["Dragoncat"]); break;
		case GENERATOR: /* TODO */ break;
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
	if (subtype == DRAGONCAT)
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
			if (parent->player == NULL)
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

void Enemy::update()
{
	if (hittimer > 0)
	{
		hittimer--;
		if (hittimer == 0)
		{
			state = 0;
		}
	}

	if (parent->player != NULL)
	{
		double delta = x - parent->player->getx();
		if (fabs (delta) > 1000)
			return; // out of range, don't update.
	}

	phase = (phase + 1) % period;

	SpriteEx::update();
	switch (enemyType)
	{
	case 0:
		update1();
		break;
	case 1:
		update2();
		break;
	case 2:
		update3();
		break;
	case 3:
		update4();
		break;
	case 4:
		update5();
	}

	return;
}

void Enemy::kill()
{
	SpriteEx::kill();
	if (subtype == DRAGONCAT)
	{
		parent->setTimer(50, Game::MSG_PLAYER_WIN);
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
		case Bullet::WPN_ROCK: hp -= 2; break;
		case Bullet::WPN_ICE: hp -= 3; break;
		case Bullet::WPN_LASER: hp -= 1; break;
		case Bullet::WPN_BAZOOKA: hp -= 5; break;
		}
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
	else if (st == ST_TILE || st == ST_BOUNDS || st == ST_ENEMY || st == ST_PLATFORM)
	{
		if (subtype != DRAGONCAT)
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
