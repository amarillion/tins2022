#ifndef ENGINE_H
#define ENGINE_H

#include "container.h"
#include <memory>

class Input;

class Engine : public Container
{
public:	
	// menu states
	enum { E_NONE, E_START_OR_RESUME, E_NEXT_LEVEL, 
		E_EXITSCREEN,
		MENU_PLAYER_NUM, MENU_KEYS_1, MENU_KEYS_2, E_QUIT, MENU_MAIN, COVER,
		GS_PLAY, MENU_PAUSE, GS_DONE, GS_GAME_OVER, E_TOGGLE_FULLSCREEN};
	
	virtual ~Engine() {}	
	virtual int init() = 0; // call once during startup
		
	virtual Input* getInput(int p) = 0;
	virtual bool isDebug() = 0;

	static std::shared_ptr<Engine> newInstance();
	virtual void logAchievement(const std::string &achievement) = 0;
};

#endif
