#pragma once

#include "component.h"
#include "componentbuilder.h"

//TODO: this doesn't work yet because key events are not passed on to components unless they have focus.
//TODO: add an accelerator property to all widgets.
/**
 * send message after any key or mouse click
 * TODO: also check for joystick buttons
 */
class AnyKey : public Component
{
private:
	int msg;
public:
	AnyKey(int _msg) : msg(_msg) {}
	virtual void handleEvent(ALLEGRO_EVENT &event) override;
	static ComponentBuilder<AnyKey> build(int actionMsg);
};

/** TODO: very similar to class Shortcut. Merge them */
class Input : public Component
{
private:
	int scancode; // contains keyboard scancode
	int altcode;
	
	bool prevState;
	int msg;
public:
	Input (int _scancode = 0, int _altcode = 0, int _msg = 0)
	{ scancode = _scancode; altcode = _altcode; prevState = false; msg = _msg; setVisible(false); }
	bool getState();
	
	virtual ~Input() {}
	virtual void update() override;

	/**
		returns true if 
		the state of this button was false on last call and true on this call
	*/
	bool justPressed();

	void setScancode (int newcode) { scancode = newcode; }
	void setAltcode (int newcode) { altcode = newcode; }
	int getScancode () { return scancode; }
	const char *name();
};

// install the keyboard lowlevel callback
void initKeyboard ();

// reset scancode to 0 so you can start monitoring for the next keypress
void resetLastScancode();

// return last pressed key. Should return 0 after resetLastScancode
int getLastScancode();
