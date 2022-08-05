#include "game.h"
#include "container.h"
#include <allegro5/allegro.h>
#include "color.h"
#include <allegro5/allegro_font.h>
#include "util.h"
#include "mainloop.h"
#include "strutil.h"

using namespace std;

Game::Game(Engine *engine, Settings *_settings) : objects(), parent(engine), settings(_settings)
{
	player[0] = NULL;
	player[1] = NULL;
	level = NULL;
	roomSet = NULL;

	btnPause.setScancode(ALLEGRO_KEY_ESCAPE);

	messages = make_shared<Messages>();
	add(messages);

	view[0] = make_shared<View>(this);	
	view[1] = make_shared<View>(this);
}

Game::~Game()
{
	if (level != NULL) delete level;
	objects.killAll();
	if (roomSet != NULL) delete roomSet;

	for (int i = 0; i < ICON_NUM; ++i) {
		if (icons[i]) al_destroy_bitmap(icons[i]);
	}
}

void Game::draw (const GraphicsContext &gc)
{
	al_set_target_bitmap(gc.buffer);
	al_clear_to_color (BLACK);
	//~ teg_draw (buffer, level, 0, camera_x, 0);
	//~ objects.draw(buffer, camera_x, 0);
	for (int i = 0; i < settings->numPlayers; ++i)
	{
		if (!ps[i].died)
		{
			view[i]->draw(gc);
		}
		drawStatus (gc.buffer, view[i]->status_x, view[i]->status_y, &ps[i]);
	}
	int min = (gameTimer / 60000);
	int sec = (gameTimer / 1000) % 60;
	int csec = (gameTimer / 10) % 100;
	al_draw_textf (gamefont, WHITE, 0, 464, ALLEGRO_ALIGN_LEFT, "%02i:%02i:%02i", min, sec, csec);

	messages->draw(gc);
}

void Game::update()
{
	if (gameTimer % 1000 == 0) {
		if (gameTimer == 60000) {
				messages->showMessage("One minute remaining", Messages::RIGHT_TO_LEFT);
		}

		if (gameTimer == 30000) {
				messages->showMessage("Hurry up!", Messages::RIGHT_TO_LEFT);
		}

		if (gameTimer >= 1000 && gameTimer <= 10000) {
				messages->showMessage(string_format("%i", (gameTimer / 1000) - 1 ), Messages::RIGHT_TO_LEFT);
		}
	}

	messages->update();

	if (btnPause.justPressed())
	{
		pushMsg(Engine::MENU_PAUSE);
	}

	// add objects based on position
	gameTimer -= MainLoop::getMainLoop()->getLogicIntervalMsec();
	objects.update();
	bool gameover = true;
	int bananasGot = 0;

	for (int i = 0; i < settings->numPlayers; ++i)
	{
		view[i]->update();
		
		if (!ps[i].died) gameover = false;
		bananasGot += ps[i].bananas;
	}
	if (gameover)
	{
		gameover_message = "GAME OVER";
		parent->logAchievement(string_format("players_%i_level_%i_gameover", settings->numPlayers, currentLevel));

		pushMsg(Engine::GS_GAME_OVER);
	}
	if (gameTimer <= 0)
	{
		gameover_message = "TIME UP";
		parent->logAchievement(string_format("players_%i_level_%i_timeup", settings->numPlayers, currentLevel));
		pushMsg(Engine::GS_GAME_OVER);
	}
	if (bananaCount - bananasGot == 0)
	{
		doneLevel();
		nextLevel();
		pushMsg(Engine::E_NEXT_LEVEL);
		return;
	}
}

void Game::drawStatus (ALLEGRO_BITMAP *buffer, int x, int y, PlayerState *xps)
{
	// draw bananas
	int bananasGot = 0;
	for (int i = 0; i < settings->numPlayers; ++i) bananasGot += ps[i].bananas;
	int total = bananaCount - bananasGot + xps->bananas;
	for (int i = 0; i < total; ++i) {
		al_draw_bitmap(icons[i < xps->bananas ? ICON_BANANA : ICON_BANANA_PLACEHOLDER], x - 8 + (i * 32), y, 0);
	}
	
	// draw keys
	for (int i = 0; i < xps->keys; ++i) {
		al_draw_bitmap(icons[ICON_KEY], x + 120 + i * 32, y, 0);
	}
	
	// draw health bar
	float x1 = x;
	float y1 = y + 44;
	float hpPerPixel = 0.5;
	float wMax = xps->hpMax * hpPerPixel;
	float wHp = xps->hp * hpPerPixel;
	al_draw_filled_rectangle(x1, y1, x1 + wHp, y1 + 12, RED);
	al_draw_rectangle(x1, y1, x1 + wMax, y1 + 12, WHITE, 1.0);

	al_draw_textf (gamefont, WHITE, x + 128, y + 44, ALLEGRO_ALIGN_LEFT, "XP %05i", xps->xp);
}

void Game::initGame ()
{
	// initialize game and player stats.
	// to start a new game.
	currentLevel = 0;
	gameTimer = defaultGameTime;

	// create a level
	// TODO: dynamic
	ps[0] = PlayerState();
	ps[1] = PlayerState();

	monsterHp = defaultMonsterHp;
	initLevel();
}

Player * initPlayer(PlayerState *ps, Room *room, int i) {
	auto result = new Player (ps, room, i);
	int x, y;
	room->getPlayerLocation(i, &x, &y);
	result->setLocation(x * TILE_SIZE, y * TILE_SIZE);
	cout << "Initializing player " << i << " at [" << x << ", " << y << "]\n";
	return result;
}

void Game::initLevel()
{
	// initialize objects
	level = createLevel(roomSet, &objects, currentLevel + (settings->numPlayers == 1 ? 4 : 6), monsterHp);

	player[0] = initPlayer(&ps[0], level->getStartRoom(0), 0);
	objects.add (player[0]);

	if (settings->numPlayers == 2)
	{
		player[1] = initPlayer (&ps[1], level->getStartRoom(1), 1);
		objects.add (player[1]);
	}
	else
	{
		player[1] = nullptr;
	}

	view[0]->init (settings->numPlayers, 0, getw(), geth());
 	view[0]->player = player[0];
	view[1]->init (2, 1, getw(), geth());
	view[1]->player = player[1];

	bananaCount = level->getBananaCount();

	messages->showMessage(string_format("Collect %i Bananas", bananaCount), Messages::RIGHT_TO_LEFT);

}

void Game::doneLevel()
{
	objects.killAll();
	if (level != NULL)
	{
		delete level;
		level = NULL;
	}
}

// called when exit found
void Game::nextLevel()
{
	parent->logAchievement(string_format("players_%i_level_%i", settings->numPlayers, currentLevel));

	// go on to next level
	currentLevel++;
	
	// int max = 0;
	// int maxPlayer = 0;
	// int i;
	// for (i = 0; i < settings->numPlayers; ++i) {
	// 	if (ps[i].bananas > max) {
	// 		maxPlayer = i;
	// 		max = ps[i].bananas;
	// 	}
	// }
	// ps[maxPlayer].hpMax += 10;
	
	for (int i = 0; i < settings->numPlayers; ++i) {
		ps[i].bananas = 0;
		ps[i].keys = 0;
	}

	gameTimer += gameTimeIncrease; // extra minute
	monsterHp += monsterHpIncrease;
	initLevel();
}

Player *Game::getNearestPlayer (Object *o)
{
	assert (o);
	if (settings->numPlayers == 2)
	{
		Room *r1, *r2;
		r1 = player[0]->getRoom();
		r2 = player[1]->getRoom();
		if (r1 == o->getRoom() && r2 == o->getRoom())
		{
			double dx1 = player[0]->getx() - o->getx();
			double dy1 = player[0]->gety() - o->gety();
			double dx2 = player[1]->getx() - o->getx();
			double dy2 = player[1]->gety() - o->gety();
			if (dx1 * dx1 + dy1 * dy1 < dx2 * dx2 + dy2 * dy2)
			{
				return player[1];
			}
			else
			{
				return player[0];
			}
		}
		else if (r1 == o->getRoom())
		{
			return player[0];
		}
		else if (r2 == o->getRoom())
		{
			return player[1];
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		if (player[0]->getRoom() == o->getRoom())
		{
			return player[0];
		}
		else
		{
			return NULL;
		}
	}
}

void Game::init (shared_ptr<Resources> resources)
{
	roomSet = RoomSet::init(resources);
	gamefont = resources->getFont("builtin_font")->get();
	messages->setFont(resources->getFont("SpicyRice-Regular")->get(48));
	
	ALLEGRO_BITMAP *iconsheet = resources->getBitmap("pickups");
	icons[ICON_BANANA_PLACEHOLDER] = al_create_sub_bitmap(iconsheet, 320, 0, 40, 40);
	icons[ICON_BANANA] = al_create_sub_bitmap(iconsheet, 120, 0, 40, 40);
	icons[ICON_KEY] = al_create_sub_bitmap(iconsheet, 80, 0, 40, 40);
}
