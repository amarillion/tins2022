#include "menu.h"
#include "color.h"
#include "engine.h"
#include "settings.h"
#include "level.h"
#include "game.h"
#include "keymenuitem.h"

using namespace std;

void LevelMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return;

	switch (event.keyboard.keycode)
	{
		case ALLEGRO_KEY_LEFT:
		case ALLEGRO_KEY_UP:
			pushMsg (MenuItem::MENU_PREV);
			break;
		case ALLEGRO_KEY_DOWN:
		case ALLEGRO_KEY_RIGHT:
			pushMsg (MenuItem::MENU_NEXT);
			break;
		case ALLEGRO_KEY_ENTER:
		case ALLEGRO_KEY_SPACE:
			if (isUnlocked())
			{
				engine->setNextLevel(index);
				pushMsg(Engine::E_STARTGAME);
			}
			break;
	}
}

bool LevelMenuItem::isUnlocked()
{
	return 
		engine->getGame()->getTotalStars() >= levelData[index].starsreq;
}

string LevelMenuItem::getText()
{
	string result = levelData[index].title;
	if (engine->getGame()->getStarsPerLevel(index) > 0) result += " *";
	if (engine->getGame()->getStarsPerLevel(index) == 2) result += "*";
	return result;
}

string LevelMenuItem::getHint()
{
	char buf[256];
	if (isUnlocked())
	{
		snprintf (buf, sizeof(buf), "unlocked - hiscore %06i", 
			engine->getGame()->hiscore_per_level[index]);
	}
	else
	{
		snprintf (buf, sizeof(buf), "%i more stars required - hiscore %06i", 
			levelData[index].starsreq - engine->getGame()->getTotalStars(),
			engine->getGame()->hiscore_per_level[index]);
	}
	return string (buf);
}

ALLEGRO_COLOR LevelMenuItem::getColor() {
	
	if (isSelected()) { return WHITE; }

	if (isUnlocked())
	{
		if (engine->getGame()->hiscore_per_level[index]
			>= levelData[index].scoreToBeat)
			return GREEN;
		else
			return YELLOW;
	}
	else return GREY;

}

