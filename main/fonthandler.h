#ifndef _FONTHANDLER_H
#define _FONTHANDLER_H

#include "demolib_prefs.h"
#if DEMOLIB_OPENGL_FONT_TTF

#include "main/event.h"
#include "opengl/fontttf.h"
#include "packer/file.h"

class FontHandler : public Event {
public:
	FontHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~FontHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	File *fontfile;
	FontTTF *font[128];
	float linedistance;
	int num_lines;
	bool additive_blend;
};

#endif /* DEMOLIB_FONT_TTF */

#endif /* defined(_FONTHANDLER_H) */
