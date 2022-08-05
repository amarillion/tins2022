#ifndef SETTINGS_H
#define SETTINGS_H

#include "input.h"
#include <vector>
#include <string>

enum {	btnLeft = 0, btnRight, btnDown, btnUp, btnAction, btnAlt};

struct ALLEGRO_CONFIG;

extern const char *config_keys[2][6];

class Settings
{
public:	
	int numPlayers;
	
	enum RunMode { RM_NORMAL, RM_TESTVIEW, RM_SMOKETEST };
	RunMode runMode;
	
	Input button[2][6];
	
	Settings(); //  set defaults
	void getFromConfig(ALLEGRO_CONFIG *config);
	void getFromArgs(std::vector<std::string> &opts);
	
	Input* getInput(int p) { return button[p]; }
};

#endif
