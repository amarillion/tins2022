#ifndef ENGINE_H
#define ENGINE_H

#include <allegro5/allegro.h>
#include "input.h"
#include "settings.h"
#include <list>
#include "playercontrol.h"
#include "script.h"
#include "insults.h"
#include "component.h"
#include "mainloop.h"
#include "menubase.h"
#include <memory>
#include "updatechecker.h"

#define APPLICATION_VERSION "0.64"

const int DIRNUM = 8;
extern const char *DIRECTIONS[];
class Resources;

class Engine : public Container
{
private:
	MenuScreenPtr mMain;
	MenuScreenPtr mKeys;
	MenuScreenPtr mPause;
	ALLEGRO_FONT *menufont;

	bool debug;
	
	Settings settings;
	std::shared_ptr<Resources> resources;
	
	std::string gameover_message;
	
	std::shared_ptr<PlayerControl> playerControl;
	std::shared_ptr<Script> script;
	std::shared_ptr<UpdateChecker> updates;
	
	Input btnScreenshot;

#ifdef DEBUG
	Input btnAbort;
	Input btnDebugMode;
#endif
	
	ComponentPtr initMenuBackground();
	void initMenu();
public:	
	virtual void handleMessage (ComponentPtr src, int msg) override;

	enum { E_NONE, E_MAINMENU, E_STARTGAME, E_NEXTLEVEL, E_NEXTSCRIPT, E_PAUSE, E_RESUME, E_QUIT, MENU_SETTINGS, E_EXITSCREEN, E_TOGGLE_FULLSCREEN };

	Engine ();
	virtual ~Engine() {}
	std::shared_ptr<Resources> getResources() { return resources; }
	int init(); // call once during startup

	void initLevel(int level);
	void done();
	
	virtual void onUpdate () override;
	void spawn();
	
	bool isDebug () { return debug; }

	int getCounter ();
	int getMaxZOrder ();

	std::shared_ptr<PlayerControl> getPlayerControl() { return playerControl; }

	Input* getInput();
};

#endif
