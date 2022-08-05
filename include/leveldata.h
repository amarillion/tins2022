#ifndef LEVELDATA_H
#define LEVELDATA_H

class LevelData
{
	public:
		int map_sizex;
		int map_sizey;
		
		int targets;
		int enemy_tanks;
		int enemy_turrets;
		int enemy_sam;
		int enemy_bunker;
		int buildings;
		bool haschopper;
		bool hastank;
};

const int LEVEL_NUM = 4;
const int LEVEL_WON = -1;
const int LEVEL_GAMEOVER = -2;

const LevelData levels[LEVEL_NUM] =
{
	{	// level 1
		/* map_sizex */ 2048,
		/* map_sizey */ 2048,
		/* targets */ 1,
		/* enemy_tanks */ 1,
		/* enemy_turrets */ 20,
		/* enemy_sam */ 0,
		/* enemy_bunker */ 8,
		/* buildings */ 20,
		/* haschopper */ false,
		/* hastank */ true,
	},
	{	// level 2
		/* map_sizex */ 3072,
		/* map_sizey */ 3072,
		/* targets */ 1,
		/* enemy_tanks */ 2,
		/* enemy_turrets */ 20,
		/* enemy_sam */ 0, // no sam, because you don't have tank to help
		/* enemy_bunker */ 9,
		/* buildings */ 30,
		/* haschopper */ true,
		/* hastank */ false,
	},
	{
		/* map_sizex */ 4096,
		/* map_sizey */ 4096,
		/* targets */ 1,
		/* enemy_tanks */ 5,
		/* enemy_turrets */ 10,
		/* enemy_sam */ 10,
		/* enemy_bunker */ 10,
		/* buildings */ 40,
		/* haschopper */ true,
		/* hastank */ true,
	},
	{
		/* map_sizex */ 4096,
		/* map_sizey */ 4096,
		/* targets */ 1,
		/* enemy_tanks */ 8,
		/* enemy_turrets */ 20,
		/* enemy_sam */ 15,
		/* enemy_bunker */ 15,
		/* buildings */ 40,
		/* haschopper */ true,
		/* hastank */ true,
	},
};

#endif
