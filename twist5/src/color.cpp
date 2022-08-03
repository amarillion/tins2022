#include "color.h"
#include <allegro5/allegro_color.h>

ALLEGRO_COLOR getRainbowColor(int index, int span)
{
	float r;
	float g;
	float b;
	al_color_hsv_to_rgb (index * 360 / span, 1, 1, &r, &g, &b);
	return al_map_rgb_f (r, g, b);
}
