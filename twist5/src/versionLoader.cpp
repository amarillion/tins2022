#include "versionLoader.h"

ALLEGRO_CONFIG *VersionLoader::getConfig() {
	if (!config) {
		config = al_load_config_file(this->configFile);
		if (!config) { config = al_create_config(); }
	}
	return config;
}