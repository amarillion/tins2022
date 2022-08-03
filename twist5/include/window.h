#pragma once

#include "dialog.h"

class Button;
class Panel;
class Label;

class Window : public Container
{
	std::shared_ptr<Panel> back;
	std::shared_ptr<Container> client;
	std::shared_ptr<Label> title;
	std::shared_ptr<Panel> titleBar;
	std::shared_ptr<Button> btnClose;

public:

	/**
	 * Adds itself to the root panel and returns immediately.
	 * any messages will be routed to parent.
	 */
	int Popup(ComponentPtr parent);

	/**
	 * Adds itself to the root panel.
	 * Will not return until the dialog has been closed.
	 */
	int PopupModal();

	void Resize(int w, int h);
	void Centre();
	// void HandleEvent (Widget &obj, int msg, int arg1, int arg2);
	void setTitle(const char *value);
	Window();
	virtual ~Window() {}
	std::shared_ptr<Container> getClient() { return client; }
};
