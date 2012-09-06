#ifndef _FONTTTF_H
#define _FONTTTF_H

#include "demolib_prefs.h"
#if DEMOLIB_OPENGL_FONT_TTF

#include <ft2build.h>
#include FT_FREETYPE_H

#include "packer/file.h"

class FontTTF {
public:
	FontTTF(File *ttffile, const char *text, int size);
	~FontTTF();
		
	void draw_object(float xpos, float ypos,
		       	 float red, float green, float blue,
		 	 float alpha, bool additive);
	
protected:
	int draw_text(const char *str, bool real_render);

	FT_Library   library;
	FT_Face	     face;

	int width, realwidth;
	
	unsigned int texnum;
	unsigned char *texmap;
};

#endif /* DEMOLIB_FONT_TTF */
#endif /* !defined(_FONTTTF_H) */
