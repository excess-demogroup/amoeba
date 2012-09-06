#ifndef _AUTOSPLINECURVE_H
#define _AUTOSPLINECURVE_H

#include "main/curve.h"

class AutoSplineCurve : public Curve {
protected:
	float deriv[256];

public:
	AutoSplineCurve();
	~AutoSplineCurve();

	void add_curvepoint(float x, float y);
	void end_curvepoints(float start, float length);
	float get_value(float x);

	int get_curvetype() { return CURVE_AUTOSPLINE; }
};

#endif /* !defined(_AUTOSPLINECURVE_H) */
