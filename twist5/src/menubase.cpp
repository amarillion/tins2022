#include "menubase.h"
#include <allegro5/allegro.h>
#include "color.h"
#include <assert.h>
#include "audio.h"
#include <stdio.h>
#include <allegro5/allegro_font.h>
#include "util.h"
#include "strutil.h"
#include <math.h>

using namespace std;

const int MENU_ITEM_HEIGHT = 64;

void MenuItem::draw(const GraphicsContext &gc)
{
	assert (parent);

	ALLEGRO_COLOR color = getColor();

	al_set_target_bitmap (gc.buffer);
	al_draw_text (parent->sfont, color, getx() + getw() / 2, y, ALLEGRO_ALIGN_CENTER, getText().c_str());
}

ALLEGRO_COLOR MenuItem::getColor()
{
	ALLEGRO_COLOR color = parent->colorNormal;
	if (!enabled) color = parent->colorDisabled;
	if (flashing)
	{
		if ((parent->tFlash % 10) < 5) color = parent->colorFlash2;
	}
	else if (isSelected() && (parent->tFlash % 30) < 15) color = parent->colorFlash1;
	return color;
}

bool MenuItem::isSelected()
{
	return (parent->getSelectedItem().get() == this);
}

void ActionMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return; // not interested in any others...

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
			pushMsg(action);
			break;
	}
}

void ToggleMenuItem::handleEvent(ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return; // not interested in any others...

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
			toggle = !toggle;
			pushMsg(action);
			break;
	}
}

ToggleMenuItem::ToggleMenuItem(int _action, std::string _a, std::string _b, std::string _hint) :
		action(_action), a(_a), b(_b), hint(_hint), toggle(false) {}

SliderMenuItem::SliderMenuItem(RangeModel<float> *model, std::string _label, std::string _hint) :
	label(_label), hint(_hint), model(model) {
		model->AddListener([=](int){ updateText(); });
		updateText();
	}

float round10(float val) {
	return round(val * 10.0) / 10.0;
}

void SliderMenuItem::handleEvent(ALLEGRO_EVENT &event) {
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return; // not interested in any others...

	float notch = (model->getMax() - model->getMin()) / 10.0;

	switch (event.keyboard.keycode)
	{
		case ALLEGRO_KEY_UP:
			pushMsg(MenuItem::MENU_PREV);
			break;
		case ALLEGRO_KEY_DOWN:
			pushMsg(MenuItem::MENU_NEXT);
			break;
		case ALLEGRO_KEY_LEFT:
			model->set(round10(model->get() - notch));
			break;
		case ALLEGRO_KEY_RIGHT:
			model->set(round10(model->get() + notch));
			break;
	}

}

void SliderMenuItem::updateText() { 
	char buffer[12] = "-----------"; 
		// 11 for the slider: 0-10 inclusive 
		//  plus one for zero terminator
	float val = model->getNormalised() * 10.0f;
	int pos = bound(0, (int)round(val), 10);
	buffer[pos] = '|';
	text = label + ": [" + string(buffer) + ']';
}

void MenuScreen::selectFirstEnabled()
{
	selected = 0;
	bool wrapped = false;
	while (!(items[selected]->isEnabled() &&
			items[selected]->isVisible()))
	{
		if (++selected >= items.size()) {
			selected = 0;
			if (wrapped) { break; } // avoid infinite loop
			wrapped = true;
		}
	}

}

void MenuScreen::onFocus()
{
	State::onFocus();
	selectFirstEnabled();
	tFlash = 0;
}

void MenuScreen::onUpdate()
{
	tFlash++;
}

void MenuScreen::handleEvent(ALLEGRO_EVENT &event)
{
	items[selected]->handleEvent(event);

	while (items[selected]->hasMsg())
	{
		int action = items[selected]->popMsg();
		switch (action)
		{
		case MenuItem::MENU_NEXT:
			next();
			break;
		case MenuItem::MENU_PREV:
			prev();
			break;
		case MenuItem::MENU_NONE:
			// do nothing.
			break;
		default:
			// let parent class handle action
			if (sound_enter && sound) sound->playSample (sound_enter);
			pushMsg(action);
			break;
		}
	}
}

void Hint::draw(const GraphicsContext &gc)
{
	MenuItemPtr item = parent->getSelectedItem();
	assert (sfont);
	string hint = item->getHint();
	al_set_target_bitmap (gc.buffer);
	al_draw_text (sfont, WHITE, getw() / 2,
		y, ALLEGRO_ALIGN_CENTER, hint.c_str());
}

void MenuScreen::prev()
{
	do
	{
		if (selected == 0) selected = items.size() - 1;
		else selected--;
	} while (!(items[selected]->isEnabled() &&
			items[selected]->isVisible()));
	tFlash = 0;
	if (sound_cursor && sound) sound->playSample (sound_cursor);
}

MenuScreen::MenuScreen(Audio *_sound)
{
	sound = _sound;
	sound_enter = NULL; sound_cursor = NULL;
	awake = false;
	setVisible(false);

	topMargin = 60;
	bottomMargin = 100;

	colorNormal = YELLOW;
	colorFlash1 = RED;
	colorFlash2 = GREEN;
	colorDisabled = GREY;

	setGroupLayout(1, MenuScreen::layoutFunction);
	hint = make_shared<Hint>(this);
	hint->setLayout(Layout::LEFT_BOTTOM_RIGHT_H, 0, 0, 0, MENU_ITEM_HEIGHT);
	add (hint);
}

void MenuScreen::next()
{
	do
	{
		if (++selected >= items.size()) selected = 0;
	} while (!(items[selected]->isEnabled() &&
			items[selected]->isVisible()));
	tFlash = 0;
	if (sound_cursor && sound) sound->playSample (sound_cursor);
}

const Rect MenuScreen::layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int count, const Rect &p)
{
	int y = p.y();
	int dy = p.h() / (count + 2);

	return Rect(p.x(), y + (idx + 1) * dy, p.w(), dy);
}

const Rect MenuScreen::twoColumnLayoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int count, const Rect &p)
{
	int y = p.y();
	int colbreak = (count / 2);
	int dy = p.h() / (colbreak + 2);

	int colw = p.w() / 2;
	int row = idx < colbreak ? idx : idx - colbreak;

	return Rect(p.x() + (idx < colbreak ? 0 : colw), y + (row * dy), colw, dy);
}

const Rect MenuScreen::marginAdjustedlayout(int marginTop, int marginBottom, ComponentPtr comp, ComponentPtr prev, int idx, int count, const Rect &p)
{
	int y = p.y() + marginTop;
	int dy = (p.h() - marginTop - marginBottom) / (count + 2);

	return Rect(p.x(), y + (idx + 1) * dy, p.w(), dy);
}

MenuBuilder::MenuBuilder(Container *parent, Audio *sound)
{
	result = make_shared<MenuScreen>(sound);
	parent->add(result, Container::FLAG_SLEEP);
	assert (parent->getFont());
	assert (result->getFont());
}
