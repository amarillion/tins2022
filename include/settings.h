#pragma once

#include "input.h"

const int NUM_BUTTONS = 5;
enum {	btnUp = 0, btnDown, btnLeft, btnRight, btnAction };

extern const char *config_keys[NUM_BUTTONS];

struct ALLEGRO_CONFIG;

class Settings
{
	
	Input button[NUM_BUTTONS];
public:
	
	// game options
	bool fpsOn;
	bool windowed;
	
	int numPlayers;
	
	Settings(); //  set defaults
	void getFromConfig(ALLEGRO_CONFIG *config);
	
	Input* getInput() { return button; }

private:	
	bool soundOn;
	bool music;
};
