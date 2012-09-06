/*
 * Intended to be a _fast_ and simple curve type. :-)
 */

#include "main/linecurve.h"
#include "demolib_prefs.h"
#include "exception.h"

#include <stdio.h>

#if DEMOLIB_MAINLOOP

LineCurve::LineCurve()
{
	this->num_points = 0;
}
LineCurve::~LineCurve() {}

void LineCurve::add_curvepoint(float x, float y)
{
	this->x[this->num_points] = x;
	this->y[this->num_points] = y;
	this->num_points++;
}
void LineCurve::end_curvepoints(float start, float length)
{
	if (this->num_points < 1 || this->num_points > 2)
		throw new FatalException("Need exactly two points for line movement!");

	/* just trust that they come in order ;-) */
	this->base = this->y[0];
	this->step = (this->num_points == 1) ? 0.0f : (this->y[1] - base);
}
float LineCurve::get_value(float x)
{
	return this->base + this->step * x;
}

#endif 
