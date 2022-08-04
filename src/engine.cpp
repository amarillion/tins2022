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

using namespace std;

const int DIRNUM = 4;
const char *DIRECTIONS[DIRNUM] = { "n", "e", "s", "w" };

class EngineImpl : public Engine
{
private:
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

	virtual int init() override
	{
		Anim::setDirectionModel (make_shared<DirectionModel>(DIRECTIONS, DIRNUM));
		srand(time(0));

		if (!(
			resources->addFiles("data/*.ttf")
			))
		{
			allegro_message ("Error while loading resources!\n%s", resources->getErrorMsg());
			return 1;
		}

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
			pushMsg(MSG_QUIT);
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

	void initMenu()
	{
		miSound = make_shared<SliderMenuItem>(&MainLoop::getMainLoop()->soundVolume, 
			"Sfx", "Press left or right to change sound volume");
		miSound->setEnabled(MainLoop::getMainLoop()->isSoundInstalled());

		miMusic = make_shared<SliderMenuItem>(&MainLoop::getMainLoop()->musicVolume, 
			"Music", "Press left or right to change music volume");
		miMusic->setEnabled(MainLoop::getMainLoop()->isSoundInstalled());


		miStart = make_shared<ActionMenuItem>(MSG_PLAY, "Start game", "");
		mMain = MenuBuilder(this, NULL)
			.push_back (miStart)
			.push_back (miSound)
			.push_back (miMusic)
			.push_back (make_shared<ActionMenuItem>(MSG_TOGGLE_WINDOWED, "Toggle Fullscreen", "Switch fullscreen / windowed mode"))
			.push_back (make_shared<ActionMenuItem>(MSG_QUIT, "Quit", ""))
			.build();
		mMain->add(ClearScreen::build(BLACK).get(), FLAG_BOTTOM);
		mMain->setMargin(160, 80);

		mMain->add(Text::build(GREY, ALLEGRO_ALIGN_RIGHT, string_format("v%s.%s", version.version.get(), version.buildDate.get()))
				.layout(Layout::RIGHT_BOTTOM_W_H, 4, 4, 200, 28).get());

		add(mMain);
	}

	virtual void handleMessage(ComponentPtr src, int code) override
	{
		switch (code)
		{
		case MSG_MAIN_MENU:
			setFocus(mMain);
			break;
		case MSG_QUIT:
			metrics->logSessionClose();
			pushMsg(MSG_CLOSE);
			break;
		case MSG_PLAY:
			setFocus(game);
			break;
		case MSG_TOGGLE_WINDOWED:
			MainLoop::getMainLoop()->toggleWindowed();
			break;
		case MSG_TOGGLE_MUSIC:
		{
			bool enabled = MainLoop::getMainLoop()->isMusicOn();
			enabled = !enabled;
			MainLoop::getMainLoop()->musicVolume.set(enabled ? 1.0 : 0.0);
			if (enabled) startMusic();
		}
			break;
		}
	}
};

shared_ptr<Engine> Engine::newInstance()
{
	return make_shared<EngineImpl>();
}
