#pragma once

#include "deprecated.h"
#include "component.h"
#include <memory>
#include <functional>

class ITimer
{
public:
	virtual int getMsecCounter() = 0;
};

typedef std::function<void()> ActionFunc;

class Timer : public Component //TODO: Updateable
{
	int counter;
	int msg;
	int maxCounter;
	int mode;
	ActionFunc action;
public:
	// use ActionFunc instead
	DEPRECATED 
	Timer(int maxCounter, int msg, int mode = ONCE) :
		counter(0), msg(msg), maxCounter(maxCounter), mode(mode), action()
		{ setAwake(true); setVisible(false); }

	Timer(int maxCounter, ActionFunc action, int mode = ONCE) :
		counter(0), msg(0), maxCounter(maxCounter), mode(mode), action(action) {
	}

	virtual ~Timer();
	virtual void update() override;

	enum { ONCE, REPEATING };

	virtual std::string const className() const override { return "Timer"; }

};

typedef std::shared_ptr<Timer> TimerPtr;
