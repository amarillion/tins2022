#pragma once

#include "container.h"

class Resources;
class Input;

class Engine : public Container
{
private:
public:
	virtual int init() = 0; // call once during startup
	virtual void done() = 0;
	virtual std::shared_ptr<Resources> getResources() = 0;
	static std::shared_ptr<Engine> newInstance();
	virtual bool isDebug() = 0;
	virtual Input* getInput() = 0;
	virtual void playSample (const char *name) = 0;

	enum {
		E_SHOW_MAIN_MENU = 100,
		E_BACK,
		E_LEVEL_INTRO,
		E_SHOW_WIN_SCREEN,
		E_ENTER_MAP, /* switch maps, which must be called from outside game loop */
		E_STOPGAME, /* When you press stop from the pause menu */
		E_CODE_ENTERED, /* When user entered code in passcode menu */
		E_SHOW_SETTINGS_MENU,
		E_SHOW_GAME_OVER /* show game over message */,
		E_ACTION, /* start or resume the game action */
		E_PAUSE,
		E_TOGGLE_FULLSCREEN,
		E_TOGGLE_MUSIC,
		E_QUIT,
		E_NEWGAME
	};


};
