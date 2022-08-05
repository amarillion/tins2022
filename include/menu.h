#include "color.h"
#include "menubase.h"

#ifndef MENU_H
#define MENU_H

struct ALLEGRO_FONT;

class Menu;
class Engine;

const int NUM_TEXTURES = 5;

class LevelState;

class CodeMenuItem : public MenuItem
{
	int code;
	LevelState &state;
	int pos;
public:
	CodeMenuItem (LevelState &state, int pos) : code(0), state(state), pos(pos) {}
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual std::string getHint() { return std::string("Enter code"); }
	virtual std::string getText() { return ""; }
	virtual void draw(const GraphicsContext &gc) override;

	static const Rect layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p);
};

class Anim;

class LevelMenuItem : public MenuItem
{
	LevelState &state;
	int level;
	Anim *anim;
	bool animating;
	int animStart;
	int counter;
public:
	LevelMenuItem (LevelState &state, int level, Anim *anim);

	virtual void update() override;
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual std::string getText();
	virtual std::string getHint();
	virtual bool isEnabled();
	virtual void draw(const GraphicsContext &gc);

	static const Rect layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p);
};

#endif
