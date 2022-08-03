#pragma once

#include "dialog.h"
#include "button.h"
#include "viewport.h"
#include "data.h"
#include "panel.h"

class Scroller : public Container
{
public:
	enum { HORIZONTAL, VERTICAL };
protected:
	int buttonw, buttonh; // size of arrow buttons, derived from skin.

	/**
         The underlaying background widget.
	 */
	std::shared_ptr<ClearScreen> back;
	/**
         The underlaying up/right arrow button.
	 */
	std::shared_ptr<Button> bUp;
	/**
         The underlaying down/left arrow button.
	 */
	std::shared_ptr<Button> bDown;
	/**
         The underlaying gripper button.
	 */
	std::shared_ptr<Button> bGrip;
	/**
         The number of items the scrolled object can display on one screen.
         The scroll button is resized to reflect the relation between the
         number of visible items and the number of all items.
	 */
	int visibleItems = 0;
	/**
         The orientation of the scroller. If 0 the scroller is vertical otherwise it is horizontal.
	 */
	int orientation = 0;

	/**
         The offset of the point on the gripper where the user grabbed it to drag from the centre.
	 */
	Point gOffset;

	/**
         Vertical and horizontal increment values.
	 */
	int hInc = 16, vInc = 16;

protected:
	/**
         Updates the position of the draggable scroller button dependin on current position and orientation.
	 */
	//TODO
	//      void CalculateGripperPosition();
	/**
         Updates the size and position of all the widgets depending on the
         size of the scroller and its orientation. If the scroller is horizontal
         its height is set to the height of the background bitmap othwerwise its
         width is set to the width if the background bitmap.
	 */
	//TODO
	//      void UpdateSize();
	/**
         Returns false to indicate the scroller itself can't be tabbed to.
         Instead the underlaying widgets can be tabbed to.
	 */
	//TODO
	//      bool MsgWantTab();

public:
	void SetIncrement(int value) {} //TODO

	Scroller();
	virtual ~Scroller() {};

	/**
         Shortcut for Widget::Setup(), SetRange(), SetPosition() and SetOrientation().
	 */
	//TODO
	//      void Setup(int x, int y, int w, int h, int key, int flags, int totalItems, int visibleItems, int pos, int o);

	/**
         Main event handler.
         Handles events such as button clicks and keypresses so that the scroller
         is updated whenever a button is clicked or dragged, etc.
	 */
	//TODO
	//      void HandleEvent(Widget& obj, int msg, intptr_t arg1=0, intptr_t arg2=0);

	//TODO
	//      void OriginalHandleEvent(Widget& obj, int msg, intptr_t arg1=0, intptr_t arg2=0);

	/**
         Sets the Scroller's orientation. Pass 0 for a vertical Scroller, 1 for a horizontal one.
	 */
	virtual void SetOrientation(int o);
	/**
         Returns the orientation of the scroller.
	 */
	virtual int GetOrientation() { return orientation; }
	/**
         Sets the range of values in which the scroller may be scrolled.
         Note that this function takes different parameters than
         ScrollObject::SetRange(min, max). TotalItems is the number of all
         items the scroller can scroll (min is implicitlly 0 and max equals
         totalItems) and visibleItems is the number of items the scrolled
         object is capable of displaying on one screen. This is needed
         because the little draggable scroll button is sized proportionally
         to the number of currently visible items.
	 */
	void SetRange(int totalItems, int visibleItems);
	/**
         Returns the number of items the object attached to the scroller can display
         on one "screen".
	 */
	//TODO
	//      int GetVisibleItems();
	/**
         Sets the position of the scroller.
	 */
	void SetPosition(int pos);
	void setRangeAndPosition(int pos, int gripWidth, int max);
	/**
         Moves the scroller position right or up.
	 */
	//TODO
	//      void Increment();
	/**
         Moves the scroller position left or down.
	 */
	//TODO
	//void Decrement();

	void SetHorizontalIncrement( int inc ) { hInc = inc; }
	void SetVerticalIncrement( int inc ) { vInc = inc; }
	int GetHorizontalIncrement( void ) { return hInc; }
	int GetVerticalIncrement( void ) { return vInc; }

	virtual std::string const className() const override { return "Scroller"; }
	virtual void handleMessage(ComponentPtr src, int msg) override;

};

/** A scrollbox decorates a viewport with optional scrollbars */
class ScrollBox : public Container, public DataListener
{
	std::shared_ptr<Scroller> hScroller;
	std::shared_ptr<Scroller> vScroller;
	std::shared_ptr<ViewPort> viewport;
	std::shared_ptr<Panel> panel;

	// horizontal / vertical scrollbar on / off / auto
	int hopt, vopt;

	// size of the panel edges
	int bufb, bufr, bufl, buft;

	// scrollers on or off
	bool hashscroller, hasvscroller;

	int GetHScrollerSize() { return hashscroller ? hScroller->geth() : 0; }
	int GetVScrollerSize() { return hasvscroller ? vScroller->getw() : 0; }
	void updateGripPositions();
	void updateScrollerPositions();
protected:

	virtual void handleMessage(ComponentPtr src, int msg) override;
public:
	int GetClientHeight() { return viewport->getClientWidth(); }
	int GetClientWidth() { return viewport->getClientHeight(); }
	bool IsInClientArea(const Point &p) const;
	std::shared_ptr<ViewPort> getViewPort() { return viewport; }

	enum { MSG_SCROLL_UP = MSG_SUSER /* TODO... define in dialog.h? */, MSG_SCROLL_DOWN };
	enum { SCROLLER_OFF, SCROLLER_ON, SCROLLER_AUTO };

	int GetXOffset() const;
	int GetYOffset() const;
	void SetScrollSize (int w, int h);
	void ScrollToArea (int x, int y, int w, int h);
	void Center();
	void SetPos(int x, int y);

	ScrollBox(ComponentPtr client = nullptr);

	void setClient(ComponentPtr client) { viewport->setChild(client); }

	void SetScrollOption (int newhopt, int newvopt);
	int GetHScrollOption() { return hopt; }
	int GetVScrollOption() { return vopt; }

	static ComponentBuilder<ScrollBox> build(ComponentPtr child = nullptr) { return ComponentBuilder<ScrollBox>(std::make_shared<ScrollBox>(child)); }
	virtual std::string const className() const override { return "ScrollBox"; }
	virtual void changed (int code = 0) override;
};
