// 2-D Point functions

#include "point.hpp"

// Euclidean distance
double
Point::distance(double x1, double y1, double x2, double y2)
{
	double d,t;

	t = x1-x2;
	t *= t;
	d = t;
	t = y1-y2;
	t *= t;
	d += t;
	return(sqrt(d));
}
