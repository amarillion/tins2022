#include <assert.h>
#include "view.h"
#include "game.h"

void View::init (int numPlayers, int player, int pw, int ph)
{
	int HALF_W = pw / 2;

	const int defaults[3][6] =
	{
		{ 64, 64,          pw - 128, ph - 128,                64, 0 },
		{ 0, 0,            HALF_W - 8, 320,                   0, 320 },
		{ HALF_W + 8, 160, HALF_W - 8, 320,                   HALF_W, 0 }
	};
	camera_x = 0;
	camera_y = 0;
	
	int i = (numPlayers == 1 ? 0 : player + 1);
	
	x = defaults[i][0];
	y = defaults[i][1];
	w = defaults[i][2];
	h = defaults[i][3];
	status_x = defaults[i][4];
	status_y = defaults[i][5];
}

void View::update() {
	// update camera
	int newx, newy;
	newx = player->getx() - (w / 2);
	newy = player->gety() - (h / 2);
	camera_x = newx;
	camera_y = newy;
}

void View::draw(const GraphicsContext &gc) {
	TEG_MAP *map = player->getMap();
	teg_partdraw (map, 0,
			x, y, w, h,
			camera_x - x, camera_y - y);
	teg_partdraw (map, 1,
			x, y, w, h,
			camera_x - x, camera_y - y);
	GraphicsContext gc2;
	gc2.buffer = gc.buffer;
	gc2.xofst = camera_x - x;
	gc2.yofst = camera_y - y;
	parent->getObjects()->draw(gc2, player->getRoom(), x, y, w, h);
}