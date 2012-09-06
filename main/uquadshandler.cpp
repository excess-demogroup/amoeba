#include <stdio.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "math/vector.h"
#include "exception.h"
#include "uquadshandler.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

UQuadsHandler::UQuadsHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Object(ml, title, elem, attr)
{
	this->xnum = attr->get_int("xnum");
	if (this->xnum <= 0)
		throw new FatalException(elem, "xnum= must be a positive integer!");
	
	this->ynum = attr->get_int("ynum");
	if (this->ynum <= 0)
		throw new FatalException(elem, "ynum= must be a positive integer!");
	
	this->znum = attr->get_int("znum");
	if (this->znum <= 0)
		throw new FatalException(elem, "znum= must be a positive integer!");

	this->xspace = attr->get_float("xspace");
	this->yspace = attr->get_float("yspace");
	this->zspace = attr->get_float("zspace");
	
	this->xstrength = attr->get_float("xstrength");
	this->ystrength = attr->get_float("ystrength");
	this->zstrength = attr->get_float("zstrength");
	
	this->no_zbuffer = true;

	for (int i = 0; i < xnum * ynum * znum; i++) {
		struct vertex v;
		v.x = v.y = v.z = 0.0f;
		this->vertices.add_end(v);
		this->vertices.add_end(v);
		this->vertices.add_end(v);
		this->vertices.add_end(v);
		
		struct texcoord tc;
		tc.u = 0.0f; tc.v = 0.0f; this->texcoords.add_end(tc);
		tc.u = 1.0f; tc.v = 0.0f; this->texcoords.add_end(tc);
		tc.u = 1.0f; tc.v = 1.0f; this->texcoords.add_end(tc);
		tc.u = 0.0f; tc.v = 1.0f; this->texcoords.add_end(tc);

		struct normal n;
		n.nx = 0.0f;
		n.ny = 1.0f;
		n.nz = 0.0f;
		this->normals.add_end(n);
		this->normals.add_end(n);
		this->normals.add_end(n);
		this->normals.add_end(n);

		this->faces.add_end(i * 4);
		this->faces.add_end(i * 4 + 1);
		this->faces.add_end(i * 4 + 2);
		this->faces.add_end(i * 4 + 3);
	}

	this->vertices_per_face = 4;
	this->pure_indices = true;
}

UQuadsHandler::~UQuadsHandler()
{
}

void UQuadsHandler::start_effect()
{
	Object::start_effect();
}

void UQuadsHandler::draw_scene(float progress)
{
	float xfreq = this->get_val("user1", progress);
	float yfreq = this->get_val("user2", progress);
	float zfreq = this->get_val("user3", progress);
	float prog  = this->get_val("user4", progress);

	this->unlock_object();
	
	for (int i = 0; i < this->xnum; i++) {
		int ci = i - xnum/2;
		const float x = (xspace + sin((float)ci * xfreq + prog) * xstrength) * ci;
		for (int j = 0; j < this->ynum; j++) {
			int cj = j - ynum/2;
			const float y = (yspace + sin((float)cj * yfreq + prog) * ystrength) * cj;
			for (int k = 0; k < this->znum; k++) {
				int ck = k - znum/2;
				const float z = (zspace + sin((float)ck * zfreq + prog) * zstrength) * ck;
				
				this->vertices[(k + (j + i * ynum) * znum) * 4    ].x = x - 0.5f;
				this->vertices[(k + (j + i * ynum) * znum) * 4    ].y = y;
				this->vertices[(k + (j + i * ynum) * znum) * 4    ].z = z + 0.5f;
				
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 1].x = x + 0.5f;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 1].y = y;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 1].z = z + 0.5f;
				
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 2].x = x + 0.5f;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 2].y = y;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 2].z = z - 0.5f;
				
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 3].x = x - 0.5f;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 3].y = y;
				this->vertices[(k + (j + i * ynum) * znum) * 4 + 3].z = z - 0.5f;
			}
		}
	}

	Object::draw_scene(progress);
}

void UQuadsHandler::end_effect()
{
	Object::end_effect();
}

#endif
