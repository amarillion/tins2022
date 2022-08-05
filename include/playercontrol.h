#ifndef PLAYERCONTROL_H
#define PLAYERCONTROL_H

#include "input.h"
#include "state.h"
#include "leveldata.h"
#include "sprite.h"
#include "isomap.h"

class Player;
class Sprite;

const int OUTRO_TIME = 240;
const float MAXZ = 320;

class Engine;
class ViewPort;

class MapComponent : public Component {
	IsoView *isoview = nullptr;
public:
	MapComponent(IsoView *isoview = nullptr) : isoview(isoview) {}

	void setMap (IsoView *_isoview)
	{
		isoview = _isoview;
	}

	virtual void draw(const GraphicsContext &gc) override
	{
		if (isoview)
		{
			drawMap(gc, isoview);
		}

		// for debugging: mark origin
		/*
		float rx0, ry0;
		map->canvasFromIso_f(0, 0, 0, rx0, ry0);
		al_draw_filled_circle(rx0 + gc.xofst, ry0 + gc.yofst, 2.0, RED);
		*/

	}
};

class PlayerControl : public State {
	private:
		int health; // value from 0 to 100
		int lives;
		int score;
		int targetsRemaining;
		int currentLevel;
		Engine &parent;
		Player *tank;
		Player *chopper;
		Player *current; // either tank or chopper
		void nextLevel();
		void gameOver();
		Input btnCheat;
		Input btnEsc;
		int tOutro;
		bool outro;
		Sprite *nextTarget;
		int bloodOverlay;
		ALLEGRO_FONT *font;
		std::shared_ptr<ViewPort> view;
		std::shared_ptr<Container> gamescreen;
		std::shared_ptr<SpriteLayer> sprites;
		std::shared_ptr<MapComponent> mapComp;
		IsoView *isoview = nullptr;
		ALLEGRO_BITMAP *tiles = nullptr;

		// map creation...
		void createMap(int w, int h, ALLEGRO_BITMAP *tiles);

		void createHills();
		void makeFlat(int mx, int my);
		void addEnemy(SpriteSubType sst);
		void addEnemyNear(int clusterx, int clustery, SpriteSubType sst);
	public:
		void init();
		void takeDamage (int amount);
		PlayerControl(Engine &e);
		void decreaseTargets();
		void decreaseLife();
		void playerKilled (Player *n);
		void initGame(); // set lives to 5 etc.
		void initLevel(LevelData const * current); // start level
		void initChopper ();
		void initTank ();
		void handleInput();
		virtual void draw (const GraphicsContext &gc) override;
		virtual void update() override;
		Player *getNearestPlayer (float x, float y, float maxrange);
		Player *getChopper () { return chopper; }
		int getCurrentLevel() { return currentLevel; }

		virtual ~PlayerControl() { if (isoview != nullptr) delete isoview; }
		std::shared_ptr<ViewPort> getViewPort() { return view; }
		bool isDebug();
		IsoView *getMap() { return isoview; }
};

#endif
