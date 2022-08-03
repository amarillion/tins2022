/*
 * DrawStrategy.cpp
 *
 *  Created on: 3 Aug 2012
 *      Author: martijn
 */

#include "DrawStrategy.h"
#include "color.h"
#include <allegro5/allegro.h>
#include "component.h"
#include <tegel5.h>
#include "mainloop.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "anim.h"
#include "componentbuilder.h"

using namespace std;

ComponentBuilder<ClearScreen> ClearScreen::build(ALLEGRO_COLOR color)
{
	return ComponentBuilder<ClearScreen>(make_shared<ClearScreen>(color));
}

void ClearScreen::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);
	al_clear_to_color(color);
}

ComponentBuilder<Pattern> Pattern::build(ALLEGRO_BITMAP *bmp)
{
	return ComponentBuilder<Pattern>(make_shared<Pattern>(bmp));
}

ComponentBuilder<Pattern> Pattern::build(shared_ptr<ALLEGRO_BITMAP> bmp)
{
	return ComponentBuilder<Pattern>(make_shared<Pattern>(bmp));
}

void Pattern::draw(const GraphicsContext &gc)
{
	int xofst = gc.xofst;
	int yofst = gc.yofst;

	if (motion)
	{
		xofst = motion->getdx(counter);
		yofst = motion->getdy(counter);
	}

	int TILEW = al_get_bitmap_width(texture);
	int TILEH = al_get_bitmap_height(texture);

	al_set_target_bitmap (gc.buffer);

	for (int x = (xofst % TILEW) - TILEW; x < al_get_bitmap_width(gc.buffer) + TILEW; x += TILEW)
	{
		for (int y = (yofst % TILEH) - TILEH; y < al_get_bitmap_height(gc.buffer) + TILEH; y += TILEH)
		{
			al_draw_bitmap (texture, x, y, 0);
		}
	}
}

TileMap::TileMap(const TEG_MAP *tilemap)
{
	this->tilemap = tilemap;
	setDimension(teg_pixelw(tilemap), teg_pixelh(tilemap));
}

void TileMap::draw(const GraphicsContext &gc)
{
	int xofst = 0;
	int yofst = 0;

	if (motion)
	{
		xofst = motion->getdx(counter);
		yofst = motion->getdy(counter);
	}

	xofst += gc.xofst;
	yofst += gc.yofst;

	// TODO: right now, frame interval is hardcoded at 500 msec. Make customizable.
	int frame = (MSEC_FROM_TICKS(counter) / 500) % tilemap->tilelist->animsteps;
//	teg_draw_frame (tilemap, 0, xofst, yofst, frame);
	teg_draw_repeated(gc.buffer, tilemap, 0, -xofst, -yofst, frame);
}

ComponentBuilder<TileMap> TileMap::build(TEG_MAP *map)
{
	return ComponentBuilder<TileMap>(make_shared<TileMap>(map));
}

void BitmapComp::draw (const GraphicsContext &gc)
{
	int xofst = 0;
	int yofst = 0;

	xofst = getx() + (motion ? motion->getdx(counter) : 0);
	yofst = gety() + (motion ? motion->getdy(counter) : 0);

	xofst += gc.xofst;
	yofst += gc.yofst;

	if (isZoomed)
	{

		int origw = al_get_bitmap_width(rle);
		int origh = al_get_bitmap_height(rle);

		int neww = origw * zoom;
		int newh = origh * zoom;

		double invzoom = 1.0 - zoom;

		al_draw_scaled_bitmap(rle,
			0, 0, origw, origh,
			xofst + (invzoom * hx), yofst + (invzoom * hy), neww, newh, 0);
	}
	else
	{
		al_set_target_bitmap (gc.buffer);
		al_draw_bitmap(rle, xofst, yofst, 0);
	}

}

ComponentBuilder<BitmapComp> BitmapComp::build(shared_ptr<ALLEGRO_BITMAP> rle)
{
	return ComponentBuilder<BitmapComp>(make_shared<BitmapComp>(rle));
}

ComponentBuilder<BitmapComp> BitmapComp::build(ALLEGRO_BITMAP *_rle)
{
	return ComponentBuilder<BitmapComp>(make_shared<BitmapComp>(_rle));
}


ComponentBuilder<AnimComponent> AnimComponent::build(Anim *anim)
{
	return ComponentBuilder<AnimComponent>(make_shared<AnimComponent>(anim));
}

AnimComponent::AnimComponent (Anim *anim) : exitCode(0), anim(anim), dir(0), state(0) {}

void AnimComponent::draw(const GraphicsContext &gc)
{
	anim->drawFrame(state, dir, (counter * 20), getx(), gety());
}
