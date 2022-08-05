#include <assert.h>
#include "engine.h"
#include "color.h"
#include "screenshot.h"
#include "motionimpl.h"
#include "menubase.h"
#include "util.h"
#include <memory>
#include "keymenuitem.h"
#include <allegro5/allegro_primitives.h>
#include "bitmap.h"
#include "DrawStrategy.h"
#include "twirl.h"
#include "text.h"
#include "strutil.h"
#include "anim.h"

//#include "steps.h" //TODO

using namespace std;

int Engine::getCounter ()
{
	return MainLoop::getMainLoop()->getMsecCounter();
}

Engine::Engine () : resources(Resources::newInstance()), settings(), layers (), version("data/version.ini"), nextLevel (-1)
{
	game = make_shared<Game>(this);
	postmortem = PostMortem::newInstance(this, game);
	debug = false;
	
	add (game, Container::FLAG_SLEEP);
	add (postmortem, Container::FLAG_SLEEP);
	
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

const int DIRNUM = 4;
const char *DIRECTIONS[DIRNUM] = { "n", "e", "s", "w" };

int Engine::init()
{
	Anim::setDirectionModel (make_shared<DirectionModel>(DIRECTIONS, DIRNUM));

	settings.getFromConfig(MainLoop::getMainLoop()->getConfig());
	initKeyboard(); // install custom keyboard handler of input.cpp

	if (!(
		resources->addFiles("data/*.ttf") &&
		resources->addFiles("data/*.png") &&
		resources->addFiles("data/clothes/*.png") &&
		resources->addFiles("data/*.tll") &&
		resources->addFiles("data/*.xml") &&
		resources->addFiles("data/*.wav") &&
		resources->addFiles("data/*.glsl")
		))
	{
		allegro_message ("Could not load all resources!\n%s", resources->getErrorMsg());
		return 1;
	}

	//TODO: store tileset reference in map itself.
	for (int i = 0; i < LEVEL_NUM; ++i)
	{
		int result = resources->addJsonMapFile(levelData[i].title, levelData[i].filename, "TILES");
		if (!result)
		{
			allegro_message ("Could not load all resources!\n%s", resources->getErrorMsg());
			return 0;

		}
		levelData[i].map = resources->getJsonMap(levelData[i].title);
		assert (levelData[i].map->map->tilelist);
	}

 	srand(time(0));

 	TwirlEffect::init(resources);
 	game->init();
 	initMenu(); // this must be called after initing game

 	postmortem->init();

	std::string versionStr = version.version.get();
	// to prevent malicious manipulation //TODO: should be done in metrics module
	if (versionStr.length() > 50) versionStr = versionStr.substr(0, 50);

	ALLEGRO_PATH *localAppData = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	string cacheDir = al_path_cstr(localAppData, ALLEGRO_NATIVE_PATH_SEP);
	updates = UpdateChecker::newInstance(cacheDir, versionStr, E_QUIT);
 	add (updates, Container::FLAG_SLEEP);
 	updates->start_check_thread("fashionista", "en");
 	updates->setFont(resources->getFont("Vera")->get(24));

	metrics = Metrics::newInstance("fashionista", versionStr);
	metrics->logSessionStart();
// 	initIntro(); // TODO: enable when initIntro is complete
 	setFocus(mMain); //TODO: disable
	return 0;
}


static const int TILEW = 128;
static const int TILEH = 128;

ComponentPtr Engine::initBackground()
{
	shared_ptr<ALLEGRO_BITMAP> texture = make_shared_bitmap (TILEW, TILEH);
	assert (texture);
	al_set_target_bitmap (texture.get());
	
	ALLEGRO_COLOR c1 = WHITE;
	ALLEGRO_COLOR c2 = al_map_rgb (128, 128, 128);
	al_clear_to_color (c1);
	al_draw_filled_rectangle (0, 0, TILEW / 2, TILEW / 2, c2);
	al_draw_filled_rectangle (TILEW / 2 + 1, TILEH / 2 + 1, TILEW, TILEW, c2);
	
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			al_draw_bitmap (clothesData[x + 4 * y].smallbmp, 32 * x, 32 * y, 0);
		}
	}
	
	//TODO
//	drawing_mode (DRAW_MODE_TRANS, NULL, 0, 0);
//	set_trans_blender (0, 0, 0, 160);
	al_draw_filled_rectangle (0, 0, TILEW, TILEH, al_map_rgba (0, 0, 255, 160));
//	solid_mode();
	
	auto menubg = make_shared<Pattern>(texture);
	menubg->setMotion (make_shared<Lissajous>(300, 80, 750, 60));
	return menubg;
}

void Engine::handleMessage(ComponentPtr src, int code)
{
	switch (code)
	{
		case MENU_CHOOSELEVEL:
			setFocus (mLevels);
		break;
		case MENU_SETTINGS:
			setFocus (mKeys);
		break;
		case E_MAINMENU:
			setFocus(mMain);
			break;
		case E_STARTGAME:
			assert (nextLevel >= 0);
			game->initLevel(nextLevel);
			setFocus(game);
			break;
		case E_POSTMORTEM_DEBUG:
			game->debugLevelFinished();
			postmortem->prepare();
			setFocus(postmortem);
			break;
		case E_POSTMORTEM:
			postmortem->prepare();
			setFocus(postmortem);
			break;
		case E_TOGGLE_FULLSCREEN:
			MainLoop::getMainLoop()->toggleWindowed();
			break;
		case E_EXITSCREEN:
			setFocus(updates);
			metrics->logSessionClose();
			break;
		case E_QUIT:
			metrics->done();
			pushMsg(MSG_CLOSE);
			break;
	}
}

void Engine::onUpdate ()
{
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
		pushMsg(E_EXITSCREEN);
	}
#endif
}

void Engine::initIntro()
{
	ContainerPtr intro = make_shared<Container>();

	intro->add(ClearScreen::build(BLACK).get());

	intro->add(
			AnimComponent::build(getResources()->getAnim("player")).xy(100, 100).get());

//	intro->add(
//			AnimComponent::build(getResources()->getAnim("player")).xy(0, 300).steps(
//
//					Steps::build()
//
//						.setState(0 /* WALKING */)
//						.linear(100, +1, 0)
//						.setState(1 /* SCARED */)
//						.wait(50)
//						.setState(2 /* RUNNING */)
//						.linear(100, -2, 0)
//						.get()
//
//					).get()
//	);

	intro->setTimer (50, E_MAINMENU); //TODO - this won't kill the intro. I want a method that kills and returns event at the same time.

	add(intro);
	setFocus(intro);
}

void Engine::initMenu()
{
 	auto menubg = initBackground();

	sfont = getResources()->getFont("Vera")->get(32);

	mMain = MenuBuilder(this, NULL)
			.push_back (make_shared<ActionMenuItem>(MENU_CHOOSELEVEL, "Start", "Start a new game"))
			.push_back (make_shared<ActionMenuItem>(MENU_SETTINGS, "Settings", "Configure keys and other settings"))
#ifdef DEBUG
			// .push_back (make_shared<ActionMenuItem>(E_POSTMORTEM_DEBUG, "Postmortem", "Debugging postmortem screen"))
#endif
			.push_back (make_shared<ActionMenuItem>(E_EXITSCREEN, "Quit", "Exit game"))
			.build();


	mMain->setFont(sfont);
	mMain->add (menubg, Container::FLAG_BOTTOM);

	mMain->add(Text::build(GREY, ALLEGRO_ALIGN_RIGHT, string_format("v%s.%s", version.version.get(), version.buildDate.get()))
			.layout(Layout::RIGHT_BOTTOM_W_H, 4, 4, 300, 28).get());

	ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();
	mKeys = MenuBuilder(this, NULL)
			.push_back (make_shared<KeyMenuItem>("Up", config_keys[btnUp], getInput()[btnUp], config))
			.push_back (make_shared<KeyMenuItem>("Down", config_keys[btnDown], getInput()[btnDown], config))
			.push_back (make_shared<KeyMenuItem>("Left", config_keys[btnLeft], getInput()[btnLeft], config))
			.push_back (make_shared<KeyMenuItem>("Right", config_keys[btnRight], getInput()[btnRight], config))
			.push_back (make_shared<KeyMenuItem>("Action", config_keys[btnAction], getInput()[btnAction], config))
			.push_back (make_shared<ActionMenuItem>(E_TOGGLE_FULLSCREEN, "Toggle Fullscreen", "Switch fullscreen / windowed mode"))
			.push_back (make_shared<ActionMenuItem>(E_MAINMENU, "Main Menu", "Return to the main menu"))
			.build();
	mKeys->add (menubg, Container::FLAG_BOTTOM);
	mKeys->setFont(sfont);

	mPause = MenuBuilder(this, NULL)
		.push_back (make_shared<ActionMenuItem> (E_RESUME, "Resume", "Resume game"))
		.push_back (MenuItemPtr(new ActionMenuItem (E_MAINMENU,
			"Exit to Main Menu", "Stop game and exit to main menu")))
		.build();

	mPause->setFont(sfont);
	mPause->add (menubg, Container::FLAG_BOTTOM);

	MenuBuilder mb = MenuBuilder(this, NULL);
	for (int i = 0; i < LEVEL_NUM; ++i)
	{
		mb.push_back (make_shared<LevelMenuItem>(this, i));
	}
	mb.push_back (make_shared<ActionMenuItem>(E_MAINMENU, "Main Menu", "Return to the main menu"));
	mLevels = mb.build();
	mLevels->setFont(sfont);
	mLevels->add (menubg, Container::FLAG_BOTTOM);

//	setFocus(mMain);
}
