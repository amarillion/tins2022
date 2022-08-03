#pragma once

#include <list>
#include "widget.h"
#include <functional>

#include "componentbuilder.h"
#include "rect.h"
#include "updateable.h"

typedef std::function <const Rect (ComponentPtr comp, ComponentPtr prev, int idx, int size, const Rect &p)> LayoutFunction;

/**
 * A Container groups child widgets
 * unlike specializations like Dialog, it doesn't do anything special about focus.
 */
class Container : public Widget
{
protected:
	std::list<ComponentPtr> children;
	UpdateableList updateables;
	ComponentPtr focus;
	ComponentPtr mouseFocus;
	LayoutFunction groupLayouts[MAX_GROUP_ID];
public:
	enum {
		/** use add (..., FLAG_SLEEP) to add an element without giving it focus, to be awoken later. */
		FLAG_SLEEP = 1,
		/** use add (..., FLAG_BOTTOM) for it to be drawn as background */
		FLAG_BOTTOM = 2 };
	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override;
	virtual void handleEvent (ALLEGRO_EVENT &evt) override;
	virtual void setFont(ALLEGRO_FONT *font) override;
	DEPRECATED void setTimer(int msec, int event);
	void setTimer(int msec, ActionFunc event, bool repeating = false);
	virtual void purge();
	virtual void killAll();
	virtual void add (ComponentPtr item, int flags = 0);
	virtual void add (UpdateablePtr item);
	void setFocus(ComponentPtr _focus);
	void setGroupLayout (int groupId, LayoutFunction func) { groupLayouts[groupId] = func; }
	Container() : children(), focus(), mouseFocus(), groupLayouts{} { }
	virtual void handleMessage(ComponentPtr src, int msg) override;

private:

protected:
	/**
	 * override this to do extra processing after a component was added
	 */
	virtual void onAdd(ComponentPtr src) {}

	/** called once before any other events */
	virtual void MsgStart() override {}

	/** overriding disallowed. Override onResize() instead. */
	virtual void UpdateSize() override final;

public:
	virtual bool MsgXChar(int keycode, int modifiers) override;
	virtual void repr(int indent, std::ostream &out) const override;
	virtual std::string const className() const override { return "Container"; }

	void resizeToChildren();

	/** get the maximum right value of all children */
	double getMaxRight();

	/** get the minimum left value of all children */
	double getMinLeft();


	/** get the minimum top value of all children */
	double getMinTop();

	/** get the maximum bottom value of all children */
	double getMaxBottom();

	static ComponentBuilder<Container> build()
	{
		return ComponentBuilder<Container>(std::make_shared<Container>());
	}
};

typedef std::shared_ptr<Container> ContainerPtr;
