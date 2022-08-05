#include "sprite.h"
#include "spritedata.h"
#include <memory>

#ifndef PLAYER_H
#define PLAYER_H

class Anim;
class PlayerControl;
struct ALLEGRO_SAMPLE;

const int RELOAD_TIME = 20;
const int LIVE_TIME = 50;
const int MISSILE_LIVE_TIME = 100;
const float PLAYER_SPEED_INC = 0.1;
const float PLAYER_MAX_SPEED = 5.0;

float sq_dist (Sprite *src, Sprite *dest);
void delta_vector (Sprite *src, Sprite *dest, float &sq_dist, float &radians);

/* makes angle go between -M_PI and M_PI */
void normalize_angle (float &angle);

// base class for anything that can shoot, move, has gravity, explode, etc.
class ActiveSprite : public Sprite
{
private:
	int state;
	bool has_state50;
	bool has_rubble;
protected:
	bool dead;
public:
	float speed;
	float angle;
	float turning_speed = 0.07;
	ActiveSprite (PlayerControl *g, SpriteSubType _st);
	int getHealth() const { return health; }
	Anim *current;
	Sprite *target;
	float nozzlex;
	float nozzley;
	float nozzlez;
	bool isDead() const { return dead; }
protected:
	int tKill;
	float dz;
	SpriteSubType subType;
	int health;
	void explode (int scale);
	void setState (int value);
	Sprite *getTarget () { return target; }
	virtual void handleBlock ();
	void turnOnGravity() { dz = 0; gravity = true; } // e.g. when chopper is damaged
	//vector<Weapon> weapons;
	virtual void update();
public:
	void setTarget (Sprite *);
	virtual ~ActiveSprite() { setTarget (NULL); } // release lock
	SpriteSubType getSubType() { return subType; }
	
	// fire a weapon from the weapons array
	//void fireWeapon (Weapon & w, Sprite *targe);
	//void inRange (Weapon & w);
	
	void fireBullet (); // for chopper
	void fireBullet2 (); // for enemy bunker
	void fireBullet3 (); // for player tank
	void fireBullet4 (); // for enemy turret / tank
	void fireMissile (); // for sam sites
	
	void draw(const GraphicsContext &gc);
	static void init(std::shared_ptr<Resources> res);
	virtual void decreaseHealth(int amount);

	static ALLEGRO_SAMPLE *expl1;
	static ALLEGRO_SAMPLE *expl2;
	static ALLEGRO_SAMPLE *shoot1;
	static ALLEGRO_SAMPLE *shoot2;
	static ALLEGRO_SAMPLE *shoot3;
	static ALLEGRO_SAMPLE *shoot4;
	static ALLEGRO_SAMPLE *hurt;
	static ALLEGRO_SAMPLE *lose;
	static ALLEGRO_SAMPLE *gift;

};

class Player : public ActiveSprite
{
private:
	bool accelerate;
public:
	Player (PlayerControl *g, SpriteSubType st);
	virtual SpriteType getType () const { return ST_PLAYER; } 
	virtual void handleCollision(Sprite *s);
	void setAccelerate();
	virtual void update();	
	void handleBlock ();
	void lift(float dz);
	virtual void decreaseHealth(int amount);
};

class Enemy : public ActiveSprite
{
private:
	int tReload;
	bool mobile; // mobile yes / no?
	int viewRange; // range where you start shooting
	int tEvasive;
public:
	Enemy (PlayerControl *g, SpriteSubType st);
	virtual SpriteType getType () const { return ST_ENEMY; } 
	virtual void handleCollision(Sprite *s);
	virtual void handleBlock();
	virtual void update();
};

class Bonus : public ActiveSprite
{
public:
	Bonus(PlayerControl *g, SpriteSubType st);
	virtual SpriteType getType () const { return ST_BONUS; } 
	virtual void handleCollision(Sprite *s) {}
};

class Vaporeal : public ActiveSprite
{
private:
	float dx, dy, dz;
	int tLive;
public:
	Vaporeal(PlayerControl *g, SpriteSubType st, float _dx, float _dy, float _dz);
	virtual SpriteType getType () const { return ST_VAPOREAL; }
	virtual void handleCollision(Sprite *s) {}
	virtual void update() override;
};

class Solid : public ActiveSprite
{
private:
public:
	Solid (PlayerControl *g, SpriteSubType st);
	virtual SpriteType getType () const { return ST_OTHER; } 
	virtual void handleCollision(Sprite *s);
};

class Bullet : public ActiveSprite
{
private:
	Sprite *source;
	float dx, dy, dz;
	float speed;
	int tLive;
	int maxLive;
	int damage;
	void setSource (Sprite *_source);
public:
	int getDamage() { return damage; }
	Sprite *getSource() { return source; }
	Bullet(PlayerControl *g, Sprite *_source, Sprite *_target, Direction startdir, float _speed);
	Bullet(PlayerControl *g, Sprite *_source, float dx, float dy, float dz);
	virtual SpriteType getType () const { return ST_BULLET; } // release lock
	virtual void handleCollision(Sprite *s);
	virtual void handleLanding() override;
	virtual void update();
	virtual ~Bullet() { setSource (NULL); }
};

class Explosion : public Sprite
{
	int tLive;
	int maxLive;
public:
	Explosion(PlayerControl *g, int size);
	virtual SpriteType getType () const { return ST_PLAYER; } 
	virtual void handleCollision(Sprite *s) {}
	virtual void draw(const GraphicsContext &gc);
	virtual void update();
};

#endif
