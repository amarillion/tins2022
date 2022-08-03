#include "viewport.h"
#include "component.h"

using namespace std;

void ViewPort::setChild(ComponentPtr value)
{
	child = value;
	if (!child->getFont()) child->setFont(getFont());

	auto scrollable = dynamic_pointer_cast<Scrollable>(value);
	if (scrollable)
	{
		scrollable->setParent(this); //NOTE: weak pointer.
	}

	FireEvent(EVT_CHILD_CHANGED);
}

void ViewPort::update()
{
	Component::update();

	int dx = destx - basex;
	int dy = desty - basey;

	// if close by, move directly
	if ((dx * dx + dy * dy) < 20.0)
	{
		basex = destx;
		basey = desty;
	}
	else
	{
		// move there in increasingly smaller steps
		basex += dx / 20;
		basey += dy / 20;
	}

	int newxofst = basex + quake.getdx(counter);
	int newyofst = basey + quake.getdy(counter);

	if (xofst != newxofst || yofst != newyofst)
	{
		xofst = newxofst;
		yofst = newyofst;
		FireEvent(EVT_OFFSET_CHANGED);
	}

	if (child)
	{
		child->update();
		checkMessages(child);
	}
}

void ViewPort::moveTo(int _xofst, int _yofst)
{
	destx = _xofst;
	desty = _yofst;
}

void ViewPort::tremble(float _ampl)
{
	quake.start(_ampl, counter);
}

void ViewPort::draw(const GraphicsContext &gc)
{
	al_set_clipping_rectangle(getx(), gety(), getw(), geth());
	GraphicsContext gc2 = GraphicsContext();
	gc2.buffer = gc.buffer;
	gc2.xofst = getXofst() + getx();
	gc2.yofst = getYofst() + gety();

	if (child)
	{
		child->draw(gc2);
	}
}


void ViewPort::repr(int indent, std::ostream &out) const
{
	for (int i = 0; i < indent; ++i)
	{
		out << "  ";
	}

	out << className() << " [" << x << ", " << y << " - " << w << "x" << h << "] "
			<< (isVisible() ? "v" : "!v") << " "
			<< "ofst: " << xofst << ", " << yofst
			<< endl;

	if (child)
	{
		child->repr(indent + 1, out);
	}
}

void ViewPort::handleEvent (ALLEGRO_EVENT &event)
{
	ALLEGRO_EVENT event2;
	memcpy (&event2, &event, sizeof(ALLEGRO_EVENT));

	switch (event.type)
	{
	case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
	case ALLEGRO_EVENT_MOUSE_AXES:
	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
	{
		event2.mouse.x -= (xofst + getx());
		event2.mouse.y -= (yofst + gety());
	}
	break;
	}

	if (child)
	{
		child->handleEvent(event2);
		checkMessages(child);
	}
}

bool ViewPort::MsgXChar (int kc, int mod) {

	if (child) {
		bool result = child->MsgXChar(kc, mod);
		checkMessages(child);
		return result;
	}

	return false;
}

void Scrollable::setParent(ViewPort *value)
{
	scrollableParent = value;
}
