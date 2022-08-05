#include <string>
#include <vector>
#include "color.h"
#include "menubase.h"

#ifndef MENU_H
#define MENU_H

class Engine;

class LevelMenuItem : public MenuItem
{
private:
	int index;
	bool isUnlocked();
	Engine *engine;
public:
	LevelMenuItem (Engine * _engine, int _index) : index (_index), engine (_engine) {}
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual std::string getText();
	virtual std::string getHint();
	virtual ALLEGRO_COLOR getColor() override;
};

#endif
