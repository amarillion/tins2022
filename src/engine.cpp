#include "engine.h"
#include "game.h"
#include "resources.h"
#include "mainloop.h"
#include "DrawStrategy.h" // clearscreen
#include "color.h"
#include "text.h"
#include "input.h"
#include "metrics.h"
#include "anim.h"
#include "versionLoader.h"
#include "strutil.h"
#include "menubase.h"
#include "settings.h"
#include "keymenuitem.h"

using namespace std;

const int DIRNUM = 2;
const char *DIRECTIONS[DIRNUM] = { "left", "right" };

class EngineImpl : public Engine
{
private:
	Settings settings;
	shared_ptr<Game> game;
	shared_ptr<Resources> resources;

	shared_ptr<Metrics> metrics;
	bool isContinue = false;
	VersionLoader version { "data/version.ini" };
	bool debug = false;
#ifdef DEBUG
	Input btnAbort;
	Input btnDebugMode;
#endif

public:
	EngineImpl() : game(Game::newInstance(this)), 
		resources(Resources::newInstance()), mMain()
	{
#ifdef DEBUG
		btnAbort.setScancode (ALLEGRO_KEY_F10);
		btnDebugMode.setScancode (ALLEGRO_KEY_F11);
#endif
	}

	virtual shared_ptr<Resources> getResources() override { return resources; }

	virtual bool isDebug() override { return debug; }
	virtual int init() override
	{
		Anim::setDirectionModel (make_shared<DirectionModel>(DIRECTIONS, DIRNUM));
		srand(time(0));

		if (!(
			// resources->addFiles("data/*.png") &&
			// resources->addFiles("data/*.tll") &&
			// resources->addFiles("data/*.xml") &&
			resources->addFiles("data/*.ttf") 
			))
		{
			allegro_message ("Error while loading resources!\n%s", resources->getErrorMsg());
			return -1;
		}

		// if (!(
		// 	resources->addJsonMapFile("data/bg1.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/map1.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/map2.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/map3.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/map4.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/bg2.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/bg3.json", "tiles1") &&
		// 	resources->addJsonMapFile("data/bg4.json", "tiles1")
		// ))
		// {
		// 	allegro_message ("Error while loading resources!\n%s", resources->getErrorMsg());
		// 	return -1;
		// }

		sfont = resources->getFont("DejaVuSans")->get(16);

		add(game, FLAG_SLEEP);
		initMenu();
		setFocus(mMain);

		game->init();
		startMusic();

		ALLEGRO_PATH *localAppData = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
		string cacheDir = al_path_cstr(localAppData, ALLEGRO_NATIVE_PATH_SEP);
		al_destroy_path(localAppData);

		std::string versionStr = version.version.get();
		// to prevent malicious manipulation //TODO: should be done in metrics module
		if (versionStr.length() > 50) versionStr = versionStr.substr(0, 50);

		metrics = Metrics::newInstance("tins22", versionStr);
		metrics->logSessionStart();
		return 0;
	}

	virtual void done () override
	{
		game->done();
		metrics->done();
	}

	virtual void onUpdate() override {
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

	void startMusic()
	{
		// MainLoop::getMainLoop()->playMusic(resources->getMusic("TODO"));
	}

	std::shared_ptr<SliderMenuItem> miSound;
	std::shared_ptr<SliderMenuItem> miMusic;
	std::shared_ptr<ActionMenuItem> miStart;

	MenuScreenPtr mMain;
	MenuScreenPtr mKeys;
	MenuScreenPtr mPause;

	void initMenu()
	{
		miSound = make_shared<SliderMenuItem>(&MainLoop::getMainLoop()->soundVolume, 
			"Sfx", "Press left or right to change sound volume");
		miSound->setEnabled(MainLoop::getMainLoop()->isSoundInstalled());

		miMusic = make_shared<SliderMenuItem>(&MainLoop::getMainLoop()->musicVolume, 
			"Music", "Press left or right to change music volume");
		miMusic->setEnabled(MainLoop::getMainLoop()->isSoundInstalled());

		miStart = make_shared<ActionMenuItem>(E_LEVEL_INTRO, "Start game", "");
		mMain = MenuBuilder(this, NULL)
			.push_back (miStart)
			.push_back (miSound)
			.push_back (miMusic)
			.push_back (make_shared<ActionMenuItem>(E_TOGGLE_FULLSCREEN, "Toggle Fullscreen", "Switch fullscreen / windowed mode"))
			.push_back (make_shared<ActionMenuItem>(E_QUIT, "Quit", ""))
			.build();
		mMain->add(ClearScreen::build(BLACK).get(), FLAG_BOTTOM);
		mMain->setMargin(160, 80);

		mMain->add(Text::build(GREY, ALLEGRO_ALIGN_RIGHT, string_format("v%s.%s", version.version.get(), version.buildDate.get()))
				.layout(Layout::RIGHT_BOTTOM_W_H, 4, 4, 200, 28).get());

		add(mMain);

		ALLEGRO_CONFIG *config = MainLoop::getMainLoop()->getConfig();
		mKeys = MenuBuilder(this, NULL)
			.push_back (make_shared<KeyMenuItem>("Up", config_keys[btnUp], getInput()[btnUp], config))
			.push_back (make_shared<KeyMenuItem>("Down", config_keys[btnDown], getInput()[btnDown], config))
			.push_back (make_shared<KeyMenuItem>("Left", config_keys[btnLeft], getInput()[btnLeft], config))
			.push_back (make_shared<KeyMenuItem>("Right", config_keys[btnRight], getInput()[btnRight], config))
			.push_back (make_shared<KeyMenuItem>("Action", config_keys[btnAction], getInput()[btnAction], config))
			.push_back (make_shared<ActionMenuItem>(E_TOGGLE_FULLSCREEN, "Toggle Fullscreen", "Switch fullscreen / windowed mode"))
			.push_back (make_shared<ActionMenuItem>(E_SHOW_MAIN_MENU, "Main Menu", "Return to the main menu"))
			.build();
		mKeys->setFont(resources->getFont("GP32")->get(40));
		mKeys->add(ClearScreen::build(BLACK).get(), FLAG_BOTTOM);

		mPause = MenuBuilder(this, NULL)
			.push_back (make_shared<ActionMenuItem>(E_ACTION, "Resume", "Resume game"))
			.push_back (make_shared<ActionMenuItem>(E_STOPGAME,
					"Exit to Main Menu", "Stop game and exit to main menu"))
			.build();
		mPause->setFont(resources->getFont("GP32")->get(40));
		mPause->add(ClearScreen::build(BLACK).get(), FLAG_BOTTOM);
	}

	virtual void handleMessage(ComponentPtr src, int code) override
	{
		switch (code)
		{
		case E_QUIT:
			metrics->logSessionClose();
			pushMsg(MSG_CLOSE);
			break;
		case E_TOGGLE_MUSIC: {
			bool enabled = MainLoop::getMainLoop()->isMusicOn();
			enabled = !enabled;
			MainLoop::getMainLoop()->musicVolume.set(enabled ? 1.0 : 0.0);
			if (enabled) startMusic();
		}
			break;

		case E_SHOW_SETTINGS_MENU:
			setFocus (mKeys);
			break;
		case E_ACTION:
			setFocus(game);
			break;
		case E_PAUSE:
			setFocus(mPause);
			break;
		case E_TOGGLE_FULLSCREEN:
			MainLoop::getMainLoop()->toggleWindowed();
			break;
		case E_SHOW_MAIN_MENU:
			setFocus(mMain);
			break;
		case E_STOPGAME:
			game->killAll();
			setFocus(mMain);
			break;
		case E_LEVEL_INTRO: {
				game->initLevel();
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(BLACK).get());
				intro->add(Text::buildf(WHITE, "LIFE %02i", game->getLives())
					.xy(getw() / 2, 130).get());
				intro->add(Text::build(WHITE, "GET READY!")
					.center().get());
				intro->setTimer (50, MSG_KILL);
				setFocus(intro);
				setTimer(50, EngineImpl::E_ACTION);
			}
			break;
		case E_LEVEL_CLEAR:
			{
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(BLACK).get());
				intro->add(Text::build(WHITE, "CONGRATULATIONS!")
					.xy(getw() / 2, 130).get());
				intro->add(Text::build(WHITE, "GAME COMPLETE!")
					.center().get());
				intro->setTimer (50, MSG_KILL);
				setFocus(intro);
				setTimer(50, EngineImpl::E_ACTION);
			}
			break;
		case E_SHOW_GAME_OVER:
			game->killAll();
			{
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(BLACK).get());
				intro->add(Text::build(WHITE, "GAME OVER").center().get());
				intro->setTimer(200, MSG_KILL);
				setFocus(intro);
				setTimer(200, E_SHOW_MAIN_MENU);
			}
			break;

		}
	}

	virtual Input* getInput() override {
		return settings.getInput();
	}

	virtual void playSample (const char *name) override {
		ALLEGRO_SAMPLE *sample = resources->getSampleIfExists(string(name));
		if (sample != NULL)
			MainLoop::getMainLoop()->playSample (sample);
		else
			log ("Could not play sample %s", name);
	}

};

shared_ptr<Engine> Engine::newInstance()
{
	return make_shared<EngineImpl>();
}
