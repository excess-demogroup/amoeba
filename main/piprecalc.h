#ifndef _PIPRECALC_H
#define _PIPRECALC_H 1

#include "opengl/texture.h"
#include "opengl/glwindow.h"

class PiPrecalc {
public:
	PiPrecalc(GLWindow *win);
	~PiPrecalc();

	void update(float progress);
	
protected:
	int last_status;
	
	GLWindow *win;
	Texture *font;
};

#endif /* !defined(_PIPRECALC_H) */
