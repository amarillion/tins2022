#include "menubar.h"
#include "button.h"
#include "panel.h"
#include "mainloop.h"
#include <allegro5/allegro_font.h>
#include "strutil.h"
#include "util.h"

using namespace std;

static const int MENU_ITEM_MARGIN = 16; //TODO: add to skin
static const int TOPBAR_MENU_ITEM_MARGIN = 4; //TODO not yet in skin

/**
 * A menu item can be either in the top menubar (arranged horizontally),
 * or in a popup menu (arranged vertically)
 *
 * Behaviour of menu items:
 *
 * Focus & WantFocus:
 *     In the context of menu items, if a
 *     menu item has the focus, it is highlighted, and
 *     should respond to left/right to select the next menu, and up/down to select the next item
 *     Menu Items want focus, but separators do not want focus.
 *
 * Selected and Toggle
 *     Only toggle menu items can be selected / deselected. They will show a check mark
 *
 */
class MenuItemImpl : public MenuItem {

private:
	int actionCode;
	ActionFunc actionFunc;
	bool topBar;
	bool isSeparator;
	bool isSubMenu;
	std::shared_ptr<Menu> subMenu;

	/** text that is shown on top of the menu item, excluding control characters passed to the constructor */
	std::string displayText;

	int underline; // index of character to underline

	/** keyboard shortcut that can be used in combination with ALT to activate the menu item. */
	char accelerator;

	// detect ampersand and extract accelerator and displayText
	void parseGuiText(const std::string &text) {
		int pos = text.find_first_of('&', 0);
		if (pos < 0) {
			underline = -1;
			displayText = text;
			accelerator = '\0';
		}
		else {
			displayText = text.substr(0, pos) + text.substr(pos+1);
			accelerator = text[pos+1];
			underline = pos;
		}
		SetFlag(D_DIRTY);
	}

protected:
	virtual bool wantsFocus () override { return true; }

	virtual void SetText(const char *text) override {
		assert (!isSeparator);
		parseGuiText(text);
	}

	virtual void setFont(ALLEGRO_FONT *font) {
		if (sfont != font) {
			this->sfont = font;
			pushMsg(MSG_CHILD_RESIZED);
		}
	}

	double getPreferredWidth()
	{
		int horizontalMargin = getHorizontalMargin();
		double result = 2 * horizontalMargin;
		if (sfont) { result += al_get_text_width(sfont, displayText.c_str()); }
		return result;
	}

	virtual void onMouseEnter() override {
		pushMsg(MSG_REQUEST_FOCUS);
	}

	void doAction() {
		if (actionFunc) { actionFunc(); }
		else pushMsg(actionCode);
	}

	virtual void handleEvent(ALLEGRO_EVENT &event) override {
		switch (event.type)
		{
			case ALLEGRO_EVENT_TWIST_MOUSE_ENTER_WIDGET:
			{
				onMouseEnter();
			}
				break;
			case ALLEGRO_EVENT_TWIST_MOUSE_LEAVE_WIDGET:
			{
				onMouseLeave();
			}
				break;
			case ALLEGRO_EVENT_TOUCH_BEGIN:
				doAction();
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				if (!isEnabled()) break;
				if (isToggleButton())
				{
					Toggle();
				}
				else
				{
					Select();
				}
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				if (isSelected())
				{
					doAction();
				}
				if (!isToggleButton())
				{
					Deselect();  //TODO - also when mouse exits... // guarantee exit event
				}
				break;
			case ALLEGRO_EVENT_KEY_CHAR:
			{
				if (!isEnabled()) break;
				switch (event.keyboard.keycode)
				{
					case ALLEGRO_KEY_SPACE: case ALLEGRO_KEY_ENTER:
						if (isToggleButton())
						{
							Toggle();
						}
						doAction();
						break;
				}
				break;
			}
		}
	}

	int getHorizontalMargin() {
		if (topBar) {
			return TOPBAR_MENU_ITEM_MARGIN;
		}
		else {
			return MENU_ITEM_MARGIN;
		}
	}

	void doDrawText(int state, int x1, int y1, int x2, int y2) {

		ALLEGRO_COLOR fg = theSkin->fcol[Skin::INFO_MENU][state];

		// The menu text (left part)
		auto fields = split(displayText, '\t');
		if (fields.size() == 0) {
			return; // nothing to draw...
		}

		int horizontalMargin = getHorizontalMargin();

		int middle = al_get_font_line_height(sfont) / 2;
		int ascent = middle + al_get_font_ascent(sfont);

		al_draw_text (sfont, fg,
			x1 + horizontalMargin, y1 + middle, ALLEGRO_ALIGN_LEFT,
			fields[0].c_str());

		if (underline >= 0) {
			int underlineStart = al_get_text_width(sfont, displayText.substr(0, underline).c_str());
			char textbuf[2] = " ";
			textbuf[0] = displayText[underline];
			int underlineWidth = al_get_text_width(sfont, textbuf);
			al_draw_line(x1 + horizontalMargin + underlineStart, y1 + ascent,
					x1 + horizontalMargin + underlineStart + underlineWidth, y1 + ascent, fg, 1.0);
		}

		// right-aligned part of the menu text after tab.
		if (fields.size() > 1)
		{
			al_draw_text (sfont, fg,
				x2 - horizontalMargin, y1 + middle, ALLEGRO_ALIGN_RIGHT,
				fields[1].c_str());
		}
	}

	virtual void doDraw (const GraphicsContext &gc) override
	{
		int x1 = getx() + gc.xofst;
		int y1 = gety() + gc.yofst;
		int x2 = x1 + w;
		int y2 = y1 + h;

		// get the button state
		int state;

		if (topBar) {
			state = isDisabled() ? 2 : ((isSelected() /* || open */) ? 3 : 0);
		}
		else {
			state = isDisabled() ? 2 : ((hasFocus() /* || open */) ? 3 : (isSelected() ? 1 : 0));
		}

		// Get the subbitmap count and offset for a particular type of menu item
		int vcount;
		int off;

		if (topBar) {
			vcount = 2;
			off = (hasFocus() /* || open */) ? 1 : 0;
		}
		else if (isSeparator) {
			// it's a separator...
			off = 2;
			vcount = 7;
		}
		else {
			vcount = 7;
			if (hasFocus() /* || open */) {
//				if (child)				off = 4; //TODO - for submenus
//				else
				if (isSelected())	off = 6;
				else					off = 1;
			}
			else {
//				if (child)				off = 3; //TODO - for submenus
//				else
				if (isSelected())	off = 5;
				else					off = 0;
			}
		}

		// Draw a normal text menu item
		Bitmap bmp = theSkin->GetBitmap(topBar ? Skin::MENU_BUTTON : Skin::MENU_ITEM);

		Bitmap canvas = Bitmap(gc.buffer);
		bmp.TiledBlit(canvas, 0, off, x1, y1, getw(), geth(), 1, vcount);

		if (!isSeparator) {
			doDrawText(state, x1, y1, x2, y2);
		}
	}

public:
	virtual void setActionFunc(ActionFunc value) override { actionFunc = value; }
	virtual shared_ptr<Menu> getSubMenu() { return subMenu; }

	virtual void reprBase(std::ostream &out) const override {
		out << "[" << x << ", " << y << " - " << w << "x" << h << "] ";
		out << (isVisible() ? "v" : ".")
			<< (TestFlag(D_DISABLED) ? "." : "e")
			<< (TestFlag(D_GOTFOCUS) ? "f " : ". ");
		out << displayText;
	}

	MenuItemImpl(std::string text, bool topBar, std::shared_ptr<Menu> subMenu) : actionCode(MSG_CHILD_POPUP), topBar(topBar), isSeparator(false), isSubMenu(true), subMenu(subMenu) {
		SetFlag(D_DOUBLEBUFFER);
		parseGuiText(text);
	}

	MenuItemImpl(int actionCode, std::string text, bool topBar) : actionCode(actionCode), topBar(topBar), isSeparator(false) {
		SetFlag(D_DOUBLEBUFFER);
		parseGuiText(text);
	}

	/** separator */
	MenuItemImpl() : actionCode(-1), actionFunc(), topBar(false), isSeparator(true), isSubMenu(false), subMenu(nullptr), displayText(), underline(-1), accelerator('\0') {
		SetFlag(D_DOUBLEBUFFER);
	}
};

ComponentBuilder<MenuItem> MenuItem::build(int actionCode, std::string text, bool topBar)
{
	auto result = make_shared<MenuItemImpl>(actionCode, text, topBar);
	return ComponentBuilder<MenuItem>(result);
}

ComponentBuilder<MenuItem> MenuItem::build(std::string text, bool topBar, std::shared_ptr<Menu> subMenu)
{
	auto result = make_shared<MenuItemImpl>(text, topBar, subMenu);
	return ComponentBuilder<MenuItem>(result);
}

ComponentBuilder<MenuItem> MenuItem::buildSpacer()
{
	auto result = make_shared<MenuItemImpl>();
	return ComponentBuilder<MenuItem>(result);
}

class PopupMenuImpl;

class MenuBaseImpl : public Menu
{
protected:
	// ordered list of menu items is kept here (in addition to Container's children)
	std::vector<shared_ptr<MenuItem>> items;

	virtual void recalculateLayout() = 0;
	ComponentPtr childPopup = nullptr;
public:

	virtual shared_ptr<MenuItem> &GetItem(size_t index) override { assert (index < items.size()); return items[index]; }
	virtual void Check(size_t pos) override { assert (pos < items.size()); items[pos]->Select(); }
	virtual void Uncheck(size_t pos) override { assert (pos < items.size()); items[pos]->Deselect(); }

	virtual void addItem(const char *item, int msg) override
	{
		auto btn = MenuItem::build(0, string(item), false).get();
		btn->setActionFunc( [=] () {
			this->setVisible(false);
			this->setAwake(false);
			this->kill();
			pushMsg(msg);
		});
		btn->setGroupId(2);
		add (btn);
		items.push_back(btn);
		recalculateLayout();
	}

	virtual bool onHandleMessage(ComponentPtr src, int msg)
	{
		switch(msg) {
		case MSG_CHILD_POPUP:
		{
			auto menuItem = dynamic_pointer_cast<MenuItem>(src);
			assert (menuItem);
			auto menu = menuItem->getSubMenu();
			if (this->childPopup && this->childPopup != menu)
			{
				this->childPopup->setVisible(false);
				this->childPopup->setAwake(false);
				this->childPopup = nullptr;
			}

			menu->setLayout(Layout::LEFT_TOP_W_H, src->getLeft() + 10, src->getBottom(), 0, 0);
			menu->setVisible(true);
			menu->setAwake(true);
			this->childPopup = menu;
			MainLoop::getMainLoop()->popup(menu, shared_from_this());

			return true;
		}
		case MSG_CHILD_RESIZED:
			recalculateLayout();
			return true;
		case MSG_REQUEST_FOCUS:
			setFocus(src);
			return true;
		}

		return false;
	}

	virtual void addSeparator() override {
		auto spacer = MenuItem::buildSpacer().get();
		spacer->setGroupId(2);
		add (spacer);
		items.push_back(spacer);

		recalculateLayout();
	}

	virtual void addItem(const char *submenu, shared_ptr<Menu> menu) override
	{
		auto btn = MenuItem::build(submenu, true, menu).get();
		btn->setGroupId(2);
		add (btn);
		items.push_back(btn);
		recalculateLayout();
	}

	virtual ~MenuBaseImpl() {}
};

class PopupMenuImpl : public MenuBaseImpl
{
	static const int MENU_ITEM_HEIGHT = 16; //TODO: part of skin
	static const int BORDER = 3; // TODO: part of skin

	static const Rect layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p)
	{
		int dy = MENU_ITEM_HEIGHT;
		return Rect(p.x() + BORDER, p.y() + BORDER + idx * dy, p.w() - (2 * BORDER), MENU_ITEM_HEIGHT);
	}

	void recalculateLayout() override
	{
		double maxW = 0;
		for (auto ch : children) {
			maxW = std::max (maxW, ch->getPreferredWidth()  + (2 * BORDER));
		}
		setDimension(maxW, (items.size() * MENU_ITEM_HEIGHT) + (2 * BORDER));
	}

public:
	PopupMenuImpl()
	{
		auto panel = Panel::build(Skin::PANEL_RAISED).layout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 0, 0, 0).get();
		add (panel);

		setGroupLayout(2, PopupMenuImpl::layoutFunction);
	}

	virtual void close()
	{
		pushMsg(MSG_CLOSE); // should trigger closing of popup...
	}

	virtual void onMouseLeave() override
	{
		close();
	}

	virtual ~PopupMenuImpl() {}
};

class MenuBarImpl : public MenuBaseImpl
{
	static const int MENU_ITEM_HEIGHT = 16; //TODO: part of skin
	static const int MENU_ITEM_WIDTH = 160; //TODO: part of skin
private:
	static const Rect layoutFunction(ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p)
	{
		int xco;
		if (prev)
		{
			xco = prev->getRight();
		}
		else
		{
			xco = p.x();
		}

		return Rect(xco, p.y(), comp->getPreferredWidth(), MENU_ITEM_HEIGHT);
	}

	virtual void recalculateLayout() override
	{
		setLayout(Layout::LEFT_TOP_W_H, getx(), gety(), items.size() * MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT);
	}

public:
	MenuBarImpl()
	{
		setGroupLayout(2, MenuBarImpl::layoutFunction);
	}

	virtual ~MenuBarImpl() {}

	virtual void update() override {
		MenuBaseImpl::update();
		if (childPopup != nullptr)
		{
			checkMessages(childPopup);
		}
	}

};

ComponentBuilder<Menu> Menu::build(bool fMenuBar)
{
	if (fMenuBar)
	{
		return ComponentBuilder<Menu>(make_shared<MenuBarImpl>());
	}
	else
	{
		return ComponentBuilder<Menu>(make_shared<PopupMenuImpl>());
	}
}
