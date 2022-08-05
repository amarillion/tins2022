#ifndef SETTINGS_H
#define SETTINGS_H

#include "input.h"

const int NUM_BUTTONS = 5;
enum {	btnUp = 0, btnDown, btnLeft, btnRight, btnAction };

extern const char *config_keys[NUM_BUTTONS];

class Sound;
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
	void getFromArgs(int argc, const char *const *argv);
	
	Input* getInput() { return button; }
};

#endif
