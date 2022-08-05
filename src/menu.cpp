#include "engine.h"
#include "menu.h"
#include "color.h"
#include "settings.h"
#include "game.h"
#include "keymenuitem.h"
#include "componentbuilder.h"
#include <math.h>
#include <allegro5/allegro_primitives.h>
#include "mainloop.h"
#include "anim.h"

using namespace std;

const int LEVELMENU_SIZE = 256;
const int LEVELMENU_FILLW = 198;

void CodeMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return;

	switch (event.keyboard.keycode)
	{
		case ALLEGRO_KEY_UP:
			code--;
			if (code < 0) code = 9;
			state.setDigit(pos, code);
			break;
		case ALLEGRO_KEY_DOWN:
			code ++;
			if (code > 9) code = 0;
			state.setDigit(pos, code);
			break;
		case ALLEGRO_KEY_LEFT:
			pushMsg(MenuItem::MENU_PREV);
			break;
		case ALLEGRO_KEY_RIGHT:
			pushMsg(MenuItem::MENU_NEXT);
			break;
		default:
			char ascii = event.keyboard.unichar & 0xFF;
			if (ascii >= '0' && ascii <= '9')
			{
				code = ascii - '0';
				state.setDigit(pos, code);
				pushMsg(MenuItem::MENU_NEXT);
			}
			break;
	}
}

void CodeMenuItem::draw(const GraphicsContext &gc)
{
	ALLEGRO_COLOR color = getColor();

	al_set_target_bitmap (gc.buffer);
	al_draw_filled_rectangle (x, y, x + w, y + h, color);
	char buf[13]; /* PRIME */
	snprintf (buf, sizeof(buf), "%i", code);
	int texth = al_get_font_line_height(sfont);
	int textw = al_get_text_width(sfont, buf);
	al_draw_text (sfont, BLACK, x + (w - textw) / 2, y + (h - texth) / 2, ALLEGRO_ALIGN_LEFT, buf);
}

const Rect CodeMenuItem::layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int count, const Rect &p)
{
	int block_size = 83; /* PRIME */
	double spacing = p.w() / count;
	return Rect(
		(idx * spacing) + ((spacing - block_size) / 2),
		MAIN_HEIGHT / 5,
		block_size,
		block_size
	);
}

LevelMenuItem::LevelMenuItem(LevelState &state, int level, Anim *anim) :
			state(state), level(level), anim(anim),
		animating(false), animStart(0), counter(0)
{
	w = LEVELMENU_SIZE;
	h = LEVELMENU_SIZE;
}

std::string LevelMenuItem::getText()
{
	char buf[256];
	snprintf (buf, sizeof(buf), "Level %i", level);
	return string(buf);
}

void LevelMenuItem::update()
{
	counter++;
	if (animating)
	{
		int timer = MSEC_FROM_TICKS(counter - animStart);
		if (timer > 1200)
		{
			animating = false;
			pushMsg(Engine::E_LEVEL_INTRO);
		}
	}
}

void LevelMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (!animating)
	{
		if (event.type != ALLEGRO_EVENT_KEY_DOWN) return;
		switch (event.keyboard.keycode)
		{
			case ALLEGRO_KEY_LEFT:
			case ALLEGRO_KEY_UP:
				pushMsg(MenuItem::MENU_PREV);
				break;
			case ALLEGRO_KEY_DOWN:
			case ALLEGRO_KEY_RIGHT:
				pushMsg(MenuItem::MENU_NEXT);
				break;
			case ALLEGRO_KEY_ENTER:
			case ALLEGRO_KEY_SPACE:
				state.selectLevel(level);
				animating = true;
				animStart = counter;
				break;
		}
	}
}

void LevelMenuItem::draw(const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);
	int timer = MSEC_FROM_TICKS(counter - animStart);
	// TODO: 20 is used to convert to msec. Check this.

	anim->drawFrame(0, 0,
			animating ? timer : 0 , x, y);

	ALLEGRO_COLOR color = getColor();

	// fill area must be different from clip area.
	int fill_x2 = x + LEVELMENU_FILLW;
	int fill_y2 = y + LEVELMENU_SIZE;

	if (state.isLevelCleared(level))
	{
		al_draw_filled_rectangle (x, y, fill_x2, fill_y2, al_map_rgba(0,0,255,120));
	}
	if (isSelected() && !animating && counter % 40 > 20)
	{
		al_draw_filled_rectangle (x, y, fill_x2, fill_y2, al_map_rgba(255,0,0,120));
	}
	if (!state.isLevelEnabled(level))
	{
		al_draw_filled_rectangle (x, y, fill_x2, fill_y2, al_map_rgba(128,128,128,200));
	}
//	solid_mode();
}

string LevelMenuItem::getHint()
{
	return std::string("");
}

bool LevelMenuItem::isEnabled()
{
	return state.isLevelEnabled(level);
}

const Rect LevelMenuItem::layoutFunction(ComponentPtr comp, ComponentPtr prev, int level, int size, const Rect &p)
{
	int my = level / 2;
	int mx = level % 2;

	double tx = (p.w() / 2);
	double ty = (p.h() / 2);

	return Rect(
		mx * tx + ((tx - comp->getw()) / 2),
		my * ty + ((ty - comp->geth()) / 2),
		comp->getw(),
		comp->geth()
	);

}
