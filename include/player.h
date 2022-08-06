#pragma once

#include "spriteex.h"
#include "input.h"

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
	void hit(int attackDamage, double delta);
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
