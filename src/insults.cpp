#include <allegro5/allegro.h>
#include "insults.h"
#include "engine.h"
#include "color.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "util.h"
#include "textstyle.h"
#include "resources.h"

const char *insults[INSULT_NUM] =
{
	"Christmas is doomed. I'll keep all the gifts for myself!",
	"I'm Dr. Evil F. Fear me!",
	"Is that all you got? Come on, put some effort in it!",
	"Your mother was a hamster and your father smelt of elderberry!",
	"Go away or I shall taunt you a second time!",
	"I'll get you even if it is the last thing I'll ever do!",
	"Is that all you got? You silly monkey!",
};

Insults::Insults (Engine &e) : parent(e)
{
	tInsult = MIN_INSULT_TIME + rand() % INSULT_VARIATION_TIME;
	active = false;
	insult = NULL;
	pos = 0;
}

void Insults::draw (const GraphicsContext &gc)
{
	al_set_target_bitmap (gc.buffer);
	if (pos > 0)
	{
		ALLEGRO_BITMAP *current = fole2;
		al_draw_bitmap (current, MAIN_WIDTH - pos, MAIN_HEIGHT - al_get_bitmap_height(current), 0);
		
		if (pos == al_get_bitmap_width(current))
		{
			draw_text_with_background(font, BLACK, WHITE, MAIN_WIDTH / 2, MAIN_HEIGHT - 16, ALLEGRO_ALIGN_CENTER, insult);
		}
	}
}

void Insults::update ()
{
	tInsult--;
	
	if (active)
	{
		pos += 3;
		int FOLE_W = al_get_bitmap_width(fole1);
		if (pos > FOLE_W) pos = FOLE_W;
	}
	if (!active)
	{
		pos -= 3;
		if (pos < 0) pos = 0;
	}
	
	if (tInsult <= 0)
	{
		active = !active;
		if (active)
		{
			tInsult = INSULT_DISPLAY_TIME;
			int item;
			do
			{
				item = rand() % INSULT_NUM;
			} while (insults[item] == insult); // prevent same one twice in a row
			
			insult = insults[item];
			pos = 0;
		}
		else
		{
			tInsult = MIN_INSULT_TIME + rand() % INSULT_VARIATION_TIME;
		}
	}
}

void Insults::init ()
{
	fole1 = parent.getResources()->getBitmap("FoleTrans1a");
	fole2 = parent.getResources()->getBitmap("FoleTrans2a");
	font = parent.getResources()->getFont("a4_font")->get();
}
