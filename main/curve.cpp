#include "curve.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

Curve::Curve() {}
Curve::~Curve() {}

void Curve::add_curvepoint(float x, float y) {}
void Curve::end_curvepoints(float start, float length) {}
float Curve::get_value(float x) { return x; }

#endif 
