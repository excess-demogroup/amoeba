#ifndef _LINECURVE_H
#define _LINECURVE_H

#include "main/curve.h"

class ParticlePathHandler;

class LineCurve : public Curve {
public:
	LineCurve();
	~LineCurve();

	void add_curvepoint(float x, float y);
	void end_curvepoints(float start, float length);
	float get_value(float x);

	int get_curvetype() { return CURVE_LINE; }

	friend class ParticlePathHandler;
	
protected:
	float base, step;
};

#endif /* !defined(_LINECURVE_H) */
