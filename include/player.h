#pragma once

#include "sprite.h"
#include "input.h"

enum SpriteType
{
	ST_PLAYER, ST_ENEMY, ST_SCORE, ST_TILE, ST_BOUNDS, ST_FLOOR,
	ST_BULLET, ST_ENEMY_BULLET, ST_PLATFORM, ST_BONUS,
	ST_TELEPORTER, ST_SWITCH
};

class SpriteEx : public Sprite
{
protected:
	bool air;
	bool gravity;
	float dy;
	float dx;
	SpriteType spriteType;
	int subtype;
	int hp;
	bool unpassable; // can not overlap with other unpassable objects
	int pushForce;
public:
	int getPushForce() { return pushForce; }
	enum { DIR_UP = 1, DIR_DOWN = 2, DIR_LEFT = 4, DIR_RIGHT = 8, BLOCKED_BY_TILE = 16 };
	SpriteEx(Game *, SpriteType st, int x, int y, int subtype = 0);
	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override;
	int onPush (double dx, double dy);
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	SpriteType getSpriteType() { return spriteType; }
	int getSubType() { return subtype; }
	bool isUnpassable() { return unpassable; }
};

class Bullet : public SpriteEx
{
private:
public:
	enum { WPN_ROCK, WPN_ICE, WPN_BAZOOKA, WPN_LASER };
	Bullet(Game * game, int type, int x, int y, double dx, double dy);
	virtual void update();
	virtual void onCol (SpriteType st, Sprite *s, int dir);
};

class EBullet : public SpriteEx
{
private:
	int timer;
public:
	enum { FIRE = 10, ENERGY };
	EBullet(Game * game, int type, int x, int y, double dx, double dy);
	virtual void update();
	virtual void onCol (SpriteType st, Sprite *s, int dir);
};

class Player : public SpriteEx
{
private:
	Input *btn;
	bool isWeaponAvailable(int wpn);
	void die(); // player died
	void shoot(); // shoot a bullet
	void setState(bool hit); // set proper player state
	void hit(int subtype, double delta);
public:
	virtual void kill ();
	double getdir() { return dir; }
	int jumpTimer = 0;
	int hitTimer = 0;
	int currentWeapon = 0;
	int shootTimer = 0;
	bool control = true;
	Player(Game *, int x, int y);
	virtual void update();
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	virtual void draw(const GraphicsContext &gc);

};

class Enemy : public SpriteEx
{
	int enemyType;
	int phase;
	int period;
	float hsign;
	float vsign;
	int hittimer;

	int estate;
	double destx;
	double desty;
	int bulletTimer;
	int jumpTimer;
public:

	Enemy(Game *, int x, int y, int _type);
	virtual void draw(const GraphicsContext &gc);
	virtual void update();
	void spawn(int val);
	void generatorSpawn();
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	virtual void kill();
	void moveTo (double destx, double desty, double speed);
	bool nearDest ();
	void update1();
	void update2();
	void update3();
	void update4();
	void update5();

	enum { ELECTRICAT, SLINKYCAT, SPIDERCAT, DRAGONCAT, GENERATOR };
};

class Bonus : public SpriteEx
{
	int index; // index in RedSock array
public:
	Bonus (Game *, int x, int y, int bonusType);
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	
	enum { ONEUP, SOCK, RING };
};

class Switch : public SpriteEx {
	int coolDown = 0;
public:
	Switch(Game *, int x, int y, int startState);
	virtual void onCol(SpriteType st, Sprite *s, int dir) override;
	virtual void update() override { if(coolDown > 0) coolDown--; }
	enum { OFF = 0, ON = 1 };
};

class Teleporter : public SpriteEx {
public:
	Teleporter (Game *, int x, int y);
	virtual void onCol (SpriteType st, Sprite *s, int dir);
};

class Platform : public SpriteEx
{
public:
	Platform (Game *, int x, int y, int subtype);
	virtual void onCol (SpriteType st, Sprite *s, int dir);
	enum { FALLING, CRATE, SMALLCRATE };
};

class Explosion : public Sprite
{
	int tLive;
	int maxLive;
	ALLEGRO_COLOR color;
	int size;
public:
	Explosion(Game *e, int x, int y, int life);
	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override;
};
