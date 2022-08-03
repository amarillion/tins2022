#pragma once

#include <list>
#include <memory>

class Updateable {
private:
	bool alive = true;
public:
	virtual void update() = 0;
	void kill() { alive = false; }
	bool isAlive() { return alive; }
};

typedef std::shared_ptr<Updateable> UpdateablePtr;

class UpdateableList {
private:
	std::list<std::shared_ptr<Updateable>> items;
public:
	virtual void update() {
		for (auto &i : items) {
			if (i->isAlive()) i->update();
		}
		items.remove_if ( [](UpdateablePtr i) { return !(i->isAlive()); });
	}

	void push_back(const UpdateablePtr &val) {
		items.push_back(val);
	}
};
