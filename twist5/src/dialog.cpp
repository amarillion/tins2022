#include "dialog.h"
#include "color.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include "button.h"
#include "mainloop.h"
#include "panel.h"
#include "text.h"
#include <sstream>
#include <algorithm>

using namespace std;

void MASSetMouseCursor (Cursor &val) { /* TODO */ }

void Dialog::close() { pushMsg(MSG_CLOSE); kill(); }

void Dialog::draw(const GraphicsContext &gc)
{
	Container::draw(gc);
}

void Dialog::handleEvent(ALLEGRO_EVENT &event) {

	switch(event.type)
	{
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			// first handle tab events.
			// TODO: this may be better handled by the widget itself?
			bool eventHandled = false;
			int dir = 0;
			if (event.keyboard.keycode == ALLEGRO_KEY_TAB) {
				if (event.keyboard.modifiers & ALLEGRO_KEYMOD_SHIFT) {
					dir = -1;
					eventHandled = true;
				}
				else {
					dir = 1;
					eventHandled = true;
				}
			}
			if (eventHandled) {
				moveFocus(dir);
			}
			break;
		}
	}
	Container::handleEvent(event);
}

function<bool(ComponentPtr)> CAN_FOCUS =
		[](ComponentPtr child){ return (!child->TestFlag(D_DISABLED) && child->wantsFocus()); };

void Dialog::initFocusIfNeeded() {
	if (!focus) {
		auto candidate = find_if(
			children.begin(),
			children.end(),
			CAN_FOCUS
		);
		if (candidate != children.end()) {
			setFocus(*candidate);
		}
	}
}

void Dialog::moveFocus(int dir) {

	// forward
	if (dir > 0) {
		list<ComponentPtr>::iterator it;

		if (focus) {
			it = find(children.begin(), children.end(), focus);
			it++;
		}
		else {
			it = children.begin();
		}

		auto result = find_if (it, children.end(), CAN_FOCUS);
		if (result == children.end()) {
			result = find_if (children.begin(), it, CAN_FOCUS);
		}

		if (result != children.end()) {
			setFocus(*result);
		}
	}
	else {
		list<ComponentPtr>::reverse_iterator it;

		if (focus) {
			it = find(children.rbegin(), children.rend(), focus);
			it++;
		}
		else {
			it = children.rbegin();
		}

		auto result = find_if (it, children.rend(), CAN_FOCUS);
		if (result == children.rend()) {
			result = find_if (children.rbegin(), it, CAN_FOCUS);
		}

		if (result != children.rend()) {
			setFocus(*result);
		}
	}
}

void Shortcut::Setup (int scancode, int keymod)
{
	this->scancode = scancode;
	this->keymod = keymod;
}

bool Shortcut::MsgXChar(int keycode, int modifiers)
{
	if (keycode == this->scancode && modifiers == this->keymod) {
		pushMsg(MSG_ACTIVATE);
		return true;
	}
	return false;
}

void TabPanel::Attach(shared_ptr<Widget> widget, const char *title) { /* TODO */ }

void Accelerator::Key(int scancode) { /* TODO */ }

ComponentBuilder<Shortcut> Shortcut::build(int scancode, int keymod)
{
	return ComponentBuilder<Shortcut>(make_shared<Shortcut>(scancode, keymod));
}
