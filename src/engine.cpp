#include <assert.h>
#include "engine.h"
#include "color.h"
#include "screenshot.h"
#include "motion.h"
#include "motionimpl.h"
#include "componentbuilder.h"
#include "CutScene.h"
#include "text.h"
#include "keymenuitem.h"
#include "menu.h"
#include <math.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "mainloop.h"
#include "DrawStrategy.h"
#include "menu.h"
#include "game.h"
#include "input.h"
#include "settings.h"
#include "resources.h"
#include "updatechecker.h"
#include "anim.h"

class EngineImpl : public Engine
{
private:
	Settings settings;
	shared_ptr<Resources> resources;
	bool debug;
	
	std::shared_ptr<Game> game;
	std::shared_ptr<UpdateChecker> updates;
	ALLEGRO_BITMAP *menu_texture[NUM_TEXTURES];

	MenuScreenPtr mMain;
	MenuScreenPtr mKeys;
	MenuScreenPtr mPause;
	MenuScreenPtr mPassCode;
	MenuScreenPtr mLevelSelect;

	LevelState state;
	Input btnScreenshot;
#ifdef DEBUG
	Input btnAbort;
	Input btnDebugMode;
#endif
	void initStartSequence(shared_ptr<CutScene> cutscene);
	void initEndSequence(shared_ptr<CutScene> cutsence);
public:

	EngineImpl ();
	virtual ~EngineImpl() {}

	Settings *getSettings() { return &settings; }
	virtual std::shared_ptr<Resources> getResources() override { return resources; }

	virtual int init() override; // call once during startup
	void initMenu();
	ALLEGRO_BITMAP *createTexture (const char *msg1, const char* msg2, ALLEGRO_COLOR color);

	virtual bool onHandleMessage(ComponentPtr src, int code) override;
	virtual void onUpdate () override;
	
	virtual void done() override; // stop music
	virtual bool isDebug () override { return debug; }

	int getCounter ();
		
	virtual void playSample (const char * name) override;

	virtual Input* getInput() override;
	virtual LevelState &getLevelState () override { return state; }
};

int EngineImpl::getCounter ()
{
	return MainLoop::getMainLoop()->getMsecCounter();
}

EngineImpl::EngineImpl() : settings(), resources(Resources::newInstance())
{
	debug = false;

	game = make_shared<Game>(this);
	
	btnScreenshot.setScancode (ALLEGRO_KEY_F12);

#ifdef DEBUG
	btnAbort.setScancode (ALLEGRO_KEY_F10);
	btnDebugMode.setScancode (ALLEGRO_KEY_F11);
#endif
}

Input* EngineImpl::getInput()
{
	return settings.getInput();
}

const int DIRNUM = 2;
const char *DIRECTIONS[DIRNUM] = { "left", "right" };

int EngineImpl::init()
{
	// return 0 on success, -1 on failure
	Anim::setDirectionModel (make_shared<DirectionModel>(DIRECTIONS, DIRNUM));

	settings.getFromConfig(MainLoop::getMainLoop()->getConfig());
	initKeyboard(); // install custom keyboard handler of Button.cpp

	if (!(
		resources->addFiles("data/*.ttf") &&
		resources->addFiles("data/*.png") &&
		resources->addFiles("data/*.tll") &&
		resources->addFiles("data/*.xml") &&
		resources->addFiles("data/*.ogg")
		))
	{
		allegro_message ("Error while loading resources!\n%s", resources->getErrorMsg());
		return -1;
	}

	if (!(
		resources->addJsonMapFile("data/bg1.json", "tiles1") &&
		resources->addJsonMapFile("data/map1.json", "tiles1") &&
		resources->addJsonMapFile("data/map2.json", "tiles1") &&
		resources->addJsonMapFile("data/map3.json", "tiles1") &&
		resources->addJsonMapFile("data/map4.json", "tiles1") &&
		resources->addJsonMapFile("data/bg2.json", "tiles1") &&
		resources->addJsonMapFile("data/bg3.json", "tiles1") &&
		resources->addJsonMapFile("data/bg4.json", "tiles1")
	))
	{
		allegro_message ("Error while loading resources!\n%s", resources->getErrorMsg());
		return -1;
	}

	setFont (resources->getFont("megaman_2")->get(16));
#ifdef DEBUG
	MainLoop::getMainLoop()->setFpsOn(true);
#endif

 	srand(time(0));
 	state.clear();

 	add (game, Container::FLAG_SLEEP);

 	initMenu();
 	game->init();

	ALLEGRO_PATH *localAppData = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	string cacheDir = al_path_cstr(localAppData, ALLEGRO_NATIVE_PATH_SEP);
	updates = UpdateChecker::newInstance(cacheDir, "0.1", E_QUIT);
 	add (updates, Container::FLAG_SLEEP);
 	updates->start_check_thread("laundry", "en");

 	setFocus(mMain);
 	return 0;
}

void EngineImpl::onUpdate ()
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

void EngineImpl::playSample (const char *name)
{
	ALLEGRO_SAMPLE *sample = resources->getSampleIfExists(string(name));
	if (sample != NULL)
		MainLoop::getMainLoop()->playSample (sample);
	else
		log ("Could not play sample %s", name);
}

void EngineImpl::done()
{
}

void EngineImpl::initStartSequence(shared_ptr<CutScene> cutscene)
{

	int ww = resources->getAnim("Open")->sizex;
	int hh = resources->getAnim("Open")->sizey;

	{
	ContainerPtr p1 = cutscene->newPage();
	p1->add( AnimComponent::build(resources->getAnim("Open")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());
	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"One day, we Find Fole and Raul\n"
			"sitting in their living room\n"
			"What shall we do tonight, Fole?\n"
			"The same we do every night, Raul\n"
			"Try to take over the world\n"
			"\n"
			"But it's raining...\n"
			"\n"
			"Ok, let's do the laundry then.\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p1->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 1")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 2")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 3")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"Hello, technical support?\n"
			"\n"
			"... Have you tried turning it off ...\n"
			"and then on again?'\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 4")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 5")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"Fole? \n"
			"\n"
			"Fole ???\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Laundry room 6")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"Meh...\n"
			"\n"
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}

}

void EngineImpl::initEndSequence(shared_ptr<CutScene> cutscene)
{
	int ww = resources->getAnim("Close")->sizex;
	int hh = resources->getAnim("Close")->sizey;

	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Close")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"My laundry machine accidentally my cat.\n"
			"The whole of it.\n"
			"\n"
			"Will it be OK?\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}
	{
	ContainerPtr p = cutscene->newPage();
	p->add( AnimComponent::build(resources->getAnim("Close")).layout(Layout::CENTER_TOP_W_H, 0, 0, ww, hh).get());

	auto t = Text::build(WHITE, ALLEGRO_ALIGN_CENTER,
			"\n"
			"\n"
			"Thank you for playing!\n"
			"\n"
			"Max and Amarillion\n"
			"\n"
			"\n"
			"\n"
			"\n").get();
	t->startTypewriter(Text::LINE_BY_LINE);
	t->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 320, 0, 0);
	p->add(t);
	}
}

ALLEGRO_BITMAP *EngineImpl::createTexture (const char *msg1, const char *msg2, ALLEGRO_COLOR color)
{
	const int TILESIZE = 181; // prime number
	const int TILESQRT = (int)(sqrt(2) * TILESIZE);

	ALLEGRO_BITMAP *texture = al_create_bitmap (TILESQRT, TILESQRT);
	assert (texture);
	ALLEGRO_BITMAP *temp = al_create_bitmap (TILESIZE, TILESIZE);
	assert (temp);

	ALLEGRO_COLOR c1 = WHITE;
	ALLEGRO_COLOR c2 = GREY;
	ALLEGRO_COLOR c3 = LIGHT_GREY;
	al_set_target_bitmap (temp);
	al_clear_to_color (c1);
	al_draw_filled_rectangle (0, 0, TILESIZE, TILESIZE / 2, c3);

	int fonth = al_get_font_line_height(sfont);
	al_draw_text (sfont, c2, 0, (TILESIZE - fonth) / 4, ALLEGRO_ALIGN_LEFT, msg1);
	al_draw_text (sfont, c3, 0, (TILESIZE - fonth) * 3 / 4, ALLEGRO_ALIGN_LEFT, msg2);

	unsigned char r, g, b;
	al_unmap_rgb(color, &r, &g, &b);
	ALLEGRO_COLOR c4 = al_map_rgba(r / 2, g / 2, b / 2, 128);
	al_draw_filled_rectangle (0, 0, TILESIZE, TILESIZE, c4);

	al_set_target_bitmap (texture);

	al_draw_rotated_bitmap(temp, 0, 0, 0, 0, M_PI / 4, 0);
	al_draw_rotated_bitmap(temp, 0, 0, TILESQRT / 2, -TILESQRT / 2, M_PI / 4, 0);
	al_draw_rotated_bitmap(temp, 0, 0, TILESQRT / 2, TILESQRT / 2, M_PI / 4, 0);
	al_draw_rotated_bitmap(temp, 0, 0, TILESQRT, 0, M_PI / 4, 0);

	al_destroy_bitmap (temp);

	return texture;
}

void EngineImpl::initMenu()
{
	assert (sfont);

	menu_texture[0] = createTexture ("TINS", "2012", GREEN);
	menu_texture[1] = createTexture ("CHOOSE", "LEVEL", CYAN);
	menu_texture[2] = createTexture ("SET", "KEYS", ORANGE);
	menu_texture[3] = createTexture ("PASS", "CODE", RED);
	menu_texture[4] = createTexture ("PAUSE", "PAUSE", MAGENTA);

	mMain = MenuBuilder(this, NULL)
		.push_back (make_shared<ActionMenuItem>(E_NEWGAME, "New game", "Start a new game"))
		.push_back (make_shared<ActionMenuItem>(E_SHOW_PASSCODE_MENU, "Enter pass", "Continue a game"))
		.push_back (make_shared<ActionMenuItem>(E_SHOW_SETTINGS_MENU, "Settings", "Configure keys"))
		.push_back (make_shared<ActionMenuItem>(E_SHOW_BYE_SCREEN, "Quit", "Exit game"))
		.build();
	mMain->setFont(resources->getFont("GP32")->get(40));
	mMain->add(Pattern::build (menu_texture[0]).linear(3, 1).get(), Container::FLAG_BOTTOM);

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
	mKeys->add(Pattern::build (menu_texture[2]).linear(1, 3).get(), Container::FLAG_BOTTOM);

	mPause = MenuBuilder(this, NULL)
		.push_back (make_shared<ActionMenuItem>(E_ACTION, "Resume", "Resume game"))
		.push_back (make_shared<ActionMenuItem>(E_STOPGAME,
				"Exit to Main Menu", "Stop game and exit to main menu"))
		.build();
	mPause->setFont(resources->getFont("GP32")->get(40));
	mPause->add(Pattern::build (menu_texture[4]).linear(0, 4).get(), Container::FLAG_BOTTOM);

	mPassCode = MenuBuilder(this, NULL)
		.push_back (make_shared<CodeMenuItem> (getLevelState(), 0), 2)
		.push_back (make_shared<CodeMenuItem> (getLevelState(), 1), 2)
		.push_back (make_shared<CodeMenuItem> (getLevelState(), 2), 2)
		.push_back (make_shared<CodeMenuItem> (getLevelState(), 3), 2)
		.push_back (make_shared<ActionMenuItem> (E_CODE_ENTERED, "Enter", "Start playing with this code"))
		.push_back (make_shared<ActionMenuItem> (E_SHOW_MAIN_MENU, "Main Menu", "Return to the main menu"))
		.build();
	mPassCode->setGroupLayout(2, CodeMenuItem::layoutFunction);

	using namespace std::placeholders;
	LayoutFunction adjustedLayout = std::bind (&MenuScreen::marginAdjustedlayout, 200, 0, _1, _2, _3, _4, _5);
	mPassCode->setGroupLayout(1, adjustedLayout);

	mPassCode->setFont(resources->getFont("GP32")->get(40));
	mPassCode->add(Pattern::build (menu_texture[3]).linear(-3, 1).get(), Container::FLAG_BOTTOM);

	Anim *anim = resources->getAnim("Laundry machine");
	mLevelSelect = MenuBuilder(this, NULL)
		.push_back (make_shared<LevelMenuItem> (getLevelState(), 0, anim), 2)
		.push_back (make_shared<LevelMenuItem> (getLevelState(), 1, anim), 2)
		.push_back (make_shared<LevelMenuItem> (getLevelState(), 2, anim), 2)
		.push_back (make_shared<LevelMenuItem> (getLevelState(), 3, anim), 2)
		.build();
	mLevelSelect->setFont(resources->getFont("GP32")->get(40));
	mLevelSelect->add(Pattern::build (menu_texture[1]).linear(-4, 0).get(), Container::FLAG_BOTTOM);
	mLevelSelect->setGroupLayout(2, LevelMenuItem::layoutFunction);
}

bool EngineImpl::onHandleMessage(ComponentPtr src, int code)
{
	switch (code)
	{
		case E_CODE_ENTERED:
			if (getLevelState().isCodeValid())
			{
				getLevelState().enterCode();
				setFocus(mLevelSelect);
			}
			else
			{
				playSample("Sound6");
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(RED).get());
				intro->add(Text::build(WHITE, "INVALID PASS CODE!")
					.center().get());
				intro->setTimer (50, MSG_KILL);

				setFocus(intro);

				setTimer(50, EngineImpl::E_SHOW_MAIN_MENU);
			}
			break;
		case E_SHOW_SETTINGS_MENU:
			setFocus (mKeys);
			break;
		case E_NEWGAME:
			game->initGame(); // reset game state
			{
				shared_ptr<CutScene> cutscene = make_shared<CutScene>(E_SHOW_CHOOSELEVEL_MENU);
				add (cutscene);
				cutscene->setFont(resources->getFont("megaman_2")->get(16));

				initStartSequence (cutscene);
				setFocus(cutscene);
			}
			break;
		case E_SHOW_CHOOSELEVEL_MENU:
			setFocus (mLevelSelect);
			break;
		case E_SHOW_PASSCODE_MENU:
			game->initGame();
			setFocus (mPassCode);
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
		case E_SHOW_BYE_SCREEN:
			setFocus(updates);
			break;
		case E_QUIT:
			pushMsg(MSG_CLOSE);
			break;
		case E_SHOW_MAIN_MENU:
			setFocus(mMain);
			break;
		case E_STOPGAME:
			game->killAll();
			setFocus(mMain);
			break;
		case E_LEVEL_INTRO:
			game->initLevel();
			{
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(BLACK).get());
				intro->add(Text::buildf(WHITE, "LEVEL %i", state.getSelectedLevel() + 1)
					.xy(getw() / 2, 50).get());
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
				state.setLevelCleared();

				if (state.allClear())
				{
					shared_ptr<CutScene> cutscene = make_shared<CutScene>(E_SHOW_MAIN_MENU);
					add (cutscene);
					cutscene->setFont(resources->getFont("megaman_2")->get(16));

					initEndSequence (cutscene);
					setFocus(cutscene);
				}
				else
				{
					ContainerPtr intro = make_shared<Container>();
					add(intro);
					intro->add(ClearScreen::build(BLACK).get());
					intro->add(Text::build(WHITE, "LEVEL CLEAR!")
						.center().get());
					intro->add(Text::build(WHITE, "NEW WEAPON\n\nUse DOWN button to change")
						.xy(getw() / 2, geth() * 3 / 4).get());
					intro->setTimer (100, MSG_KILL);
					setFocus(intro);

					setTimer(100, E_SHOW_CHOOSELEVEL_MENU);
				}
			}
			break;
		case E_SHOW_GAME_OVER:
			game->killAll();
			{
				ContainerPtr intro = make_shared<Container>();
				add(intro);
				intro->add(ClearScreen::build(BLACK).get());
				intro->add(Text::build(WHITE, "GAME OVER").center().get());
				int passcode = state.calculateCode();
				intro->add(Text::buildf(WHITE, "PASSCODE: %i", passcode)
					.xy(getw() / 2, 50).get());
				intro->setTimer (200, MSG_KILL);
				setFocus(intro);

				setTimer(200, E_SHOW_MAIN_MENU);
			}
			break;
	}
	return true;
}

shared_ptr<Engine> Engine::newInstance() {
	return make_shared<EngineImpl>();
}