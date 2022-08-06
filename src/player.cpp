#include "sprite.h"
#include "player.h"
#include "color.h"
#include "engine.h"
#include <math.h>
#include <motionimpl.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "textstyle.h"
#include "mainloop.h"
#include "anim.h"
#include "settings.h"
#include "tegel5.h"
#include "game.h"

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

	switch (spriteType)
	{
	case ST_PLAYER:
		unpassable = true;
		setAnim(anims["Raul"]); break;
	case ST_ENEMY:
		unpassable = true;
		switch (subtype % 5)
		{
			case Enemy::ELECTRICAT: setAnim(anims["Electricat"]); break;
			case Enemy::SLINKYCAT: setAnim(anims["Slinkycat"]); break;
			case Enemy::SPIDERCAT: setAnim(anims["Spidercat"]); break;
			case Enemy::DRAGONCAT: setAnim(anims["Dragoncat"]); break;
			case Enemy::GENERATOR: /* TODO */ break;
		}
		break;
	case ST_BULLET:
		switch (subtype % 4)
		{
			case Bullet::WPN_ROCK: setAnim(anims["Rock"]); break;
			case Bullet::WPN_ICE: setAnim(anims["Ice"]); break;
			case Bullet::WPN_BAZOOKA: setAnim(anims["Bullet"]); break;
			case Bullet::WPN_LASER: setAnim(anims["Laser"]); break;
		}
		break;
	case ST_ENEMY_BULLET:
		switch (subtype)
		{
			case EBullet::FIRE: setAnim(anims["Fire bubble"]); break;
			case EBullet::ENERGY: setAnim(anims["Enemy bullet"]); break;
		}
		break;
	default: /* do nothing */ break;
	}
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

void Player::setState (bool hit)
{
	if (hit)
		state = 4;
	else
		switch (currentWeapon)
		{
		case Bullet::WPN_ROCK: state = 0; break;
		case Bullet::WPN_ICE: state = 1; break;
		case Bullet::WPN_BAZOOKA: state = 3; break;
		case Bullet::WPN_LASER: state = 2; break;
		}
}

void Player::hit(int subtype, double delta)
{
	setState (true);
	hitTimer = HIT_ANIM_LENGTH;

	// make a little jump
	dy = -6;
	if (delta > 0) dx = -6; else dx = 6;

	switch (subtype)
	{
	case Enemy::ELECTRICAT:
	case Enemy::SLINKYCAT:
	case Enemy::SPIDERCAT:
		hp -= 4;
		break;
	case Enemy::DRAGONCAT:
		hp -= 6;
		break;
	case EBullet::ENERGY:
		hp -= 2;
		break;
	case EBullet::FIRE:
		hp -= 3;
		break;
	}

	if (hp <= 0)
	{
		hp = 0;
		die();
	}

}

void Player::onCol(SpriteType st, Sprite *s, int dir)
{
	if (!control) return;
	
	SpriteEx *se = dynamic_cast<SpriteEx *>(s);
	switch (st)
	{
	case ST_ENEMY:
	case ST_ENEMY_BULLET:
		if (hitTimer == 0)
		{
			hit(se->getSubType(), se->getx() - x);
		}
		break;
	case ST_FLOOR:
		die();
		break;
	case ST_TILE: case ST_PLATFORM:
		// if going down, we landed
		if ((dir & DIR_DOWN) > 0)
		{
			air = false;
			dy = 0;
			jumpTimer = 0;
		}
		break;
	case ST_BOUNDS: {
		cout << "Hit bounds" << endl;
		control = false;
		gravity = false;
		
		// we're going to moonwalk out of the level...
		dy = 0;
		dx = dir == DIR_LEFT ? -AIR_HSPEED: AIR_HSPEED;
		int yy = gety();

		parent->setTimer(20, [=](){
			parent->exitMap(dir, yy);
		});
		break;
	}
	default: /* do nothing */ break;
	}
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
		al_draw_textf(parent->smallfont, GREEN, x, y - 16, ALLEGRO_ALIGN_LEFT, "hp: %i", hp);
	}
#endif
}

Player::Player (Game *game, int x, int y) : SpriteEx (game, ST_PLAYER, x, y)
{
	gravity = true;
	state = 0;
	hp = PLAYER_HP;
	btn = game->getParent()->getInput();
}

void Player::kill()
{
	cout << "Player killed" << endl;
	SpriteEx::kill();
	parent->player = NULL;
}

void Player::die()
{
	// create a timer
	parent->setTimer(50, Game::MSG_PLAYER_DIED);
	Explosion *e = new Explosion(parent, getx(), gety(), 100);
	parent->addSprite (e);
	kill(); // kill this sprite
}

void Player::update()
{
	if (control) {
		//	float speed = air ? AIR_HSPEED : LAND_HSPEED;
		if (btn[btnLeft].getState() && hitTimer == 0)
		{
			dx = -AIR_HSPEED;
			dir = 0;
		}
		else if (btn[btnRight].getState() && hitTimer == 0)
		{
			dx = AIR_HSPEED;
			dir = 1;
		}
		else if (hitTimer == 0)
		{
			dx = 0;
		}

		if (hitTimer > 0)
		{
			hitTimer--;
			if (hitTimer == 0)
			{
				setState (false);
			}
		}

		if (btn[btnUp].getState() && hitTimer == 0)
		{
			if (!air)
			{
				parent->getParent()->playSample("Sound4");
				jumpTimer = MAX_JUMPTIMER;
				air = true;
			}

			// while jump timer is counting
			if (jumpTimer > 0)
			{
				// constant acceleration
				dy = -JUMP_SPEED;
				jumpTimer--;
			}
		}
		else
		{
			jumpTimer = 0;
		}
	}
	
	SpriteEx::update();

	if (control) {
		// handle shooting
		if (shootTimer > 0) shootTimer--;
		if (btn[btnAction].justPressed() && shootTimer == 0 && hitTimer == 0)
		{
			shoot();
		}
	}
}

void Player::shoot()
{
	int ox = getx() + ((dir == 0) ? -1 : getw() + 1);
	int oy = gety() + 20;

	parent->getParent()->playSample("Sound3");
	double dxsign = (dir == 0) ? -1 : 1;
	switch (currentWeapon)
	{
		case Bullet::WPN_ROCK:
		{
			Bullet *b = new Bullet (parent, Bullet::WPN_ROCK, ox, oy, 13 * dxsign, -7);
			parent->addSprite(b);
			shootTimer = 19;
			break;
		}
		case Bullet::WPN_ICE:
		{
			Bullet *b = new Bullet (parent, Bullet::WPN_ICE, ox, oy, 13 * dxsign, -7);
			parent->addSprite(b);
			shootTimer = 19;
			break;
		}
		case Bullet::WPN_BAZOOKA:
		{
			Bullet *b = new Bullet (parent, Bullet::WPN_BAZOOKA, ox, oy, 7 * dxsign, 0);
			parent->addSprite(b);
			shootTimer = 37;
			break;
		}
		case Bullet::WPN_LASER:
		{
			Bullet *b = new Bullet (parent, Bullet::WPN_LASER, ox, oy, 17 * dxsign, 0);
			parent->addSprite(b);
			shootTimer = 7;
			break;
		}
	}
}

bool Player::isWeaponAvailable(int wpn)
{
	assert (wpn >=0 && wpn <= 3);
	if (wpn == 0) {
		return true;
	}
	return false;
}

void Player::draw(const GraphicsContext &gc)
{
	SpriteEx::draw(gc);
	draw_textf_with_background (parent->gamefont, CYAN, BLACK, MAIN_WIDTH, 0, ALLEGRO_ALIGN_RIGHT, "HP: %02i", hp);
}

const int MAX_ENEMY_TYPE = 5;

Enemy::Enemy (Game *game, int x, int y, int _type) : SpriteEx(game, ST_ENEMY, x, y, _type)
{
	enemyType = (_type % MAX_ENEMY_TYPE);
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
		dx = 4;
		dy = 0;
		break;
	case SLINKYCAT:
		gravity = true;
		hp = 3; /* PRIME */
		dx = 3;
		dy = 0;
		break;
	case SPIDERCAT:
		gravity = false;
		blockedByTiles = false;
		hp = 5; /* PRIME */
		dx = 4;
		dy = 0;
		period = 101; /* PRIME */
		break;
	case DRAGONCAT: /* DragonCat */
		hp = 31; /* PRIME */
		gravity = false;
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
	Enemy *e = new Enemy(parent, x, y, Enemy::SLINKYCAT);
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

EBullet::EBullet(Game * game, int type, int x, int y, double _dx, double _dy)
	: SpriteEx (game, ST_ENEMY_BULLET, x, y, type),
	  timer (0)
{
	dx = _dx;
	dy = _dy;
	switch (subtype)
	{
	case FIRE:
		gravity = true;
		blockedByTiles = true;
		timer = 301;
		break;
	case ENERGY:
		gravity = false;
		blockedByTiles = false;
		break;
	}
}

void EBullet::update()
{
	SpriteEx::update();
	switch (subtype)
	{
	case FIRE:
		timer--;
		if (timer <= 0) kill();
		break;
	case ENERGY:
		// just continue moving
		break;
	}
}

void EBullet::onCol (SpriteType st, Sprite *s, int dir)
{
	switch (st)
	{
		case ST_PLAYER:
		case ST_BOUNDS:
			kill();
			break;
		default:
			break;
	}
}

Platform::Platform(Game * game, int x, int y, int type) : SpriteEx(game, ST_PLATFORM, x, y)
{
	unpassable = true;
	pushForce = 10; // pushes everything else

	switch(type) {
		case FALLING: setAnim(anims["Platform"]); break;
		case CRATE: setAnim(anims["BigCrate"]); break;
		case SMALLCRATE: setAnim(anims["SmallCrate"]); break;
	}
	IMotionPtr ptr = IMotionPtr(new Lissajous(100, 5, 50, 0));
	gravity = false;
	setMotion(ptr);
}

void Platform::onCol(SpriteType st, Sprite *s, int dir)
{
	switch (st)
	{
		case ST_PLAYER:
			// make player move with platform
			if ((dir & SpriteEx::DIR_DOWN) > 0)
				s->try_move (dx, dy);
			break;
		default:
			break;
	}
}

Bullet::Bullet(Game * game, int type, int x, int y, double _dx, double _dy)
	: SpriteEx (game, ST_BULLET, x, y, type)
{
	dx = _dx;
	dy = _dy;

	switch (subtype)
	{
	case WPN_ROCK:
		blockedByTiles = true;
		gravity = true;
		break;
	case WPN_ICE:
		blockedByTiles = true;
		gravity = true;
		break;
	case WPN_BAZOOKA:
		blockedByTiles = true;
		gravity = false;
		break;
	case WPN_LASER:
		blockedByTiles = false;
		gravity = false;
		break;
	}
}

void Bullet::onCol(SpriteType st, Sprite *s, int dir)
{
	switch (st)
	{
		case ST_TILE: case ST_PLATFORM:
		{
			if (subtype == WPN_LASER || subtype == WPN_ROCK)
			{
				gravity = true;
				blockedByTiles = false;
				if (dx > 0) dx = -2; else dx = 2; // bounce a little to the other side
				dy = -3; // bounce up a little
			}
			else if (subtype == WPN_BAZOOKA)
			{
				Explosion *expl = new Explosion(parent, x, y, 50);
				parent->addSprite(expl);
				kill();
			}
		}
		break;
		case ST_BOUNDS:
			kill();
			break;
		case ST_ENEMY:
			if (subtype == WPN_LASER || subtype == WPN_ROCK)
			{
				// simply gets absorbed
				kill();
			}
			else if (subtype == WPN_BAZOOKA)
			{
				Explosion *expl = new Explosion(parent, x, y, 50);
				parent->addSprite(expl);
				kill();
			}
			break;
		default:
			break;
	}
}

void Bullet::update()
{
	SpriteEx::update();
	if (subtype == WPN_BAZOOKA)
	{
		if (counter % 3 == 0) /* PRIME */
		{
			// parent->smoke->addParticle(x, y);
		}
		if (dx > 0)
			dx += BAZOOKA_ACC;
		else
			dx -= BAZOOKA_ACC;
	}
}

Explosion::Explosion(Game *e, int x, int y, int life) : Sprite (e, x, y)
{
	blockedByTiles = false;
	color = YELLOW;
	maxLive = life;
	size = 1;
	tLive = 0;
}

void Explosion::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);

	int x = getx() + gc.xofst;
	int y = gety() + gc.yofst;

	al_draw_filled_circle (x, y, size, al_map_rgba(255, 0, 0, 128));
	al_draw_filled_circle (x, y, size * 2 / 3, al_map_rgba(255, 255, 0, 128));
}

void Explosion::update()
{
	size++;
	if (++tLive > maxLive)
	{
		kill();
	}
}


Bonus::Bonus (Game * parent, int x, int y, int index) : SpriteEx (parent, ST_BONUS, x, y), index(index)
{
	gravity = false;
	switch(index) {
		case SOCK: setAnim(anims["Redsock"]); break;
		case RING: setAnim(anims["Ring"]); break;
		case ONEUP: setAnim(anims["1UP"]); break;
	}
}

void Bonus::onCol (SpriteType st, Sprite *s, int dir)
{
	if (st == ST_PLAYER)
	{
		parent->collectBonus(index);
		kill(); // disappear
	}
}

Switch::Switch (Game *game, int x, int y, int startState) : SpriteEx(game, ST_SWITCH, x, y) {
	gravity = false;
	state = startState;
	setAnim(anims["Switch"]);
}

void Switch::onCol (SpriteType st, Sprite *s, int dir) {
	if (coolDown > 0) return;

	if (st == ST_PLAYER)
	{
		coolDown = 50;
		if (state == Switch::OFF) {
			state = Switch::ON;
			parent->updateWaterLevel();
		}
	}
}

Teleporter::Teleporter (Game *game, int x, int y) : SpriteEx(game, ST_TELEPORTER, x, y) {
	// gravity = false;
	setAnim(anims["Teleporter"]);
}

void Teleporter::onCol (SpriteType st, Sprite *s, int dir) {
	if (st == ST_PLAYER)
	{
		//TODO: teleport player
	}
}
