#pragma once
#include "setting.h"

struct ALLEGRO_CONFIG;

class Constants
{
public:
	// allow overriding start weapon type for debugging
	Setting<int, Constants> playerStartWeaponType { this, "fole", "playerStartWeaponType", 0 };

	Constants() {}
	ALLEGRO_CONFIG *getConfig();

	// game options
//	bool fpsOn;
//	bool windowed;
	

private:	
//	bool soundOn;
//	bool music;
};

