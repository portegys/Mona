// 2-D Point definitions

#ifdef UNIX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <cstdio>
#include <cstdlib>
#include <string>
#endif
#include <math.h>
#include <assert.h>
#include "common.h"

class Point
{

public:

	// Coordinates
	double x, y;

	// Constructors
	Point() { Point(0.0, 0.0); }
	Point(int px, int py) { Point((double)px, (double)py); }
	Point(double px, double py)
	{
		x = px;
		y = py;
	}

	// Euclidean distance
	double distance(Point p) { return(distance(*this, p)); }
	double distance(double px, double py)
	{
		Point p;
		p.x = px;
		p.y = py;
		return(distance(*this, p));
	}
	double distance(Point p1, Point p2) { return(distance(p1.x, p1.y, p2.x, p2.y)); }
	double distance(double x1, double y1, double x2, double y2);
};

typedef class Point POINT;
