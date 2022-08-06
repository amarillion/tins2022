#pragma once

#include "sprite.h"

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
	int damage = 0; // damage dealt when hitting or attacking.
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
	int getAttackDamage() { return damage; }
	bool isUnpassable() { return unpassable; }
	bool isUnderWater();
};
