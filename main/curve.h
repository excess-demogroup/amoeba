#ifndef _CURVE_H
#define _CURVE_H

#define CURVE_NONE	-1
#define CURVE_LINEAR	 1
#define CURVE_AUTOSPLINE 3
#define CURVE_LINE       4

#define MAX_POINTS       4096

class CurveDebugger;

class Curve {
public:
	Curve();
	virtual ~Curve();

	virtual void add_curvepoint(float x, float y) = 0;
	virtual void end_curvepoints(float start, float length) = 0;
	virtual float get_value(float x) = 0;

	virtual int get_curvetype() = 0;

	friend class CurveDebugger;
	
protected:
	float x[MAX_POINTS], y[MAX_POINTS];
	int num_points;
};

#endif /* !defined(_CURVE_H) */
