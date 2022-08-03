#pragma once

#include <list>
#include "timer.h"
#include <allegro5/allegro.h>

class Anim;

class ObjectBase
{
private:
	bool alive;
	bool visible;
public:
	bool isAlive() { return alive; }
	bool isVisible() { return visible; }
	void setVisible(bool value) { visible = value; }
	void kill() { alive = false; }
	virtual void update() {};
protected:
	int animstart;
	int animstate;
	Anim *current;

	static bool *debugFlag;
	static ITimer *timer;
	double x, y;
	int dir;
	int w, h;
	bool solid;
	virtual void setAnim (Anim *a, int state = 0);
public:
	ObjectBase () : alive(true), visible(true), animstart(0), animstate(0), current(NULL), x(0), y(0), dir(0), w(0), h(0), solid(true) {}
	double getx () { return x; }
	double gety () { return y; }
	int getw () { return w; }
	int geth () { return h; }
	bool isSolid() { return solid; } // can collide with other stuff?
	void setSolid(bool value) { solid = value; }
	virtual void draw (const GraphicsContext &gc);
	virtual void handleCollission(ObjectBase *o) = 0;
	virtual int getType () = 0;
	int getDir() { return dir; }
	void setState(int val) { animstate = val; } 
	virtual void setDir(int _dir) { dir = _dir; }
	virtual void setLocation (double nx, double ny);
	static void init(bool *_debugFlag, ITimer *_timer) { debugFlag = _debugFlag; timer = _timer; }
};

/**
 * Simple list of game objects.
 *
 * Game object must implement:
 * - isAlive(), isAwake, isVisible()
 * - update()
 * - draw()
 */
template <class T>
class SimpleObjectList : public Component {
protected:
	std::list<T *> objects;

	void purge() {
		objects.remove_if ([&](T *o){
			if (!o->isAlive()) { delete o; return 1; }
			return 0;
		});
	}

	virtual void onAdd(T *o) {};
	virtual void onUpdate() {};

public:
	/** can't be overridden. Override onUpdate instead */
	virtual void update() override final {
		for (auto i : objects) {
			if (i->isAlive()) i->update();
		}
		onUpdate();
		purge();
	}

	virtual void draw (const GraphicsContext &gc) override {
		for (auto o : objects) {
			if (o->isVisible() && o->isAlive()) {
				o->draw(gc);
			}
		}
	}

	void killAll() {
		for (auto i : objects) {
			delete (i);
		}
		objects.clear();
	}

	unsigned int size () { return objects.size(); }

	/** can't be overridden. Override onAdd instead */
	void add(T *o) {
		objects.push_back(o);
		onAdd(o);
	}
};

/**
 * Also handles collision detection.
 *
 * Game object must also implement:
 *
 *  getx(), gety(), getw(), geth() //TODO: overlap???
 *  handleCollission() // TODO: callback???
 *
 *  */
template <class T>
class ObjectListBase : public SimpleObjectList<T>
{
public:
	virtual ~ObjectListBase() {}
	virtual void onUpdate() {
		// collission detection!
		for (auto i = this->objects.begin(); i != this->objects.end(); i++)
			for (auto j = this->objects.begin(); j != i; j++)
		{
			// see if bb interesect
			if ((*i)->isAlive() && (*j)->isAlive())
			{
				int x1 = (*i)->getx();
				int y1 = (*i)->gety();
				int w1 = (*i)->getw();
				int h1 = (*i)->geth();
				int x2 = (*j)->getx();
				int y2 = (*j)->gety();
				int w2 = (*j)->getw();
				int h2 = (*j)->geth();
				if(!((x1 >= x2+w2) || (x2 >= x1+w1) || (y1 >= y2+h2) || (y2 >= y1+h1)))
				{
					(*i)->handleCollission ((*j));
					(*j)->handleCollission ((*i));
				}
			}
		}

	}
};
