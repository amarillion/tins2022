#ifndef LEVEL_H
#define LEVEL_H

#include "tilemap.h"

struct TEG_MAP;
class LevelData
{
	public:
		const char *filename;
		const char *tilesname;
		const char *uid;
		const char *title;
		int starsreq;
		int time;
		int scoreToBeat;
		Tilemap *map;
};

const int LEVEL_NUM = 11;
extern LevelData levelData[LEVEL_NUM];

#endif
