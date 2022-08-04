#pragma once

#include "container.h"

class Resources;

class Engine : public Container
{
private:
public:
	virtual int init() = 0; // call once during startup
	virtual void done() = 0;
	virtual std::shared_ptr<Resources> getResources() = 0;
	static std::shared_ptr<Engine> newInstance();

	enum Msg { MSG_MAIN_MENU, 
		MSG_QUIT = 100, MSG_PLAY, 
		MSG_TOGGLE_WINDOWED, MSG_TOGGLE_MUSIC
	};

};
