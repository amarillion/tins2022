#ifndef ENGINE_H
#define ENGINE_H

#include <allegro5/allegro.h>
#include <list>
#include "input.h"
#include "settings.h"
#include "resources.h"
#include "menu.h"
#include "game.h"
#include "mainloop.h"
#include "component.h"
#include "sound.h"
#include "container.h"
#include <memory>
#include "updatechecker.h"
#include "postmortem.h"
#include "metrics.h"
#include "versionLoader.h"

class Engine : public Container, public Sound
{
private:
	std::shared_ptr<Resources> resources;
	Settings settings;
	
	bool debug;
	
	std::string gameover_message;
	
	std::list<Component*> layers;

	std::shared_ptr<Game> game;
	std::shared_ptr<PostMortem> postmortem;
	std::shared_ptr<UpdateChecker> updates;
	std::shared_ptr<Metrics> metrics;

	Input btnScreenshot;

	MenuScreenPtr mMain;
	MenuScreenPtr mKeys;
	MenuScreenPtr mPause;
	MenuScreenPtr mLevels;

	VersionLoader version;
#ifdef DEBUG
	Input btnAbort;
	Input btnDebugMode;
#endif
	int nextLevel;
	
	ComponentPtr initBackground();
public:	
	void logAchievement(const std::string &id) { metrics->logAchievement(id); }
	enum { E_NONE, E_MAINMENU, E_STARTGAME, E_PAUSE, E_RESUME, E_POSTMORTEM, E_POSTMORTEM_DEBUG, E_EXITSCREEN, E_QUIT,
		E_TWIRLOUT, MENU_CHOOSELEVEL, MENU_SETTINGS, E_TOGGLE_FULLSCREEN };

	Settings *getSettings() { return &settings; }
	std::shared_ptr<Resources> getResources () { return resources; }

//	virtual void handleEvent (ALLEGRO_EVENT &event) override;

	Engine ();
	virtual ~Engine() {}

	Game *getGame() { return game.get(); }
	int init(); // call once during startup
	void initMenu();
	void initIntro();
	
	virtual void handleMessage(ComponentPtr src, int code) override;
	virtual void onUpdate () override;
	
	bool isDebug () { return debug; }
	void setNextLevel (int _level) { nextLevel = _level; }

	int getCounter ();
		
	Input* getInput();
};

#endif
