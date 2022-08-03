#pragma once

#include <assert.h>
#include <stddef.h>

template <class T> class Map3D
{
private:
	T *data = nullptr;

	// dimensions of the map in map units
	size_t dim_mx;
	size_t dim_my;
	size_t dim_mz;

	inline int indexFromXYZ(int mx, int my, int mz) const
	{
		return mx + dim_mx * (my + dim_my * mz);
	}

public:
	Map3D (int xsize, int ysize, int zsize)
	{
		assert (xsize >= 0);
		assert (ysize >= 0);
		assert (zsize >= 0);
		data = new T[xsize * ysize * zsize];
		dim_mx = xsize;
		dim_my = ysize;
		dim_mz = zsize;
	}

	~Map3D()
	{
		if (data != nullptr) delete[] data;
	}

	T &get (int mx, int my, int mz)
	{
		assert (inBounds(mx, my, mz));
		assert (data);
		return data[indexFromXYZ(mx, my, mz)];
	}

	const T &get (int mx, int my, int mz) const
	{
		assert (inBounds(mx, my, mz));
		assert (data);
		return data[indexFromXYZ(mx, my, mz)];
	}

	/** use parentheses to simulate array notation */
	T& operator()(size_t mx, size_t my, size_t mz)
	{
		assert (inBounds(mx, my, mz));
		assert (data);
		return data[indexFromXYZ(mx, my, mz)];
	}

	/** use parentheses to simulate array notation */
	const T& operator()(size_t mx, size_t my, size_t mz) const
	{
		assert (inBounds(mx, my, mz));
		assert (data);
		return data[indexFromXYZ(mx, my, mz)];
	}

	size_t getDimMX() const { return dim_mx; }
	size_t getDimMY() const { return dim_my; }
	size_t getDimMZ() const { return dim_mz; }

	inline bool inBounds(int mx, int my, int mz) const
	{
		return
				(mx >= 0) && (my >= 0) && (mz >= 0) &&
				(mx < (int)dim_mx) && (my < (int)dim_my) && (mz < (int)dim_mz);
	}

};
