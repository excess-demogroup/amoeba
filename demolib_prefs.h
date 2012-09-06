/*
 * Compile-time prefs settings
 */

/* define for a `silent' demo (no enter/leave messages etc.) */
#define DEMOLIB_SILENT			1

/* these need their appropriate libs */
#define DEMOLIB_IMAGE_JPEG		1
#define DEMOLIB_IMAGE_PNG		1

/* general vector routines -- at least the 3D stuff _needs_ this */
#define DEMOLIB_MATH_VECTOR		1

#define DEMOLIB_DATA_DIR_NONMMAP	0
#define DEMOLIB_DATA_PAKFILE		1

/* this one is needed by the .ogg handler, specifically */
#define DEMOLIB_DATA_FILEREADER		1

/* this one reads raw zlib compressed files -- needs zlib, of course :-) */
#define DEMOLIB_DATA_ZLIB		1

#define DEMOLIB_TEXTURES		1
#define DEMOLIB_FPSCOUNTER		0
#define DEMOLIB_BLOBS			0
#define DEMOLIB_OPENGL_FONT_TTF		1

/* needs the expat XML parser :-D */
#define DEMOLIB_MAINLOOP		1

/* the sound system -- needs libvorbis/libvorbisfile/libogg ATM */
#define DEMOLIB_SOUND			1

/* define the wanted screen's X and Y aspects */
#define DEMOLIB_XASPECT 		4.0f
#define DEMOLIB_YASPECT			3.0f
