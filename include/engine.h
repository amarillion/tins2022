#ifndef ENGINE_H
#define ENGINE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <list>
#include "container.h"

using namespace std;

class Main;

class LevelState
{
	int code[4];
	bool levelCleared[4];
	bool levelEnabled[4];
	int selectedLevel;
public:
	void clear()
	{
		for (int i = 0; i < 4; ++i)
		{
			levelCleared[i] = false;
			levelEnabled[i] = true;
		}
		// disable last level
		levelEnabled[3] = false;
		calculateCode();
		selectedLevel = 0;
	}

	int getDigit(int i) { return code[i]; }
	void setDigit(int i, int val) { code[i] = val; }

	bool allClear()
	{
		return levelCleared[0] &
				levelCleared[1] &
				levelCleared[2] &
				levelCleared[3];
	}

	bool isCodeValid()
	{
		bool valid = true;
		int value = valueFromDigits();

		valid &= (((value % 2) == 0) ^ ((value % 3 == 0)));
		valid &= (((value % 5) == 0) ^ ((value % 7 == 0)));
		valid &= (((value % 13) == 0) ^ ((value % 11 == 0)));
		valid &= (((value % 17) == 0) ^ ((value % 19 == 0)));

		return valid;
	}

	int valueFromDigits ()
	{
		int value = 0;
		for (int i = 0; i < 4; ++i)
		{
			value *= 10;
			value += code[i];
		}
		return value;
	}

	void enterCode()
	{
		int value = valueFromDigits();
		bool temp[4];

		if ((value % 2) == 0) temp[0] = false;
		else if ((value % 3) == 0) temp[0] = true;
		else return; // invalid code
		if ((value % 5) == 0) temp[1] = false;
		else if ((value % 7) == 0) temp[1] = true;
		else return; // invalid code
		if ((value % 13) == 0) temp[2] = false;
		else if ((value % 11) == 0) temp[2] = true;
		else return; // invalid code
		if ((value % 17) == 0) temp[3] = false;
		else if ((value % 19) == 0) temp[3] = true;
		else return; // invalid code

		for (int i = 0; i < 4; ++i)
			levelCleared[i] = temp[i];
		if (levelCleared[0] & levelCleared[1] & levelCleared[2])
			levelEnabled[3] = true;
	}

	bool isLevelCleared(int l) { return levelCleared[l]; }
	bool isLevelEnabled(int l) { return levelEnabled[l]; }
	void setLevelCleared()
	{
		levelCleared[selectedLevel] = true;
		if (levelCleared[0] & levelCleared[1] & levelCleared[2])
			levelEnabled[3] = true;
	}

	int calculateCode()
	{
		int result =
				(levelCleared[0] ? 3 : 2) *
				(levelCleared[1] ? 7 : 5) *
				(levelCleared[2] ? 11 : 13) *
				(levelCleared[3] ? 19 : 17);

		int temp = result;
		for (int i = 3; i >= 0; --i)
		{
			code[i] = temp % 10;
			temp /= 10;
		}
		return result;
	}

	void selectLevel (int level)
	{
		selectedLevel = level;
	}

	int getSelectedLevel()
	{
		return selectedLevel;
	}

};

class CutScene;
class Input;
class Resources;

class Engine : public Container {
public:
	enum {
		E_NONE,
		E_SHOW_MAIN_MENU,
		E_BACK,
		E_LEVEL_INTRO,
		E_LEVEL_CLEAR,
		E_STOPGAME, /* When you press stop from the pause menu */
		E_CODE_ENTERED, /* When user entered code in passcode menu */
		E_SHOW_SETTINGS_MENU,
		E_SHOW_GAME_OVER /* show game over message */,
		E_ACTION, /* start or resume the game action */
		E_PAUSE,
		E_SHOW_BYE_SCREEN,
		E_TOGGLE_FULLSCREEN,
		E_QUIT,
		E_NEWGAME,
		E_SHOW_PASSCODE_MENU,
		E_SHOW_CHOOSELEVEL_MENU, /* show the choose-level menu */
	};

	virtual bool isDebug() = 0;
	virtual void playSample (const char * name) = 0;
	virtual LevelState &getLevelState () = 0;
	virtual Input* getInput() = 0;
	virtual std::shared_ptr<Resources> getResources() = 0;
	virtual int init() = 0;
	virtual void done() = 0;

	static std::shared_ptr<Engine> newInstance();
	virtual ~Engine() {}
};

#endif
