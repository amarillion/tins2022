#include <assert.h>
#include "monster.h"
#include "resources.h"
#include "engine.h"
#include <stdlib.h>
// #include <lua.h> //TODO
#include "game.h"
#include "mainloop.h"
#include "util.h"

Anim *Monster::sprites[MONSTER_NUM];
ALLEGRO_SAMPLE *Monster::samples[MONSTER_SAMPLE_NUM];

Monster::Monster(Room *r, int subType, int _hp) : Object (r, OT_MONSTER), monsterType (subType)
{
	setVisible(true);
	solid = true;
	setAnim(sprites[monsterType]);
	setDir (rand() % 4);
	hp = _hp;
}

void Monster::init(std::shared_ptr<Resources> res)
{
	sprites[BLUE_FLOWER] = res->getAnim ("FlowerBlue");
	sprites[BLUE_BELL] = res->getAnim ("BellBlue");
	sprites[ORANGE_FLOWER] = res->getAnim ("FlowerOrange");
	sprites[ORANGE_BELL] = res->getAnim ("BellOrange");
	sprites[GREEN_BUG] = res->getAnim ("BugGreen");
	sprites[RED_BUG] = res->getAnim ("BugRed");
	sprites[BLUE_BUG] = res->getAnim ("BugBlue");
	
	samples[0] = res->getSample ("fugly2");
	samples[1] = res->getSample ("fugly2");
	samples[2] = res->getSample ("mugly2");
	samples[3] = res->getSample ("tux");
}

void Monster::newState(State newState, int time) {
	switch(newState) {
		case MOVERANDOM: {
			setState(0);
			Player *p = game->getNearestPlayer (this);
			if (p && (rand() % 100 > 30))
			{
				int dx = p->getx() - getx();
				int dy = p->gety() - gety();
				if (dx * dx + dy * dy < chaseRadius * chaseRadius)
				{
					if (abs (dx) > abs(dy))
					{
						if (dx > 0)				
							setDir (RIGHT);
						else
							setDir (LEFT);
					}
					else
					{
						if (dy > 0)				
							setDir (DOWN);
						else
							setDir (UP);				
					}
				}
				else
				{
					setDir (rand() % 4);
				}				
			}
			else
			{
				setDir (rand() % 4);
			}
		}
			break;
		case WAIT:
			setState (3);
			break;
		case KNOCKBACK:
			if (hp < 0) {
				// death animation. Change type so it can't collide with player in its new state.
				setState(2);
				type = OT_DEADMONSTER;
			}
			else {
				setState (1);
			}
			break;
		case SHOOT:
			setState (4);
			break;
	}
	count = time;
	eState = newState;
};

void Monster::determineNextState() {
	switch (monsterType) {
		case BLUE_FLOWER: // Flower
		case ORANGE_FLOWER:
			if (eState != SHOOT && random(100) > 30) {
				newState(SHOOT, defaultTelegraphingDelay);
			}
			else {
				if (eState == WAIT) {
					newState(MOVERANDOM, rand() % 50 + 25);
				}
				else {
					newState(WAIT, rand() % 50 + 25);
				}
			}
			break;
		case BLUE_BELL:
		case ORANGE_BELL:
			newState(SHOOT, rand() % 80 + 50);
			break;
		CASE: RED_BUG:
			if (eState == WAIT) {
				newState(MOVERANDOM, rand() % 50 + 25);
			}
			else {
				newState(WAIT, rand() % 50 + 25);
			}
			break;
		case GREEN_BUG:
		case BLUE_BUG:
			newState(MOVERANDOM, rand() % 50 + 25);
			break;
	}
}

void Monster::update()
{
	// update on a given state
	count--;
	if (count <= 0)
	{
		// finish action determine next action..
		switch(eState) {
			case KNOCKBACK:
				if (hp < 0) { kill(); }
				break;
			case SHOOT:
				for (int i = 0; i < 4; ++i) {
					const double angle = (double)i * 0.5 * M_PI;
					Bullet *bullet = new Bullet(getRoom(), angle, defaultBulletSpeed, defaultEnemyRange, 
						defaultEnemyDmg, nullptr);
					game->getObjects()->add (bullet);
					bullet->setLocation (getx() + 8, gety());
				}
				break;
			default:
				// do nothing
				break;
		}

		determineNextState();
	}

	// continue doing current action...
	int speedFactor = 0;

	switch (eState) {
		case KNOCKBACK:
			speedFactor = 4;
			break;
		case WAIT:
			speedFactor = 0;
			break;
		case SHOOT:
			speedFactor = 0;
			break;
		case MOVERANDOM:
			speedFactor = 2;
			break;
	};

	if (speedFactor > 0) {
		double dx = speedFactor * LEGACY_DIR[getDir()].dx;
		double dy = speedFactor * LEGACY_DIR[getDir()].dy;
		try_move (dx, dy);
	}
		
	Object::update();
}

void Monster::createPickup(Player *p) {
	p->ps->lootTableCounter++;
	if ((p->ps->lootTableCounter % 8) == 0) {
		PickUp *heart = new PickUp(getRoom(), OT_HEALTH);
		game->getObjects()->add (heart);
		heart->setLocation (getx() + 8, gety());
	}
}

void Monster::handleCollission(ObjectBase *o)
{
	
	if (o->getType() == OT_BULLET) // player attack
	{
		if (hp < 0) return; // already dead, no need to become deader
	
		Bullet *b = dynamic_cast<Bullet *>(o);
		Player *p = b->getPlayer();
		hp -= p->ps->wpnDamage;
		if (hp < 0)
		{
			p->ps->xp += defaultXpValue;
			createPickup(p);
		} // killed after hitcount == 0 again
		
		// bullet impact changes dir
		setDir (b->getDir());
		
		newState(KNOCKBACK, defaultHitDelay);
		MainLoop::getMainLoop()->playSample (samples[monsterType % MONSTER_SAMPLE_NUM]);
	}	
}
