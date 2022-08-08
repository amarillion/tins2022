#include "sprite.h"
#include "player.h"
#include "color.h"
#include "engine.h"
#include <math.h>
#include "motionimpl.h"
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "textstyle.h"
#include "mainloop.h"
#include "anim.h"
#include "settings.h"
#include "tegel5.h"
#include "game.h"

using namespace std;

void Player::setState (bool hit)
{
	if (hit)
		state = 4;
	else {
		if (swimming) { state = 5; }
		else {
			switch (currentWeapon)
			{
			case Bullet::WPN_ROCK: state = 0; break;
			case Bullet::WPN_ICE: state = 1; break;
			case Bullet::WPN_BAZOOKA: state = 3; break;
			case Bullet::WPN_LASER: state = 2; break;
			}
		}
	}
}

void Player::hit(int attackDamage, double delta)
{
	setState (true);
	hitTimer = HIT_ANIM_LENGTH;

	// make a little jump
	dy = -6;
	if (delta > 0) dx = -6; else dx = 6;

	hp -= attackDamage;
	if (hp <= 0)
	{
		hp = 0;
		die();
	}
	parent->setPlayerHp(hp);
}

void Player::onCol(SpriteType st, Sprite *s, int dir)
{
	if (!control) return;
	
	SpriteEx *se = dynamic_cast<SpriteEx *>(s);
	switch (st)
	{
	case ST_ENEMY: case ST_ENEMY_BULLET:
		if (hitTimer == 0)
		{
			hit(se->getAttackDamage(), se->getx() - x);
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
		int yy = gety() + geth();
		parent->setTimer(20, [=](){
			parent->exitMap(dir, yy);
		});
		break;
	}
	case ST_BONUS: {
			if (se->getSubType() == Bonus::RING) {
				if (hp < PLAYER_HP) { hp++; }
				parent->setPlayerHp(hp);
			}
		}
		break;
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
		draw_shaded_textf(parent->smallfont, WHITE, BLACK, x, y - 32, ALLEGRO_ALIGN_LEFT, "%i %i %ix%i", (int)getx(), (int)gety(), w, h);
	}
#endif
}

Player::Player (Game *game, int x, int y, int _hp = PLAYER_HP) : SpriteEx (game, ST_PLAYER, x, y)
{
	unpassable = true;
	setAnim(anims["Raul"]);
	gravity = true;
	state = 0;
	hp = _hp;
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

void Player::updateWater() {
	
	// switch from land to water
	if (!swimming) {
		swimming = true;
		gravity = false;
		setState(false);
	}

	if (hitTimer == 0) {

		if (btn[btnLeft].getState()) {
			dx = -AIR_HSPEED;
			dir = 0;
		}
		else if (btn[btnRight].getState()) {
			dx = AIR_HSPEED;
			dir = 1;
		}
		else {
			dx = 0;
		}
		
		if (btn[btnUp].getState()) {
			dy = -AIR_HSPEED;
		}
		else if (btn[btnDown].getState()) {
			dy = AIR_HSPEED;
		}
		else {
			dy = 0;
		}
	}
}

void Player::updateLand() {
	
	// switch from water to land
	if (swimming) {
		swimming = false;
		gravity = true;
		if (btn[btnJump].getState() && hitTimer == 0) {
			// short jump out of water
			parent->getParent()->playSample("Sound4");
			jumpTimer = MAX_JUMPTIMER_FROM_WATER;
			air = true;
		}
		setState(false);
	}

	//	float speed = air ? AIR_HSPEED : LAND_HSPEED;
	if (hitTimer == 0) {
		if (btn[btnLeft].getState()) {
			dx = -AIR_HSPEED;
			dir = 0;
		}
		else if (btn[btnRight].getState()) {
			dx = AIR_HSPEED;
			dir = 1;
		}
		else {
			dx = 0;
		}

		if (btn[btnJump].getState()) {
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

	// handle shooting
	if (shootTimer > 0) shootTimer--;
	if (btn[btnAction].justPressed() && shootTimer == 0 && hitTimer == 0) {
		shoot();
	}
}

void Player::update()
{
	if (control) {
		if (isUnderWater()) {
			updateWater();
		}
		else {
			updateLand();
		}
	}

	if (hitTimer > 0) {
		hitTimer--;
		if (hitTimer == 0) {
			setState (false);
		}
	}

	SpriteEx::update();
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

EBullet::EBullet(Game * game, int type, int x, int y, double _dx, double _dy)
	: SpriteEx (game, ST_ENEMY_BULLET, x, y, type),
	  timer (0)
{
	dx = _dx;
	dy = _dy;

	switch (subtype)
	{
	case FIRE:
		setAnim(anims["Fire bubble"]);
		gravity = true;
		damage = 3;
		blockedByTiles = true;
		timer = 301;
		break;
	case ENERGY:
		setAnim(anims["Enemy bullet"]);
		gravity = false;
		damage = 2;
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
		case FALLING: {
			setAnim(anims["Platform"]); 
			gravity = false;
			IMotionPtr ptr = IMotionPtr(new Lissajous(100, 5, 50, 0));
			setMotion(ptr);
		}
		break;
		case CRATE: 
			setAnim(anims["BigCrate"]); 
			gravity = true;
			buoyant = true;
			break;
		case SMALLCRATE: 
			gravity = true;
			buoyant = true;
			setAnim(anims["SmallCrate"]); 
			break;
	}
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
		setAnim(anims["Rock"]);
		blockedByTiles = true;
		gravity = true;
		break;
	case WPN_ICE:
		setAnim(anims["Ice"]);
		blockedByTiles = true;
		gravity = true;
		break;
	case WPN_BAZOOKA:
		setAnim(anims["Bullet"]);
		blockedByTiles = true;
		gravity = false;
		break;
	case WPN_LASER:
		setAnim(anims["Laser"]); 
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


Bonus::Bonus (Game * parent, int x, int y, int _subtype, function<void()> onCollected) 
	: SpriteEx (parent, ST_BONUS, x, y, _subtype), onCollected(onCollected)
{
	gravity = false;
	switch(subtype) {
		case SOCK: setAnim(anims["Redsock"]); break;
		case RING: setAnim(anims["Ring"]); break;
		case ONEUP: setAnim(anims["1UP"]); break;
	}
}

void Bonus::onCol (SpriteType st, Sprite *s, int dir)
{
	if (st == ST_PLAYER)
	{
		onCollected();
		parent->collectBonus(subtype);
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

Teleporter::Teleporter (Game *game, int x, int y, Point globalTargetPos) 
	: SpriteEx(game, ST_TELEPORTER, x, y), globalTargetPos(globalTargetPos) 
{
	// gravity = false;
	setAnim(anims["Teleporter"]);
}

void Teleporter::onCol (SpriteType st, Sprite *s, int dir) {
	static int globalCoolDown = 0;
	if (globalCoolDown > 0) { globalCoolDown--; return; } 
	
	if (st == ST_PLAYER)
	{
		// TODO: animation
		parent->teleport(globalTargetPos);
		globalCoolDown = 200;
	}
}
