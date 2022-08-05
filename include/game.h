#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <list>
#include <map>
#include <vector>
#include "input.h"
#include "component.h"
#include "container.h"
#include "viewport.h"
#include "state.h"

struct TEG_MAP;
class SpriteLayer;
class Engine;
class Resources;
class Game;
class Anim;

class Player;
class SpriteEx;
class GraphicsContext;

// tile stack properties
#define TS_SOLID 0x01
#define TS_SOFT 0x04

class Sprite
{
	friend class Game;
	friend class Player;
protected:
	bool awake, alive, visible;
	int counter;
	double x, y;
	int w, h;

	IMotionPtr motion;

	Game* parent;
	Anim *anim;
	int state;
	int dir;
	int animStart;
	bool blockedByTiles; /* means that this object can not move through solid tiles */

private:
	int waitTimer;
	void setParent (Game *_parent);
public:
	double gety() { return y; }
	double getx() { return x; }
	int getw () const { return w; }
	int geth () const { return h; }
	void sety(double _y) { y = _y; }
	void setx(double _x) { x = _x; }

	int getTileStackFlags(int mx, int my);
	int try_move (double dx, double dy, int push_force = 0);
	Sprite (Game * game, int amx, int amy);
	void setAnim (Anim *_anim) { anim = _anim; }
	virtual ~Sprite () {}

	void setMotion(const IMotionPtr &value) { motion = value; }
	IMotionPtr getMotion() { return motion; }

	// if alive is set to false, this component will be removed
	// neither update() nor draw() will be called.
	bool isAlive() { return alive; }
	void kill() { alive = false; }

	// if visible is set to false, draw() will not be called
	bool isVisible() { return visible; }
	void setVisible(bool value) { visible = value; }

	virtual void draw(const GraphicsContext &gc);

	virtual void update();
	bool hasOverlap (Sprite *s2);
	bool hasXOverlap (Sprite *s2);
};

// general constants
const int START_HP = 19; /* PRIME */
const int START_LIVES = 3; /* PRIME */

// player movement constants
const int CEIL = 0;
const float GRAVITY_ACC = 1.0;
const int MAX_JUMPTIMER = 13; // number of ticks a jump can be sustained /* PRIME */
const float MAX_Y = 13.0; // maximum vertical speed, both up and down /* PRIME */
const float JUMP_SPEED = 13.0; // constant speed while pressing jump button /* PRIME */
const float AIR_HSPEED = 7.0; /* PRIME */
const int DEFAULT_SPRITE_W = 47; /* PRIME */
const int DEFAULT_SPRITE_H = 47; /* PRIME */
const double BAZOOKA_ACC = 0.3;
const int PLAYER_HP = 31; /* PRIME */

const int ANIM_NUM = 8;
const int HIT_ANIM_LENGTH = 20;

class DerivedViewPort : public ViewPort
{
private:
	std::shared_ptr<ViewPort> parent;
	int factor;
public:
	DerivedViewPort (std::shared_ptr<ViewPort> _parent, int _factor) : parent(_parent), factor(_factor) {}
	virtual int getXofst() { return parent->getXofst() / factor; }
	virtual int getYofst() { return parent->getYofst() / factor; }
};

class Particle
{
	public:
		float x;
		float y;
		float dx;
		float dy;
		float r;
		float fr;
		int life;
		bool alive;
};

class Smoke : public Component {
	std::list<Particle> particles;

	public:
		void reset();
		virtual void draw(const GraphicsContext &gc) override;
		virtual void update() override;
		void addParticle (int x, int y);
};

class Game : public State
{
	TEG_MAP *map;
	std::shared_ptr<ViewPort> aViewPort;

	std::list <Sprite*> sprites;


	// per-game, initialized in init
	int lives;
	int bonusCollected;

	void updateObjects();

	Input bEsc;
	Input bCheat;
	Engine *engine;

	std::map<SpriteEx *, std::map <SpriteEx *, int > > collisions;

public:
	void collectBonus (int index);

	// per-level, initialized in initLevel
	Player *player; //may be NULL // TODO turn into weak ptr.

	enum { MSG_PLAYER_DIED = 4001, MSG_PLAYER_WINLEVEL }; /* PRIME */
	static std::map<std::string, Anim*> anims;
	ALLEGRO_FONT *gamefont;
	ALLEGRO_FONT *smallfont;

	std::shared_ptr<Smoke> smoke;

	virtual ~Game() { killAll(); }
	virtual void killAll();
	Game (Engine *engine);
	
	void init();
	void initLevel();
	void initGame();

	std::list<Sprite*> &getSprites() { return sprites; }
	void addSprite(Sprite *o);
	int getLives() { return lives; }

	virtual void onUpdate() override;
	virtual void draw(const GraphicsContext &gc) override;
	virtual bool onHandleMessage(ComponentPtr src, int code) override;

	void addCollision(SpriteEx *a, SpriteEx *b, int dir);

	Engine *getParent() { return engine; }
	TEG_MAP *getMap() { return map; }
};

#endif
