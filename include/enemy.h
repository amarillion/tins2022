#pragma once

#include "spriteex.h"

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

	enum { ELECTRICAT, SLINKYCAT, SPIDERCAT, 
		DRAGONCAT, GENERATOR, TELECAT, ROLLINGCAT, SHARKCAT };
};

