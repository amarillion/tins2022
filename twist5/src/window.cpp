#include "window.h"
#include "color.h"
#include "mainloop.h"
#include "label.h"
#include "button.h"
#include "panel.h"

using namespace std;

int Window::Popup(ComponentPtr parent)
{
	MainLoop::getMainLoop()->popup(this->shared_from_this(), parent);
	return 0; /* TODO */
}

int Window::PopupModal() {
	MainLoop::getMainLoop()->popup(this->shared_from_this());
	MainLoop::getMainLoop()->pumpMessages();
	kill();
	return 0; /* TODO */
}

// TODO: deprecate - use setDimension instead.
void Window::Resize(int w, int h) { setDimension(w, h); }

void Window::Centre() { setxy((MAIN_WIDTH - getw()) / 2, (MAIN_HEIGHT - geth()) / 2); }

// void Window::HandleEvent (Widget &obj, int msg, int arg1, int arg2) { /* TODO */ }

void Window::setTitle(const char *val) { title->SetText(val); }

Window::Window()
{
	back = make_shared<Panel>(Skin::PANEL_RAISED);
	add (back);

	titleBar = Panel::build(Skin::PANEL_RAISED).layout(Layout::LEFT_TOP_RIGHT_H, 2, 2, 2, 16).get();
	add (titleBar);

	title = make_shared<Label>();
	title->setLayout(Layout::LEFT_TOP_RIGHT_H, 4, 4, 16, 12);
	title->SetText("Unnamed Window");

	btnClose = make_shared<Button>(MSG_CLOSE, "x");
	btnClose->setLayout(Layout::RIGHT_TOP_W_H, 4, 4, 12, 12);
	add(btnClose);

	client = make_shared<Dialog>();
	client->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 2, 18, 2, 2);
	add(client);

	add (title);

	setFocus(client);
}
