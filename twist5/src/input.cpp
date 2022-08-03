#include <assert.h>
#include "input.h"
#include <allegro5/allegro.h>

bool keyboardInited = false;

//TODO
//volatile int lastScancode; // last keypress, not release

//TODO
//void keyboard_handler (int scancode)
//{
//	if ((scancode & 128) == 0) lastScancode = scancode;
//}
//END_OF_FUNCTION(keyboard_handler);

void initKeyboard ()
{
	//TODO
//	LOCK_VARIABLE(last_scancode);
//	LOCK_FUNCTION(keyboard_handler);
//	lastScancode = 0;
//	keyboard_lowlevel_callback = keyboard_handler;
//	keyboardInited = true;
}

void resetLastScancode()
{
	//TODO
//	lastScancode = 0;
}

int getLastScancode()
{
//	return lastScancode;
	return 0;
}

void Input::update()
{
	if (justPressed()) { pushMsg(msg); }
}

bool Input::getState()
{
	ALLEGRO_KEYBOARD_STATE kbd;
	al_get_keyboard_state(&kbd);
	return al_key_down(&kbd, scancode) || al_key_down(&kbd, altcode);
}

bool Input::justPressed()
{
	bool newState = getState();
	bool result = (newState && !prevState);
	prevState = newState;
	return result;
}

const char *Input::name()
{ 
	return al_keycode_to_name(scancode);
}

void AnyKey::handleEvent(ALLEGRO_EVENT &event)
{
	switch (event.type)
	{
	case ALLEGRO_EVENT_KEY_DOWN: // fall through
	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		pushMsg(msg);
		break;
	default:
		// nothing
		break;
	}
}

ComponentBuilder<AnyKey> AnyKey::build(int actionMsg)
{
	return ComponentBuilder<AnyKey>(std::make_shared<AnyKey>(actionMsg));
}
