#ifndef _WIN32_CONFIG
#define _WIN32_CONFIG 1

#include <windows.h>
#include "math/array.h"

/* a singleton class, to fit well into Win32s stupidness ;-) */

class Win32Config
{
private:
	LPGUID sound;
	bool fullscreen;
	int mode;
	int zbuffer;
	Win32Config();

public:
	static Win32Config *instance();
	//~Win32Config();
	
	void dialog();
	
	void set_fullscreen(bool fullscreen) { this->fullscreen = fullscreen; }
	bool get_fullscreen() { return fullscreen; }

	void set_sound(LPGUID sound) { this->sound = sound; }
	LPGUID get_sound() { return sound; }

	void set_mode(int mode) { this->mode = mode; }
	int get_mode() { return mode; }

	void set_zbuffer(int zbuffer) { this->zbuffer = zbuffer; }
	int get_zbuffer() { return zbuffer; }
};

struct dsenum_pass_struct {
	HWND sound_list;
	Array<LPGUID> *sound_guids;
};

#endif /* !defined(_WIN32_CONFIG) */
