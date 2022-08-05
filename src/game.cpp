#include <assert.h>
#include "engine.h"
#include "color.h"
#include <math.h>
#include <list>
#include <algorithm>
#include "game.h"
#include "engine.h"
#include "color.h"
#include "anim.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "player.h"
#include "componentbuilder.h"
#include "util.h"
#include <map>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "textstyle.h"
#include "DrawStrategy.h"
#include "mainloop.h"
#include "resources.h"
#include "tilemap.h"

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

void Game::onUpdate ()
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

void Game::updateObjects()
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

Game::Game (Engine *engine) : aViewPort(), sprites(), bonusCollected(0), bEsc(ALLEGRO_KEY_ESCAPE), bCheat(ALLEGRO_KEY_F9)
{
	this->engine = engine;
}

void Game::draw (const GraphicsContext &gc)
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
	if (engine->isDebug())
	{
		al_draw_textf (gamefont, GREEN, 0, 16, ALLEGRO_ALIGN_LEFT, "%i sprites", (int)sprites.size());
	}
#endif
}

map<string, Anim*> Game::anims;

void Game::init()
{
	gamefont = engine->getResources()->getFont("megaman_2")->get(24);
	smallfont = engine->getResources()->getFont("megaman_2")->get(16);
	anims = engine->getResources()->getAnims();
}

void Game::initGame()
{
	lives = START_LIVES;
	bonusCollected = 0;
	engine->getLevelState().clear();
}

void Game::initLevel()
{
	killAll();
	TEG_MAP *bg = NULL;
	TEG_MAP *bg2 = NULL;

	switch (engine->getLevelState().getSelectedLevel())
	{
	case 0:
		map = engine->getResources()->getJsonMap("map1")->map;
		bg2 = engine->getResources()->getJsonMap("bg2")->map;
		bg =  engine->getResources()->getJsonMap("bg1")->map;
		break;
	case 1:
		map = engine->getResources()->getJsonMap("map2")->map;
		bg2 = engine->getResources()->getJsonMap("bg3")->map;
		bg =  engine->getResources()->getJsonMap("bg1")->map;
		break;
	case 2:
		map = engine->getResources()->getJsonMap("map3")->map;
		bg2 = engine->getResources()->getJsonMap("bg4")->map;
		bg =  engine->getResources()->getJsonMap("bg1")->map;
		break;
	case 3:
		map = engine->getResources()->getJsonMap("map4")->map;
		bg2 = engine->getResources()->getJsonMap("bg2")->map;
		bg = engine->getResources()->getJsonMap("bg1")->map;
		break;
	default:
		assert(false); // undefined level
		break;
	}

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

	smoke = make_shared<Smoke>();

	aView->add (smoke);
	aView->resizeToChildren();

	repr(0, cout);
}

void Game::collectBonus (int index)
{
	bonusCollected++;
}

void Game::killAll()
{
	Container::killAll();
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	sprites.clear();
	smoke.reset();
}

void Game::addSprite(Sprite *o)
{
	sprites.push_back (o);
}

bool Game::onHandleMessage(ComponentPtr src, int event)
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

Sprite::Sprite (Game *game, int amx, int amy) : awake(true), alive(true), visible(true), counter(0),
		x(0), y(0), w(0), h(0), motion(), anim(NULL), state(0), dir(0), animStart(0)
{
	setx (amx);
	sety (amy);
	alive = true;
	visible = true;
	blockedByTiles = true;
	parent = game;
}

void Sprite::update()
{
	assert (parent);
}

void Sprite::draw(const GraphicsContext &gc)
{
	int x = getx() + gc.xofst;
	int y = gety() + gc.yofst;
	int time = 0;
	if (anim != NULL)
		anim->drawFrame (state, dir, time, x, y);
}

bool Sprite::hasXOverlap (Sprite *s2)
{
	int x1 = x;
	int dx1 = w;
	int x2 = s2->getx();
	int dx2 = s2->getw();
	return !(
			(x1 >= x2+dx2) || (x2 >= x1+dx1)
		);
}

bool Sprite::hasOverlap (Sprite *s2)
{
	int y1 = y;
	int dy1 = h;
	int y2 = s2->gety();
	int dy2 = s2->geth();
	return hasXOverlap(s2) &&
		!(
			(y1 >= y2+dy2) || (y2 >= y1+dy1)
		);
}

int Sprite::getTileStackFlags(int mx, int my)
{
	int result = 0;
	TEG_MAP *map = parent->getMap();
	if (!map) return 0; // no map found !!!

	if (mx < 0 || my < 0 || mx >= map->w || my >= map->h)
	{
		return 0;
	}
	else
	{
		int i1, i2, f1, f2;
		i1 = teg_mapget (map, 0, mx, my);
		i2 = teg_mapget (map, 1, mx, my);
		if (i1 >= 0) f1 = map->tilelist->tiles[i1].flags; else f1 = 0;
		if (i2 >= 0) f2 = map->tilelist->tiles[i2].flags; else f2 = 0;

		// check for solids
		if (f1 == 1 || f2 == 1) result |= TS_SOLID;

		return result;
	}
}

/*
 * Attempt to move dx, dy.
 * We divide the movement (dx, dy) in increments of one pixel,
 * taking turns moving horizontal and vertical until
 * the entire movement has been filled up.
 *
 * If any sprite or player blocks, the movement is stopped and a non-zero value is returned.
 *
 * Collisions with sprites are stored in a list, and later processed, calling onCol() twice for each collision.
 *
 * The push_force parameter is initially set at the pushForce value of the sprite doing the movement.
 * When a move is blocked by another sprite, we attempt to move it as well, using the push_force of the primary mover.
 */
int Sprite::try_move (double dx, double dy, int push_force)
{
	double dxleft = dx, dyleft = dy;
	int ddx = dx > 0 ? 1 : -1;
	int hdir = dx > 0 ? SpriteEx::DIR_RIGHT : SpriteEx::DIR_LEFT;
	int ddy = dy > 0 ? 1 : -1;
	int vdir = dy > 0 ? SpriteEx::DIR_DOWN: SpriteEx::DIR_UP;
	double trydx, trydy;
	int result = 0;

	// we keep processing until dxleft & dyleft are 0.
	while ((fabs(dxleft) > 0 || fabs (dyleft) > 0))
	{
		bool valid = true;
		int dir = 0;

		// we only move (0,+/-1) or (+/-1,0), nothing in between.
		// if there is more horizontal movement left than vertical, we move horizontal.
		if (fabs(dxleft) > fabs(dyleft))
		{
			trydy = 0;
			dir = hdir;
			if (fabs(dxleft) >= 1)
				trydx = ddx;
			else
				trydx = dxleft;
		}
		else
		{
			dir = vdir;
			trydx = 0;
			if (fabs(dyleft) >= 1)
				trydy = ddy;
			else
				trydy = dyleft;
		}

		// check with tilemap background, but only if object is solid.
		if (blockedByTiles)
		{
			// check if (x +  |trydx, y + trydy) is valid
			int mx1, my1, mx2, my2;
			int ix, iy;
			TEG_MAP *map = parent->getMap();
			mx1 = ((int)x + trydx) / map->tilelist->tilew;
			my1 = ((int)y + trydy) / map->tilelist->tileh;
			mx2 = ((int)x + trydx + w - 1) / map->tilelist->tilew;
			my2 = ((int)y + trydy + h - 1) / map->tilelist->tileh;

			// loop through all map positions we touch with the solid region
			for (ix = mx1; ix <= mx2; ++ix)
			{
				for (iy = my1; iy <= my2; ++iy)
				{
					// see if there is a solid tile at this position
					if (getTileStackFlags (ix, iy) & TS_SOLID)
					{
						valid = false;
						result |= dir; // multiply by 16 to turn DIR* into TILE_DIR*
						result |= SpriteEx::BLOCKED_BY_TILE;
					}
				}
			}
		}

		// loop through all sprites and see if we touch a solid sprite
		// detect collisions
		list<Sprite*>::iterator i;

		// temporarily move for testing. Need to adjust x, y temporarily for hasOverlap()
		double oldx = x;
		double oldy = y;

		x += trydx;
		y += trydy;

		for (i = parent->getSprites().begin(); i != parent->getSprites().end(); i++)
		{
			if (!(*i)->isAlive()) continue;
			if (*i == this) continue;
			SpriteEx * sei = dynamic_cast<SpriteEx*>(*i);
			SpriteEx * sej = dynamic_cast<SpriteEx*>(this);

			if (sei == NULL || sej == NULL) continue; // not a SpriteEx (must be explosion)

			if (sej->hasOverlap(sei))
			{
				// platforms players and enemies are unpassable.
				if (sei->isUnpassable() && sej->isUnpassable())
				{
					// attempt to push
					if (sei->getPushForce() < push_force)
					{
						//TODO: make sure we're not in an infinite loop somehow...
						int result2 = sei->try_move(trydx, trydy, push_force);
						if (result2 > 0)
						{
							// couldn't push.
							valid = false;
							result |= dir;
						}
						else
						{
							// pushing worked, we're free to continue moving.
						}
					}
					else
					{
						valid = false;
						result |= dir;
					}
				}
				// record the collision for later playback.
				parent->addCollision(sej, sei, dir);
			}
		}

		// restore x & y values
		x = oldx;
		y = oldy;

		// if we have been blocked
		if (!valid)
		{
			if (trydx == 0)
			{
				dyleft = 0;
			}
			else
			{
				dxleft = 0;
			}
		}
		else
		{
			x += trydx;
			dxleft -= trydx;
			y += trydy;
			dyleft -= trydy;
		}
	}
	return result;
}

void Smoke::reset()
{
	particles.clear();
}

void Smoke::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);
	int xofst = gc.xofst;
	int yofst = gc.yofst;

	for (list<Particle>::iterator i = particles.begin(); i != particles.end(); ++i)
	{
		if (i->alive)
		{
			int trans = 100 - i->life;
			ALLEGRO_COLOR smoke = al_map_rgba(0, 0, 0, trans);
			al_draw_filled_circle (i->x + xofst, i->y + yofst, i->r, smoke);
		}
	}
}

class MyParticleRemover
{
   public:
	  bool operator()(Particle &o)
	  {
		 if (!o.alive)
		 {
			return 1;
		 }
		 return 0;
	  }
};

void Smoke::update()
{
	for (list<Particle>::iterator i = particles.begin(); i != particles.end(); ++i)
	{
		if (i->alive)
		{
			i->life++;
			if (i->life > 100) { i->alive = false; }
			i->y += i->dy;
			i->x += i->dx;
			i->r *= i->fr;
		}
	}

	// remove all that are not alive!
	particles.remove_if (MyParticleRemover());
}

void Smoke::addParticle (int x, int y)
{
	Particle p;
	p.x = x;
	p.y = y;
	p.dy = -0.3f;
	p.dx = ((rand() % 20) - 10) * 0.05f;
	p.life = 0;
	p.alive = true;
	p.r = 3.0f;
	p.fr = 1.03;
	particles.push_back (p);
}

void Game::addCollision(SpriteEx *a, SpriteEx *b, int dir)
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
