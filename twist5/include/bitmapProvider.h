#pragma once

#include <string>
struct ALLEGRO_BITMAP;

class IBitmapProvider
{
public:
	   virtual ALLEGRO_BITMAP *getBitmap (const std::string &id) = 0;
	   virtual void putBitmap(const std::string &id, ALLEGRO_BITMAP *bmp) = 0;
};