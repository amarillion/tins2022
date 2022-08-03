#pragma once

#include "deprecated.h"
#include "assert.h"

class Rect;

template<typename T>
class Vec2
{
protected:
	/** The underlaying x coordinate. */
	T posx;

	/** The underlaying y coordinate. */
	T posy;
public:
	/** Default constructor. */
	Vec2(T x = 0, T y = 0) : posx(x), posy(y) {}
	
	void operator+=(const Vec2<T> &p) {
		posx += p.x();
		posy += p.y();
	}
	
	void operator-=(const Vec2<T> &p) {
		posx -= p.x();
		posy -= p.y();
	}
	

	/**
         Operators for doing simple vector arithmetic with two points.
         Examples:
<pre>
      Point a(10, 20);
      Point b(30, 40);

      Point c = a + b;   // c = (40, 60)
      Point d = b - a;   // d = (20, 20)
      c += a;            // c = (50, 80)
      c -= b;            // c = (20, 40)
</pre>
      */
	Vec2<T> operator-(const Vec2<T> &p) const {
		Vec2<T> res;
		res.x(posx - p.x());
		res.y(posy - p.y());
		return res;
	}

	Vec2<T> operator+(const Vec2<T> &p) const {
		Vec2<T> res;
		res.x(p.x() + posx);
		res.y(p.y() + posy);
		return res;
	}

	/**
	 * Multiply by scalar of any type (int, double, etc)
	 */
	template<typename U>
	Vec2<T> operator*(U scalar) const {
		Vec2<T> res;
		res.x(posx * scalar);
		res.y(posy * scalar);
		return res;
	}

	bool operator==(const Vec2<T> &p) const {
		return (x() == p.x()) && (y() == p.y());
	}

	bool operator!=(const Vec2<T> &p) const {
		return (x() != p.x()) || (y() != p.y());
	}

	/* Get and set functions */
	T x() const { return posx; }
	T y() const { return posy; }
	void x(T v) { posx = v; }
	void y(T v) { posy = v; }

	/* Alternatives if you want to use this as a size */
	DEPRECATED T w() const { return posx; }
	DEPRECATED T h() const { return posy; }
	DEPRECATED void w(T v) { posx = v; }
	DEPRECATED void h(T v) { posy = v; }
};

typedef Vec2<int> Point;
typedef Vec2<float> Vec2f;
DEPRECATED typedef Point Size;
