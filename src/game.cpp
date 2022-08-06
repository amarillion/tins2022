#include "game.h"

#include <assert.h>

#include "engine.h"
#include "text.h"
#include "resources.h"
#include "DrawStrategy.h"
#include "viewport.h"
#include "sprite.h"
#include "player.h"
#include "util.h"
#include "textstyle.h"
#include "tilemap.h"
#include "mainloop.h"

using namespace std;

class MyObjectRemover
{
public:
	bool operator()(Sprite *s)
	{
		if (!s->isAlive())
		{
			delete s;
			return true;
		}
		else
			return false;
	}
};

class GameImpl : public Game
{
	Engine *parent;

public:
	GameImpl(Engine *parent) : 
			parent(parent),
			aViewPort(), sprites()
	{}

	virtual ~GameImpl() {
		killAll();
	}

	virtual void init() override {
		auto res = parent->getResources();
		gamefont = res->getFont("megaman_2")->get(24);
		smallfont = res->getFont("megaman_2")->get(16);
		Sprite::anims = res->getAnims();
	}

	virtual void done() override {

	}

	TEG_MAP *map;
	std::shared_ptr<ViewPort> aViewPort;

	std::list <Sprite*> sprites;


	// per-game, initialized in init
	int lives;
	int bonusCollected = 0;

	void updateObjects();

	Input bEsc { ALLEGRO_KEY_ESCAPE };
	Input bCheat { ALLEGRO_KEY_F9 };

	std::map<SpriteEx *, std::map <SpriteEx *, int > > collisions;

public:
	void collectBonus (int index);

	virtual void killAll();
	
	virtual void initLevel() override;
	void initGame();

	virtual std::list<Sprite*> &getSprites() { return sprites; }
	virtual void addSprite(Sprite *o);
	virtual int getLives() { return lives; }

	virtual void onUpdate() override;
	virtual void draw(const GraphicsContext &gc) override;
	virtual bool onHandleMessage(ComponentPtr src, int code) override;

	virtual void addCollision(SpriteEx *a, SpriteEx *b, int dir) override;

	virtual Engine *getParent() override { return parent; }
	virtual TEG_MAP *getMap() override { return map; }

};

void GameImpl::addCollision(SpriteEx *a, SpriteEx *b, int dir)
{
	if (collisions.find(a) == collisions.end())
	{
		std::map<SpriteEx*, int> submap = std::map<SpriteEx*, int>();
		collisions.insert (
				pair<SpriteEx *, std::map<SpriteEx *, int> > (a, submap)
			);
	}

	if (collisions.find(b) == collisions.end())
	{
		std::map<SpriteEx *, int> submap = std::map<SpriteEx *, int>();
		collisions.insert (
				pair<SpriteEx *, std::map<SpriteEx *, int> > (b, submap)
			);
	}

	{
		std::map<SpriteEx *, int> submap = collisions[a];
		if (submap.find(b) == submap.end())
		{
			submap.insert (
					pair<SpriteEx *, int> (b, 0)
				);
		}
	}

	{
		std::map<SpriteEx *, int> submap = collisions[b];
		if (submap.find(a) == submap.end())
		{
			submap.insert (
					pair<SpriteEx *, int> (a, 0)
				);
		}
	}

	collisions[a][b] |= dir;
	collisions[b][a] |= dir;
}


void GameImpl::onUpdate ()
{
	updateObjects();

	if (player != NULL)
	{
		int lookat = player->getx() + ((player->getdir() == 1) ? 300 : -300);
		int x = lookat + aViewPort->getXofst();
		int newx = -aViewPort->getXofst();
		int newy = -aViewPort->getYofst();

		int delta = x - MAIN_WIDTH / 2;

		if (abs(delta) > MAIN_WIDTH)
		{
			newx = x - MAIN_WIDTH / 2;
		}
		else if (delta < -250)
		{
			newx -= 8;
		}
		else if (delta > 250)
		{
			newx += 8;
		}

		if (y < MAIN_HEIGHT / 4)
		{
			newy = player->gety() - geth() / 4;
		}
		else if (y > ((MAIN_HEIGHT / 4) * 3))
		{
			newy = player->gety() - (MAIN_HEIGHT / 4) * 3;
		}

		int xofst = bound (0, newx, teg_pixelw(map) - MAIN_WIDTH);
		int yofst = bound (0, newy, teg_pixelh(map) - MAIN_HEIGHT);
		aViewPort->setOfst (-xofst, -yofst);
	}

	if (bEsc.justPressed())
	{
		pushMsg(Engine::E_PAUSE);
	}
}

void GameImpl::updateObjects()
{
	collisions.clear();

	//update
	// collisions are detected during update and added with call to Game::addCollision.
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive()) (*i)->update();
	}

	// play back all recorded collisions
	std::map<SpriteEx *, std::map<SpriteEx *, int> >::iterator j;
	for (j = collisions.begin(); j != collisions.end(); ++j)
	{
		SpriteEx * sej = j->first;
		std::map<SpriteEx *, int>:: iterator k;
		for (k = j->second.begin(); k != j->second.end(); ++k)
		{
			SpriteEx * sek = k->first;

			sej->onCol(sek->getSpriteType(), sek, k->second);
		}
	}

	// remove all that are not alive!
	sprites.remove_if (MyObjectRemover());
}

void GameImpl::draw (const GraphicsContext &gc)
{
	Container::draw(gc);

	// gc2 is a graphicsContext using aViewport's offset
	// just for drawing sprites
	// TODO it would be simpler to add a sprite drawing component to aViewport,
	// but it would need direct access to the sprite list...

	GraphicsContext gc2 = GraphicsContext();
	gc2.xofst = aViewPort->getXofst();
	gc2.yofst = aViewPort->getYofst();
	gc2.buffer = gc.buffer;

	al_set_target_bitmap (gc2.buffer);

	draw_textf_with_background(gamefont, GREEN, BLACK, 0, 0, ALLEGRO_ALIGN_LEFT, "LIFE: %02i", lives);

	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isVisible()) (*i)->draw(gc2);
	}

#ifdef DEBUG
	if (parent->isDebug())
	{
		al_draw_textf (gamefont, GREEN, 0, 16, ALLEGRO_ALIGN_LEFT, "%i sprites", (int)sprites.size());
	}
#endif
}

void GameImpl::initGame()
{
	lives = START_LIVES;
	bonusCollected = 0;
}

void GameImpl::initLevel()
{
	killAll();
	TEG_MAP *bg = NULL;
	TEG_MAP *bg2 = NULL;

	map = parent->getResources()->getJsonMap("mapA")->map;
	bg2 = parent->getResources()->getJsonMap("bg2")->map;
	bg =  parent->getResources()->getJsonMap("bg1")->map;

	assert (bg);
	assert (bg2);

	add(ClearScreen::build(al_map_rgb (170, 170, 255)).get());

	// set up game background
	aViewPort = make_shared<ViewPort>();

	auto cViewPort = make_shared<DerivedViewPort>(aViewPort, 4);
	add(cViewPort);
	cViewPort->setChild(TileMap::build(bg2).get());

	auto bViewPort = make_shared<DerivedViewPort>(aViewPort, 2);
	add(bViewPort);
	bViewPort->setChild(TileMap::build(bg).get());

	add(aViewPort);
	auto aView = make_shared<Container>();
	aViewPort->setChild(aView);

	aView->add(TileMap::build(map).get()); // TODO: tilemap not animated

	player = new Player(this, 128, 128);
	addSprite (player);

	for (int mx = 0; mx < map->w; ++mx)
	{
		for (int my = 0; my < map->h; ++my)
		{
			int tile = teg_mapget(map, 2, mx, my);
			if (tile < 0) continue;

			int flags = map->tilelist->tiles[tile].flags;

			int xx = mx * map->tilelist->tilew;
			int yy = my * map->tilelist->tileh - 64;
			SpriteEx *e = NULL;
			switch (flags)
			{
			case 0: case 1: case 2: case 3: case 4:
				/** these values have a very different meaning */
				break;
			case 5: e = new Enemy(this, xx, yy, 0);
				addSprite (e); break;
			case 6: e = new Enemy(this, xx, yy, 1);
				addSprite (e); break;
			case 7: e = new Enemy(this, xx, yy, 2);
				addSprite (e); break;
			case 8: e = new Enemy(this, xx, yy, 3);
				addSprite (e); break;
			case 9: e = new Platform(this, xx, yy);
				addSprite (e); break;
			case 10: e = new Enemy(this, xx, yy, 4);
				addSprite (e); break;
			case 11: e = new Bonus(this, xx, yy, 1);
				printf ("add bonus at %i, %i", xx, yy);
				addSprite (e); break;
			default:
				log ("unrecognised flags %i for tile %i at (%i, %i)", flags, tile, mx, my);
				//TODO: would be good to have a more interesting assert implementation that allows logging as well.
				assert (false); /* unimplemented */
				break;
			}
		}
	}

	aView->resizeToChildren();

	repr(0, cout);
}

void GameImpl::collectBonus (int index)
{
	bonusCollected++;
}

void GameImpl::killAll()
{
	Container::killAll();
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	sprites.clear();
}

void GameImpl::addSprite(Sprite *o)
{
	sprites.push_back (o);
}

bool GameImpl::onHandleMessage(ComponentPtr src, int event)
{
	switch (event)
	{
	case MSG_PLAYER_DIED:
		lives--;
		if (lives == 0)
		{
			pushMsg(Engine::E_SHOW_GAME_OVER);
		}
		else
		{
			pushMsg(Engine::E_LEVEL_INTRO);
		}
		return true;
	case MSG_PLAYER_WINLEVEL:
		pushMsg(Engine::E_LEVEL_CLEAR);
		return true;
	}
	return false;
}

shared_ptr<Game> Game::newInstance(Engine *parent)
{
	return make_shared<GameImpl>(parent);
}
