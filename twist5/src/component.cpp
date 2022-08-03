#include "component.h"
#include <stdio.h>
#include <iostream>
#include "strutil.h"

using namespace std;

string msgToString(int msg) {

	switch (msg) {
	case Component::MSG_FOCUS: return "MSG_FOCUS";
	case Component::MSG_UNFOCUS: return "MSG_UNFOCUS";
	case Component::MSG_KILL: return "MSG_KILL";
//	case MSG_CLOSE: return "MSG_CLOSE";
	//TODO: add more here
	default:
		return string_format("%i", msg);
	}
}


void Component::draw(const GraphicsContext &gc)
{
	if (!(alive && isVisible())) return;
}

/** helper to handle messages */
void Component::checkMessages(ComponentPtr ptr)
{
	while (ptr->hasMsg())
	{
		int msg = ptr->popMsg();
		handleMessage(ptr, msg);
	}
}

/**
 * Component version handles MSG_FOCUS and MSG_KILL.
 * Any other message is passed to onHandleMessage.
 */
void Component::handleMessage(std::shared_ptr<Component> src, int msg)
{
	switch (msg)
	{
	case MSG_FOCUS:
		onFocus();
		break;
	case MSG_KILL:
		kill();
		break;
	default:
		bool handled = onHandleMessage(src, msg);
		if (!handled)
		{
			pushMsg(msg); // let parent handle it.
		}
		break;
	}

}

void Component::reprBase(std::ostream &out) const {
	out << "[" << x << ", " << y << " - " << w << "x" << h << "] " << className() << " ";
	out << (isVisible() ? "v" : ".")
		<< (TestFlag(D_DISABLED) ? "." : "e")
		<< (TestFlag(D_GOTFOCUS) ? "f" : ".");
}

void Component::repr(int indent, std::ostream &out) const {
	for (int i = 0; i < indent; ++i) {
		out << "  ";
	}
	reprBase(out);
	out << endl;
}

void Component::doLayout(int px, int py, int pw, int ph)
{
	if (layout_flags & Layout::DISABLED) return; // automatic layout disabled
	layout_initialised = true;

	int sub_flags;

	sub_flags = layout_flags & (Layout::_Impl::LEFT | Layout::_Impl::CENTER | Layout::_Impl::RIGHT);
	switch (sub_flags)
	{
	case Layout::_Impl::LEFT:
		x = layout_x1 + px;
		break;
	case Layout::_Impl::CENTER:
		x = layout_x1 + px + (pw - layout_x2) / 2 ;
		break;
	case Layout::_Impl::RIGHT:
		x = (px + pw) - layout_x1 - layout_x2;
		break;
	default:
		assert(false); /* "Invalid flag combination" */
		break;
	}

	if (layout_flags & Layout::_Impl::_W)
	{
		w = layout_x2;
	}
	else if (layout_flags & Layout::_Impl::TO_RIGHT)
	{
		w = (px + pw) - layout_x2 - x;
	}
	else { assert(false); /* "Invalid flag combination" */ }


	sub_flags = layout_flags & (Layout::_Impl::TOP | Layout::_Impl::MIDDLE | Layout::_Impl::BOTTOM);
	switch (sub_flags)
	{
	case Layout::_Impl::TOP:
		y = layout_y1 + py;
		break;
	case Layout::_Impl::MIDDLE:
		y = layout_y1 + (ph - layout_y2) / 2;
		break;
	case Layout::_Impl::BOTTOM:
		y = (py + ph) - layout_y1 - layout_y2;
		break;
	default:
		assert(false); /* "Invalid flag combination" */
		break;
	}


	if (layout_flags & Layout::_Impl::_H)
	{
		h = layout_y2;
	}
	else if (layout_flags & Layout::_Impl::TO_BOTTOM)
	{
		h = (py + ph) - layout_y2 - y;
	}
	else { assert(false); /* "Invalid flag combination" */ }


	UpdateSize();
}


void Component::ToggleFlag(int flag) {
	//XOR
	flags ^= flag;
};
void Component::SetFlag(int flag) { flags |= flag; }
void Component::setFlagVal(int flag, bool value) { if (value) SetFlag(flag); else ClearFlag(flag); }
void Component::ClearFlag(int flag) {
	int inverse = (-flag - 1);
	flags &= inverse;
}
bool Component::TestFlag(int flag) const { return ((flags & flag) == flag); }
