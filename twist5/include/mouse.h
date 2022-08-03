#pragma once

#include <allegro5/allegro.h>

class Mouse;

// MAS wrapper class
class Cursor // wrapper for mouse cursor
{
private:
	ALLEGRO_MOUSE_CURSOR *data = nullptr;
	int xfocus = 0;
	int yfocus = 0;
	ALLEGRO_BITMAP *bmp = nullptr;
	friend class Mouse;
public:
	void Create(ALLEGRO_BITMAP *bmp = nullptr, int n = 1);
	void SetAnimationInterval(int interval) { /* TODO */ };
	void Destroy();
	void SetFocus(int, int) { /* TODO */ }
	static void ReadSkinData(ALLEGRO_CONFIG *config) { /* TODO */ }

	~Cursor() {
		Destroy();
	}

};

class Mouse {
public:
	static void setCursor(Cursor &cursor);
};

