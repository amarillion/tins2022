#include <assert.h>
#include "settings.h"
#include <allegro5/allegro.h>
#include <string.h>
#include "util.h"

const char *config_keys[NUM_BUTTONS] = 
{
	"key_up", "key_down", "key_left", "key_right", "key_action"
};

Settings::Settings()
{
	button[btnUp].setScancode (ALLEGRO_KEY_UP);
	button[btnDown].setScancode (ALLEGRO_KEY_DOWN);
	button[btnLeft].setScancode (ALLEGRO_KEY_LEFT);
	button[btnRight].setScancode (ALLEGRO_KEY_RIGHT);
	button[btnAction].setScancode (ALLEGRO_KEY_SPACE);
	button[btnAction].setAltcode (ALLEGRO_KEY_ENTER);
}

void Settings::getFromConfig(ALLEGRO_CONFIG *config)
{
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		button[i].setScancode (get_config_int (config, "twist", config_keys[i],
			button[i].getScancode()));
	}
}	
