#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "extensions.h"
#include "demolib_prefs.h"
#include "exception.h"

#include <GL/gl.h>

#if __linux__
#include <GL/glx.h>
extern "C" {
	void (*glXGetProcAddressARB(const GLubyte *procName))();
}
#endif

bool GLExtensions::has_ext(const char *ext)
{
	/*
	 * ATI drivers on Windows have known to be very buggy on this
	 * extension (unless my code is really fucked up, it could of 
	 * course be), so disable it if we're using an ATI card :-)
	 */
	if (strcmp(ext, "GL_EXT_draw_range_elements") == 0) {
		const char *vendor = (const char *)glGetString(GL_VENDOR);
		if (vendor == NULL || strstr(vendor, "ATI ") != NULL) {
			return false;
		}
	}
	
	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	if (extensions == NULL) return false;

	int extlen = strlen(ext);
	const char *ptr = extensions;

	while (ptr && *ptr) {
		if (strncmp(ptr, ext, extlen) == 0 &&
		    (ptr[extlen] == ' ' || ptr[extlen] == '\0')) {
			return true;
		}
		ptr = strchr(ptr, ' ');
		if (ptr != NULL) ptr++;
	}
	return false;
}

void *GLExtensions::func_ptr(const char *function)
{
#if __linux__
	void *ptr = (void *)glXGetProcAddressARB((GLubyte *)function);
#else
	void *ptr = (void *)wglGetProcAddress(function);
#endif

	/*
	 * Some OpenGL drivers (like the Kyro2 drivers on Linux) expose stuff
	 * like glDrawRangeElements but _not_ glDrawRangeElementsEXT, even
	 * though they advertise the extension. Thus, if getting an -EXT or
	 * -ARB function pointer fails (which it should _never_ do, since we
	 *  always check for extension availability first), we simply try
	 *  without the suffix before finally giving up.
	 */
	
	const char *suffix = function + strlen(function) - 3;   /* ugly? ;-) */
	if (ptr == NULL &&
	    (strcmp(suffix, "EXT") == 0 || strcmp(suffix, "ARB") == 0)) {
		char *tmp = strdup(function);
		tmp[strlen(tmp) - 3] = '\0';
#if __linux__
		ptr = (void *)glXGetProcAddressARB((GLubyte *)tmp);
#else
		ptr = (void *)wglGetProcAddress(tmp);
#endif
		free(tmp);
	}
	if (ptr == NULL) {
		throw new FatalException(function, "Not found in OpenGL libraries!");
	}
	return ptr;
}
