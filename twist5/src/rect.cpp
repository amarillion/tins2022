#include "rect.h"
#include "strutil.h"

using namespace std;

const string Rect::toString() const {
	return string_format("[%i,%i %ix%i]", origin.x(), origin.y(), dim.x(), dim.y()); 
}
