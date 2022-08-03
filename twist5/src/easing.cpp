
#include "easing.h"

#define _USE_MATH_DEFINES // enables contants like M_E
#include <cmath> // std::abs
#include <algorithm> // std::min

using namespace std;

/*
https://develop.open.wolframcloud.com/
Plot[1./(1. + E^(-10. (-0.5 + x))), {x, 0, 1.0}]
Plot[-2.667*x^2  + 3.667*x , {x, 0, 1.0}]
Plot[((E^(x))-1.0)/(E-1.0), {x, 0, 1.0}]
*/

/**
 * Each step, output increases by a constant multiplier
 * Useful for zooming.
 */
double exponential(double val) {
	return (exp(val)-1.0)/(M_E-1.0);
}

/** 0 to 1, stay on 1, then back again */
double makeAppearance(double x) {
	if (x < 0.3) return x / 0.3;
	if (x > 0.7) return (1 - x) / 0.3;
	return 1;
}

/**
 *  Combines polynomial with abs() to get bounce effect.
 * Bounces back two times before settling on target.
 */
double bounce(double x) {
	double x2 = x*x;
	return 1.0 - abs(1.0 - 16.2*x2*x2 + 31.3*x2*x -15.5*x2 - 0.6*x);
}

/**
 * Linear movement
 *
 * f(x) = x
 */
double linear(double val) {
	return val;
}

/**
 *
 * Overshoot the goal, then return to it.
 * Based on quadratic curve
 *
 * f(x) = ax2 + bx + c
 * f(0) = 0
 * f(1) = 1
 * f(0.75) = 1.25
 *
 * c = 0
 * a + b = 1
 * b = 1 - a
 *
 * a * -0.1875 = 0.5
 *
 * a = -2.667
 * b = 3.667
 */
double overshoot(double val) {
	return (val * val * -2.667) + (val * 3.667);
}

/**
 * Sigmoid curve
 * Makes the object move slow-fast-slow
 *
 * Using logistic function:
 * f(x) = L / (1 + exp(-k * (val - x0))
 * L -> maximum  = 1
 * k -> steepness = 10.0
 * x0 -> x-value of mid-point = 0.5
 * f(0) = ~0.01;
 * f(1) = ~0.99
 * f(0.5) = 0.5
 *
 */
double sigmoid(double val) {
	return 1.0 / (1.0 + exp(-10.0 * (val - 0.5)));
}
