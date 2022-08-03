#include "mouse.h"
#include "util.h"

void Cursor::Destroy() {
	if (data != nullptr) {
		al_destroy_mouse_cursor(data);
	}
}

void Cursor::Create(ALLEGRO_BITMAP *aBmp, int n) {
	if (data != nullptr) {
		al_destroy_mouse_cursor(data);
	}
	bmp = aBmp;
}


void Mouse::setCursor(Cursor &cursor) {
	if (!cursor.data) {
		if (cursor.bmp != nullptr) {
			cursor.data = al_create_mouse_cursor(cursor.bmp, cursor.xfocus, cursor.yfocus);
		}
	}

	Assert (cursor.data, "cursor was not initialized");
	al_set_mouse_cursor(al_get_current_display(), cursor.data);
}


