/*
 * A twisting object deformation.
 *
 * Quite close to the deadline now, we're trying to do interesting stuff, so we're
 * combining the twister and the normalspike -- this results to code duplication
 * (and forced fixation of the wave parameter), but that's how it has to be when
 * you don't have time to make better generic "object filters" or something :-)
 */
#include <stdio.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "math/vector.h"
#include "exception.h"
#include "twisthandler.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

TwistHandler::TwistHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	ObjHandler(ml, title, elem, attr)
{
	float wave = attr->get_float("wave");
	
	if (!this->pure_indices)
		throw new FatalException(elem, "Needs optimized .obj files!");
	if (this->normals.num_elems() != this->vertices.num_elems())
		throw new FatalException(elem, "Needs .obj files with normals!");
	if (this->vertices_per_face != 3)
		throw new FatalException(elem, "Needs triangulated .obj files!");
	
	/* take a backup of the original vertices */
	this->orig_vertices = this->vertices;
	
	/*
	 * make an atan2() cache, for speedup (not strictly a vertex, but
	 * the xyz mapping makes sense)
	 */
	this->atan_cache = new vertex[vertices.num_elems()];
	int i;
	for (i = 0; i < this->vertices.num_elems(); i++) {
		struct vertex v;
		v.x = atan2(vertices[i].z, vertices[i].y) * wave;
		v.y = atan2(vertices[i].x, vertices[i].z) * wave;
		v.z = atan2(vertices[i].y, vertices[i].x) * wave;
		this->atan_cache[i] = v;
	}

	if (attr->exists("normalspike") && attr->get_bool("normalspike")) {
		this->normalspike = true;
		this->normalspiketex = texture::load(attr->get_str("normalspiketexture"));

		this->spike_vert = new vertex[this->vertices.num_elems() * 4];
		this->spike_tc = new texcoord[this->vertices.num_elems() * 4];

		for (int i = 0; i < this->vertices.num_elems(); i++) {
			spike_tc[i * 4    ].u = 0.0f;
			spike_tc[i * 4    ].v = 1.0f;
			
			spike_tc[i * 4 + 1].u = 1.0f;
			spike_tc[i * 4 + 1].v = 1.0f;
			
			spike_tc[i * 4 + 2].u = 1.0f;
			spike_tc[i * 4 + 2].v = 0.0f;
			
			spike_tc[i * 4 + 3].u = 0.0f;
			spike_tc[i * 4 + 3].v = 0.0f;
		}
	} else {
		this->normalspike = false;
		this->normalspiketex = NULL;
		this->spike_vert = NULL;
		this->spike_tc = NULL;
	}

	/*
	 * For the normal mapping, we need an accurate index of what points
	 * are connected to each other (so we can average their face normals).
	 * When no points are duplicated, this is a simple matter, and the
	 * .obj files we get from Maya are usually that way. However, separate
	 * vertex/texcoord/normal indices isn't good when we want to use
	 * vertex arrays (or really, _anything_ :-P), so the optimized .obj
	 * files we use "group" points by the vertex/texcoord/normal combination.
	 * However, this causes `seams' in points where positions are equal
	 * but texture coordinates are not (typical example is a sphere where
	 * (0.0,0.0) != (1.0,0.0) for texcoords), so we need to regroup the
	 * information here somehow.
	 *
	 * The solution is to make a list of duplicate points. For each and
	 * every point, we have a singly linked list. Every int in this list
	 * is an index to another point in the group, and the list is looped
	 * so that we can start the traversal wherever we want. A point
	 * with no duplicates simply points to its own index.
	 *
	 * The critera for "duplicate" involves equal position and normals,
	 * so texcoords aren't counted -- they really shouldn't either when
	 * it comes to object structure. However, points not sharing the
	 * same normal shouldn't usually be averaged either (think sharp
	 * edges here), so we don't want to _just_ consider the positions.
	 * Thus the (position,normal) combination.
	 */
	this->vertex_alias = new int[this->vertices.num_elems()];
	for (int i = 0; i < this->vertices.num_elems(); i++) {
		struct vertex *v = &(this->vertices[i]);
		struct normal *n = &(this->normals[i]);
		
		this->vertex_alias[i] = i;
		
		for (int j = 0; j < i; j++) {
			if (fabs(this->vertices[j].x - v->x) < 0.0001f &&
			    fabs(this->vertices[j].y - v->y) < 0.0001f &&
			    fabs(this->vertices[j].z - v->z) < 0.0001f &&
			    fabs(this->normals[j].nx - n->nx) < 0.0001f &&
			    fabs(this->normals[j].ny - n->ny) < 0.0001f &&
			    fabs(this->normals[j].nz - n->nz) < 0.0001f) {
				/*
				 * find the _last_ point in the chain (ie.
				 * the one pointing back at j)
				 */
				int lp = j;
				while (this->vertex_alias[lp] != j)
					lp = this->vertex_alias[lp];
			
				/* insert i in this chain */
				this->vertex_alias[lp] = i;
				this->vertex_alias[i] = j;
				break;
			}
		}
	}
}

TwistHandler::~TwistHandler()
{
	delete[] this->atan_cache;
	this->atan_cache = NULL;
	
	delete[] this->vertex_alias;
	this->vertex_alias = NULL;

	delete[] this->spike_vert;
	this->spike_vert = NULL;
	
	delete[] this->spike_tc;
	this->spike_tc = NULL;
	
	delete this->normalspiketex;
	this->normalspiketex = NULL;
}

void TwistHandler::start_effect()
{
	Object::start_effect();
}

void TwistHandler::draw_scene(float progress)
{
	int i;
	float wobble = this->get_val("user1", progress);
	float power  = this->get_val("user2", progress);

	this->unlock_object();
	for (i = 0; i < this->vertices.num_elems(); i++){
		float xrot = 1.0f + (float)sin( wobble + atan_cache[i].x) * power;
		float yrot = 1.0f + (float)sin( wobble - atan_cache[i].y) * power;
		float zrot = 1.0f + (float)sin(-wobble + atan_cache[i].z) * power;

		this->vertices[i].x = this->orig_vertices[i].x * yrot * zrot;
		this->vertices[i].y = this->orig_vertices[i].y * zrot * xrot;
		this->vertices[i].z = this->orig_vertices[i].z * xrot * yrot;
	}

	/* 
	 * now regenerate the normals -- this isn't particularily quick or pretty,
	 * but it works
	 */
	for (i = 0; i < this->normals.num_elems(); i++) {
		this->normals[i].nx = 0.0f;
		this->normals[i].ny = 0.0f;
		this->normals[i].nz = 0.0f;
	}
		
	for (i = 0; i < this->faces.num_elems(); i += 3) {
		vertex v[3];
		v[0] = this->vertices[this->faces[i    ]];
		v[1] = this->vertices[this->faces[i + 1]];
		v[2] = this->vertices[this->faces[i + 2]];

		Vector up   (v[2].x - v[0].x, v[2].y - v[0].y, v[2].z - v[0].z);
		Vector right(v[1].x - v[0].x, v[1].y - v[0].y, v[1].z - v[0].z);
		Vector normal = right.cross_product(up);

		for (int j = 0; j < 3; j++) {
			int start_ind = this->faces[i + j];
			int ind = start_ind;

			do {
 				this->normals[ind].nx += normal.x;
				this->normals[ind].ny += normal.y;
				this->normals[ind].nz += normal.z;
				ind = this->vertex_alias[ind];
			} while (ind != start_ind);
		}
	}

	/* normalize */
	for (i = 0; i < this->normals.num_elems(); i++) {
		const float mulfac = 1.0f / sqrt(
			this->normals[i].nx * this->normals[i].nx +
			this->normals[i].ny * this->normals[i].ny +
			this->normals[i].nz * this->normals[i].nz);

			this->normals[i].nx *= mulfac;
			this->normals[i].ny *= mulfac;
			this->normals[i].nz *= mulfac;
	}

	Object::draw_scene(progress);
	
	if (this->normalspike && this->get_val("user3", progress) != 0.0f) {	
		/*
		 * set up a fake viewport we can map to an orthographic projection
		 * later
		 */
		GLint viewport[4] = { 0, 0, 1, 1 };
		GLdouble modelview[16];
		GLdouble projection[16];
	
		const float length = this->get_val("user3", progress);
		const float base   = this->get_val("user4", progress);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
	
		float x = this->get_val("xpos", progress);
	        float y = this->get_val("ypos", progress);
	        float z = this->get_val("zpos", progress);
	        float xr = this->get_val("xrot", progress);
	        float yr = this->get_val("yrot", progress);
	        float zr = this->get_val("zrot", progress);
	
	        glTranslatef(x,y,z);
		if (this->inverse_rotorder) {
			glRotatef(zr, 0.0f, 0.0f, 1.0f);
			glRotatef(yr, 0.0f, 1.0f, 0.0f);
			glRotatef(xr, 1.0f, 0.0f, 0.0f);
		} else {
			glRotatef(xr, 1.0f, 0.0f, 0.0f);
			glRotatef(yr, 0.0f, 1.0f, 0.0f);
			glRotatef(zr, 0.0f, 0.0f, 1.0f);
		}
	
	        float scale = this->get_val("scale", progress);
	        glScalef(scale, scale, scale);
		
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
		glLoadIdentity();
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
	
		glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f);
	
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	
		glEnable(GL_TEXTURE_2D);
		normalspiketex->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		
		for (int i = 0; i < vertices.num_elems(); i++) {
			/* transform first the base point */
			double bwx, bwy, bwz;
			gluProject((double)(vertices[i].x),
				   (double)(vertices[i].y),
				   (double)(vertices[i].z),
				   modelview, projection, viewport,
				   &bwx, &bwy, &bwz);
			
			/* then the tip of the spike */

			double ox = (double)(vertices[i].x + normals[i].nx * length);
			double oy = (double)(vertices[i].y + normals[i].ny * length);
			double oz = (double)(vertices[i].z + normals[i].nz * length);
			double wx, wy, wz;
			
			gluProject(ox, oy, oz, modelview, projection, viewport,
				&wx, &wy, &wz);
	
			/* now make a left and a right base */
			float lbx = bwx - (wy-bwy) * base;
			float lby = bwy + (wx-bwx) * base;
			
			float rbx = bwx + (wy-bwy) * base;
			float rby = bwy - (wx-bwx) * base;
	
			/* now make the triangle */
			spike_vert[i * 4    ].x = lbx;
			spike_vert[i * 4    ].y = lby;
			spike_vert[i * 4    ].z = (float)bwz;
			
			spike_vert[i * 4 + 1].x = rbx;
			spike_vert[i * 4 + 1].y = rby;
			spike_vert[i * 4 + 1].z = (float)bwz;
			
			spike_vert[i * 4 + 2].x = (float)wx;
			spike_vert[i * 4 + 2].y = (float)wy;
			spike_vert[i * 4 + 2].z = (float)wz;
			
			spike_vert[i * 4 + 3].x = (float)wx;
			spike_vert[i * 4 + 3].y = (float)wy;
			spike_vert[i * 4 + 3].z = (float)wz;
		}
	
		glVertexPointer(3, GL_FLOAT, 0, spike_vert);
		glTexCoordPointer(2, GL_FLOAT, 0, spike_tc);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	
		if (this->has_compiled_vertex_array)
			(*this->glLockArraysEXT)(0, this->vertices.num_elems() * 4);
		
		glDrawArrays(GL_QUADS, 0, this->vertices.num_elems() * 4);
		
		if (this->has_compiled_vertex_array)
			(*this->glUnlockArraysEXT)();
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		
		glEnable(GL_LIGHTING);
		glDepthMask(GL_TRUE);
		
		glPopMatrix();
	}
}

void TwistHandler::end_effect()
{
	Object::end_effect();
}

#endif
