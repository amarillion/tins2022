#pragma once

#include <string>
#include <vector>
#include "component.h"
#include <memory>
#include "state.h"
#include "data.h"

struct ALLEGRO_BITMAP;
struct ALLEGRO_SAMPLE;
struct ALLEGRO_FONT;

class Sound;

class MenuScreen;

class MenuItem : public Component
{
	protected:
		MenuScreen *parent;
		bool flashing;
		bool enabled;
	public:
		enum { MENU_NONE = 1000, MENU_PREV, MENU_NEXT };
		MenuItem () : parent(NULL), flashing (false), enabled(true) {}
		virtual ~MenuItem() {}
		bool isSelected();
		void setParent (MenuScreen *val) { parent = val; }
		virtual std::string getText() = 0;
		virtual std::string getHint() = 0;
		virtual void draw(const GraphicsContext &gc) override;
		void setEnabled (bool value) { enabled = value; }
		virtual bool isEnabled() { return enabled; }

		virtual ALLEGRO_COLOR getColor();

		virtual std::string const className() const override { return "MenuItem"; }
};

class ActionMenuItem : public MenuItem
{
	private:
		int action; // code returned from handleInput
		std::string text;
		std::string hint;
	public:
		ActionMenuItem (int _action, std::string _text, std::string _hint) :
			MenuItem(), action (_action), text (_text), hint (_hint) {}
		virtual void handleEvent(ALLEGRO_EVENT &event) override;
		virtual std::string getText() override { return text; }
		virtual std::string getHint() override { return hint; }
		void setText (std::string value) { text = value; }
};

class ToggleMenuItem : public MenuItem
{
private:
	int action;
	std::string a;
	std::string b;
	std::string hint;
	bool toggle;
public:
	ToggleMenuItem(int _action, std::string _a, std::string _b, std::string _hint);
	void setToggle(bool value) { toggle = value; }
	bool getToggle() { return toggle; }
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual std::string getText() override { return toggle ? a : b; }
	virtual std::string getHint() override { return hint; }
};

class SliderMenuItem : public MenuItem
{
private:
	std::string label;
	std::string hint;
	std::string text = "";
	RangeModel<float> *model;
	void updateText();
public:
	SliderMenuItem(RangeModel<float> *model, std::string label, std::string _hint);
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual std::string getText() override { return text; }
	virtual std::string getHint() override { return hint; }
};

typedef std::shared_ptr<MenuItem> MenuItemPtr;

struct ALLEGRO_SAMPLE;

class MenuScreen : public State
{
private:
	ComponentPtr hint;
	std::vector<MenuItemPtr> items;
	ALLEGRO_SAMPLE *sound_enter;
	ALLEGRO_SAMPLE *sound_cursor;
	Sound *sound;
	ALLEGRO_COLOR colorNormal;
	ALLEGRO_COLOR colorFlash1;
	ALLEGRO_COLOR colorFlash2;
	ALLEGRO_COLOR colorDisabled;
	friend class MenuItem;
	void prev();
	void next();
	unsigned int selected;
	int tFlash;
	int topMargin;
	int bottomMargin;
	void selectFirstEnabled();
public:
	MenuScreen(Sound *sound = NULL);
	virtual ~MenuScreen() {}
	unsigned int size () { return items.size(); }
	void push_back(MenuItemPtr item)
	{
		// add both to items and children.
		items.push_back(item);
		item->setParent(this);
		add (item);
	}
	MenuItemPtr& operator[] (int idx) { return items[idx]; }
	void setMargin(int top, int bottom) { topMargin = top; bottomMargin = bottom; }
	virtual void onUpdate() override;
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	virtual void onFocus () override;
	int getSelectedIdx() { return selected; }
	MenuItemPtr getSelectedItem() { return items[selected]; }

	int getItemIdx(const MenuItemPtr &item) {
		int result = 0;
		for (auto i : items) {
			if (i == item) return result;
			result++;
		}
		return -1;
	}

	void setSelectedIdx(int idx) {
		if (idx >= 0 && idx < (int)items.size()) {
			selected = idx;
		}
	}

	void setSelectedItem(const MenuItemPtr &item) {
		int idx = getItemIdx(item);
		if (idx >= 0) selected = idx;
	}

	virtual std::string const className() const override { return "MenuScreen"; }

	/** Slightly more customizable layout, still with vertical equal spacing, but allows setting top and bottom margins */
	static const Rect  marginAdjustedlayout(int topMargin, int bottomMargin, ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p);
	/** Standard menu layout, stacks menu items vertically, equally spaced */
	static const Rect layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p);
	
	/** Standard menu layout, stacks menu items vertically in two columns */
	static const Rect twoColumnLayoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p);
};

typedef std::shared_ptr<MenuScreen> MenuScreenPtr;

class MenuBuilder
{
public:
	MenuBuilder(Container *parent, Sound *sound);

	/** by default, menu items will be in layout group 1 */
	MenuBuilder &push_back (MenuItemPtr item, int groupId = 1)
	{
		item->setGroupId(groupId);
		result->push_back(item);
		return *this;
	}

	MenuScreenPtr build () { return result; }

private:
	MenuScreenPtr result;
};

class Hint : public Component
{
private:
	MenuScreen *parent;
public:
	Hint(MenuScreen *parent) { this->parent = parent; }
	virtual void draw (const GraphicsContext &gc) override;

	virtual std::string const className() const override { return "Hint"; }
};
