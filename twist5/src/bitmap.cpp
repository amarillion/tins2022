#include "bitmap.h"

#include <allegro5/allegro.h>

std::shared_ptr<ALLEGRO_BITMAP> make_shared_bitmap (int w, int h)
{
	ALLEGRO_BITMAP *delegate = al_create_bitmap(w, h);
	return delegate ? std::shared_ptr<ALLEGRO_BITMAP>(delegate, al_destroy_bitmap) : std::shared_ptr<ALLEGRO_BITMAP>();
}
