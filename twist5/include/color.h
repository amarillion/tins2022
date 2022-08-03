#pragma once

#define RED al_map_rgb(255, 0, 0)
#define LIGHT_BLUE al_map_rgb(128, 128, 255)
#define BLUE al_map_rgb(0, 0, 255)
#define DARK_BLUE al_map_rgb(0, 0, 192)
#define GREEN al_map_rgb(0, 255, 0)
#define DARK_GREEN al_map_rgb(0, 192, 0)
#define YELLOW al_map_rgb(255, 255, 0)
#define BLACK al_map_rgb(0, 0, 0)
#define WHITE al_map_rgb(255, 255, 255)
#define LIGHT_GREY al_map_rgb(192, 192, 192)
#define GREY al_map_rgb(128, 128, 128)
#define DARK_GREY al_map_rgb(64, 64, 64)
#define CYAN al_map_rgb(0, 255, 255)
#define TEAL al_map_rgb(0, 128, 128)
#define MAGENTA al_map_rgb(255, 0, 255)
// TODO: replace use of MAGIC_PINK with TRANSPARENT
#define MAGIC_PINK al_map_rgba(0, 0, 0, 0)
#define TRANSPARENT al_map_rgba(0, 0, 0, 0)
#define ORANGE al_map_rgb(255, 192, 0)
#define BROWN al_map_rgb(192, 168, 128)

struct ALLEGRO_COLOR;

// color generation functions
ALLEGRO_COLOR getRainbowColor(int index, int span);
