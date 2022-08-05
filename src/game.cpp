#include <allegro5/allegro.h>
#include <assert.h>
#include "engine.h"
#include "color.h"
#include <math.h>
#include <list>
#include <algorithm>
#include "game.h"
#include "engine.h"
#include "color.h"
#include "level.h"
#include "anim.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "mainloop.h"
#include "text.h"
#include "bitmap.h"
#include "twirl.h"
//#include "steps.h"

using namespace std;

const float PLAYER_SPEED_EMPTY = 3.6;
const float PLAYER_SPEED_EAT = 3.2;
const float FALL_SPEED = 3.2;
const float TUMBLE_SPEED = 3.2;
const float ENEMY_SPEED = 3.2;
const float PUSH_SPEED = 2.4;
const int TURN_WAIT = 5;
const int OUTRO_WAIT = 96;
const int SIDEBAR_WIDTH = 128;
const int EXPLODE_TIME = 32;
const int PUSH_DELAY = 10;
const int TIME_MULTIPLIER = 5;
const int MATCH_BONUS = 1000;

const char *clothesTypeNames[CLOTHINGTYPE_NUM] =
{ "BOTTOM", "SHOES", "TOP", "ACCESSORY" };

enum TileFlags
{
	TF_EMPTY = 0,
	TF_SOFT = 1,
	TF_SQUARE = 2,
	TF_ROUND = 3,
	TF_ROCK = 4,
	TF_BONUS = 5,
	TF_ENEMY = 6,
	TF_EXIT = 7,
	TF_START = 8,
};

class MyObjectRemover
{
   public:
	  bool operator()(Sprite *s)
	  {
		 if (s->canDestroy())
		 {
			delete s;
			return 1;
		 }
		 return 0;
	  }
};

void Game::update ()
{
	if (outroTimer > 0)
	{
		outroTimer--;
		if (outroTimer == 0) 
		{ 
			if (gameover)
			{
				pushMsg(Engine::E_MAINMENU);
			}
			else
			{
				pushMsg(Engine::E_POSTMORTEM);
			}
		}
	}
	
	if (player != NULL)
	{
		int portw = (MAIN_WIDTH - SIDEBAR_WIDTH);
		int porth = MAIN_HEIGHT;
		int newx = player->getx() - (portw - 32) / 2;
		int newy = player->gety() - (porth - 32) / 2;
		int maxw = teg_pixelw (current);
		int maxh = teg_pixelh (current);
		if (newx + portw > maxw) newx = maxw - portw;
		if (newy + porth > maxh) newy = maxh - porth;
		if (newx < 0) newx = 0;
		if (newy < 0) newy = 0;
		xofst = -newx;
		yofst = -newy;
	}
	
	// check if any rocks can be made active
	for (int x = 1; x < current->w-1; ++x)
	{
		for (int y = 0; y < current->h-1; ++y)
		{
			if (getFlagsAt(x, y) == TF_ROCK)
			{
				if (isclear (x, y + 1) || 
					canTumble (x, y, +1) || 
					canTumble (x, y, -1)
					)
				{
					tileToSprite (x, y, ST_ROCK);
				}
			}
		}
	}


	//update
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive()) (*i)->update();
	}
	
	// remove all that are not alive!
	sprites.remove_if (MyObjectRemover());	
}

bool Game::isclear (int x, int y)
{	
	if (!spriteMap.inBounds(x, y)) { return false; }
	int flags = getFlagsAt (x, y);
	Sprite *sprite = spriteMap(x, y);
	
	return (flags == TF_EMPTY && sprite == nullptr);
}

bool Game::canTumble (int x, int y, int dx)
{
	assert (dx == -1 || dx == 1);
	int base = getFlagsAt (x, y + 1);
	return 
 		isclear (x + dx, y + 1) &&
 		isclear (x + dx, y) &&
		(base == TF_ROCK || base == TF_ROUND || base == TF_BONUS);
}

Game::Game (Engine *engine) : sprites(), clothesPickups()
{ 
	parent = engine;
	current = NULL;
	outroTimer = 0;

	xofst = 0;
	yofst = 0;
}

Game::~Game()
{
	killAll();

	if (current != NULL)
	{
		teg_destroymap (current);
		current = NULL;
	}
}

void Game::initLevel(int currentLevel)
{
	assert (currentLevel >= 0 && currentLevel < LEVEL_NUM);
	
	killAll();
	lastLevel = currentLevel;
	player = NULL;
	outroTimer = 0;
	gameover = false;
	clothesOk = false;
	startTime = parent->getCounter();
	
	for (int i = 0; i < 4; ++i)
	{
		clothes[i] = -1;
	}
	
	// create a fresh copy from resources
	if (current != NULL) 
	{
		teg_destroymap (current);
		current = NULL;
	}
		
	Tilemap *level = levelData[currentLevel].map;
	
	current = teg_create_copy (level->map);
	assert (current);
	spriteMap.resizeAndClear(current->w, current->h);
	
	// init clothes list
	clothesPickups.clear();
	lights.clear();

	// for looking up clothes by their id
	map<string, int> sortedList;
	for (int i = 0; i < CLOTHES_NUM; ++i) sortedList[clothesData[i].id] = i;
	
	for (int x = 0; x < current->w; ++x)
	{
		for (int y = 0; y < current->h; ++y)
		{
			spriteMap(x, y) = nullptr;
			
			switch (getFlagsAt(x, y))
			{
			case TF_ENEMY: // spider
 				tileToSprite (x, y, ST_ENEMY);
				break;
			case TF_START: // player
				assert (player == NULL); // only one player!
 				player = tileToSprite (x, y, ST_PLAYER);
				break;
			}
		}
	}

	JsonNode node = level->rawData;
	map<string, int> layerToIndex;
	for (size_t l = 0; l < node.getArray("layers").size(); ++l) {
		string name = node.getArray("layers")[l].getString("name");
		layerToIndex[name] = l;
	}

	// find object layer
	for (auto l : node.getArray("layers")) {
		if (l.getString("type") == "objectgroup") {

			// go through objects...
			// initialze lights
			for (auto o : l.getArray("objects")) {

				int mx = (o.getInt("x") + 15) / 32; // round up or down to nearest tile
				int my = (o.getInt("y") + 15) / 32;

				string type = o.getString("type");
				if (type == "Button") {
					Light light;
					light.mx = mx;
					light.my = my;
					string lightName = o.getString("name");
					string onLayerName = lightName + " on";
					string offLayerName = lightName + " off";
					assert (layerToIndex.find(onLayerName) != layerToIndex.end());
					assert (layerToIndex.find(offLayerName) != layerToIndex.end());
					light.onLayer = layerToIndex[onLayerName];
					light.offLayer = layerToIndex[offLayerName];
					lights.push_back(light);
				}
				else if (type == "Collectable") {
					ClothesPickup pickup;
					pickup.mx = mx;
					pickup.my = my;
					string pickupId = o.getString("name");
					assert (sortedList.find(pickupId) != sortedList.end());
					pickup.index = sortedList[pickupId];
					clothesPickups.push_back (pickup);
				}
			}

		}
	}

	assert (player != NULL);
}

Sprite *Game::tileToSprite (int mx, int my, SpriteType type)
{
	Sprite *result;
	teg_mapput (current, 0, mx, my, TF_EMPTY);
	result = new Sprite(parent, this, mx, my, type);
	add(result);
	return result;
}

void Game::draw (const GraphicsContext &gc3)
{
	GraphicsContext gc;
	gc.buffer = gc3.buffer;
	gc.xofst = gc3.xofst;
	gc.yofst = gc3.yofst;

	if (outroTimer > 0) {
		gc.buffer = outroBitmap;
	}
	al_set_target_bitmap (gc.buffer);
	
	teg_draw (current, 0, -xofst, -yofst);
	
	vector<ClothesPickup>::iterator j;
	for (j = clothesPickups.begin(); j != clothesPickups.end(); ++j)
	{
		al_draw_bitmap (clothesData[j->index].smallbmp,
			j->mx * 32 + xofst, j->my * 32 + yofst, 0);
	}
	
	GraphicsContext gc2;
	gc2.buffer = gc.buffer;
	gc2.xofst = xofst;
	gc2.yofst = yofst;

	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); i++)
	{
		if ((*i)->isAlive() && (*i)->isVisible() ) (*i)->draw(gc2);
	}

	teg_draw (current, 1, -xofst, -yofst);

	for (auto l : lights) {
		if (getFlagsAt(l.mx, l.my) == 0) {
			teg_draw (current, l.offLayer, -xofst, -yofst);
		}
		else {
			teg_draw (current, l.onLayer, -xofst, -yofst);
		}
	}

#ifdef DEBUG
	if (parent->isDebug())
	{
		//TODO: font
		/*
		al_draw_textf (font, GREEN, 0, 16, ALLEGRO_ALIGN_LEFT, "%d sprites", sprites.size());
		
		for (int x = 0; x < current->w; ++x)
		{
			for (int y = 0; y < current->h; ++y)
			{
				al_draw_textf (font, YELLOW, x * 32, y * 32,
					ALLEGRO_ALIGN_LEFT, "%d", teg_mapget (current, 0, x, y));
			}
		}
		*/
	}
#endif

	drawSidePanel(gc.buffer);

	al_set_target_bitmap (gc3.buffer);

	if (outroTimer > 0) {
		twirl->enable((float)(OUTRO_WAIT-outroTimer)/10.0, outroBitmap);
		al_draw_filled_rectangle(0, 0, MAIN_WIDTH, MAIN_HEIGHT, BLACK);
		twirl->disable();
	}

}

void Game::score(int mx, int my)
{
	vector<ClothesPickup>::iterator i;
	for (i = clothesPickups.begin(); i != clothesPickups.end(); ++i)
	{
		if (mx == i->mx && my == i->my)
		{
			//found!
			ClothingType type = clothesData[i->index].ct;
			clothes[type] = i->index;
			break;
		}
	}
	
	clothesOk = true;
	for (int j = 0; j < CLOTHINGTYPE_NUM; ++j)
	{
		if (clothes[j] < 0) clothesOk = false;
	}
}

void Game::startOutro()
{
	outroTimer = OUTRO_WAIT;
}

void Game::calculateScores()
{
	totalScore = 0;
	
	int series = clothesData[clothes[0]].id[0];
	bool match = true;
	for (int i = 0; i < CLOTHINGTYPE_NUM; ++i)
	{
		if (clothesData[clothes[i]].id[0] != series) {
			match = false;
		}
	}
	
	matchBonus = match;
	if (match) { totalScore += MATCH_BONUS; }

	timeScore = TIME_MULTIPLIER * getSeconds();
	totalScore += timeScore;
	
	previousHiScore = hiscore_per_level[lastLevel];
	
	int scoreToBeat = levelData[lastLevel].scoreToBeat;
	
	newLevelUnlocked = false;
	
	// you get a star if you go from below scoreToBeat, to above
	if (hiscore_per_level[lastLevel] < scoreToBeat &&
		totalScore >= scoreToBeat)
	{
		newLevelUnlocked = true;
	}
	
	if (totalScore > hiscore_per_level[lastLevel])
	hiscore_per_level[lastLevel] = totalScore;
	
	// submit metrics
	parent->logAchievement(levelData[lastLevel].title);

	write();
}

void Game::init()
{
	gamefont = parent->getResources()->getFont("Vera")->get(16);

	hitSample = parent->getResources()->getSample("HIT");
	explosionSample = parent->getResources()->getSample("EXPLOSION");
	
	for (int i = 0; i < CLOTHES_NUM; ++i)
	{
		clothesData[i].smallbmp = parent->getResources()->getBitmap(clothesData[i].small);
		clothesData[i].largebmp = parent->getResources()->getBitmap(clothesData[i].large);
	}
	doll[0] = parent->getResources()->getBitmap("Base");
	doll[1] = parent->getResources()->getBitmap("Hair");
	
	anims[ST_ENEMY] = parent->getResources()->getAnim("enemy");
	anims[ST_PLAYER] = parent->getResources()->getAnim("player");
	anims[ST_ROCK] = parent->getResources()->getAnim("rock");
	anims[ST_EXPLOSION] = parent->getResources()->getAnim("explosion");
	
	read();

	awake = false;

	twirl = make_shared<TwirlEffect>();
	outroBitmap = al_create_bitmap(640, 480);
}

void Game::drawSidePanel (ALLEGRO_BITMAP *dest)
{
	al_set_target_bitmap (dest);
	al_draw_filled_rectangle (MAIN_WIDTH - SIDEBAR_WIDTH, 0, MAIN_WIDTH, MAIN_HEIGHT,
		clothesOk ? GREEN : YELLOW);
	
	int x = MAIN_WIDTH - SIDEBAR_WIDTH + 16;
	int y = 16;
	
	for (int i = 0; i < 4; ++i)
	{
		al_draw_text (gamefont, BLUE, x, y, ALLEGRO_ALIGN_LEFT, clothesTypeNames[i]);
		y += 16;
		if (clothes[i] >= 0)
			al_draw_bitmap (clothesData[clothes[i]].smallbmp, x, y, 0);
		y += 32;
	}

	int timeInSeconds = getSeconds();
	al_draw_textf (gamefont, BLUE, x, y, ALLEGRO_ALIGN_LEFT, "SECONDS: %03i", timeInSeconds);
	y += 16;

}

void Sprite::setDir(Direction _dir) 
{ 
	if (_dir != dir) 
	{ 
		dir = _dir; 
	} 
}

Sprite::Sprite (Engine *e, Game *game, int amx, int amy, SpriteType st) : engine (e)
{
	lock = 0;
	mx = amx;
	my = amy;
	dir = E;
	mdx = DIR_DX[dir];
	mdy = DIR_DY[dir];
	subx = 0;
	suby = 0;
	subtileCounter = 0;
	speed = 0;
	alive = true;
	visible = true;
	moving = false;
	parent = NULL;
	spriteType = st;
	waitTimer = (st == ST_EXPLOSION ? EXPLODE_TIME : 0);
	parent = game;
	occupyMap (0, 0);
	enemyGo = false;
	pushBuildup = 0;
	anim = parent->anims[st];
	animStart = engine->getCounter(); // make sure anim starts at 0
	animState = 0;
}

void Game::killAll()
{
	list<Sprite*>::iterator i;
	for (i = sprites.begin(); i != sprites.end(); ++i)
	{
		delete (*i);
		(*i) = NULL;
	}
	sprites.clear();
}

int Sprite::getMapAt (int dx, int dy)
{
	return parent->getFlagsAt (mx + dx, my + dy);
}


int Game::getFlagsAt (int mx, int my)
{
	assert (current);
	assert (current->tilelist);
	if (mx < 0 || my < 0 || mx >= current->w || my >= current->h) {
		return TF_SQUARE; // out of bounds -> impasseble
	}
	int tile = teg_mapget (current, 0, mx, my);
	assert (tile >= 0 && tile < current->tilelist->tilenum);
	return current->tilelist->tiles[tile].flags;
}

Sprite *Sprite::getSpriteAt (int dx, int dy)
{
	int nx = mx + dx;
	int ny = my + dy;
	if (!parent->spriteMap.inBounds(nx, ny)) {
		return nullptr;
	}

	// returns NULL if nothing found
	return parent->spriteMap(nx, ny);
}

void Sprite::setMapAt (int dx, int dy, int val)
{
	int nx = mx + dx;
	int ny = my + dy;
	if (nx < 0 || ny < 0 || nx >= parent->current->w || ny >= parent->current->h) {
		return; // ignore out of bounds
	}
	teg_mapput (parent->current, 0, nx, ny, val);
}

void Sprite::occupyMap (int dx, int dy)
{
	int nx = mx + dx;
	int ny = my + dy;
	if (!parent->spriteMap.inBounds(nx, ny)) {
		return;
	}
	assert (parent->spriteMap(nx, ny) == nullptr);
	parent->spriteMap(nx, ny) = this;
}

void Sprite::clearMap (int dx, int dy)
{
	int nx = mx + dx;
	int ny = my + dy;
	if (!parent->spriteMap.inBounds(nx, ny)) {
		return;
	}

	assert (parent->spriteMap(nx, ny) == this);
	parent->spriteMap(nx, ny) = nullptr;
}

bool Sprite::tryMove(int aDx, int aDy, float aSpeed)
{
	Sprite *dest = getSpriteAt (aDx, aDy);
	if (dest != NULL)
	{
		if (spriteType == dest->spriteType ||
			(spriteType == ST_ROCK && dest->spriteType == ST_EXPLOSION))
		{
			// just wait
			moving = false;
			return false;
		}
		else
		{
			kill();
			dest->kill();
			parent->explode (mx, my);
			return false;
		}
	}
	moving = true;
	occupyMap (aDx, aDy);
	mdx = aDx;
	mdy = aDy;
	speed = aSpeed;

	cleanupTail = false; // normally clean up tail later

	if (spriteType == ST_ROCK)
	{
		if (mdx != 0) // moving sideways (means being pushed or tumbling)
		{
			animState = ((mdx > 0) ? 2 : 1); // initiate rolling animation
			cleanupTail = true;
		}
		else
		{
			animState = 0; // straight falling animation
		}
	}

	// for blocks being pushed
	if (cleanupTail)
	{
		clearMap (0, 0);
	}

	return true;
}

void Game::explode(int x, int y)
{
	for (int xi = x - 1; xi <= x + 1; ++xi) {
		for (int yi = y - 1; yi <= y + 1; ++yi) {
			if (!spriteMap.inBounds(xi, yi)) continue;

			teg_mapput (current, 0, xi, yi, TF_EMPTY);
			// kill anything present here
			if (spriteMap(xi, yi) != nullptr) {
				spriteMap(xi, yi)->kill();
			}
			add (new Sprite (parent, this, xi, yi, ST_EXPLOSION));
		}
	}
	parent->playSample (explosionSample);
}

bool Sprite::isClearForEnemy(int dx, int dy)
{
	int flags = getMapAt(dx, dy);
	Sprite *dest = getSpriteAt(dx, dy);
	// empty, and not with another enemy in it, or with an explosion in it
	return (flags == TF_EMPTY && 
		(dest == NULL || 
			(dest->spriteType != ST_ENEMY && dest->spriteType != ST_EXPLOSION)));
}

/**
 * Handle sub-tile movement.
 * Return true if the sub-tile movement has completed.
 */
bool Sprite::subTileMovement()
{
	subtileCounter += speed;
	if (isTumbling()) // tumbling
	{
		subx = mdx * 32.0f * sin(subtileCounter * M_PI / 64.0f);
		suby = 32.0f - (32.0f * cos(subtileCounter * M_PI / 64.0f));
	}
	else
	{
		subx = subtileCounter * mdx;
		suby = subtileCounter * mdy;
	}

	if (subtileCounter > 32)
	{
		// current movement is done
		subtileCounter = 0;
		if (!cleanupTail) { clearMap (0, 0); }
		mx += mdx;
		my += mdy;
		subx = 0;
		suby = 0;
		return false;
	}
	else
	{
		return true;
	}
}

bool Sprite::isTumbling()
{
	// we are a tumbling rock if we are moving diagonally down...
	return (moving && (mdx != 0) && (mdy == 1));
}

int Sprite::update()
{
	assert (parent);
	
	if (waitTimer > 0)
	{
		waitTimer--;
		return 0;
	}
	
	// continue current movement
	if (moving)
	{
		moving = subTileMovement();
	}
	
	if (!moving)
	{
		// make a new decision
		switch (spriteType)
		{
		case ST_EXPLOSION:
			{
				//stop as soon as wait timer is over
				kill();
			}
			break;
		case ST_PLAYER:
			updatePlayer();
			break;
		case ST_ENEMY:
			updateEnemy();
			break;
		case ST_ROCK:
			{
				//if (parent->isclear (mx, my + 1))
				//when continuing to fall, we check only for empty space, not sprites.
				if (getMapAt(0, 1) == TF_EMPTY)
				{
					tryMove (0, 1, FALL_SPEED);
				}
				else if (parent->canTumble (mx, my, +1))
				{
					tryMove (1, 1, TUMBLE_SPEED);
				}
				else if (parent->canTumble (mx, my, -1))
				{
					tryMove (-1, 1, TUMBLE_SPEED);
				}
				else
				{
					parent->parent->playSample (parent->hitSample);
					setMapAt(0, 0, TF_ROCK);
					kill();
				}
			}
			break;
		case ST_BONUS: /* do nothing */ break; 
		}
	}
	return 0;
}

void Sprite::updatePlayer() {
	ALLEGRO_KEYBOARD_STATE kbd;
	al_get_keyboard_state(&kbd);
	if (al_key_down (&kbd, ALLEGRO_KEY_ESCAPE))
	{
		kill();
		parent->explode (mx, my);
	}
	else
	{
		bool anyInput = false;
		if (engine->getInput()[btnUp].getState()) {
			anyInput = true;
			dir = N;
		}
		if (engine->getInput()[btnDown].getState()) {
			anyInput = true;
			dir = S;
		}
		if (engine->getInput()[btnLeft].getState()) {
			anyInput = true;
			dir = E;
		}
		if (engine->getInput()[btnRight].getState()) {
			anyInput = true;
			dir = W;
		}
		if (anyInput)
		{
			int ndx = DIR_DX[dir];
			int ndy = DIR_DY[dir];
			int flags = getMapAt (ndx, ndy);
			switch (flags)
			{
				case TF_EMPTY:
				case TF_SOFT:
				case TF_BONUS:
					tryMove (ndx, ndy, flags == TF_SOFT ? 
						PLAYER_SPEED_EMPTY : PLAYER_SPEED_EAT);
					if (flags == TF_BONUS) parent->score(mx + ndx, my + ndy);
					if (flags == TF_SOFT) setMapAt (ndx, ndy, TF_EMPTY);
					pushBuildup = 0;
					animState = 0;
					break;
				case TF_EXIT: //exit -> we won!
					if (parent->clothesOk) 
					{
						parent->calculateScores();
						parent->startOutro();
						waitTimer = OUTRO_WAIT; // prevent further movement
						animState = 1; // set winning animation
					}
					break;
				case TF_ROCK:
					if (ndy == 0) // only left or right
					{
						animState = 2;
						pushBuildup++;
						if (pushBuildup > PUSH_DELAY && parent->isclear(mx + (ndx * 2), my))
						{
							Sprite* rock = parent->tileToSprite (mx + ndx, my, ST_ROCK);

							// check if we can make it tumble by pushing.
							if (parent->isclear (mx + (ndx * 2), my + 1))
							{
								rock->tryMove (ndx, ndy + 1, TUMBLE_SPEED);
							}
							else
							{
								rock->tryMove (ndx, ndy, PUSH_SPEED);
							}
							pushBuildup = 0;
							tryMove (ndx, ndy, PUSH_SPEED);
						}
					}
					break;
				default:
					moving = false;
			}
		}
		else
		{
			pushBuildup = 0;
			animState = 0;
		}
	}
}

void Sprite::updateEnemy() {
	// turn one way if we can
	Direction newDir = (Direction)((dir + 1) % 4);
	int nx = DIR_DX[newDir];
	int ny = DIR_DY[newDir];
	if (!enemyGo && isClearForEnemy(nx, ny))
	{
		// turn one way
		dir = newDir;
		mdx = nx;
		mdy = ny;
		waitTimer = TURN_WAIT;
		enemyGo = true; // do not turn twice in a row this way
	}
	else if (isClearForEnemy (mdx, mdy))
	{
		tryMove (mdx, mdy, ENEMY_SPEED);
		enemyGo = false;
	}
	else
	{
		// turn the other way
		dir = (Direction)((dir + 3) % 4);
		mdx = DIR_DX[dir];
		mdy = DIR_DY[dir];
		waitTimer = TURN_WAIT;
	}
}

void Sprite::kill()
{
	alive = false;
	
	// clear all positions around
	for (int xi = -1; xi <= 1; ++xi)
	{
		for (int yi = -1; yi <= 1; ++yi)
		{
			if (getSpriteAt(xi, yi) == this)
				clearMap(xi, yi);
		}
	}
	
	if (spriteType == ST_PLAYER)
	{
		// we're dead
		parent->gameover = true;
		parent->startOutro();
		parent->player = NULL;
	}
}

void Game::add(Sprite *o)
{
	sprites.push_back (o);
}

void Sprite::draw(const GraphicsContext &gc)
{
	int x = getx() + gc.xofst;
	int y = gety() + gc.yofst;
	anim->drawFrame (animState, dir, engine->getCounter() - animStart, x, y);
}

void Game::debugLevelFinished() {
	clothes[0] = 1;
	clothes[1] = 3;
	clothes[2] = 2;
	clothes[3] = 0;
}

// NOTE: the order Top, Bottom, Accessories, Shoes should match the digit in the ID (To make level editing easier).
ClothesData clothesData [CLOTHES_NUM] =
{
	{ id: "A1", large: "XmasTop",           small: "XmasTop_Icon",           ct: CT_TOP,       order: 0, smallbmp: nullptr, largebmp: nullptr },
	{ id: "A2", large: "XmasBottom",        small: "XmasBottom_Icon",        ct: CT_BOTTOM,    order: 4, smallbmp: nullptr, largebmp: nullptr },
	{ id: "A3", large: "XmasAccessories",   small: "XmasAccessories_Icon",   ct: CT_ACCESSORY, order: 8, smallbmp: nullptr, largebmp: nullptr },
	{ id: "A4", large: "XmasShoes",         small: "XmasShoes_Icon",         ct: CT_SHOES,     order: 12, smallbmp: nullptr, largebmp: nullptr },
	{ id: "B1", large: "OL_Top",            small: "OL_Top_Icon",            ct: CT_TOP,       order: 1, smallbmp: nullptr, largebmp: nullptr },
	{ id: "B2", large: "OL_Bottom",         small: "OL_Bottom_Icon",         ct: CT_BOTTOM,    order : 5, smallbmp: nullptr, largebmp: nullptr },
	{ id: "B3", large: "OL_Accessories",    small: "OL_Accessories_Icon",    ct: CT_ACCESSORY, order: 9, smallbmp: nullptr, largebmp: nullptr },
	{ id: "B4", large: "OL_Shoes",          small: "OL_Shoes_Icon",          ct: CT_SHOES,     order: 13, smallbmp: nullptr, largebmp: nullptr },
	{ id: "C1", large: "KiltTop",           small: "KiltTop_Icon",           ct: CT_TOP,       order: 2, smallbmp: nullptr, largebmp: nullptr },
	{ id: "C2", large: "KiltBottom",        small: "KiltBottom_Icon",        ct: CT_BOTTOM,    order: 6, smallbmp: nullptr, largebmp: nullptr },
	{ id: "C3", large: "KiltHat",           small: "KiltHat_Icon",           ct: CT_ACCESSORY, order: 14, smallbmp: nullptr, largebmp: nullptr },
	{ id: "C4", large: "KiltShoes",         small: "KiltShoes_Icon",         ct: CT_SHOES,     order: 10, smallbmp: nullptr, largebmp: nullptr },
	{ id: "D1", large: "Sport_Top",         small: "Sport_Top_Icon",         ct: CT_TOP,       order : 3, smallbmp: nullptr, largebmp: nullptr },
	{ id: "D2", large: "Sport_Bottom",      small: "Sport_Bottom_Icon",      ct: CT_BOTTOM,    order : 11, smallbmp: nullptr, largebmp: nullptr },
	{ id: "D3", large: "Sport_Accessories", small: "Sport_Accessories_Icon", ct: CT_ACCESSORY, order: 7, smallbmp: nullptr, largebmp: nullptr },
	{ id: "D4", large: "Sport_Shoes",       small: "Sport_Shoes_Icon",       ct: CT_SHOES,     order: 15, smallbmp: nullptr, largebmp: nullptr },
	{ id: "E1", large: "Roller_Top",         small: "Roller_Top_Icon",         ct: CT_TOP,       order : 3, smallbmp: nullptr, largebmp: nullptr },
	{ id: "E2", large: "Roller_Bottom",      small: "Roller_Bottom_Icon",      ct: CT_BOTTOM,    order : 11, smallbmp: nullptr, largebmp: nullptr },
	{ id: "E3", large: "Roller_Accessories", small: "Roller_Accessories_Icon", ct: CT_ACCESSORY, order: 7, smallbmp: nullptr, largebmp: nullptr },
	{ id: "E4", large: "Roller_Shoes",       small: "Roller_Shoes_Icon",       ct: CT_SHOES,     order: 15, smallbmp: nullptr, largebmp: nullptr },
};

int Game::getSeconds()
{
	int avail = levelData[lastLevel].time;
	int used = (parent->getCounter() - startTime) / 1000;
	return (avail - used < 0 ? 0 : avail - used);
}

void Game::read()
{
	ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();

	for (int i = 0; i < LEVEL_NUM; ++i) hiscore_per_level[i] = 0;
	
	for (int i = 0; i < LEVEL_NUM; ++i)
	{
		char buf[256];
		snprintf (buf, sizeof(buf), "level_%s_score", levelData[i].uid);
		hiscore_per_level[i] = get_config_int (config, "sh09", buf, hiscore_per_level[i]);
	}

}

void Game::write()
{
	ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();

	for (int i = 0; i < LEVEL_NUM; ++i)
	{
		char buf[256];
		snprintf (buf, sizeof(buf), "level_%s_score", levelData[i].uid);
		set_config_int (config, "sh09", buf, hiscore_per_level[i]);
	}
	set_config_int (config, "sh09", "stars", getTotalStars()); // NB: dummy - value is never used
}
