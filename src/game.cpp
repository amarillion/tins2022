#include "game.h"
#include "constants.h"
#include <assert.h>
#include "animator.h"
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
#include "enemy.h"

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

struct SpriteInitializer {
	int mx;
	int my;
	int code;
	bool collected = false;
	Point teleporterTarget = Point(0,0);
};

enum TileIndexes {
	TIDX_ENEMY1 = 7,
	TIDX_ENEMY2 = 15,
	TIDX_ENEMY3 = 23,
	TIDX_UNUSED = 31,
	TIDX_ENEMY5 = 39,
	TIDX_ENEMY6 = 47,
	TIDX_TELECAT = 135,
	TIDX_ROLLINGCAT = 143,
	TIDX_SHARK = 151,
	TIDX_SOCK = 152,
	TIDX_ONEUP = 153,
	TIDX_TELEPORTER = 154,
	TIDX_CRATE = 155,
	TIDX_SMALL_CRATE = 156,
	TIDX_RING = 157, 
	TIDX_FALLING_PLATFORM = 158, 
	TIDX_SWITCH = 159
};

struct MapLayout {
	string mapId; // for refreshing
	TEG_MAP *bg1;
	TEG_MAP *bg2;
	Rect bounds;
	vector<shared_ptr<SpriteInitializer>> spriteInitializers;
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
		//TODO: should be done by Container destructor, right?
		Container::killAll();
		Container::purge();
		updateables.clear(); 

		for (auto &i : sprites) {
			delete i;
		}
		list<Sprite*>::iterator i;
		sprites.clear();
	}

	virtual void init() override {
		auto res = parent->getResources();
		gamefont = res->getFont("megaman_2")->get(24);
		smallfont = res->getFont("megaman_2")->get(8);
		Sprite::anims = res->getAnims();
		parseMaps(res->getJson("levelset"));
	}

	int totalRedSocks;

	void parseMaps(const JsonNode &json) {
		totalRedSocks = 0;

		// quick & dirty: works for one pair of teleporters
		shared_ptr<SpriteInitializer> otherTeleporter = nullptr;
		Point otherTeleporterPos;

		try {
			auto res = parent->getResources();
			for (auto &node : json.getArray("layout")) {
				MapLayout mapLayout;
				string mapId = node.getString("map");;
				TEG_MAP *map = res->getJsonMap(mapId)->map;
				mapLayout.mapId = mapId;
				mapLayout.bg1 = res->getJsonMap(node.getString("bg1"))->map;
				mapLayout.bg2 = res->getJsonMap(node.getString("bg2"))->map;
				mapLayout.bounds = Rect(
					node.getInt("x"),
					node.getInt("y"),
					map->w,
					map->h
				);

				for (int mx = 0; mx < map->w; ++mx) {
					for (int my = 0; my < map->h; ++my) {
						int tile = teg_mapget(map, 2, mx, my);
						if (tile < 0) continue;

						auto sprInit = make_shared<SpriteInitializer>();
						sprInit->mx = mx;
						sprInit->my = my;
						sprInit->code = tile;
						mapLayout.spriteInitializers.push_back(sprInit);

						if (tile == TIDX_TELEPORTER) {
							Point absolutePos = mapLayout.bounds.topLeft() + Point(mx, my);
							if (otherTeleporter == nullptr) {
								otherTeleporter = sprInit;
								otherTeleporterPos = absolutePos;
							}
							else {
								otherTeleporter->teleporterTarget = absolutePos;
								sprInit->teleporterTarget = otherTeleporterPos;
							}
						}
						else if (tile == TIDX_SOCK) {
							totalRedSocks++;
						}
					}
				}

				maps.push_back(mapLayout);
			}
			playerFirstStart = Point(
				json["start"].getInt("x"),
				json["start"].getInt("y")
			);
			globalWaterLevel = json["start"].getInt("waterLevel");
		}
		catch (JsonException &e) {
			allegro_message("Error parsing map layout, %s", e.what());
			assert(false);
		}
	}

	virtual void done() override {

	}

	TEG_MAP *map;
	std::shared_ptr<ViewPort> aViewPort;
	std::list <Sprite*> sprites;

	// per-game, initialized in init
	int lives;
	int ringsCollected = 0;
	int redSocksCollected = 0;

	int globalWaterLevel = 0;
	int localWaterLevel;

	void updateObjects();

	Input bEsc { ALLEGRO_KEY_ESCAPE };
	Input bCheat { ALLEGRO_KEY_F9 };

	std::map<SpriteEx *, std::map <SpriteEx *, int > > collisions;

public:
	void collectBonus (int index);

	virtual void initMap() override;
	virtual void initGame() override;

	virtual std::list<Sprite*> &getSprites() { return sprites; }
	virtual void addSprite(Sprite *o);
	virtual int getLives() { return lives; }

	virtual void onUpdate() override;
	virtual void draw(const GraphicsContext &gc) override;
	virtual bool onHandleMessage(ComponentPtr src, int code) override;

	virtual void addCollision(SpriteEx *a, SpriteEx *b, int dir) override;

	virtual Engine *getParent() override { return parent; }
	virtual TEG_MAP *getMap() override { return map; }

	virtual void teleport(Point targetPos) override {
		playerMapEntryPos = targetPos;
		pushMsg(Engine::E_ENTER_MAP);
	}
	
	virtual void exitMap(int dir, int y) override {
		int my = ((y + 31) / 32); // always round up
		MapLayout *currentMap = getMapAt(playerMapEntryPos);
		if (dir == SpriteEx::DIR_RIGHT) {
			playerMapEntryPos = Point(
				currentMap->bounds.x2(),
				my + currentMap->bounds.y()
			);
		}
		else if (dir == SpriteEx::DIR_LEFT) {
			playerMapEntryPos = Point(
				currentMap->bounds.x() - 1,
				my + currentMap->bounds.y()
			);
		}
		else {
			assert(false && "Fell out of this world?");
		}

		pushMsg(Engine::E_ENTER_MAP);
	}

	virtual void updateWaterLevel() {
		MapLayout *currentMap = getMapAt(playerMapEntryPos);

		// update global water level immediately
		globalWaterLevel = currentMap->bounds.y() + 15;

		int newWaterLevel = (globalWaterLevel - currentMap->bounds.y()) * 32;
		auto animator = make_shared<Animator<int>>(
			localWaterLevel, newWaterLevel, 100,
			[=](int val){ this->localWaterLevel = val; }
		);
		add(animator);
	}

	virtual int getLocalWaterLevel() {
		return localWaterLevel;
	}

private:
	std::vector<MapLayout> maps;

	Point playerMapEntryPos;
	Point playerFirstStart; // first starting point, read from json.
	MapLayout *getMapAt(Point pos) {
		for (auto &mapLayout: maps) {
			if (mapLayout.bounds.contains(pos)) {
				return &mapLayout;
			}
		}
		// TODO: just reset to origin if this happens
		assert(false && "You're out of bounds! Couldn't find a map");
	}

	/** marks all objects as killed, but does not immediately delete
	 * so you can call this safely from inside the game loop
	 */
	virtual void killAll() override {
		Container::killAll();
		
		for (auto &i : sprites) {
			i->kill();
		}
		updateables.killAll(); // especially to clear water level animator

		player = nullptr; // avoid illegal access to player
	}

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

	if (player != nullptr)
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

	// draw water
	al_draw_filled_rectangle(
		0, localWaterLevel - 16 + gc2.yofst,
		GAME_WIDTH, GAME_HEIGHT,
		al_map_rgba(0,0,255,64)
	);
}

void GameImpl::initGame()
{
	lives = START_LIVES;
	ringsCollected = 0;
	redSocksCollected = 0;
	cout << "Reset player map entry pos" << endl;
	playerMapEntryPos = playerFirstStart;
	initMap();
}

void GameImpl::initMap()
{
	MapLayout *currentMap = getMapAt(playerMapEntryPos);
	assert(currentMap && "out of level bounds");

	killAll();

	cout << "Initializing map " << currentMap->mapId << endl;
	auto res = parent->getResources();
	map = res->getJsonMap(currentMap->mapId)->map;

	TEG_MAP *bg2 = currentMap->bg2;
	TEG_MAP *bg =  currentMap->bg1;

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

	auto pos = (playerMapEntryPos - currentMap->bounds.topLeft()) * TILE_SIZE; 
	player = new Player(this, pos.x(), pos.y());
	addSprite (player);

	localWaterLevel = (globalWaterLevel - currentMap->bounds.y()) * 32;

	for (auto &sprInit : currentMap->spriteInitializers) {
		if (sprInit->collected) continue;

		int tile = sprInit->code;
		int xx = sprInit->mx * map->tilelist->tilew;
		int yy = (sprInit->my + 1) * map->tilelist->tileh;
		SpriteEx *e = NULL;
		switch (tile)
		{
		case TIDX_ENEMY1: e = new Enemy(this, xx, yy, Enemy::ELECTRICAT);
			addSprite (e); break;
		case TIDX_ENEMY2: e = new Enemy(this, xx, yy, Enemy::SLINKYCAT);
			addSprite (e); break;
		case TIDX_ENEMY3: e = new Enemy(this, xx, yy, Enemy::SPIDERCAT);
			addSprite (e); break;
		case TIDX_UNUSED: break;
		case TIDX_ENEMY5: e = new Enemy(this, xx, yy, Enemy::DRAGONCAT);
			addSprite (e); break;
		case TIDX_ENEMY6: e = new Enemy(this, xx, yy, Enemy::GENERATOR);
			addSprite (e); break;
		case TIDX_ROLLINGCAT: e = new Enemy(this, xx, yy, Enemy::ROLLINGCAT);
			addSprite (e); break;
		case TIDX_SHARK: e = new Enemy(this, xx, yy, Enemy::SHARKCAT);
			addSprite (e); break;
		case TIDX_TELECAT: e = new Enemy(this, xx, yy, Enemy::TELECAT);
			addSprite (e); break;
		case TIDX_SOCK: e = new Bonus(this, xx, yy, Bonus::SOCK, [=](){ sprInit->collected = true; });
			addSprite (e); break;
		case TIDX_ONEUP: e = new Bonus(this, xx, yy, Bonus::ONEUP, [=](){ sprInit->collected = true; });
			addSprite (e); break;
		case TIDX_TELEPORTER: e = new Teleporter(this, xx, yy, sprInit->teleporterTarget); 
			addSprite (e); break;
		case TIDX_CRATE: e = new Platform(this, xx, yy, Platform::CRATE);
			addSprite (e); break;
		case TIDX_SMALL_CRATE: e = new Platform(this, xx, yy, Platform::SMALLCRATE);
			addSprite (e); break;
		case TIDX_RING: e = new Bonus(this, xx, yy, Bonus::RING, [=](){ sprInit->collected = true; });
			addSprite (e); break;
		case TIDX_FALLING_PLATFORM: e = new Platform(this, xx, yy, Platform::FALLING);
			addSprite (e); break;
		case TIDX_SWITCH: {
			int delta = globalWaterLevel - currentMap->bounds.y() - 1;
			bool waterHere = (delta >= 0 && delta < currentMap->bounds.h());
			e = new Switch(this, xx, yy, waterHere ? Switch::ON : Switch::OFF);
			addSprite (e); break;
		}
		default:
			log ("unrecognised tile %i at (%i, %i)", tile, sprInit->mx, sprInit->my);
			//TODO: would be good to have a more interesting assert implementation that allows logging as well.
			assert (false); /* unimplemented */
			break;
		}
	}

	aView->resizeToChildren();
}

void GameImpl::collectBonus (int index)
{
	switch(index) {
		case Bonus::SOCK:
			redSocksCollected++;
			break;
		case Bonus::ONEUP:
			lives++;
			break;
		case Bonus::RING:
			ringsCollected++;
			break;
	}
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
	case MSG_PLAYER_WIN:
		pushMsg(Engine::E_SHOW_WIN_SCREEN);
		return true;
	}
	return false;
}

shared_ptr<Game> Game::newInstance(Engine *parent)
{
	return make_shared<GameImpl>(parent);
}
