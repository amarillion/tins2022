#include "playercontrol.h"
#include "player.h"
#include "input.h"
#include <assert.h>
#include "engine.h"
#include "color.h"
#include "math.h"
#include "leveldata.h"
#include "isometric.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include "util.h"
#include "mainloop.h"
#include "viewport.h"
#include "DrawStrategy.h"
#include "textstyle.h"
#include "anim.h"
#include "resources.h"

using namespace std;

PlayerControl::PlayerControl(Engine &e) : parent(e)
{
	btnCheat.setScancode(ALLEGRO_KEY_F9);
	btnEsc.setScancode(ALLEGRO_KEY_ESCAPE);
}

void PlayerControl::init()
{
	font = parent.getResources()->getFont("a4_font")->get();

	tiles = parent.getResources()->getBitmap("tiles");

	auto bg = ClearScreen::build(BLACK).get();
	add (bg);

	view = make_shared<ViewPort>();
	view->setOfst (0, 0);
	add (view);

	gamescreen = make_shared<Container>();
	view->setChild (gamescreen);

	mapComp = make_shared<MapComponent>();
	gamescreen->add (mapComp);

	auto insults = make_shared<Insults>(parent);
	add(insults);
 	insults->init();

	sprites = make_shared<SpriteLayer>(this);
	gamescreen->add(sprites);
}

bool PlayerControl::isDebug() { return parent.isDebug(); }

void PlayerControl::initGame()
{
	score = 0;
	lives = 5;
	currentLevel = 0;
	chopper = NULL;
	tank = NULL;
}

void PlayerControl::makeFlat (int mx, int my)
{
	if (!isoview->map.inBounds(mx, my)) return;

	int z = isoview->map.get(mx, my).getMaxHeight ();

	isoview->map.setHillAtLeast(mx, my, z);
	isoview->map.setHillAtLeast(mx + 1, my, z);
	isoview->map.setHillAtLeast(mx, my + 1, z);
	isoview->map.setHillAtLeast(mx + 1, my + 1, z);
}

void PlayerControl::createMap(int ww, int hh, ALLEGRO_BITMAP *tiles)
{
	isoview = new IsoView(ww, hh, 128, 128, 64);
	for (int x = 0; x < ww; ++x)
		for (int y = 0; y < hh; ++y)
	{
		Cell &c = isoview->map.get(x, y);
		c.z = 0;
		c.dzleft = 0;
		c.dzright = 0;
		c.dzbot = 0;
	}
	isoview->grid.setTileSize(128, 128, 64);
	isoview->grid.setTexture(tiles, 32, 32);
}

void PlayerControl::createHills()
{
	int numHills = isoview->map.getDimMX();
	for (int i = 1; i <= numHills; i++)
	{
		int x, y;

		x = rand() % isoview->map.getDimMX();
		y = rand() % isoview->map.getDimMY();

		int height = rand() % 2 + rand() % 2 + 1;

		isoview->map.setHillAtLeast(x, y, height);

		// create level spots around hill...
		for (int i = 0; i < 8; ++i)
		{
			int x2 = x + (rand() % 11) - 5;
			int y2 = y + (rand() % 11) - 5;

			makeFlat (x2, y2);
		}
	}

}

class LocationFinder
{
private:
	IsoView *isoview;
	std::shared_ptr<SpriteLayer> sprites;
	SpriteSubType sst;

	bool fAwayFrom = false;
	int awayFromx = 0;
	int awayFromy = 0;
	int awayFromDistance = 0;

	bool fNear = false;
	int nearRadius = 0;
	int nearx = 0;
	int neary = 0;

	bool fMaxZ = false;
	int maxzVal = 0;

	bool fMinZ = false;
	int minzVal = 0;

	bool fFlatSurface = false;
public:
	LocationFinder (IsoView *isoview, std::shared_ptr<SpriteLayer> sprites, SpriteSubType sst) : isoview(isoview), sprites(sprites), sst(sst) {};

	LocationFinder & awayFrom(int rx, int ry, int distance) { fAwayFrom = true; awayFromDistance = distance; awayFromx = rx; awayFromy = ry; return *this; }
	LocationFinder & near(int rx, int ry, int distance) { fNear = true; nearRadius = distance; nearx = rx; neary = ry; return *this; }
	LocationFinder & flatSurface() { fFlatSurface = true; return *this; }
	LocationFinder & maxz(int val) { fMaxZ = true; maxzVal = val; return *this; }
	LocationFinder & minz(int val) { fMinZ = true; minzVal = val; return *this; }

	bool get (int &rx, int &ry)
	{
		int maxtries = 100;

		SpriteData *s = &spriteData[sst];
		int sizex = s->anim->sizex;
		int sizey = s->anim->sizey;
		int sizez = s->anim->sizez;

		int xlimit = isoview->grid.getDimIX() - sizex;
		int ylimit = isoview->grid.getDimIY() - sizey;

		while (--maxtries > 0)
		{

			if (fNear)
			{
				rx = nearx + (rand() % nearRadius) - nearRadius / 2;
				ry = neary + (rand() % nearRadius) - nearRadius / 2;

				if (rx < 0) rx = -rx;
				if (ry < 0) ry = -ry;
				if (rx >= xlimit) { rx = xlimit - rx; }
				if (ry >= ylimit) { ry = ylimit - ry; }
			}
			else
			{
				rx = rand() % xlimit;
				ry = rand() % ylimit;
			}

			if (fAwayFrom)
			{
				int deltax = awayFromx - rx;
				int deltay = awayFromy - ry;
				if (deltax * deltax + deltay * deltay < awayFromDistance * awayFromDistance) continue; // try again
			}

			if (fFlatSurface && s != NULL)
			{
				bool isFlat = true;
				int mx1 = rx / isoview->grid.getTilex();
				int my1 = ry / isoview->grid.getTiley();
				int mx2 = (rx + sizex) / isoview->grid.getTilex();
				int my2 = (ry + sizey) / isoview->grid.getTiley();
				for (int mx = mx1; mx <= mx2; ++mx)
					for (int my = my1; my <= my2; ++my)
					{
						if (isoview->map.inBounds(mx, my))
						{
							if (!isoview->map.get(mx, my).isFlat()) isFlat = false;
						}
					}

				if (!isFlat) continue; // try again
			}

			int rz = isoview->getSurfaceIsoz(rx, ry);
			if (fMinZ && rz <= minzVal) continue;
			if (fMaxZ && rz >= maxzVal) continue;

			// check for overlap in sprites map
			if (!sprites->checkSpace_ex(sizex, sizey, sizez, rx, ry, rz)) continue;

			if (rx < 0 || ry < 0 || rx >= xlimit || ry >= ylimit) continue; // try again

			return true;
		}
		return false;
	}
};

void PlayerControl::addEnemy(SpriteSubType sst)
{
	int rx, ry;
	bool ok = LocationFinder(isoview, sprites, sst)
			.awayFrom(0, 0, 300.0).get(rx, ry);

	if (ok)
	{
		Enemy *e = new Enemy(this, sst);
		e->setSurfaceLocation (rx, ry);
		sprites->add (e);
	}
}

void PlayerControl::addEnemyNear(int nx, int ny, SpriteSubType sst)
{
	int rx, ry;
	bool ok = LocationFinder(isoview, sprites, sst)
			.awayFrom(0, 0, 300.0)
			.near(nx, ny, 400.0)
			.get(rx, ry);

	if (ok)
	{
		Enemy *e = new Enemy(this, sst);
		e->setSurfaceLocation (rx, ry);
		sprites->add (e);
	}
}

void PlayerControl::initLevel(LevelData const * currentLevel)
{
	sprites->killAll(); // clean from previous game

	const int dim = currentLevel->map_sizex / 128;

	if (isoview != nullptr) delete isoview;
	createMap(dim, dim, tiles);
	mapComp->setMap(isoview);

	createHills();

	current = NULL;
	// add chopper && tank
	if (currentLevel->hastank) initTank();
	if (currentLevel->haschopper) initChopper();

	int tankRemain = currentLevel->enemy_tanks;
	int samRemain = currentLevel->enemy_sam;
	int turretRemain = currentLevel->enemy_turrets;

	// add goal
	for (int i = 0; i < currentLevel->targets; ++i)
	{
		// create a goal cluster...
		double clusterx, clustery;

		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_GIFT)
				.awayFrom(0, 0, 800.0)
				.flatSurface()
				.get(rx, ry);

		clusterx = rx;
		clustery = ry;

		if (!ok) { /* TODO: we couldn't add gift. we should regenerate whole map */ }

		Enemy *e = new Enemy(this, SST_GIFT);
		e->setSurfaceLocation (rx, ry);
		sprites->add (e);

		// add a turret
		if (turretRemain > 0)
		{
			addEnemyNear(clusterx, clustery, SST_TURRET);
			turretRemain--;
		}

		// add a SAM site
		if (samRemain > 0)
		{
			addEnemyNear(clusterx, clustery, SST_SAM);
			samRemain--;
		}

		// add an enemy tank
		if (tankRemain > 0)
		{
			addEnemyNear(clusterx, clustery, SST_ENEMYTANK);
			tankRemain--;
		}

	}

	// add buidings
	for (int i = 0; i < currentLevel->buildings / 4; i++)
	{
		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_HOUSE)
				.awayFrom(0, 0, 150.0)
				.flatSurface()
				.get(rx, ry);

		if (ok)
		{
			int rz = isoview->getSurfaceIsoz(rx, ry);
			Solid *e = new Solid(this, rz > 64 ? SST_LOGCABIN : SST_HOUSE);
			e->setSurfaceLocation (rx, ry);
			sprites->add (e);
		}
	}


	// add buidings
	for (int i = 0; i < currentLevel->buildings / 4; i++)
	{
		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_TALL)
				.awayFrom(0, 0, 150.0)
				.flatSurface()
				.maxz(32)
				.get(rx, ry);

		if (ok)
		{
			Solid *e = new Solid(this, SST_TALL);
			e->setSurfaceLocation (rx, ry);
			sprites->add (e);
		}
	}

	targetsRemaining = currentLevel->targets;
	tOutro = 0;
	outro = false;
	nextTarget = NULL;
	assert (view);
	view->setOfst(0, 0);
	bloodOverlay = 0;


	// add enemy bunkers
	for (int i = 0; i < currentLevel->enemy_bunker; ++i)
	{
		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_BUNKER)
				.awayFrom(0, 0, 300.0)
				.flatSurface()
				.get(rx, ry);
		if (ok)
		{
			Enemy *e = new Enemy(this, SST_BUNKER);
			e->setSurfaceLocation (rx, ry);
			sprites->add (e);
		}
	}

	// add turrets
	while (turretRemain > 0)
	{
		addEnemy (SST_TURRET);
		turretRemain--;
	}

	// add SAM sites
	while (samRemain > 0)
	{
		addEnemy(SST_SAM);
		samRemain--;
	}

	// add enemy tanks
	while (tankRemain > 0)
	{
		addEnemy(SST_ENEMYTANK);
		tankRemain--;
	}

	// add trees
	for (int i = 0; i < currentLevel->buildings; i++)
	{
		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_TREE)
				.awayFrom(0, 0, 150.0)
				.get(rx, ry);

		if (ok)
		{
			int rz = isoview->getSurfaceIsoz(rx, ry);
			Solid *e = new Solid(this, rz > 64 ? SST_XMASTREE : SST_TREE);
			e->setSurfaceLocation (rx, ry);
			sprites->add (e);
		}
	}

	// add rubble
	for (int i = 0; i < currentLevel->buildings; i++)
	{
		int rx, ry;
		bool ok = LocationFinder(isoview, sprites, SST_RUBBLE)
				.awayFrom(0, 0, 150.0)
				.get(rx, ry);

		if (ok)
		{
			Solid *e = new Solid(this, SST_RUBBLE);
			e->setSurfaceLocation (rx, ry);
			sprites->add (e);
		}
	}


	// check that buildings are on a flat area
/*
	for (Sprite *s : sprites->getChildren())
	{
		ActiveSprite *as = dynamic_cast<ActiveSprite*>(s);
		if (as)
		{
			switch (as->getSubType())
			{
				case SST_BUNKER:
				case SST_GIFT:
				case SST_HOUSE:
				case SST_TALL:
				case SST_LOGCABIN:
					int mx, my;
					mx = as->getx() / isoview->map.getTilex();
					my = as->gety() / isoview->map.getTiley();
					if (!isoview->map.get(mx, my).isFlat()) as->kill();
					break;
				default:
					// do nothing;
					break;
			}
		}
	}
*/


	gamescreen->setDimension (isoview->grid.getw(), isoview->grid.geth());

	// export map
	//TODO: utility function for any component?
	/*
	GraphicsContext gcExport;
	gcExport.buffer = al_create_bitmap(gamescreen->getw(), gamescreen->geth());
	gcExport.xofst = 0;
	gcExport.yofst = 0;
	al_set_target_bitmap(gcExport.buffer);
	gamescreen->draw(gcExport);
	al_save_bitmap("level-i.png", gcExport.buffer);
	al_destroy_bitmap(gcExport.buffer);
	*/
}

void PlayerControl::takeDamage (int amount)
{
	bloodOverlay += (5 * amount);
	if (bloodOverlay > 200) bloodOverlay = 200;
}

void PlayerControl::initTank()
{
	tank = new Player(this, SST_TANK);
	float rx, ry;
	rx = 20; ry = 10;
	float rz = isoview->getSurfaceIsoz(rx, ry);
	tank->setLocation (rx, ry, rz);
	sprites->add (tank);
	if (current == NULL) current = tank;
}

void PlayerControl::initChopper()
{
	// add player chopper
	chopper = new Player(this, SST_CHOPPER);
	float rx, ry;
	rx = 10; ry = 50;
	float rz = isoview->getSurfaceIsoz(rx, ry);
	chopper->setLocation (rx, ry, rz + 30);
	sprites->add (chopper);
	if (current == NULL) current = chopper;
}

void PlayerControl::draw (const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);

	Container::draw(gc);

	draw_textf_with_background(font, BLACK, WHITE, al_get_bitmap_width(gc.buffer), 0, ALLEGRO_ALIGN_RIGHT, "HEALTH %3i LIVES %02i",
		current->getHealth(), lives);

	if (parent.isDebug())
	{
		al_draw_textf(font, DARK_GREEN, 0, 16, ALLEGRO_ALIGN_LEFT, "X: %f Y: %f Z: %f",
			current->getx(), current->gety(), current->getz());
	}
	

	if (nextTarget)
	{
		float dx = - current->getx() + nextTarget->getx();
		float dy = - current->gety() + nextTarget->gety();
		
		float dist = dx * dx + dy * dy;
		if (dist > 400 * 400 && dist < 800 * 800)
		{
			float angle = atan2 (dy, dx);

			float rdx, rdy;

			//TODO... wrong coordinate system?
			isoToScreen_f (cos (angle), sin(angle), 0, rdx, rdy);
			
			int cx = MAIN_WIDTH / 2;
			int cy = MAIN_HEIGHT / 2;
			
			const float ARROW = 200;
			ALLEGRO_VERTEX points[] = {
					{ .x = cx + ARROW * rdx,                   .y = cy + ARROW * rdy,                   .z = 0, .u = 0, .v = 0, .color = GREEN },
					{ .x = cx + (ARROW - 20) * rdx - 10 * rdy, .y = cy + (ARROW - 20) * rdy + 10 * rdx, .z = 0, .u = 0, .v = 0, .color = GREEN },
					{ .x = cx + (ARROW - 20) * rdx + 10 * rdy, .y = cy + (ARROW - 20) * rdy - 10 * rdx, .z = 0, .u = 0, .v = 0, .color = GREEN }
			};
			
			al_draw_prim(points, NULL, NULL, 0, 3, ALLEGRO_PRIM_TRIANGLE_LIST);
			
			//line (gc.buffer, cx, cy, cx + rdx * 50, cy + rdy * 50, GREEN);
		}
	}
	
	if (bloodOverlay > 0)
	{
		ALLEGRO_COLOR blood = al_map_rgba (bloodOverlay, 0, 0, bloodOverlay);
		al_draw_filled_rectangle (0, 0, al_get_bitmap_width(gc.buffer), al_get_bitmap_height(gc.buffer), blood);
	}
}

void PlayerControl::handleInput()
{
	assert (current);
	
	if (btnEsc.justPressed())
	{
		pushMsg(Engine::E_PAUSE);
	}
	
#ifdef DEBUG
	if (btnCheat.justPressed())
	{
		nextLevel();
	}
#endif
	// handle player input
	Input *b = parent.getInput();
	
	if (b[btnSwitch].justPressed())
	{
		Player *other = (current == tank) ? chopper : tank;
		if (other != NULL)
			current = other;
	}
		
	Direction newDir = current->getDir();
	bool keyDown = false;

	float radians = radiansFromDir(current->getDir());
	float goalx = current->getx() + (current->speed * 50 * cos (radians));
	float goaly = current->gety() + (current->speed * 50 * sin (radians));
	float goalz = current->getz();

	float rx, ry;
	isoview->grid.canvasFromIso_f(goalx, goaly, goalz, rx, ry);

	view->moveTo (- rx + MAIN_WIDTH / 2, - ry + MAIN_HEIGHT / 2);

	if (!current->isDead())
	{
		if (b[btnN].getState()) { newDir = N; keyDown = true; }
		if (b[btnS].getState()) { newDir = S; keyDown = true; }
		if (b[btnE].getState()) { newDir = E; keyDown = true; }
		if (b[btnW].getState()) { newDir = W; keyDown = true; }
		if (b[btnN].getState() && b[btnE].getState()) { newDir = NE; keyDown = true; }
		if (b[btnN].getState() && b[btnW].getState()) { newDir = NW; keyDown = true; }
		if (b[btnS].getState() && b[btnE].getState()) { newDir = SE; keyDown = true; }
		if (b[btnS].getState() && b[btnW].getState()) { newDir = SW; keyDown = true; }
		if (b[btnNE].getState()) { newDir = NE; keyDown = true; }
		if (b[btnSE].getState()) { newDir = SE; keyDown = true; }
		if (b[btnNW].getState()) { newDir = NW; keyDown = true; }
		if (b[btnSW].getState()) { newDir = SW; keyDown = true; }

		if (keyDown)
		{
			current->setDir (newDir);
			current->setAccelerate();
		}

		// fly up / down with chopper
		if (current == chopper)
		{
			float dz = 0;
			if (current->getz() < MAXZ && b[btnUp].getState())
			{
				dz = 1;
			}
			if (b[btnDown].getState())
			{
				dz = -1;
			}

			current->lift (dz);
		}
	
		if (b[btnFire].justPressed())
		{
			if (current == chopper)
			{
				current->fireBullet();
			}
			else
			{
				current->fireBullet3();
			}
		}

		if (b[btnAltFire].justPressed())
		{
			/*
			Sprite *target = sprites->findNearestSprite(goalx, goaly, [=](Sprite *s, float distance) {
					return s->getType() == ST_ENEMY;
			});

			if (target != NULL)
			{
				current->setTarget(target);
				current->fireMissile();
			}
			*/
		}
	}
	
	assert (view);

}
Player* PlayerControl::getNearestPlayer(float x, float y, float maxrange)
{
	Player *min = current;
	float xx = min->getx();
	float yy = min->gety();
	float mindist = (x - xx) * (x - xx) + (y - yy) * (y - yy);

	if (tank)
	{
		xx = tank->getx();
		yy = tank->gety();
		float dist = (x - xx) * (x - xx) + (y - yy) * (y - yy);
		if (dist < mindist)
		{
			dist = mindist;
			min = tank;
		}
	}

	if (chopper)
	{
		xx = chopper->getx();
		yy = chopper->gety();
		float dist = (x - xx) * (x - xx) + (y - yy) * (y - yy);
		if (dist < mindist)
		{
			dist = mindist;
			min = chopper;
		}
	}

	if (mindist > (maxrange * maxrange))
	{
		return NULL;
	}
	else
	{
		return min;
	}
}

void PlayerControl::update()
{
	Container::update();

	if (!nextTarget)
	{
		nextTarget = sprites->getNextBunker ();
 	}

	if (outro)
	{
		if (++tOutro > OUTRO_TIME)
		{
			nextLevel();
		}
	}
	
	if (bloodOverlay > 0) { bloodOverlay--; }
	
	handleInput();
}

void PlayerControl::decreaseTargets()
{
	targetsRemaining--;
	nextTarget = NULL;
	if (targetsRemaining == 0)
	{
		outro = true;
	}
}

void PlayerControl::decreaseLife()
{
	lives--;
	if (lives == 0)
	{
		gameOver();
	}
}

void PlayerControl::gameOver()
{
	currentLevel = LEVEL_GAMEOVER;
	pushMsg (Engine::E_NEXTSCRIPT);
}

void PlayerControl::nextLevel()
{
	currentLevel++;
	if (currentLevel == LEVEL_NUM)
	{
		currentLevel = LEVEL_WON;
	}
	pushMsg (Engine::E_NEXTSCRIPT);
}

void PlayerControl::playerKilled(Player * n)
{
	decreaseLife(); // goes gameover if life == 0
	
	if (n == current) current = NULL;
	
	if (n == chopper)
	{
		chopper = NULL;
		initChopper();
	}
	if (n == tank)
	{
		tank = NULL;
		initTank();
	}
}
