#include "timer.h"
#include <stdio.h>

void Timer::update()
{
	if (++counter >= maxCounter)
	{
		if (mode == ONCE)
		{
			kill(); // timer kills itself
		}
		else
		{
			counter = 0;
		}

		if (action)
		{
			action();
		}
		else
		{
			pushMsg(msg);
		}
	}
}

Timer::~Timer() {}
