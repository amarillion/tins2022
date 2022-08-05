#ifndef PLAYER_H
#define PLAYER_H

#include "object.h"
#include "settings.h"

class Resources;
class Player;
	
class Bullet : public Object
{
	private:
		
		double dx, dy;
		double range; // range in pixels
		
		int damage;
		Player *parent;
	public:
		Player *getPlayer() { return parent; }
		Bullet(Room *r, double angleRad, double speed, int _range, int _damage, Player *_parent);
		
		virtual void update() override;
		virtual void handleCollission(ObjectBase *o);
		static void init(std::shared_ptr<Resources> res);
		
		int getDamage() { return damage; }
		static Anim *bullet[1];
};

class PickUp : public Object
{
public:		
	PickUp(Room *r, int type) : Object (r, type) { 
		solid = true; 
		switch(type) {
			case OT_BANANA: setAnim(anims[AnimType::BANANA]); break;
			case OT_HEALTH: setAnim(anims[AnimType::HEALTH]); break;
			case OT_HEALTHCONTAINER: setAnim(anims[AnimType::HEALTHCONTAINER]); break;
			case OT_KEY: setAnim(anims[AnimType::KEY]); break;
			case OT_BONUS1: setAnim(anims[AnimType::BONUS1]); break;
			case OT_BONUS2: setAnim(anims[AnimType::BONUS2]); break;
			case OT_BONUS3: setAnim(anims[AnimType::BONUS3]); break;
			case OT_BONUS4: setAnim(anims[AnimType::BONUS4]); break;
			default: assert(false);
		}

	}
	virtual void handleCollission(ObjectBase *o);
	static void init(std::shared_ptr<Resources> res);		
	
	enum AnimType { BANANA, HEALTH, KEY, BONUS1, BONUS2, BONUS3, BONUS4, HEALTHCONTAINER, ANIM_NUM };
	static Anim *anims[ANIM_NUM];
};

class PlayerState
{
	public:
	static const int defaultWpnSpeed = 20;
	static const int defaultWpnRange = 120;
	static const int defaultWpnDamage = 4;
	static const int defaultHpMax = 100;
	
	// player stats	
	int hp = defaultHpMax; // hitpoints
	int hpMax = defaultHpMax; // max hitpoints
	int wpnSpeed = defaultWpnSpeed; // weapon rate of fire
	int wpnRange = defaultWpnRange; // weapon range, distance bullets can go
	int wpnDamage = defaultWpnDamage; 
	int lootTableCounter = 0;
	int wpnType = 0;
	int bananas = 0; // bananas collected
	int keys = 0; // keys collected
	int xp = 0; // gold collected for killing monsters
	bool died = false;
};

class Player : public Object
{
	static const int transportDelay = 5;
	static const int invulnerabilityDelay = 50;
	static const int heartHealthValue = 50;
	static constexpr double playerBulletSpeed = 6.0;
	static const int defaultWpnDamage = 4;
	
	int transportCounter;

	static Anim *walk[2];
	static ALLEGRO_SAMPLE *shoot[2][6];
	enum Samples { HURT1, HURT2, UNLOCK, STEPS, PICKUP_KEY, PICKUP_OTHER, TELEPORT, SAMPLE_NUM };
	static ALLEGRO_SAMPLE *samples[SAMPLE_NUM];
	
	int hittimer;
	int attacktimer;
	bool isWalking;
		
	int playerType; // 0 for fole, 1 for raul
	
	Input *button;
	
	void hit(int damage);
	
	enum State { CONTROL, ANIMATING };
	State eState = CONTROL;
public:
	PlayerState *ps;
	Player(PlayerState *ps, Room *r, int _playerType);
	virtual void update() override;
	static void init(std::shared_ptr<Resources> res);
	virtual void handleCollission(ObjectBase *o);
};

#endif
