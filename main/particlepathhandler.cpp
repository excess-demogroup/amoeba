#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <algorithm>

#include "main/curve.h"
#include "main/linecurve.h"
#include "math/matrix.h"
#include "opengl/extensions.h"

#include "exception.h"
#include "particlepathhandler.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

class z_compare {
public:
	bool operator() (const struct particle_sort &a, const struct particle_sort &b)
	{
		return (a.z < b.z);
	}
};

ParticlePathHandler::ParticlePathHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "x:y:z:alpha")
{
	this->size = attr->get_float("size");
	this->speed = attr->get_float("speed");
	this->radius = attr->get_float("radius");

	this->headstart = 1.0f;
	if (attr->exists("headstart")) {
		this->headstart = attr->get_float("headstart");
		if (this->headstart < 0.0f || this->headstart > 1.0f) {
			throw new FatalException(elem, "headstart= must be between 0 and 1!");
		}
	}

	this->uniform = false;
	if (attr->exists("uniform")) {
		this->uniform = attr->get_bool("uniform");
	}

	this->streamlength = -1.0f;
	if (attr->exists("streamlength")) {
		this->streamlength = attr->get_float("streamlength");
	}
	
	this->tex = texture::load(attr->get_str("texture"));
	this->num_particles = attr->get_int("numparticles");
	this->particles = new particle[num_particles];
	this->vert = new vertex[num_particles * 4];
	this->tc = new texcoord[num_particles * 4];
	this->particle_sortdata = new particle_sort[num_particles];

	if (this->uniform) {
		float step = 1.0f / (float)(num_particles - 1);
		float t = 0.0f;
		for (int i = 0; i < num_particles; i++, t += step) {
			this->spawn_particle(i, t - 1.0f + headstart);

			this->tc[i * 4    ].u = 0.0f;
			this->tc[i * 4    ].v = 0.0f;
			
			this->tc[i * 4 + 1].u = 1.0f;
			this->tc[i * 4 + 1].v = 0.0f;
		
			this->tc[i * 4 + 2].u = 1.0f;
			this->tc[i * 4 + 2].v = 1.0f;
				
			this->tc[i * 4 + 3].u = 0.0f;
			this->tc[i * 4 + 3].v = 1.0f;

			/* make sure the stream is of right length */
			if (t < 1.0f - this->streamlength && this->streamlength != -1.0f) {
				particles[i].progress = -1000.0f;
			}
		}
	} else {
		srand(time(NULL));
	
		for (int i = 0; i < num_particles; i++) {
			this->spawn_particle(i, 1.0f*rand()/(RAND_MAX+1.0) - 1.0f + headstart);
			this->tc[i * 4    ].u = 0.0f;
			this->tc[i * 4    ].v = 0.0f;
			
			this->tc[i * 4 + 1].u = 1.0f;
			this->tc[i * 4 + 1].v = 0.0f;
		
			this->tc[i * 4 + 2].u = 1.0f;
			this->tc[i * 4 + 2].v = 1.0f;
				
			this->tc[i * 4 + 3].u = 0.0f;
			this->tc[i * 4 + 3].v = 1.0f;
		}
	}

	this->last_progress = 0.0f;

	if (GLExtensions::has_ext("GL_EXT_compiled_vertex_array")) {
                this->has_compiled_vertex_array = true;
                this->glLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)
                        GLExtensions::func_ptr("glLockArraysEXT");
                this->glUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)
                        GLExtensions::func_ptr("glUnlockArraysEXT");
        } else {
                this->has_compiled_vertex_array = false;
        }

}

ParticlePathHandler::~ParticlePathHandler()
{
	texture::free(this->tex);
	this->tex = NULL;
	
	delete[] this->particles;
	this->particles = NULL;
	
	delete[] this->particle_sortdata;
	this->particle_sortdata = NULL;
	
	delete[] this->vert;
	this->vert = NULL;

	delete[] this->tc;
	this->tc = NULL;
}

void ParticlePathHandler::start_effect() { }

void ParticlePathHandler::draw_scene(float progress)
{
	/*
	 * move all particles, spawn new ones if needed
	 */
	int i;
	for (i = 0; i < num_particles; i++) {
		particles[i].progress += (progress - last_progress) * speed;
		if (particles[i].progress > 1.0f) {
			if (progress > this->streamlength &&
			    this->streamlength != -1.0f) {
				/* never again a particle here :-) */
				particles[i].progress = -1000.0f;
			} else {
				this->spawn_particle(i, fmod(particles[i].progress, 1.0f));
			}
		}
	}
	this->last_progress = progress;

	/*
	 * set OpenGL state and find the matrix, use it to create an opposite
	 * billboarding matrix
	 */
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor4f(1.0f, 1.0f, 1.0f, this->get_val("alpha", progress));
	
	glEnable(GL_TEXTURE_2D);
	tex->bind();

	GLfloat mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	
	Matrix tmat(mat);

	/* 
	 * since we're going to have a LOT of curve lookups, we "cache"
	 * so we don't have to query the hashtable every time
	 */
	Curve *x = (Curve *)this->curves->lookup("x");
	Curve *y = (Curve *)this->curves->lookup("y");
	Curve *z = (Curve *)this->curves->lookup("z");

	/* generate the billboarding */
	struct vertex ul, ll, lr, ur;
	ul.x = (-tmat.matrix[0][0] - tmat.matrix[1][0]) * size;
	ul.y = (-tmat.matrix[0][1] - tmat.matrix[1][1]) * size;
	ul.z = (-tmat.matrix[0][2] - tmat.matrix[1][2]) * size;
			
	ll.x = ( tmat.matrix[0][0] - tmat.matrix[1][0]) * size;
	ll.y = ( tmat.matrix[0][1] - tmat.matrix[1][1]) * size;
	ll.z = ( tmat.matrix[0][2] - tmat.matrix[1][2]) * size;
		
	lr.x = ( tmat.matrix[0][0] + tmat.matrix[1][0]) * size;
	lr.y = ( tmat.matrix[0][1] + tmat.matrix[1][1]) * size;
	lr.z = ( tmat.matrix[0][2] + tmat.matrix[1][2]) * size;
		
	ur.x = (-tmat.matrix[0][0] + tmat.matrix[1][0]) * size;
	ur.y = (-tmat.matrix[0][1] + tmat.matrix[1][1]) * size;
	ur.z = (-tmat.matrix[0][2] + tmat.matrix[1][2]) * size;

	/* whatever you do, DON'T use this direction ;-) */
	Vector up(0.2538f, 0.19f, -0.38f);
	
	/* 
	 * do a very specific optimization trick (ie. drop curve overhead
	 * altogether and reduce the CPU load a LOT) if we deal with simple
	 * line curves (curve="line")
	 */
	if (x->get_curvetype() == CURVE_LINE &&
	    y->get_curvetype() == CURVE_LINE &&
	    z->get_curvetype() == CURVE_LINE) {
		const float xb = ((LineCurve *)x)->base;
		const float yb = ((LineCurve *)y)->base;
		const float zb = ((LineCurve *)z)->base;
		const float xs = ((LineCurve *)x)->step;
		const float ys = ((LineCurve *)y)->step;
		const float zs = ((LineCurve *)z)->step;
		
		Vector deriv(xs, ys, zs);
		
		/* whatever you do, DON'T use this direction ;-) */
		Vector up(0.2538f, 0.19f, -0.38f);
		
		Vector local_right = up.cross_product(deriv).normalize();
		Vector local_up = local_right.cross_product(deriv).normalize();
		
		for (i = 0; i < num_particles; i++) {
			/*
			 * a massively simplified version of find_rot_from_deriv
			 * from tunnelhandler.cpp ;-)
			 */
			const float t = particles[i].progress;
			if (t < 0.0f) {
				this->particle_sortdata[i].z = 0.0f;    // invalid z value
				continue;
			}
	                const float rot_sin = particles[i].angle_sin;
        	        const float rot_cos = particles[i].angle_cos;
			const float x = xb + xs * t + local_right.x * rot_cos + local_up.x * rot_sin;
			const float y = yb + ys * t + local_right.y * rot_cos + local_up.y * rot_sin;
			const float z = zb + zs * t + local_right.z * rot_cos + local_up.z * rot_sin;
			
			/* generate a Z index */
			this->particle_sortdata[i].num = i;
			this->particle_sortdata[i].z =
				x * tmat.matrix[2][0] + y * tmat.matrix[2][1] + z * tmat.matrix[2][2];
		
			vert[i * 4    ].x = x + ul.x;
			vert[i * 4    ].y = y + ul.y;
			vert[i * 4    ].z = z + ul.z;
			
			vert[i * 4 + 1].x = x + ll.x;
			vert[i * 4 + 1].y = y + ll.y;
			vert[i * 4 + 1].z = z + ll.z;
		
			vert[i * 4 + 2].x = x + lr.x;
			vert[i * 4 + 2].y = y + lr.y;
			vert[i * 4 + 2].z = z + lr.z;
		
			vert[i * 4 + 3].x = x + ur.x;
			vert[i * 4 + 3].y = y + ur.y;
			vert[i * 4 + 3].z = z + ur.z;
		}
	} else {
		/* okay, slow way... calculate the particles one by one */
		for (i = 0; i < num_particles; i++) {
			/*
			 * a massively simplified version of find_rot_from_deriv
			 * from tunnelhandler.cpp ;-)
			 */
			const float t = particles[i].progress;
			if (t < 0.0f) {
				this->particle_sortdata[i].z = 0.0f;    // invalid z value
				continue;
			}
			
			const float xn = x->get_value(t);
			const float yn = y->get_value(t);
			const float zn = z->get_value(t);
			const float xd = x->get_value(t + 0.0001f) - xn;
			const float yd = y->get_value(t + 0.0001f) - yn;
			const float zd = z->get_value(t + 0.0001f) - zn;

			Vector deriv(xd, yd, zd);
			Vector local_right = up.cross_product(deriv).normalize();
			Vector local_up = local_right.cross_product(deriv).normalize();

		
			/*
			 * would be a faster version of sin(atan2(xd, zd))... but what
			 * about the minus? ;-)
			 */
	/*        	const float hyp = 1.0f / hypot(xd, zd);
			const float sin_atan_ry = xd * hyp;
			const float cos_atan_ry = zd * hyp; */
			
	                const float rot_sin = particles[i].angle_sin;
        	        const float rot_cos = particles[i].angle_cos;

			const float x = xn + local_right.x * rot_cos + local_up.x * rot_sin;
			const float y = yn + local_right.y * rot_cos + local_up.y * rot_sin;
			const float z = zn + local_right.z * rot_cos + local_up.z * rot_sin;
	
			/* generate a Z index */
			this->particle_sortdata[i].num = i;
			this->particle_sortdata[i].z =
				x * tmat.matrix[2][0] + y * tmat.matrix[2][1] + z * tmat.matrix[2][2];
		
			vert[i * 4    ].x = x + ul.x;
			vert[i * 4    ].y = y + ul.y;
			vert[i * 4    ].z = z + ul.z;
			
			vert[i * 4 + 1].x = x + ll.x;
			vert[i * 4 + 1].y = y + ll.y;
			vert[i * 4 + 1].z = z + ll.z;
		
			vert[i * 4 + 2].x = x + lr.x;
			vert[i * 4 + 2].y = y + lr.y;
			vert[i * 4 + 2].z = z + lr.z;
		
			vert[i * 4 + 3].x = x + ur.x;
			vert[i * 4 + 3].y = y + ur.y;
			vert[i * 4 + 3].z = z + ur.z;
		}
	}

	/* Z sort */
	std::sort(&this->particle_sortdata[0], &this->particle_sortdata[num_particles - 1],
		  z_compare());
/*	qsort(this->particle_sortdata, num_particles, sizeof(struct particle_sort),
	      z_compare); */

	glVertexPointer(3, GL_FLOAT, 0, vert);
	glTexCoordPointer(2, GL_FLOAT, 0, tc);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	if (this->has_compiled_vertex_array)
		(*this->glLockArraysEXT)(0, num_particles * 4);
       
	/* probably better than messing about with a new temp array */
	glBegin(GL_QUADS);
	for (i = 0; i < num_particles; i++) {
		if (this->particle_sortdata[i].z == 0.0f) continue;
		for (int j = 0; j < 4; j++) {
			glArrayElement(this->particle_sortdata[i].num * 4 + j);
		}
       	}
	glEnd();
	
        if (this->has_compiled_vertex_array)
                (*this->glUnlockArraysEXT)();
	
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	
	glPopMatrix();
}

void ParticlePathHandler::end_effect() {}

void ParticlePathHandler::spawn_particle(int i, float progress)
{
	const float angle = 2.0f * M_PI * rand() / (RAND_MAX+1.0);
	const float radius = this->radius * M_PI * rand() / (RAND_MAX+1.0);
	
	particles[i].progress = progress;
	particles[i].angle_sin = sin(angle) * radius;
	particles[i].angle_cos = cos(angle) * radius;
}

#endif
