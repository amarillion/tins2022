#include "motionimpl.h"
#include <math.h>

#include "util.h" // for M_PI

int Lissajous::getdx(int counter)
{
	double phase = M_PI * 2.0 * double(counter) / double(xperiod);
	return xampl * sin(phase);
}

int Lissajous::getdy(int counter)
{
	double phase = M_PI * 2.0 * double(counter) / double(yperiod);
	return yampl * cos(phase);
}

int Linear::getdx(int counter)
{
	return counter * dx;
}

int Linear::getdy(int counter)
{
	return counter * dy;
}

int Quake::getdx(int counter)
{
	int delta = counter - startTime;
	float phase = 1.5 * delta;
	float ampl = magnitude - (0.2 * delta); // decay
	if (ampl < 0) return 0;

	float xval = ampl * sin(phase * 1.3);
	return (int)xval;
}

int Quake::getdy(int counter)
{
	int delta = counter - startTime;
	float phase = 1.5 * delta;
	float ampl = magnitude - (0.2 * delta); // decay
	if (ampl < 0) return 0;

	float yval = ampl * sin(phase * 0.9);
	return (int) yval;
}

void Quake::start(float _magnitude, int counter)
{
	magnitude = _magnitude;
	startTime = counter;
}
