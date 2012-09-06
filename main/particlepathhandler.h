#ifndef _PARTICLEPATHHANDLER_H
#define _PARTICLEPATHHANDLER_H 1

#include "opengl/texture.h"
#include "main/object.h"
#include "main/event.h"
#include "main/mainloop.h"

struct particle {
	float progress;
	float angle_sin, angle_cos;
};
struct particle_sort {
	int num;
	float z;
};

typedef void (APIENTRY *PFNGLLOCKARRAYSEXTPROC)(GLint first, GLsizei count);
typedef void (APIENTRY *PFNGLUNLOCKARRAYSEXTPROC)(void);

class ParticlePathHandler : public Event {
protected:
	Texture *tex;
	particle *particles;
	int num_particles;
	float size, speed, radius;
	float last_progress, headstart, streamlength;
	vertex *vert;
	texcoord *tc;
	particle_sort *particle_sortdata;

        bool has_compiled_vertex_array;
        PFNGLLOCKARRAYSEXTPROC glLockArraysEXT;
        PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT;
	
	void spawn_particle(int i, float progress);
	inline float find_roty_from_deriv(float t);

	bool uniform;
	
public:
	ParticlePathHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~ParticlePathHandler();
	
	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* !defined(_PARTICLEPATHHANDLER_H) */
