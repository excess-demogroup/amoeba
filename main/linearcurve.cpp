#include "main/linearcurve.h"
#include "demolib_prefs.h"
#include "exception.h"

#include <stdio.h>

#if DEMOLIB_MAINLOOP

LinearCurve::LinearCurve()
{
	this->num_points = 0;
}
LinearCurve::~LinearCurve() {}

void LinearCurve::add_curvepoint(float x, float y)
{
	if (this->num_points == MAX_POINTS)
		throw new FatalException("Maximum number of points reached!");

	/* check better for dupes later */
	this->x[this->num_points] = x;
	this->y[this->num_points] = y;
	this->num_points++;

	this->pos_hint = 0;
}
void LinearCurve::end_curvepoints(float start, float length)
{
	if (this->num_points == 0)
		throw new FatalException("No points for linear movement!");

	int i;
	for (i = 0; i < this->num_points; i++) {
		this->x[i] -= start;
		this->x[i] /= length;
	}

	/* whee, O(n^2) bubblesort */
	for (i = 0; i < this->num_points; i++) {
		for (int j = i; j < this->num_points; j++) {
			if (x[i] > x[j]) {
				float xt = x[j];
				float yt = y[j];

				x[j] = x[i];
				y[j] = y[i];

				x[i] = xt;
				y[i] = yt;
			}
		}
	}
}

float LinearCurve::get_value(float x) {
	if (this->num_points == 1) return this->y[0];

	/* extrapolation to the left */
	if (x < this->x[0]) {
		return (this->x[0] - x) * (this->y[1] - this->y[0]) / (this->x[1] - this->x[0]);
	}

	float leftx = 0.0f, rightx = 0.1f, lefty = 0.0f, righty = 0.0f;

	/*
	 * We have so many long linear curves we'll try to optimize this a bit:
	 * We check to see if the last used value is a usable starting point --
	 * if it is, we start from there, if not, we start from 0. we should
	 * also implement a binary search really soon. ;-) This will work real
	 * crappy if progress went backwards, but in 99% of the cases, it isn't. ;-)
	 */
	if (this->pos_hint == -1 || x > this->x[this->pos_hint]) {
		this->pos_hint = 0;
	}
	
	for (int i = this->pos_hint; i < this->num_points - 1; i++) {
		leftx = this->x[i];
		rightx = this->x[i + 1];
			
		lefty = this->y[i];
		righty = this->y[i + 1];
		
		if (x <= rightx) {
			this->pos_hint = i - 1;
			break;
		}
	}

	
	float t = (x - leftx) / (rightx - leftx);
	return lefty + (righty - lefty) * t;
}

#endif 
