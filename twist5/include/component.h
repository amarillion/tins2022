#pragma once

#include "graphicscontext.h"
#include <list>
#include "motion.h"
#include <memory>
#include "motionimpl.h"
#include <allegro5/allegro.h>
#include <iostream>

/**
 * Attempt to convert a MSG_XXX constant to a string for debugging purposes.
 */
std::string msgToString(int msg);

struct ALLEGRO_FONT;

class Layout {

public:

	// implementation constants, don't use directly
	enum _Impl {
		LEFT   = 1,
		RIGHT  = 2,
		TOP    = 4,
		_W      = 16,
		BOTTOM = 8,
		_H      = 32,
		CENTER = 64,
		MIDDLE = 128,
		TO_BOTTOM = 256,
		TO_RIGHT = 512,
		_DISABLED = 1024
	};


	enum {
		LEFT_TOP_W_H          = _Impl::LEFT   | _Impl::TOP    | _Impl::_W       | _Impl::_H,
		LEFT_TOP_RIGHT_H      = _Impl::LEFT   | _Impl::TOP    | _Impl::TO_RIGHT | _Impl::_H,
		LEFT_TOP_W_BOTTOM     = _Impl::LEFT   | _Impl::TOP    | _Impl::_W       | _Impl::TO_BOTTOM,
		LEFT_TOP_RIGHT_BOTTOM = _Impl::LEFT   | _Impl::TOP    | _Impl::TO_RIGHT | _Impl::TO_BOTTOM,
		LEFT_MIDDLE_W_H       = _Impl::LEFT   | _Impl::MIDDLE | _Impl::_W       | _Impl::_H,
		LEFT_MIDDLE_RIGHT_H   = _Impl::LEFT   | _Impl::MIDDLE | _Impl::TO_RIGHT | _Impl::_H,
		LEFT_BOTTOM_W_H       = _Impl::LEFT   | _Impl::BOTTOM | _Impl::_W       | _Impl::_H,
		LEFT_BOTTOM_RIGHT_H   = _Impl::LEFT   | _Impl::BOTTOM | _Impl::TO_RIGHT | _Impl::_H,
		CENTER_TOP_W_H        = _Impl::CENTER | _Impl::TOP    | _Impl::_W       | _Impl::_H,
		CENTER_TOP_W_BOTTOM   = _Impl::CENTER | _Impl::TOP    | _Impl::_W       | _Impl::TO_BOTTOM,
		CENTER_MIDDLE_W_H     = _Impl::CENTER | _Impl::MIDDLE | _Impl::_W       | _Impl::_H,
		CENTER_BOTTOM_W_H     = _Impl::CENTER | _Impl::BOTTOM | _Impl::_W       | _Impl::_H,
		RIGHT_TOP_W_H         = _Impl::RIGHT  | _Impl::TOP    | _Impl::_W       | _Impl::_H,
		RIGHT_TOP_W_BOTTOM    = _Impl::RIGHT  | _Impl::TOP    | _Impl::_W       | _Impl::TO_BOTTOM,
		RIGHT_MIDDLE_W_H      = _Impl::RIGHT  | _Impl::MIDDLE | _Impl::_W       | _Impl::_H,
		RIGHT_BOTTOM_W_H      = _Impl::RIGHT  | _Impl::BOTTOM | _Impl::_W       | _Impl::_H,
		DISABLED              = _Impl::_DISABLED

	};
};

/** Maximum value you can use for setGroupId */
const int MAX_GROUP_ID = 16;

class Reflectable
{
public:
	virtual std::string const className() const = 0;
	virtual ~Reflectable() {}
};

// copied from a4.
/* bits for the flags field */
#define D_EXIT          1        /* object makes the dialog exit */
#define D_SELECTED      2        /* object is selected. For toggle buttons / menu's, this means that the toggle is active. */
#define D_GOTFOCUS      4        /* object has the input focus */
#define D_GOTMOUSE      8        /* mouse is on top of object */
#define D_HIDDEN        16       /* object is not visible */
#define D_DISABLED      32       /* object is visible but inactive. The component may be 'greyed out' */
#define D_DIRTY         64       /* object needs to be redrawn. We're not doing dirty rectangles anymore,
                                    but this is still used for double-buffered components */
#define D_INTERNAL      128      /* reserved for internal use */
#define D_USER          256      /* from here on is free for your own use */

/**
 * widget is drawn to a backbuffer, that is only updated when the dirty flag is set.
 * This option is available for Widgets and up.
 */
#define		D_DOUBLEBUFFER		D_USER<<1

#define D_DISABLE_CHILD_CLIPPING	D_USER<<2 /* for container, do not clip children by bounding box */

/**
	set when the widget is being pressed down
	In MASkinG the use seems to be for when the focus is locked / mouse is captured.
*/
#define		D_PRESSED			D_USER<<6

/**
	set if the widget is resized automatically. For example a label might resize itself
	automatically according to the length of the text or a button might
	automatically resize itself according to the size of its bitmap.
*/
#define		D_AUTOSIZE			D_USER<<7

/**
	set if the widget can be toggled (a toggle button for example)
*/
#define		D_TOGGLE			D_USER<<8


/**
	set this flag to make the widget read-only; currently this is only used
	by the EditBox widget
*/
#define		D_READONLY			D_USER<<13

//TODO: also inherit from Updateable.
class Component : public std::enable_shared_from_this<Component>, public Reflectable
{
private:
	// messages destined for the parent.
	std::list<int> msgQ;

	/** layout flags determine how the layout coordinates are applied (i.e. align top-left, center, fill horizontally, etc.) */
	int layout_flags;

	/** layout_initialised indicates that doLayout has been called by the parent,
	 * and so we can expect reasonably valid coordinates to have been set. This is used by Container to
	 * determine if it's ok to call doLayout with its own coordinates. */
	bool layout_initialised;

	int layout_x1, layout_y1, layout_x2, layout_y2;

	int flags;

protected:
	// if awake is set to false, update() will not be called.
	bool awake;
	// if alive is set to false, this component will be removed
	// neither update() nor draw() will be called.
	bool alive;

	ALLEGRO_FONT *sfont;

	//TODO: perhaps the Motion should get the counter and not the component.
	int counter;

	IMotionPtr motion;

	double x, y;
	int w, h;

	int groupId;
	/**
	 * Add a message to the queue. The message queue will be read by this component's parent.
	 * pushMsg() may be called from any method including handleEvent, handleMessage and update.
	 */
	void pushMsg(int msg) {
		msgQ.push_back(msg); assert (msgQ.size() < 10);
	}

	/** Helper used by Containers to handle messages. Check the messages of a (child) component and call handleMessage if there are any in the queue */
	void checkMessages(std::shared_ptr<Component> ptr);
public:
	void SetFlag(int flag);
	void ClearFlag(int flag);
	bool TestFlag(int flag) const;
	void ToggleFlag(int flag);
	void setFlagVal(int flag, bool value);

	int Flags() const { return flags; }

	/**
	 * Is set to true after the parent has called doLayout on this component.
	 */
	bool isLayoutInitialised() { return layout_initialised; }

	/**
	 * For most components, the preferred width is the same as the width.
	 * For certain components such as labels, it's possible to resize them to fit the contents.
	 * In those cases this may be overridden to return a more optimal preferred width
	 */
	virtual double getPreferredWidth() { return w; }

	/*
	 * Check if there are any messages queued.
	 * Should be called by this component's parent.
	 */
	bool hasMsg() { return !msgQ.empty(); }

	/*
	 * Pop a message from the queue
	 * Should be called by this component's parent.
	 */
	int popMsg() { int msg = msgQ.front(); msgQ.pop_front(); return msg; }


	//TODO: MSG constants are partially defined here, partially as #defines in widget.h
	enum { MSG_FOCUS = 2000, MSG_UNFOCUS, MSG_KILL };

	virtual ~Component() {}

	Component () :
		layout_flags(Layout::LEFT_TOP_RIGHT_BOTTOM), layout_initialised(false), layout_x1(0), layout_y1(0), layout_x2(0), layout_y2(0),
		flags(0),
		awake(true), alive(true), sfont(NULL), counter(0), motion(),
		x(0), y(0), w(0), h(0),
		groupId(0)
	{}


	/**
	 * update, called for each heartbeat tick.
	 *
	 * Don't override this, override onUpdate, unless you want to change behavior drastically
	 */
	virtual void update() { counter++; onUpdate(); }

	/**
	 * update, called for each heartbeat tick.
	 * Designed to be overridden.
	 */
	virtual void onUpdate() {}

	virtual void draw(const GraphicsContext &gc);

	void setMotion(const IMotionPtr &value) { motion = value; }
	IMotionPtr getMotion() { return motion; }

	//int getCounter() { return counter; }

	/**
	 * handle a control message.
	 * Override onHandleMessage, unless you wish to alter behavior drastically
	 */
	virtual void handleMessage(std::shared_ptr<Component> src, int msg);

	/**
	 * Can be used to handle a keyboard event even if this component does not have the focus.
	 *
	 * CURRENTLY, this is called on all siblings of a focused component after any key event. Returning true does not (yet) affect this behavior.
	 *
	 * @return true if this component was triggered.
	 */
	virtual bool MsgXChar(int scancode, int modifiers) { return false; }

	/**
	 * handle Control message. destined for overriding.
	 *
	 * return true if the message has been handled, otherwise,
	 * the message will be passed on to the parent of this component.
	 */
	virtual bool onHandleMessage(std::shared_ptr<Component> src, int msg) { return false; };

	/**
	 * Called whenever this component receives a MSG_FOCUS message.
	 * By default, component will become awake and visible.
	 *
	 * Override this to do additional initialisation when focus is gained.
	 */
	virtual void onFocus() { }

	/**
	 * Override this,
	 *
	 * 	returns true in case this element can receive focus, and will be picked when searching for the next
	 * 	focus item after pressing tab.
	 *
	 * 	returns false in case this component can not receive focus, will be skipped when searching for next
	 * 	focus item after pressing tab.
	 *
	 */
    virtual bool wantsFocus () { return false; }

	/** handle input event. */
	virtual void handleEvent(ALLEGRO_EVENT &event) {}

	// if awake is set to false, update() will not be called.
	bool isAwake() const { return awake; }
	void setAwake(bool value) { awake = value; }

	// if alive is set to false, this component will be removed
	// neither update() nor draw() will be called.
	bool isAlive() const { return alive; }
	void kill() { alive = false; }
	void respawn() { alive = true; }

	// if visible is set to false, draw() will not be called
	bool isVisible() const { return !TestFlag(D_HIDDEN); }
	void setVisible(bool value) { setFlagVal(D_HIDDEN, !value); }

	virtual void setFont(ALLEGRO_FONT *font) { this->sfont = font; }
	ALLEGRO_FONT *getFont() { return sfont; }

	double gety() { return y; }
	double getx() { return x; }
	int getw () const { return w; }
	int geth () const { return h; }
	void sety(double _y) { y = _y; }
	void setx(double _x) { x = _x; }

	double getRight() { return x + w; }
	double getBottom() { return y + h; }
	double getTop() { return y; }
	double getLeft() { return x; }

	void seth(int _h) { h = _h; UpdateSize(); }
	void setxy(double _x, double _y) {
		layout_flags = Layout::DISABLED;
		x = _x; y = _y; UpdateSize();
	}
	void setLocation (double _x, double _y, int _w, int _h) {
		layout_flags = Layout::DISABLED;
		x = _x; y = _y; w = _w; h = _h; UpdateSize();
	}
	void setDimension (int _w, int _h)
	{
		w = _w;
		h = _h;
		UpdateSize();
	}

	void setLayout (int flags, int x1, int y1, int x2, int y2)
	{
		layout_flags = flags;
		layout_x1 = x1;
		layout_y1 = y1;
		layout_x2 = x2;
		layout_y2 = y2;
	}

	/** calculate position based on parent size. Must be invoked by the parent Container. After calling this at least once, layout_initialised will be true. */
	void doLayout(int px, int py, int pw, int ph);

	/** set group id. Note that value of 0 means 'no group' */
	void setGroupId(int val) { assert (groupId >= 0 && groupId < MAX_GROUP_ID); groupId = val; }
	int getGroupId() { return groupId; }
protected:
	/** NB: preferred method for Widget and up is to override 'onResize' This is called at least once at initialisation time */
	virtual void UpdateSize() {}

	// helper for repr(), without whitespace
	virtual void reprBase(std::ostream &out) const;
public:

	/**
	check if the point cx, cy is within the bounding box of the gui item.
	usefull for checking if the mouse is over the gui item.
	*/
	bool contains (int cx, int cy) {
	    return (cx >= x && cy >= y && cx < (x + w) && cy < (y + h));
	}

	/** write stats on this component for debugging purposes */
	virtual void repr(int indent, std::ostream &out) const;

	virtual std::string const className() const override { return "Component"; }
};

typedef std::shared_ptr<Component> ComponentPtr;
