#ifndef MONSTER_H
#define MONSTER_H

#include "object.h"
// #include <lua.h> //TODO

enum MonsterType {
	ORANGE_BELL, BLUE_BELL,
	ORANGE_FLOWER, BLUE_FLOWER,
	GREEN_BUG, RED_BUG, BLUE_BUG,
	MONSTER_NUM
};
const int MONSTER_SAMPLE_NUM = 4;

class Player;
class Monster : public Object
{
private:
	static const int defaultHitDelay = 20;
	static const int defaultTelegraphingDelay = 30;
	static const int defaultHitPoints = 20;
	static const int defaultXpValue = 24;
	static const int defaultEnemyRange = 200;
	static constexpr double defaultBulletSpeed = 3.0;
	static const int defaultEnemyDmg = 20;
	static const int chaseRadius = 160;
	
	static Anim *sprites[MONSTER_NUM];
	static Anim *hitsprites[MONSTER_NUM];
	static ALLEGRO_SAMPLE *samples[4];
	int monsterType;
	int hp;
	int count = 0;

	enum State { WAIT, MOVERANDOM, SHOOT, KNOCKBACK };
	State eState = WAIT;
	void createPickup(Player *p);
public:
	Monster(Room *r, int type, int _hp);
	static void init(std::shared_ptr<Resources> res);

	virtual void update() override;
	virtual void handleCollission(ObjectBase *o);

	virtual void newState(State newState, int time);
	virtual void determineNextState();

	int getDamage() { return defaultEnemyDmg; }
};

#endif
