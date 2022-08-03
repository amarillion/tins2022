#pragma once

#include "container.h"

class Menu;

class MenuItem : public Widget {
public:
	virtual void SetText(const char *text) = 0;
	virtual void setActionFunc(ActionFunc value) = 0;
	virtual std::shared_ptr<Menu> getSubMenu() = 0;

	static ComponentBuilder<MenuItem> build(int actionCode, std::string text, bool topBar);
	static ComponentBuilder<MenuItem> build(std::string text, bool topBar, std::shared_ptr<Menu> subMenu);
	static ComponentBuilder<MenuItem> buildSpacer();

	virtual std::string const className() const override { return "MenuItem"; }
};

class Menu : public Container
{
public:
	virtual void Check(size_t pos) = 0;
	virtual void Uncheck(size_t pos) = 0;

	virtual void addItem(const char *item, int msg) = 0;
	virtual void addSeparator() = 0; // insert spacer;
	virtual void addItem(const char *submenu, std::shared_ptr<Menu> menu) = 0;

	virtual std::shared_ptr<MenuItem> &GetItem(size_t index) = 0;
	virtual ~Menu() {}
	static ComponentBuilder<Menu> build(bool fMenuBar);

	virtual std::string const className() const override { return "Menu"; }
};
