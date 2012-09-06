#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP && DEMOLIB_OPENGL_FONT_TTF

#include "main/fonthandler.h"
#include "opengl/fontttf.h"
#include "exception.h"

FontHandler::FontHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "xpos:ypos:red:green:blue:alpha")
{
	int size;

	if (attr->exists("size")) {
		size = attr->get_int("size");
	} else {
		size = 24;
	}
	if (attr->exists("linedistance")) {
		this->linedistance = attr->get_float("linedistance");
	} else {
		this->linedistance = 0.15f;
	}
	this->fontfile = load_file(attr->get_str("font"));

        this->additive_blend = false;
        if (attr->exists("blend")) {
                char *str = attr->get_str("blend");
                if (strcmp(str, "blend") == 0) {
                        this->additive_blend = false;
                } else if (strcmp(str, "add") == 0) {
                        this->additive_blend = true;
                } else {
                        throw new FatalException(elem, "blend= must be `blend' or `add'!");
                }
        }
	
	this->num_lines = 0;
	
	/* parse into nice parts ;-) */
	char *text = strdup(attr->get_str("text"));
	char *ptr = strtok(text, "|");

	while (ptr) {
		this->font[this->num_lines++] =
			new FontTTF(fontfile, ptr, size);
		ptr = strtok(NULL, "|");
	}
	free(text);
}
FontHandler::~FontHandler()
{
	for (int i = 0; i < this->num_lines; i++) {
		delete this->font[i];
		this->font[i] = NULL;
	}

	delete this->fontfile;
	this->fontfile = NULL;
}

void FontHandler::start_effect() {}
void FontHandler::draw_scene(float progress)
{
	for (int i = 0; i < this->num_lines; i++) {
		font[i]->draw_object(this->get_val("xpos", progress),
		                  this->get_val("ypos", progress) + this->linedistance * i,
				  this->get_val("red", progress),
				  this->get_val("green", progress),
				  this->get_val("blue", progress),
				  this->get_val("alpha", progress),
				  this->additive_blend);
	}
}
void FontHandler::end_effect() {}

#endif /* DEMOLIB_MAINLOOP && DEMOLIB_OPENGL_FONT_TTF */
