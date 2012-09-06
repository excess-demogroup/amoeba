#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264
#endif

#include "main/shadowhandler.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "packer/file.h"
#include "exception.h"
#include "demolib_prefs.h"

#define SHADOWLENGTH 10.0f

#if DEMOLIB_MAINLOOP

ShadowHandler::ShadowHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	ObjHandler(ml, title, elem, attr)
{
	if (!this->pure_indices)
		throw new FatalException(elem, "Needs a presorted object!");
	if (this->vertices_per_face != 3)
		throw new FatalException(elem, "Can only work with triangles!");
	
	int i;
	for (i = 0; i < faces.num_elems(); i += 3) {
		struct extravertexinfo e;

		/* calculate the plane for the triangle */
		const struct vertex &v1 = vertices[faces[i]];
		const struct vertex &v2 = vertices[faces[i + 1]];
		const struct vertex &v3 = vertices[faces[i + 2]];

		e.a = v1.y*(v2.z-v3.z) + v2.y*(v3.z-v1.z) + v3.y*(v1.z-v2.z);
		e.b = v1.z*(v2.x-v3.x) + v2.z*(v3.x-v1.x) + v3.z*(v1.x-v2.x);
		e.c = v1.x*(v2.y-v3.y) + v2.x*(v3.y-v1.y) + v3.x*(v1.y-v2.y);
		e.d = -(v1.x*(v2.y*v3.z - v3.y*v2.z) +
	 	 	v2.x*(v3.y*v1.z - v1.y*v3.z) +
			v3.x*(v1.y*v2.z - v2.y*v1.z));

		e.neighbours[0] = -1;
		e.neighbours[1] = -1;
		e.neighbours[2] = -1;

		e.visible = false;

		evi.add_end(e);
	}

	/* 
	 * now find the neighbours
	 * optimization: use a vertex->face cache, which we make first :-)
	 */
	struct vfc {
		int face[6];
	};
	vfc *vfcache = new vfc[vertices.num_elems()];
	for (i = 0; i < vertices.num_elems(); i++) {
		for (int j = 0; j < 6; j++) {
			vfcache[i].face[j] = -1;
		}
	}
	
	for (i = 0; i < faces.num_elems(); i++) {
		int v = faces[i];
		bool found = true;
		for (int j = 0; j < 6; j++) {
			if (vfcache[v].face[j] == -1) {
				vfcache[v].face[j] = i/3;
				found = true;
				break;
			}
		}
		if (!found) {
			throw new FatalException("Vertex->face cache overload");
		}
	}
		
	for (i = 0; i < faces.num_elems(); i += 3) {
		for (int ea = 0; ea < 3; ea++) {
			if (evi[i/3].neighbours[ea] != -1) continue;

			for (int k = 0; k < 6; k++) {
				int j = vfcache[faces[i + ea]].face[k];
				if (j >= i/3) continue;
				if (j == -1) break;
				
				bool found = false;
				for (int eb = 0; eb < 3; eb++) {
					const int va1 = faces[i + ea];
					const int va2 = faces[i + (ea+1)%3];
					const int vb1 = faces[j*3 + eb];
					const int vb2 = faces[j*3 + (eb+1)%3];

					if ((va1 == vb1 &&
					     va2 == vb2) ||
					    (va1 == vb2 &&
					     va2 == vb1)) {
						evi[i/3].neighbours[ea] = j;
						evi[j].neighbours[eb] = i/3;
						found = true;
						break;
					}
				}
				if (found) break;
			}
		}
	}
	delete[] vfcache;
	
	this->shadowvol = new GLfloat[evi.num_elems() * 16];
}

ShadowHandler::~ShadowHandler()
{
	delete[] this->shadowvol;
	this->shadowvol = NULL;
}

void ShadowHandler::start_effect()
{
	ObjHandler::start_effect();
}
void ShadowHandler::draw_scene(float progress)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	/* translate */
	const float x = this->get_val("xpos", progress);
	const float y = this->get_val("ypos", progress);
	const float z = this->get_val("zpos", progress);
	const float xr = this->get_val("xrot", progress);
	const float yr = this->get_val("yrot", progress);
	const float zr = this->get_val("zrot", progress);

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

	const float scale = this->get_val("scale", progress);
	glScalef(scale, scale, scale);
	
	/* get the lightsource position from x/y/z */
	const float lx = this->get_val("user1", progress);
	const float ly = this->get_val("user2", progress);
	const float lz = this->get_val("user3", progress);
	const Vector lvec(lx, ly, lz);

	/* get the modelview matrix */
	float mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	Matrix mvm(mv);

	/* find the lightsource position in object space */
	const Vector olvec = mvm.transpose() * lvec;

	/* determine which faces are visible by the light */
	int i;
	const float olx = olvec.x;
	const float oly = olvec.y;
	const float olz = olvec.z;
	for (i = 0; i < evi.num_elems(); i++) {
		evi[i].visible = (evi[i].a * olx +
				  evi[i].b * oly +
				  evi[i].c * olz +
				  evi[i].d > 0);
	}

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                     GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST);
	glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFL);
	glStencilMask(0xFFFFFFFFL);

	/* generate the shadow volume */
	int svi = this->do_shadow_pass(olx, oly, olz, shadowvol);
	
	glVertexPointer(4, GL_FLOAT, 0, shadowvol);
	if (this->has_compiled_vertex_array) {
		(*this->glLockArraysEXT)(0, svi);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	/* pass 1 - increase stencil value in the shadow */
	glFrontFace(GL_CCW);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glDrawArrays(GL_QUADS, 0, svi/4);
	
	/* pass 2 - decrease stencil value in the shadow */
	glFrontFace(GL_CW);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	glDrawArrays(GL_QUADS, 0, svi/4);
	
	/* pass 3 - remove the light-culled objects from the shadow volume */
	glFrontFace(GL_CCW);

	glBegin(GL_TRIANGLES);
	for (int i = 0, ii = 0; i < evi.num_elems(); i++, ii += 3) {
		if (evi[i].visible) continue;
		const struct vertex * const v1 = &this->vertices[faces[ii]];
		const struct vertex * const v2 = &this->vertices[faces[ii + 1]];
		const struct vertex * const v3 = &this->vertices[faces[ii + 2]];
		glVertex3fv((GLfloat *)v1);
		glVertex3fv((GLfloat *)v2);
		glVertex3fv((GLfloat *)v3);
	}
	glEnd();
	
	if (this->has_compiled_vertex_array) {
		(*this->glUnlockArraysEXT)();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glPopAttrib();
	
	glPopMatrix();
}
void ShadowHandler::end_effect()
{
	ObjHandler::end_effect();
}

int ShadowHandler::do_shadow_pass(const float lx, const float ly, const float lz, GLfloat *shadowvol)
{
	int svi = 0;
	const float lxm = lx * SHADOWLENGTH;
	const float lym = ly * SHADOWLENGTH;
	const float lzm = lz * SHADOWLENGTH;
	
	for (int i = 0, ii = 0; i < evi.num_elems(); i++, ii += 3) {
		if (!evi[i].visible) continue;
		
		for (int j = 0; j < 3; j++) {
			const int ni = evi[i].neighbours[j];
			if (ni != -1 && evi[ni].visible) continue;

			const struct vertex * const v1 = &this->vertices[faces[ii + j]];
			const struct vertex * const v2 = &this->vertices[faces[ii + (j+1)%3]];

			/* calculate the two vertices in distance */
			shadowvol[svi++] = v1->x;
			shadowvol[svi++] = v1->y;
			shadowvol[svi++] = v1->z;
			shadowvol[svi++] = 1.0f;

			/* equivalent to v1->x + (v1->x - lx) * SHADOWLENGTH etc. */
			shadowvol[svi++] = v1->x * (SHADOWLENGTH + 1.0f) - lxm;
			shadowvol[svi++] = v1->y * (SHADOWLENGTH + 1.0f) - lym;
			shadowvol[svi++] = v1->z * (SHADOWLENGTH + 1.0f) - lzm;
			shadowvol[svi++] = 1.0f;
		
			/* equivalent to v2->x + (v2->x - lx) * SHADOWLENGTH etc. */
			shadowvol[svi++] = v2->x * (SHADOWLENGTH + 1.0f) - lxm;
			shadowvol[svi++] = v2->y * (SHADOWLENGTH + 1.0f) - lym;
			shadowvol[svi++] = v2->z * (SHADOWLENGTH + 1.0f) - lzm;
			shadowvol[svi++] = 1.0f;
			
			shadowvol[svi++] = v2->x;
			shadowvol[svi++] = v2->y;
			shadowvol[svi++] = v2->z;
			shadowvol[svi++] = 1.0f;
		}
	}

	return svi;
}

#endif /* DEMOLIB_MAINLOOP */

