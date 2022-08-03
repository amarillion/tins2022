#pragma once

struct ALLEGRO_BITMAP;

struct GraphicsContext
{
	ALLEGRO_BITMAP *buffer;
	int xofst;
	int yofst;
};
