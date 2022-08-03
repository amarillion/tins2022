#pragma once

struct ALLEGRO_FONT;
struct ALLEGRO_COLOR;

void draw_textf_with_background(const ALLEGRO_FONT *font, ALLEGRO_COLOR text_color, ALLEGRO_COLOR background_color, float x, float y, int alignment, const char *text, ...);
void draw_text_with_background(const ALLEGRO_FONT *font, ALLEGRO_COLOR text_color, ALLEGRO_COLOR background_color, float x, float y, int alignment, const char *text);

void draw_shaded_textf(const ALLEGRO_FONT *font, ALLEGRO_COLOR text_color, ALLEGRO_COLOR shade_color, float x, float y, int alignment, const char *text, ...);
void draw_shaded_text(const ALLEGRO_FONT *font, ALLEGRO_COLOR text_color, ALLEGRO_COLOR shade_color, float x, float y, int alignment, const char *text);

void draw_text_with_underline(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int alignment, const char *text);
