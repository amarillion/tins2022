#pragma once

#include "component.h"
#include "container.h"
#include "DrawStrategy.h"

void MASSetMouseCursor (Cursor &val);

/**
 * A dialog is a container with more advanced focus behaviour.
 *
 * A dialog will make sure that at least one focus-able component
 * is focused on the start. It checks if you press Tab or SHIFT+Tab and
 * select the prev/next focus component.
 */
class Dialog : public Container
{
public:
	void close();

	void Popup(Component *parent) {}

	virtual void draw(const GraphicsContext &gc) override;

	virtual void handleEvent (ALLEGRO_EVENT &event) override;
	virtual std::string const className() const override { return "Dialog"; }

protected:
	void moveFocus(int dir);

	/**
	 * If there is no component with focus, find first component and give it the focus.
	 */
	void initFocusIfNeeded();

	virtual void onAdd(ComponentPtr child) {
		initFocusIfNeeded();
	};

	virtual void MsgStart() override { /* TODO dialog intialisation */ }

};

/** TODO: Shortcut is very similar to class Input. Merge them. */
// TODO: figure if Accelerator, Input (former Button) and Shortcut can be merged into one.
class Shortcut : public Widget
{
private:
	int scancode;
	int keymod;
public:
	void Setup (int scancode, int keymod = 0);
	virtual std::string const className() const override { return "Shortcut"; }
	virtual bool MsgXChar(int keycode, int modifiers) override;
	Shortcut(int scancode, int keymod = 0) : scancode(scancode), keymod(keymod) { setVisible(false); }
	static ComponentBuilder<Shortcut> build(int scancode, int keymod = 0);
};

class TabPanel : public Widget
{
public:
	void Attach(std::shared_ptr<Widget> widget, const char *title);
};

class CheckBox : public TextWidget
{
};

class RadioButton : public TextWidget
{
};

// TODO: merge Accelerator, Input (former Button) and Shortcut into one.
// also combine with AnyKey
class Accelerator : public Widget
{
public:
	void Key(int scancode);
};

class PanelSunken : public Widget
{
};
