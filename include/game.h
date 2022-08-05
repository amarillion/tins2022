#ifndef GAME_H
#define GAME_H

#include "tegel5.h"
#include <allegro5/allegro.h>
#include <list>
#include <map>
#include <vector>
#include "level.h"
#include <DrawStrategy.h>
#include "componentbuilder.h"
#include "state.h"
#include "twirl.h"
#include "map2d.h"

class SpriteLayer;
class Engine;
class Resources;
class Game;
class Anim;
class BitmapComp;
struct ALLEGRO_SAMPLE;

const int SPRITETYPE_NUM = 5;
enum SpriteType { ST_BONUS, ST_ROCK, ST_PLAYER, ST_ENEMY, ST_EXPLOSION };

const int CLOTHES_NUM = 20;

enum Direction { N, E, S, W };
const int DIR_DX[4] = { 0, -1, 0, 1 };
const int DIR_DY[4] = { -1, 0, 1, 0 };

enum ActionType { AT_MOVE, AT_TURN, AT_FALL_DOWN, AT_FALL_LEFT, AT_FALL_RIGHT };

const int CLOTHINGTYPE_NUM  = 4;
enum ClothingType { CT_BOTTOM, CT_SHOES, CT_TOP, CT_ACCESSORY };

extern const char *clothesTypeNames[CLOTHINGTYPE_NUM];

class ClothesPickup
{
public:
	int mx;
	int my;
	int index;
};

class Sprite
{
	friend class Game;
private:

	int mx;
	int my;
	float subtileCounter; // from 0 to 32 (== TILESIZE)
	float speed;
	int mdx; // -1, 1, 0
	int mdy; // -1, 1, 0
	int subx; // sub-tile offset in pixels
	int suby; // sub-tile offset in pixels
	bool moving;
	bool enemyGo;
	int waitTimer;
	
	SpriteType spriteType;
	ActionType action;
	Direction dir;
	int animState;
	bool cleanupTail; // cleanupTail immediately cleans up trailing position, to make it safe to touch blocks being pushed
	
	bool tryMove (int aDx, int aDy, float aSpeed);
	void setParent (Game *_parent);
	bool alive;
	int pushBuildup;
	
	int getMapAt (int dx, int dy);
	void setMapAt (int dx, int dy, int val);
	void occupyMap (int dx, int dy);
	void clearMap (int dx, int dy);
	Sprite *getSpriteAt (int dx, int dy);
	bool isClearForEnemy (int dx, int dy);
	
	bool visible;
	
	Game* parent;
	Engine *engine;
	
	// lock == reference counting mechanism
	void kill ();
	Anim *anim;
	int animStart;

	bool subTileMovement();
	bool isTumbling();
	void updatePlayer();
	void updateEnemy();
public:
	int lock;
	Direction getDir() const { return dir; }
	void setDir(Direction _dir);

	Sprite (Engine *e, Game * game, int amx, int amy, SpriteType st);
	virtual ~Sprite () {}
	
	// only destroy if no references are held
	bool canDestroy() const { return (!alive && lock == 0); };
	
	bool isAlive() const { return alive; } // if not, scheduled to be killed
	bool isVisible() const { return visible; }
			
	virtual void draw(const GraphicsContext &gc);
		
	int getx () const { return mx * 32 + subx; }
	int gety () const { return my * 32 + suby; }
	
	virtual int update();
};

// const int BACKGROUND_FRAMES = 3;
struct Light {
	int onLayer, offLayer;
	int mx, my;
};

class Game : public State
{
	Engine *parent;
	friend class Sprite;
	std::list <Sprite*> sprites;	
	TEG_MAP *current;
	Map2D<Sprite*> spriteMap;
	int outroTimer;
	std::vector<Light> lights;
	ALLEGRO_BITMAP *outroBitmap;

	bool isclear (int x, int y);
	bool canTumble (int mx, int my, int dx);
	
	void add(Sprite *o);

	void drawSidePanel (ALLEGRO_BITMAP *dest);
	
	int getFlagsAt (int x, int y);
	
	Sprite *player;
	bool clothesOk;
	Sprite *tileToSprite (int mx, int my, SpriteType type);
	void explode (int x, int y);
	bool gameover;
	
	Anim *anims[SPRITETYPE_NUM];
	std::vector<ClothesPickup> clothesPickups;
		
	void calculateScores();
	
	int getSeconds();	
	int startTime;
	ALLEGRO_SAMPLE *hitSample;
	ALLEGRO_SAMPLE *explosionSample;
	
	int xofst;
	int yofst;

	std::shared_ptr<TwirlEffect> twirl;

public:
	bool matchBonus;
	int lastLevel;
	ALLEGRO_FONT *gamefont;
	int hiscore_per_level[LEVEL_NUM];
	ALLEGRO_BITMAP *doll[2];
	int clothes[CLOTHINGTYPE_NUM];
	int timeScore;
	int totalScore;
	bool newLevelUnlocked;
	int previousHiScore;

	void debugLevelFinished();

	void killAll();
	
	Game (Engine *engine);
	virtual ~Game();
	void write(); // read list of unlocked levels
	void read(); // read list of unlocked levels
	
	void score(int mx, int my);
	void startOutro();
	
	void init();
	void initLevel(int _level);
	
	virtual void update() override;
	virtual void draw(const GraphicsContext &gc) override;

	int getTotalStars() { 
		int stars = 0;
		for (int i = 0; i < LEVEL_NUM; ++i) {
			stars += getStarsPerLevel(i);
		}
		return stars;
	}
	int getStarsPerLevel(int level) {
		if (hiscore_per_level[level] > 1000)
			return 2;
		else if (hiscore_per_level[level] > 0)
			return 1; 
		return 0;
	}
};

class ClothesData
{
public:
	const char *id;
	const char *large;
	const char *small;
	ClothingType ct;
	int order;
	ALLEGRO_BITMAP *smallbmp;
	ALLEGRO_BITMAP *largebmp;
};

extern ClothesData clothesData[CLOTHES_NUM];

#endif
