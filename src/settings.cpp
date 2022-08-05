#include <assert.h>
#include "settings.h"
#include <allegro5/allegro.h>
#include <string.h>
#include "util.h"

const char *config_keys[NUM_BUTTONS] = 
{
	"key_north", "key_south", "key_east", "key_west", "key_northeast", "key_southeast", "key_northwest", "key_southwest", "key_up", "key_down",
	"key_fire", "key_altfire", "key_switch"
};

Settings::Settings()
{
	button[btnN].setScancode (ALLEGRO_KEY_UP);
	button[btnN].setAltcode (ALLEGRO_KEY_PAD_8);
	button[btnS].setScancode (ALLEGRO_KEY_DOWN);
	button[btnS].setAltcode (ALLEGRO_KEY_PAD_2);
	button[btnE].setScancode (ALLEGRO_KEY_RIGHT);
	button[btnE].setAltcode (ALLEGRO_KEY_PAD_6);
	button[btnW].setScancode (ALLEGRO_KEY_LEFT);
	button[btnW].setAltcode (ALLEGRO_KEY_PAD_4);
	button[btnNE].setScancode (ALLEGRO_KEY_PAD_9);
	button[btnSE].setScancode (ALLEGRO_KEY_PAD_3);
	button[btnNW].setScancode (ALLEGRO_KEY_PAD_7);
	button[btnSW].setScancode (ALLEGRO_KEY_PAD_1);
	button[btnDown].setScancode (ALLEGRO_KEY_Z);
	button[btnUp].setScancode (ALLEGRO_KEY_A);
	button[btnSwitch].setScancode (ALLEGRO_KEY_TAB);
	button[btnFire].setScancode (ALLEGRO_KEY_SPACE);
	button[btnAltFire].setScancode (ALLEGRO_KEY_LSHIFT);
	button[btnAltFire].setAltcode (ALLEGRO_KEY_RSHIFT);
}

void Settings::getFromConfig(ALLEGRO_CONFIG *config)
{
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		button[i].setScancode (get_config_int (config, "twist", config_keys[i],
			button[i].getScancode()));
	}
}	
