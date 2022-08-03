#include "keymenuitem.h"
#include <allegro5/allegro.h>
#include "util.h"

using namespace std;

void KeyMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR)
	{
		pushMsg (MenuItem::MENU_NONE);
		return;
	}

	if (waitForKey)
	{
		int c = event.keyboard.keycode;
		if (c != 0)
		{
			if (c == ALLEGRO_KEY_ESCAPE)
			{
			}
			else
			{
				btn.setScancode(c);
				set_config_int (config, "twist", btnConfigName, c);
			}
			waitForKey = false;
			flashing = false;
		}
	}
	else
	{
		switch (event.keyboard.keycode)
		{
			case ALLEGRO_KEY_LEFT:
			case ALLEGRO_KEY_UP:
				pushMsg (MenuItem::MENU_PREV); return;
				break;
			case ALLEGRO_KEY_DOWN:
			case ALLEGRO_KEY_RIGHT:
				pushMsg (MenuItem::MENU_NEXT); return;
				break;
			case ALLEGRO_KEY_ENTER:
			case ALLEGRO_KEY_SPACE:
				resetLastScancode();
				waitForKey = true;
				flashing = true;
				break;
		}
	}
}

string KeyMenuItem::getText()
{
	return string (btnName + " [" + btn.name() + "]");
}

string KeyMenuItem::getHint()
{
	if (waitForKey)
		return string ("press a new key or esc to cancel");
	else
		return string ("press enter to change key");
}

