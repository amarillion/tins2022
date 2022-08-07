/*
 * Container.cpp
 *
 *  Created on: 3 Aug 2012
 *      Author: martijn
 */

#include "container.h"
#include "timer.h"
#include <allegro5/allegro.h>
#include <numeric>
#include <algorithm>
#include "util.h"
#include <list>
#include <limits>

using namespace std;

void Container::setFont(ALLEGRO_FONT *font)
{
	// set the font and the font of children, if needed.
	Component::setFont(font);
	list<ComponentPtr>::iterator i;
	for (auto i : children)
	{
		if (!(i->getFont()))
			i->setFont(font);
	}
}

void Container::add (ComponentPtr item, int flags)
{
	assert (item != nullptr);

	// prevent duplicates
	if (find(children.begin(), children.end(), item) != children.end())
	{
		cout << "WARN: Double addition of component, ignoring" << endl;
	}
	else
	{
		if (flags & FLAG_BOTTOM)
			children.push_front(item);
		else
			children.push_back (item);
	}

	if (flags & FLAG_SLEEP)
	{
		item->setAwake(false);
		item->setVisible(false);
	}

	if (isLayoutInitialised())
	{
		item->doLayout(getx(), gety(), getw(), geth());
	}

	//TODO: might needs some rethinking.
	// Inheritance of font like this is not ideal.
	// it is not clear when to set them if they should be different from parent.

	if (sfont && !(item->getFont()))
		item->setFont(sfont);

	onAdd(item);
}

void Container::add(UpdateablePtr item) {
	assert(item != nullptr);
	updateables.push_back(item);
}

void Container::update()
{
	Component::update();
	updateables.update();

	for (auto &i : children)
	{
		if (i->isAlive() && i->isAwake())
		{
			i->update();
			checkMessages(i);
		}
	}

	purge();
}

void Container::draw(const GraphicsContext &gc)
{
	for (auto i : children)
	{
		if (i->isVisible())
		{
			assert (i->getw() * i->geth() > 0); //trying to draw an invisibly small component

			if (!TestFlag(D_DISABLE_CHILD_CLIPPING)) {
				al_set_clipping_rectangle(i->getx() + gc.xofst, i->gety() + gc.yofst, i->getw() + 1, i->geth() + 1);
				i->draw(gc);
				al_reset_clipping_rectangle();
			}
			else
			{
				i->draw(gc);
			}

		}
	}
}

void Container::setFocus(ComponentPtr _focus)
{
	assert (_focus);
	assert (find(children.begin(), children.end(), _focus) != children.end()); // trying to set focus to somethign that wasn't added first

	if (focus == _focus) return;

	if (focus)
	{
		focus->ClearFlag(D_GOTFOCUS);
		focus->SetFlag(D_DIRTY);
		focus->handleMessage(nullptr, MSG_UNFOCUS);
	}
	focus = _focus;
	focus->SetFlag(D_GOTFOCUS);
	focus->SetFlag(D_DIRTY);
	focus->handleMessage(nullptr, MSG_FOCUS);
}

void Container::handleEvent (ALLEGRO_EVENT &event)
{
	switch (event.type)
	{
		// twist events
		case TWIST_START_EVENT:
		{
			MsgStart();
			break;
		}

		// touch events
		case ALLEGRO_EVENT_TOUCH_BEGIN:
		case ALLEGRO_EVENT_TOUCH_END:
		case ALLEGRO_EVENT_TOUCH_MOVE:
		case ALLEGRO_EVENT_TOUCH_CANCEL: {
			float current_touch_x = event.touch.x;
			float current_touch_y = event.touch.y;

			// find new mouse component
			ComponentPtr touchFocus = nullptr;

			// we have to send a click message
			// go through in reverse drawing order and find the component on top.
			for (auto i = children.rbegin(); i != children.rend(); i++)
			{
				if ((*i)->contains(current_touch_x, current_touch_y) && (*i)->isVisible())
				{
					touchFocus = *i;

					if ((*i)->wantsFocus())
					   setFocus (*i);

					break; // break from loop
				}
			}
			touchFocus->handleEvent(event);
			checkMessages(touchFocus);
			break;
		}
			// mouse events

		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		case ALLEGRO_EVENT_MOUSE_AXES:
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			int current_mouse_x = event.mouse.x;
			int current_mouse_y = event.mouse.y;

			// find new mouse component
			ComponentPtr newMouseFocus = nullptr;

			// we have to send a click message
			// go through in reverse drawing order and find the component on top.
			for (auto i = children.rbegin(); i != children.rend(); i++)
			{
				if ((*i)->contains(current_mouse_x, current_mouse_y) && (*i)->isVisible())
				{
					newMouseFocus = *i;

					if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && (*i)->wantsFocus())
						setFocus (*i);

					break; // break from loop
				}
			}

			if (newMouseFocus != mouseFocus)
			{
				ALLEGRO_EVENT evt2 = event;
				evt2.type = ALLEGRO_EVENT_TWIST_MOUSE_LEAVE_WIDGET;

				if (mouseFocus)	mouseFocus->handleEvent(evt2);
				mouseFocus = newMouseFocus;

				evt2.type = ALLEGRO_EVENT_TWIST_MOUSE_ENTER_WIDGET;
				if (mouseFocus) mouseFocus->handleEvent(evt2);
			}

			if (mouseFocus)
			{
				mouseFocus->handleEvent(event);
				checkMessages(mouseFocus);
			}

//			if (!focus) searchNextFocusItem();
		}
			break;
		// non-mouse events
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			if (focus)
			{
				focus->handleEvent(event);
				if (focus->hasMsg()) {
					checkMessages(focus);
				}
			}
			break;
		}
		default:
			if (focus)
			{
				focus->handleEvent(event);
				checkMessages(focus);
			}
			break;
	}
}

void Container::handleMessage(std::shared_ptr<Component> src, int msg)
{
	Component::handleMessage(src, msg);

	switch (msg)
	{
	case MSG_CLOSE:
		// src->kill(); // TODO: need a way to schedule removal without setting awake = false;
		break;
	}
}

void Container::setTimer(int msec, int event)
{
	auto timer = make_shared<Timer>(msec, event);
	add(timer);
}

void Container::setTimer(int msec, ActionFunc event, bool repeating)
{
	auto timer = make_shared<Timer>(msec, event, repeating ? Timer::REPEATING : Timer::ONCE);
	add(timer);
}

class MyComponentRemover
{
public:
	bool operator()(ComponentPtr o)
	{
		if (!o->isAlive())
		{
			o.reset();
			return 1;
		}
		return 0;
	}
};

void Container::purge()
{
	// remove all that are not alive!
	children.remove_if(MyComponentRemover());
}

void Container::killAll() {
	for (auto &i : children) {
		i->kill();
	}
}

void Container::repr(int indent, std::ostream &out) const
{
	Component::repr (indent, out);

	for (auto child : children)
	{
		child->repr(indent + 1, out);
	}
}

void Container::UpdateSize()
{
	int groupIdx[MAX_GROUP_ID] = {0};
	int groupSizes[MAX_GROUP_ID] = {0};

	// layout children that are not part of a group while counting group sizes
	for (auto child : children)
	{
		if (child->getGroupId() == 0)
		{
			child->doLayout(getx(), gety(), getw(), geth());
		}
		else
		{
			groupSizes[child->getGroupId()] ++;
		}
	}

	// layout children that *are* part of a group
	ComponentPtr prev = ComponentPtr();
	Rect p = Rect(getx(), gety(), getw(), geth());
	for (auto &child : children)
	{
		int groupId = child->getGroupId();
		if (groupId != 0)
		{
			int groupSize = groupSizes[groupId];
			int idx = groupIdx[groupId];
			Rect rect = groupLayouts[groupId](child, prev, idx, groupSize, p);
			child->setLocation(rect.x(), rect.y(), rect.w(), rect.h());
			groupIdx[groupId]++;
			prev = child;
		}
	}

	onResize();
}

double Container::getMaxRight()
{
	return accumulate(children.begin(), children.end(),
			-numeric_limits<double>::max(),
			[] (double chain, ComponentPtr a) { return max(a->getRight(), chain); });
}

double Container::getMinLeft()
{
	return accumulate(children.begin(), children.end(),
			numeric_limits<double>::max(),
			[] (double chain, ComponentPtr a) { return min(a->getLeft(), chain); });
}

double Container::getMinTop()
{
	return accumulate(children.begin(), children.end(),
			numeric_limits<double>::max(),
			[] (double chain, ComponentPtr a) { return min(a->getTop(), chain); });
}

double Container::getMaxBottom()
{
	return accumulate(children.begin(), children.end(),
			-numeric_limits<double>::max(),
			[] (double chain, ComponentPtr a) { return max(a->getBottom(), chain); });
}

void Container::resizeToChildren()
{
	repr (0, cout);
	int a = getMinLeft();
	int b = getMinTop();
	int c = getMaxRight();
	int d = getMaxBottom();
	setLocation(a, b, c, d);
}

bool Container::MsgXChar(int keycode, int modifiers) {
	//TODO: if you override this, you need to call Container::MsgXChar
	for (auto ch : children) {
		if (!ch->isAwake()) continue;

		bool result = ch->MsgXChar(keycode, modifiers);
		checkMessages(ch);
		if (result) return true;
	}
	return false;
}

