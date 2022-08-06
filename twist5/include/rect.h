#pragma once

#include "point.h"
#include <string>

class Rect
{
protected:
	/**
         Top left point (origin).
      */
	Point origin;

	/**
         The size of the rectangle.
      */
	Point dim;
public:
	/**
         Default constructor.
      */
	Rect() : origin(0, 0),
			 dim(0, 0)
	{
	}

	/**
         Constructor for making a rectangle from coordinates of origin, width and height.
      */
	Rect(int x, int y, int w, int h) : origin(x, y),
									   dim(w, h)
	{
	}

	/**
         Constructor for making a rectangle from the origin point and size.
      */
	Rect(const Point &p, const Point &s) : origin(p),
										  dim(s) {}

	virtual ~Rect() {}

	/**
         Operator for comparing two Rects.
         Two rectangles are equal if they have the same origin point and size.
      */
	bool operator==(const Rect &r) const;

	/** */ void operator+=(const Point &p) { origin += p; }
	/** */ void operator-=(const Point &p) { origin -= p; }
	/** */ Rect operator+(const Point &p) const { return Rect(origin + p, dim); }
	/**
         Operators for doing simple arithmetic with Rects (i.e. moving).
      */
	Rect operator-(const Point &p) const { return Rect(origin - p, dim); }

	/**
         Returns true if this rectangle intersects with the passed rectangle (excluding the edges).
      */
	bool Intersects(const Rect &r) const;

	/** */ Point topLeft() const { return origin; }
	/** */ Point bottomRight() const { return Point(origin.x() + dim.x(), origin.y() + dim.y()); }
	/** */ Point size() const { return dim; }
	/** */ int x() const { return origin.x(); }
	/** */ int y() const { return origin.y(); }
	/** */ int x2() const { return origin.x() + dim.x(); }
	/** */ int y2() const { return origin.y() + dim.y(); }
	/** */ int w() const { return dim.x(); }
	/** */ int h() const { return dim.y(); }
	/** */ void topLeft(const Point &p) { origin = p; }
	/** */ void bottomRight(const Point &p)
	{
		x2(p.x());
		y2(p.y());
	}
	/** */ void size(const Point &s) { dim = s; }
	/** */ void x(int v) { origin.x(v); }
	/** */ void y(int v) { origin.y(v); }
	/** */ void x2(int v) { dim.x(v - origin.x()); }
	/** */ void y2(int v) { dim.y(v - origin.y()); }
	/** */ void w(int v) { dim.x(v); }
	/**
         Bsic get and set functions.
      */
	void h(int v) { dim.y(v); }

	const std::string toString() const;

	bool contains(const Point &p) const {
		auto norm = p - origin;
		return (norm.x() >= 0 && norm.y() >= 0 && norm.x() < dim.x() && norm.y() < dim.y());
	}
};
