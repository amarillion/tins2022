#pragma once

#include <string>
#include <vector>
#include <assert.h>

struct ALLEGRO_CONFIG;

template <typename T>
static inline T bound (T _min, T val, T _max)
{
	return std::min(std::max (_min, val), _max);
}

/** generate a random number in the range [0, i) */
int random (int i);

// pick one  at random from a vector
template<typename T>
T choice(const std::vector<T> &aContainer) {
	return aContainer[random(aContainer.size())];
}

int get_config_int(ALLEGRO_CONFIG *config, const char *section, const char *key, int defaultValue);
void set_config_int(ALLEGRO_CONFIG *config, const char *section, const char *key, int value);
std::string get_config_string(ALLEGRO_CONFIG *config, const char *section, const char *key, const char *defaultValue);

// see: http://stackoverflow.com/questions/3599160/unused-parameter-warnings-in-c-code
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

void allegro_message(const char *msg, ...);

// adds all filenames that match pattern to the vector
void glob(const std::string &, std::vector<std::string> &result);
void listDir(const std::string &dir, std::vector<std::string> &result);

// not defined for mingw...
// not officially part of standard anymore.
#ifndef M_PI
#define M_E 2.71828182845904523536
#define M_LOG2E 1.44269504088896340736
#define M_LOG10E 0.434294481903251827651
#define M_LN2 0.693147180559945309417
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.785398163397448309616
#define M_1_PI 0.318309886183790671538
#define M_2_PI 0.636619772367581343076
#define M_1_SQRTPI 0.564189583547756286948
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT_2 0.707106781186547524401
#endif

/**
 *  Like al_set_clipping_rectangle, but with transformation.
 *  al_set_clipping_rectangle normally ignores transformations on a bitmap.
 *
 *	Apply transformations to the coordinates, then set clipping rectangle.
 */
void clip_with_transform(float cx1, float cy1, float cx2, float cy2);

/**
 * Simplistic implementation just prints to STDERR.
 * The intention is to make this write to a log file eventually,
 * and to have multiple log levels: info, error, warn
 */
void log(const char* msg, ...);

// add quotes to a preprocessor symbol
#define XQUOTE(name) QUOTE(name)
// add quotes around some literal text
#define QUOTE(name) #name

// Assert with message
#define Assert(cond, msg) assert(cond && msg)

/* generate 16 random hex characters that can be used as unique id. */
std::string generateRandomId();

/*
 pick an index from a list of weights.
 for example, if weights are 50, 30, 10, 10, then there is a 50% chance that this returns 0, 30% chance
 that it returns 1, etc.
 weights do not need to add up to 100.
*/
int pickWeightedRandom(std::vector<int> const &actionWeights);
