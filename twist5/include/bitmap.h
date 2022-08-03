#pragma once

#include <memory>

struct ALLEGRO_BITMAP;

std::shared_ptr<ALLEGRO_BITMAP> make_shared_bitmap (int w, int h);
