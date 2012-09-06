#ifndef _OBJECT_H
#define _OBJECT_H 1

#include "main/event.h"
#include "main/mainloop.h"
#include "opengl/texture.h"
#include "math/array.h"

#if defined(WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

#include <GL/gl.h>

#ifndef APIENTRY
#define APIENTRY
#endif

/* from GLext.h */
typedef void (APIENTRY *PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (APIENTRY *PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
typedef void (APIENTRY *PFNGLMULTITEXCOORD2FVARBPROC)(GLenum target, const GLfloat *v);
typedef void (APIENTRY *PFNGLLOCKARRAYSEXTPROC)(GLint first, GLsizei count);
typedef void (APIENTRY *PFNGLUNLOCKARRAYSEXTPROC)(void);
typedef void (APIENTRY *PFNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

struct vertex {
	float x, y, z;
};
struct normal {
	float nx, ny, nz;
};
struct texcoord {
	float u, v;
};

class Object : public Event
{
protected:
	Array<vertex> vertices;
	Array<normal> normals;
	Array<texcoord> texcoords, *ctexcoords[2];
	Array<int> faces;
	Array<GLfloat> *tv;

	bool pure_indices;
	bool pretrans, precolor;
	int vertices_per_face;

	Texture *tex[2];
	enum gentype { none, linmap, envmap } texgen[2]; 
	float texstr0;

	bool has_multitexture;
	bool has_compiled_vertex_array;
	bool has_draw_range_elements;
	bool has_rescale_normal;

	bool additive_blend, no_zbuffer;

	/* texunit 1 parameters (for multitexture) */
	GLenum tu1combine, tu1s0, tu1o0, tu1s1, tu1o1, tu1s2, tu1o2;

	/* pass 2 blending functions (for non-multitexture) */
	GLenum p2sf, p2df;
	bool p2light;

	/* stencil mask, primarily for halos (0x0 not to stencil) */
	int stencil_mask;

	/* be sure to have unit length normals for this ;-) */
	bool draw_halo;
	float halo_length;
	struct vertex *halo_vert;

	/* for RTG compatibility */
	bool inverse_rotorder;
	
	PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
	PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
	PFNGLLOCKARRAYSEXTPROC glLockArraysEXT;
	PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT;
	PFNGLDRAWRANGEELEMENTSEXTPROC glDrawRangeElementsEXT;

	void setup_drawing(float progress);
	virtual void draw_object();
	virtual void draw_object(bool textures, bool normals);
	void add_vertex(const struct vertex v);
	void add_normal(const struct normal n);
	void add_texcoord(const struct texcoord tc);
	
	void add_face(const int a, const int b, const int c);
	void add_face(const int a, const int b, const int c, const int d);

	void unlock_object();
	void setup_vertex_arrays(bool textures, bool normals);

public:
	Object(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	virtual ~Object();

	virtual void start_effect();
	virtual void draw_scene(float progress);
	virtual void end_effect();
};

#endif /* !defined(_OBJECT_H) */
