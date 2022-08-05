#include <assert.h>
#include "settings.h"
#include <allegro5/allegro.h>
#include <string.h>
#include "util.h"

using namespace std;

const char *config_keys[2][6] = 
{
	{"p1_key_left", "p1_key_right", "p1_key_down", "p1_key_up", "p1_key_action", "p1_key_alt"},	
	{"p2_key_left", "p2_key_right", "p2_key_down", "p2_key_up", "p2_key_action", "p2_key_alt"}
};

Settings::Settings()
{
	runMode = RM_NORMAL;
	numPlayers = 1;
	
	button[0][btnLeft].setScancode (ALLEGRO_KEY_LEFT);
	button[0][btnRight].setScancode (ALLEGRO_KEY_RIGHT);
	button[0][btnDown].setScancode (ALLEGRO_KEY_DOWN);
	button[0][btnUp].setScancode (ALLEGRO_KEY_UP);
	button[0][btnAction].setScancode (ALLEGRO_KEY_RCTRL);
	button[0][btnAlt].setScancode (ALLEGRO_KEY_RSHIFT);
	
	button[1][btnLeft].setScancode (ALLEGRO_KEY_A);
	button[1][btnRight].setScancode (ALLEGRO_KEY_D);
	button[1][btnDown].setScancode (ALLEGRO_KEY_S);
	button[1][btnUp].setScancode (ALLEGRO_KEY_W);
	button[1][btnAction].setScancode (ALLEGRO_KEY_LCTRL);
	button[1][btnAlt].setScancode (ALLEGRO_KEY_LSHIFT);
	// end of config defaults
}

void Settings::getFromConfig(ALLEGRO_CONFIG *config)
{
	numPlayers = get_config_int (config, "fole1", "numplayers", numPlayers);
	
	for (int i = 0; i < 6; ++i)
	{
		button[0][i].setScancode (get_config_int (config, "fole1", config_keys[0][i],
			button[0][i].getScancode()));
		button[1][i].setScancode (get_config_int (config, "fole1", config_keys[1][i],
			button[1][i].getScancode()));
	}
}	

void Settings::getFromArgs(vector<string> &opts)
{
	vector<string>::iterator i;
	// parse command line arguments
	for (i = opts.begin(); i != opts.end(); ++i)
	{
		if (*i == "-smoketest")
		{
			runMode = RM_SMOKETEST;
		}
		else if (*i == "-testview")
		{
			runMode = RM_TESTVIEW;
		}
		else
		{
			allegro_message (
				"warning: Couldn't parse a command line argument %s",		
				i->c_str());
		}			
	}
}
