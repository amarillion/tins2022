#pragma once

/*
 * Easing func: translates time component between 0 (start) and 1 (end) to a
 * value between 0 (start) and 1 (end). The result may overshoot and undershoot,
 * i.e. generate values outside the range 0 and 1, as long as the end points
 * are fixed, i.e. func(0) -> 0 and func(1) -> 1.
 */
typedef double (*EasingFuncPtr)(double);

double linear(double val);
double overshoot(double val);
double sigmoid(double val);
double exponential(double val);
double bounce(double x);
double makeAppearance(double x);
