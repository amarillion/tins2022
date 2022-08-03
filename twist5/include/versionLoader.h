#pragma once

#include "setting.h"

struct ALLEGRO_CONFIG;

class VersionLoader
{
public:
	Setting<const char*, VersionLoader> version { this, "version", "version", "unversioned" };
	Setting<const char*, VersionLoader> buildDate { this, "version", "builddate", "" };
	Setting<const char*, VersionLoader> gitHash { this, "version", "githash", "" };

	VersionLoader(const char* configFile) : configFile(configFile), config(nullptr) {}
	ALLEGRO_CONFIG *getConfig();

private:
	const char *configFile;
	ALLEGRO_CONFIG *config;
};

