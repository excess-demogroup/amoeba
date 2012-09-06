#ifndef _LINEARCURVE_H
#define _LINEARCURVE_H

#include "main/curve.h"

class LinearCurve : public Curve {
private:
	int pos_hint;
	
public:
	LinearCurve();
	~LinearCurve();

	void add_curvepoint(float x, float y);
	void end_curvepoints(float start, float length);
	float get_value(float x);

	int get_curvetype() { return CURVE_LINEAR; }
};

#endif /* !defined(_LINEARCURVE_H) */
