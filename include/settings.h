#ifndef SETTINGS_H
#define SETTINGS_H

#include "input.h"

const int NUM_BUTTONS = 13;
enum {	btnN = 0, btnS, btnE, btnW, btnNE, btnSE, btnNW, btnSW, btnUp, btnDown, btnFire, btnAltFire, btnSwitch };

extern const char *config_keys[NUM_BUTTONS];

struct ALLEGRO_CONFIG;

class Settings
{
	Input button[NUM_BUTTONS];
public:

	Settings(); //  set defaults
	void getFromConfig(ALLEGRO_CONFIG *config);
	
	Input* getInput() { return button; }

private:	
	bool soundOn;
	bool music;
};

#endif
