#include <assert.h>
#include "player.h"
#include "resources.h"
#include "anim.h"
#include "engine.h"
#include "door.h"
#include "game.h"
#include "mainloop.h"
#include "util.h"
#include "monster.h"
#include <cmath>
#include "math.h"

Anim *Bullet::bullet[1];

Bullet::Bullet(Room *r, double angleRad, double _speed, int _range, int _damage, Player *_parent) 
	: Object (r, _parent ? OT_BULLET : OT_ENEMY_BULLET) 
{
	solid = false;
	setAnim (bullet[0], type == OT_BULLET ? 0 : 1);
	dx = cos(angleRad) * _speed;
	dy = sin(angleRad) * _speed;
	range = _range / _speed; // normalized
	damage = _damage;
	parent = _parent;
}

void Bullet::init(std::shared_ptr<Resources> res)
{
	bullet[0] = res->getAnim("bullet");
}

void Bullet::update()
{
	try_move (dx, dy);
	range -= 1.0; // We assume that speed is constant over the lifetime of the bullet

	if (range <= 0) 
	{
		kill();
		return;
	}
		
	int mx1, my1, mx2, my2;
	int ix, iy;
	TEG_MAP *map = getMap();
	mx1 = ((int)getx()) / map->tilelist->tilew;
	my1 = ((int)gety()) / map->tilelist->tileh;
	mx2 = ((int)getx() + w - 1) / map->tilelist->tilew;
	my2 = ((int)gety() + h - 1) / map->tilelist->tileh;
			
	// loop through all map positions we touch with the solid region
	for (ix = mx1; ix <= mx2; ++ix)
	{
		for (iy = my1; iy <= my2; ++iy)
		{
			// see if there is a solid tile at this position
			if (getTileStackFlags (ix, iy) & TS_SOLID)
			{
				// leave a splash
				kill();
			}
		}
	}
}


void Bullet::handleCollission(ObjectBase *o)
{
	if (type == OT_BULLET && o->getType() == OT_MONSTER) // bullet hits monster
	{
		kill();
	}
	if (type == OT_ENEMY_BULLET && o->getType() == OT_PLAYER) // enemy bullet hits player
	{
		kill();
	}
}
	
void PickUp::handleCollission(ObjectBase *o)
{
	if (o->getType() == OT_PLAYER)
	{
		kill();
	}
}

void PickUp::init(std::shared_ptr<Resources> res)
{
	anims[AnimType::BANANA] = res->getAnim("banana");
	anims[AnimType::HEALTH] = res->getAnim("health");
	anims[AnimType::KEY] = res->getAnim("key");
	anims[AnimType::BONUS1] = res->getAnim("bonus1");
	anims[AnimType::BONUS2] = res->getAnim("bonus2");
	anims[AnimType::BONUS3] = res->getAnim("bonus3");
	anims[AnimType::BONUS4] = res->getAnim("bonus4");
	anims[AnimType::HEALTHCONTAINER] = res->getAnim("healthcontainer");
}

Anim *PickUp::anims[PickUp::AnimType::ANIM_NUM];

Player::Player(PlayerState *_ps, Room *r, int _playerType) : Object (r, OT_PLAYER)
{
	ps = _ps;
	transportCounter = 0;
	setVisible(true);
	hittimer = invulnerabilityDelay; // start invulnerable, because you could start near an enemy
	attacktimer = 0;
	button = NULL;
	playerType = _playerType;
	solid = true;
	
	setAnim (walk[playerType]);	
}

Anim *Player::walk[2];
ALLEGRO_SAMPLE *Player::samples[Player::SAMPLE_NUM];
ALLEGRO_SAMPLE *Player::shoot[2][6];

void Player::init(std::shared_ptr<Resources> res)
{
	walk[0] = res->getAnim ("fole_walk");
	walk[1] = res->getAnim ("raul_walk");

 	samples[HURT1] = res->getSample ("miau2");
 	samples[HURT2] = res->getSample ("uh");
 	samples[PICKUP_KEY] = res->getSample ("Pick_up_key_2");
 	samples[PICKUP_OTHER] = res->getSample ("Pick_up_key_1");
 	samples[UNLOCK] = res->getSample ("Door_unlock");
 	samples[STEPS] = res->getSample ("Footsteps_double");
 	samples[TELEPORT] = res->getSample ("AlienSiren");

	shoot[1][0] = res->getSample ("Strum_1");
	shoot[1][1] = res->getSample ("Strum_2");
	shoot[1][2] = res->getSample ("Strum_3");
	shoot[1][3] = res->getSample ("Strum_4");
	shoot[1][4] = res->getSample ("Strum_5");
	shoot[1][5] = res->getSample ("Strum_6");
	shoot[0][0] = res->getSample ("Hit1");
	shoot[0][1] = res->getSample ("Hit2");
	shoot[0][2] = res->getSample ("Hit3");
	shoot[0][3] = res->getSample ("Hit4");
	shoot[0][4] = res->getSample ("Hit5");
	shoot[0][5] = res->getSample ("Hit6");

}

void Player::update()
{
	if (!button) button = engine->getInput(playerType);
	assert (button);

	// reduce transportCounter
	// when touching a transport (door or teleport), transportCounter is increased again in handlecollision.
	if (transportCounter > 0)
	{
		transportCounter--;
	}
	
	if (hittimer > 0)
	{
		hittimer--;
		setVisible (!(hittimer & 0x02));
	}
	
	bool freshAnim = false;
		
	// attacking
	if (attacktimer > 0)
	{
		attacktimer --;
	}
	else
	{
		if (button[btnAction].getState())
		{
			attacktimer = ps->wpnSpeed;
			int idx = random(6);
			MainLoop::getMainLoop()->playSample (shoot[ps->wpnType == 0 ? 0 : 1][idx]);
	
			double angle = LEGACY_DIR[getDir()].angle;

			// generate bullet...
			double nozx = getx() + LEGACY_DIR[getDir()].dx * 12;
			double nozy = gety() + LEGACY_DIR[getDir()].dy * 12;

			double angleDelta[] = { 0, -0.2, 0.2, -M_PI_2, M_PI_2 };
			int bulletNum;
			switch (ps->wpnType) {
				case 0: bulletNum = 1; break;
				case 1: bulletNum = 3; break;
				default: bulletNum = 5; break;
			}

			for (int i = 0; i < bulletNum; ++i) {
				double delta = angleDelta[i];
				Bullet *bullet = new Bullet(getRoom(), angle + delta, playerBulletSpeed, ps->wpnRange, ps->wpnDamage, this);
				bullet->setDir(getDir());
				game->getObjects()->add (bullet);
				bullet->setLocation (nozx, nozy);
			}
		}
	}
	
	double dx = 0, dy = 0;

	const double speedFactor = 3;
	
	int newDir = -1;
	
	if (button[btnLeft].getState()) {
		dx = -speedFactor;
		newDir = LEFT;
	} 
	else if (button[btnRight].getState()) {
		dx = speedFactor;
		newDir = RIGHT;
	}

	try_move (dx, 0);
	
	if (button[btnUp].getState()) {
		dy = -speedFactor;
		newDir = UP;
	}
	else if (button[btnDown].getState()) {
		dy = speedFactor;
		newDir = DOWN;
	} 

	try_move (0, dy);

	// walking or not?
	if (newDir != -1) {
		isWalking = false;
		if (getDir() != newDir) {
			setDir(newDir);
			setAnim (walk[playerType]);
		}
	}
	else isWalking = true;

	// handle animation
	Object::update();
}

void Player::hit(int damage)
{
	hittimer = invulnerabilityDelay;
	
	MainLoop::getMainLoop()->playSample (samples[playerType == 0 ? HURT1 : HURT2]);
	
	ps->hp -= damage;
	if (ps->hp <= 0)
	{
		// drop keys, they are essential objects for the other player
		while (ps->keys > 0) {
			ps->keys--;
			auto key = new PickUp(getRoom(), OT_KEY);
			game->getObjects()->add (key);
			key->setLocation (getx() + random(20) - 10, gety() + random(20) - 10);
		}

		ps->died = true;
		setVisible (false);
		solid = false;
		kill(); // TODO: dying animation
	}	
}

void Player::handleCollission (ObjectBase *o)
{
	if (ps->died) return;
	
	switch(o->getType()) {
		case OT_BANANA: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->bananas++;
		}
		break;
		case OT_KEY: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_KEY]);
			ps->keys++;
		}
		break; 
		case OT_HEALTH: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->hp = std::min(ps->hp + heartHealthValue, ps->hpMax);
		}
		break; 
		case OT_HEALTHCONTAINER: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->hpMax += 25;
			ps->hp = ps->hpMax;
			game->messages->showMessage("Max Health Up", Messages::POP_UP);
		}
		break; 
		case OT_BONUS1: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			game->addTime(30000); // add 30 seconds
		}
		break; 
		case OT_BONUS2: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->wpnRange += 40;
			game->messages->showMessage("Increased weapon range", Messages::POP_UP);
		}
		break; 
		case OT_BONUS3: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->wpnDamage += 2;
			game->messages->showMessage("Increased weapon power", Messages::POP_UP);
		}
		break; 
		case OT_BONUS4: {
			MainLoop::getMainLoop()->playSample (samples[PICKUP_OTHER]);
			ps->wpnType++;
			game->messages->showMessage("Weapon upgrade", Messages::POP_UP);
		}
		break; 
		case OT_MONSTER: { // monster
			Monster *m = dynamic_cast<Monster*>(o);
			if (hittimer == 0) hit(m->getDamage());
		}
		break; 
		case OT_LOCKED_DOOR: {
			if (ps->keys > 0) {
				Door *d = dynamic_cast<Door*>(o);			
				d->setLocked(false);
				MainLoop::getMainLoop()->playSample (samples[UNLOCK]);
				ps->keys--;
				// TODO avoid going through straight away
			}
		}	
		break; 
		case OT_DOOR: { // exit
			if (transportCounter == 0)
			{
				Door *d = dynamic_cast<Door*>(o);
				assert (d);
				MainLoop::getMainLoop()->playSample (samples[STEPS]);
				if (d->otherRoom != NULL)
				{
					setRoom(d->otherRoom);
					setLocation(d->otherDoor->getx(), d->otherDoor->gety());
				}
				hittimer = invulnerabilityDelay;
			}
			transportCounter = transportDelay; // make sure we don't go back
		}
		break; 
		case OT_TELEPORT: {
			if (transportCounter == 0)
			{
				Door *t = dynamic_cast<Door*>(o);
				assert (t);
				MainLoop::getMainLoop()->playSample (samples[TELEPORT]);
				if (t->otherRoom != NULL)
				{
					setRoom(t->otherRoom);
					setLocation(t->otherDoor->getx(), t->otherDoor->gety());
				}
				hittimer = invulnerabilityDelay;
			}
			transportCounter = transportDelay; // make sure we don't go back		
		}
		break; 
		case OT_ENEMY_BULLET: {		
			Bullet *b = dynamic_cast<Bullet*>(o);
			if (hittimer == 0) hit(b->getDamage());
		}
	}
}
