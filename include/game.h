#pragma once

#include "state.h"

class Engine;
struct TEG_MAP;
struct ALLEGRO_FONT;
class Sprite;
class SpriteEx;
class Player;

class Game : public State
{
public:
	// used in engine
	virtual void init() = 0;
	virtual void done() = 0;
	virtual void initGame() = 0;
	virtual void initMap() = 0;
	virtual int getLives() = 0;

	static std::shared_ptr<Game> newInstance(Engine *parent);

	// used by various sprites...
	virtual void collectBonus(int index) = 0;
	virtual std::list<Sprite*> &getSprites() = 0;
	virtual TEG_MAP *getMap() = 0;
	virtual void addSprite(Sprite *o) = 0;
	virtual void addCollision(SpriteEx *a, SpriteEx *b, int dir) = 0;
	virtual Engine *getParent() = 0;
	virtual void exitMap(int dir, int y) = 0;
	virtual void teleport(Point targetPos) = 0;
	virtual void updateWaterLevel() = 0;
	virtual int getLocalWaterLevel() = 0;
	virtual void setPlayerHp(int val) = 0;
	virtual int getTotalSocks() = 0;
	virtual int getSocksCollected() = 0;
	
	// per-level, initialized in initLevel
	// TODO: accessor
	Player *player; //may be nullptr // TODO turn into weak ptr.
	ALLEGRO_FONT *gamefont;
	ALLEGRO_FONT *smallfont;

	enum { MSG_PLAYER_DIED = 4001, MSG_PLAYER_WIN }; /* PRIME */

};
