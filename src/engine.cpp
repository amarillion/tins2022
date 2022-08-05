#include <assert.h>
#include "engine.h"
#include "color.h"
#include "screenshot.h"
#include "player.h"
#include "leveldata.h"
#include "motionimpl.h"
#include <memory>
#include "menubase.h"
#include "keymenuitem.h"
#include <allegro5/allegro_primitives.h>
#include "viewport.h"
#include "bitmap.h"
#include "util.h"
#include "DrawStrategy.h"
#include "resources.h"
#include "anim.h"

using namespace std;

const char *DIRECTIONS[] = { "SE", "S", "SW", "W", "NW", "N", "NE", "E" };

int Engine::getCounter ()
{
	return MainLoop::getMainLoop()->getMsecCounter();
}

Engine::Engine () : settings(), resources(Resources::newInstance())
{
	debug = false;
	btnScreenshot.setScancode (ALLEGRO_KEY_F12);

#ifdef DEBUG
	btnAbort.setScancode (ALLEGRO_KEY_F10);
	btnDebugMode.setScancode (ALLEGRO_KEY_F11);
#endif
}

Input* Engine::getInput()
{
	return settings.getInput();
}

int Engine::init()
{
	settings.getFromConfig(MainLoop::getMainLoop()->getConfig());
	initKeyboard(); // install custom keyboard handler
	Anim::setDirectionModel (make_shared<DirectionModel>(DIRECTIONS, DIRNUM));

	if (!(
		resources->addFiles("data/*.ttf") &&
		resources->addFiles("data/*.png") &&
		resources->addFiles("data/*.xml") &&
		resources->addFiles("data/*.wav") &&
		resources->addFiles("data/*.tga")
		))
	{
		allegro_message ("Could not load all resources!\n%s", resources->getErrorMsg());
		return 1;
	}

	srand(time(0));

 	ActiveSprite::init (resources);

	script = make_shared<Script>(*this);
	add (script, FLAG_SLEEP);

	playerControl = make_shared<PlayerControl>(*this);
	add (playerControl, FLAG_SLEEP);

 	initMenu();
 	script->init();
 	playerControl->init();

	ALLEGRO_PATH *localAppData = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	string cacheDir = al_path_cstr(localAppData, ALLEGRO_NATIVE_PATH_SEP);
	updates = UpdateChecker::newInstance(cacheDir, APPLICATION_VERSION, E_QUIT);
	updates->setFont(resources->getFont("!CRASS ROOTS OFL")->get(24));
 	add (updates, Container::FLAG_SLEEP);
 	updates->start_check_thread("drf", "en");

	handleMessage(nullptr, E_MAINMENU);

	return 0;
}

void Engine::done ()
{
	updates->done();
}

static const int TILEW = 128;
static const int TILEH = 128;

ComponentPtr Engine::initMenuBackground()
{
	auto texture = make_shared_bitmap (TILEW, TILEH);
	assert (texture);
	
	ALLEGRO_COLOR c1 = al_map_rgb (164, 0, 0);
	ALLEGRO_COLOR c2 = al_map_rgb (0, 164, 0);
	al_set_target_bitmap (texture.get());
	al_clear_to_color (al_map_rgb(164, 164, 164));
	al_draw_filled_rectangle(0, 0, TILEW / 2, TILEW / 2, c1);
	al_draw_filled_rectangle(TILEW / 2 + 1, TILEH / 2 + 1, TILEW, TILEW, c2);

	auto menubg = Pattern::build(texture).linear(+1, +1).get();
	return menubg;
}

void Engine::handleMessage(ComponentPtr src, int code)
{
	switch (code)
	{
		case E_NONE:
			// do nothing;
			break;
		case E_MAINMENU:
			setFocus(mMain);
			break;
		case E_TOGGLE_FULLSCREEN:
			MainLoop::getMainLoop()->toggleWindowed();
			break;
		case MENU_SETTINGS:
			setFocus (mKeys);
			break;
		case E_STARTGAME:
			playerControl->initGame();
			script->prepare(playerControl->getCurrentLevel());
			 setFocus(script);
			break;
		case E_NEXTLEVEL:
			initLevel(playerControl->getCurrentLevel());
			setFocus(playerControl);
			break;
		case E_NEXTSCRIPT:
			script->prepare(playerControl->getCurrentLevel());
			setFocus(script);
			break;
		case E_PAUSE:
			setFocus(mPause);
			break;
		case E_RESUME:
			setFocus(playerControl);
			break;
		case E_EXITSCREEN:
			setFocus(updates);
			break;
		case E_QUIT:
			pushMsg(MSG_CLOSE);
			break;
	}
}

void Engine::onUpdate ()
{
	//TODO: implement these buttons as children of mainloop that send action events on press
	if (btnScreenshot.justPressed())
	{
		screenshot();
	}

#ifdef DEBUG
	if (btnDebugMode.justPressed())
	{
		debug = !debug;
	}
	if (btnAbort.justPressed())
	{
		pushMsg(E_QUIT);
	}
#endif
}

void Engine::initLevel(int level)
{
	LevelData const * current = &levels[level];

	playerControl->initLevel(current);
}


void Engine::initMenu()
{
	auto menubg = initMenuBackground();

//	menufont = resources->getAlfont ("!CRASS ROOTS OFL", 40);
	sfont = resources->getFont ("!CRASS ROOTS OFL")->get(32);

	mMain = MenuBuilder(this, NULL)
				.push_back (make_shared<ActionMenuItem>(E_STARTGAME, "Start", "Start a new game"))
				.push_back (make_shared<ActionMenuItem>(MENU_SETTINGS, "Settings", "Configure keys and other settings"))
				.push_back (make_shared<ActionMenuItem>(E_EXITSCREEN, "Quit", "Exit game"))
			.build();

	mMain->add(menubg, FLAG_BOTTOM);

	ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();
	mKeys = MenuBuilder(this, NULL)
		.push_back (make_shared<KeyMenuItem>("North", config_keys[btnN], getInput()[btnN], config))
		.push_back (make_shared<KeyMenuItem>("N.East", config_keys[btnNE], getInput()[btnNE], config))
		.push_back (make_shared<KeyMenuItem>("East", config_keys[btnE], getInput()[btnE], config))
		.push_back (make_shared<KeyMenuItem>("S.East", config_keys[btnSE], getInput()[btnSE], config))
		.push_back (make_shared<KeyMenuItem>("South", config_keys[btnS], getInput()[btnS], config))
		.push_back (make_shared<KeyMenuItem>("S.West", config_keys[btnSW], getInput()[btnSW], config))
		.push_back (make_shared<KeyMenuItem>("West", config_keys[btnW], getInput()[btnW], config))
		.push_back (make_shared<KeyMenuItem>("N.West", config_keys[btnNW], getInput()[btnNW], config))
		.push_back (make_shared<KeyMenuItem>("Fire", config_keys[btnFire], getInput()[btnFire], config))
		.push_back (make_shared<KeyMenuItem>("Alt. Fire", config_keys[btnAltFire], getInput()[btnAltFire], config))
		.push_back (make_shared<KeyMenuItem>("Switch", config_keys[btnSwitch], getInput()[btnSwitch], config))
		.push_back (make_shared<ActionMenuItem>(E_TOGGLE_FULLSCREEN, "Toggle Fullscreen", "Switch fullscreen / windowed mode"))
		.push_back (make_shared<ActionMenuItem>(E_MAINMENU, "Main Menu", "Return to the main menu"))
		.build();
	mKeys->setGroupLayout(1, MenuScreen::twoColumnLayoutFunction);

	mKeys->add(menubg, FLAG_BOTTOM);

	mPause = MenuBuilder (this, NULL)
			.push_back (make_shared<ActionMenuItem>(E_RESUME, "Resume", "Resume game"))
			.push_back (make_shared<ActionMenuItem>(E_MAINMENU, "Exit to Main Menu", "Stop game and exit to main menu"))
		.build();
	mPause->add(menubg, FLAG_BOTTOM);
}
