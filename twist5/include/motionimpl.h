#pragma once

#include "motion.h"

//TODO: DEPRECATED. replace with external updater like in Peppy.

/**
	continous motion, x and y axis 
	follow a sine and cosine with independent periods.
	If periods of x and y are the same then the motion is circular.
	See also: http://en.wikipedia.org/wiki/Lissajous_curve
*/
class Lissajous : public IMotion
{
private:
	int xperiod;
	int yperiod;
	int xampl;
	int yampl;
public:
	Lissajous(int _xperiod, int _xampl, int _yperiod, int _yampl) :
		xperiod (_xperiod), yperiod(_yperiod), xampl (_xampl), yampl (_yampl)
	{ }
	
	virtual int getdx(int counter);
	virtual int getdy(int counter);
};

class Linear : public IMotion
{
private:
	int dx;
	int dy;
public:
	Linear (int _dx, int _dy) : dx(_dx), dy(_dy) {}

	virtual int getdx(int counter);
	virtual int getdy(int counter);
};

class Quake : public IMotion
{
private:
	float magnitude;
	int startTime;
public:
	void start(float magnitude, int counter);

	virtual int getdx(int counter);
	virtual int getdy(int counter);
};
