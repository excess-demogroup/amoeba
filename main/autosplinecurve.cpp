/*
 * "Automatic" (ie. you don't have to specify any extra control points)
 * splines, based on a standard Catmull-Rom spline, made "loopable" and
 * having f'''(x) = 0 all the way.
 */
#include "main/autosplinecurve.h"
#include "demolib_prefs.h"
#include "exception.h"

#if DEMOLIB_MAINLOOP

AutoSplineCurve::AutoSplineCurve()
{
	this->num_points = 0;
}
AutoSplineCurve::~AutoSplineCurve() {}

void AutoSplineCurve::add_curvepoint(float x, float y)
{
        if (this->num_points == MAX_POINTS)
		throw new FatalException("Maximum number of points reached!");
	
	/* check better for dupes later */

	this->x[this->num_points] = x;
	this->y[this->num_points] = y;
	this->num_points++;
}
void AutoSplineCurve::end_curvepoints(float start, float length)
{
	int i;

	if (this->num_points == 0)
		throw new FatalException("No points for auto spline!");

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

	if (this->num_points == 1) {
		deriv[0] = 0.0f;
	} else {
		const int e = this->num_points - 1;
		
		/* now find the derivatives */
		for (i = 1; i < e; i++) {
			const int p = i - 1, n = i + 1;
			deriv[i] = (y[n] - y[p]) / (x[n] - x[p]);
		}
	
		/* the edges are a bit special */
		deriv[0] = deriv[e] =
			((y[1] - y[e-1]) - (y[0] - y[e])) / ((x[e] - x[e-1]) + (x[1] - x[0]));
	}
}
float AutoSplineCurve::get_value(float x) {
	if (this->num_points == 1) return this->y[0];

	/* linear extrapolation to the left (uh-oh) */
	if (x < this->x[0]) {
		return (this->x[0] - x) * (this->y[1] - this->y[0]) / (this->x[1] - this->x[0]);
	}

	/* just initialize these values so gcc won't complain ;-) */
	float leftx = 0.0f, rightx = 0.1f, lefty = 0.0f, righty = 0.0f, leftd = 0.0f, rightd = 0.0f;

	/* interpolation, or extrapolation to the right */
	const int e = this->num_points - 1;
	for (int i = 0; i < e; i++) {
		leftx = this->x[i];
		rightx = this->x[i + 1];

		if (leftx <= x && rightx >= x) {
			lefty = this->y[i];
			righty = this->y[i + 1];
	
			leftd = this->deriv[i];
			rightd = this->deriv[i + 1];
		
			const float len = rightx - leftx;
			leftd *= len;
			rightd *= len;
			break;
		}
	}

	const float t = (x - leftx) / (rightx - leftx);

/*	return (1.0f - 3.0f*t*t + 2*t*t*t) * lefty +
		(3.0f*t*t - 2.0f*t*t*t) * righty +
		(t - 2.0f*t*t + t*t*t) * leftd +
		(-t*t + t*t*t) * rightd; */
	return (-righty - righty + rightd + leftd + lefty + lefty) * t*t*t +
	       (-rightd + 3.0f * righty - 2.0f * leftd - 3.0f * lefty) * t*t +
	       leftd * t +
	       lefty;
}

#endif 
