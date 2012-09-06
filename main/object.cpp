/*
 * Generic object class -- not to be used directly, but to be subclassed
 * by actual loaders. (For this reason, it's called Object, even though
 * one might call it an ObjectHandler.) It handles texture loading, but
 * not anything else related to files.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>

#include "math/array.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "main/object.h"
#include "opengl/texture.h"
#include "opengl/extensions.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

Object::Object(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
        Event(ml, title, elem, attr, "xrot:yrot:zrot:xpos:ypos:zpos:scale:red:green:blue:alpha:user1:user2:user3:user4")
{	
	if (attr->exists("stencilmask")) {
		this->stencil_mask = attr->get_int("stencilmask");
	} else {
		this->stencil_mask = 0x0;
	}
	if (attr->exists("halo") && attr->get_bool("halo")) {
		this->draw_halo = true;
		this->halo_length = attr->get_float("halolength");

		/* ensure there is a stencil mask ;-) */
		this->stencil_mask = attr->get_int("stencilmask");
	} else {
		this->draw_halo = false;
	}
	this->halo_vert = NULL;

	this->inverse_rotorder = false;
	if (attr->exists("invrot")) {
		this->inverse_rotorder = attr->get_bool("invrot");
	}
	
	this->additive_blend = false;
	if (attr->exists("blend")) {
		char *str = attr->get_str("blend");
		if (strcmp(str, "blend") == 0) {
			this->additive_blend = false;
		} else if (strcmp(str, "add") == 0) {
			this->additive_blend = true;
		} else {
			throw new FatalException(elem, "blend= must be `blend' or `add'!");
		}
	}
		
	/* this code got a bit ickier with hashes :-( */
	this->vertices_per_face = 0;
	this->tex[0] = this->tex[1] = NULL;
	this->texgen[0] = this->texgen[1] = none;

	if (attr->exists("texstr0")) {
		this->texstr0 = attr->get_float("texstr0");
	} else {
		this->texstr0 = 1.0f;
	}

	int num_texunit0 = 0;
	int num_texunit1 = 0;

	num_texunit0 += (attr->exists("texture")) ? 1 : 0;
	num_texunit0 += (attr->exists("texture0")) ? 1 : 0;
	num_texunit0 += (attr->exists("envmap")) ? 1 : 0;
	num_texunit0 += (attr->exists("envmap0")) ? 1 : 0;
	num_texunit0 += (attr->exists("linmap")) ? 1 : 0;
	num_texunit0 += (attr->exists("linmap0")) ? 1 : 0;
	
	num_texunit1 += (attr->exists("texture1")) ? 1 : 0;
	num_texunit1 += (attr->exists("envmap1")) ? 1 : 0;
	num_texunit1 += (attr->exists("linmap1")) ? 1 : 0;
	
	if (num_texunit0 > 1)
		throw new FatalException("Must have only one of texture/texture0, envmap/envmap0 and linmap/linmap0!");
	if (num_texunit1 > 1)
		throw new FatalException("Must have only one of texture1, envmap1 and linmap1!");
			
	if (attr->exists("texture"))
		this->tex[0] = texture::load(attr->get_str("texture"));
	if (attr->exists("texture0"))
		this->tex[0] = texture::load(attr->get_str("texture0"));
	
	if (attr->exists("envmap")) {
		this->tex[0] = texture::load(attr->get_str("envmap"));
		this->texgen[0] = envmap;
	}
	if (attr->exists("envmap0")) {
		this->tex[0] = texture::load(attr->get_str("envmap0"));
		this->texgen[0] = envmap;
	}
	
	if (attr->exists("linmap")) {
		this->tex[0] = texture::load(attr->get_str("linmap"));
		this->texgen[0] = linmap;
	}
	if (attr->exists("linmap0")) {
		this->tex[0] = texture::load(attr->get_str("linmap0"));
		this->texgen[0] = linmap;
	}
	
	if (attr->exists("texture1"))
		this->tex[1] = texture::load(attr->get_str("texture1"));
	if (attr->exists("envmap1")) {
		this->tex[1] = texture::load(attr->get_str("envmap1"));
		this->texgen[1] = envmap;
	}
	if (attr->exists("linmap1")) {
		this->tex[1] = texture::load(attr->get_str("linmap1"));
		this->texgen[1] = linmap;
	}

	if (this->tex[1] && !this->tex[0])
		throw new FatalException(elem,
			"texture1/envmap1/linmap1 specified, but not texture0/envmap0/linmap0!");

	if (GLExtensions::has_ext("GL_ARB_multitexture") &&
	    GLExtensions::has_ext("GL_ARB_texture_env_combine")) {
		this->has_multitexture = true;
		this->glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
			GLExtensions::func_ptr("glActiveTextureARB");
		this->glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)
			GLExtensions::func_ptr("glClientActiveTextureARB");
		this->glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)
			GLExtensions::func_ptr("glMultiTexCoord2fvARB");
	} else {
		this->has_multitexture = false;
	}

	if (GLExtensions::has_ext("GL_EXT_compiled_vertex_array")) {
		this->has_compiled_vertex_array = true;
		this->glLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)
			GLExtensions::func_ptr("glLockArraysEXT");
		this->glUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)
			GLExtensions::func_ptr("glUnlockArraysEXT");
	} else {
		this->has_compiled_vertex_array = false;
	}

	if (GLExtensions::has_ext("GL_EXT_draw_range_elements")) {
		this->has_draw_range_elements = true;
		this->glDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)
			GLExtensions::func_ptr("glDrawRangeElementsEXT");
	} else {
		this->has_draw_range_elements = false;
	}
	
	this->has_rescale_normal =
		GLExtensions::has_ext("GL_EXT_rescale_normal");

	bool interpolate_tex = true;
	
	if (attr->exists("mode")) {
		char *str = attr->get_str("mode");
		if (strcmp(str, "interpolate") == 0) {
			interpolate_tex = true;
		} else if (strcmp(str, "add") == 0) {
			interpolate_tex = false;
		} else {
			throw new FatalException(elem, "mode= must be `interpolate' or `add'!");
		}
	}

	if (interpolate_tex) {
		this->tu1combine = GL_INTERPOLATE_EXT;
		this->tu1s2 = GL_PRIMARY_COLOR_EXT;
		this->tu1o2 = GL_SRC_ALPHA;
		
		this->p2sf = GL_SRC_ALPHA;
		this->p2df = GL_ONE_MINUS_SRC_ALPHA;
	} else {
		this->tu1combine = GL_ADD;
		
		/* doesn't matter, but set to something so we won't generate errors */
		this->tu1s2 = GL_PRIMARY_COLOR_EXT;
		this->tu1o2 = GL_SRC_ALPHA;
		
		this->p2sf = GL_ONE;
		this->p2df = GL_ONE;
	}				
	
	this->tu1s0 = GL_PREVIOUS_EXT;
	this->tu1s1 = GL_TEXTURE;

	this->tu1o0 = GL_SRC_COLOR;
	this->tu1o1 = GL_SRC_COLOR;

	this->p2light = false;

	this->no_zbuffer = false;
	
	this->tv = NULL;
	this->pure_indices = false;

	this->precolor = false;
	this->pretrans = false;

	this->ctexcoords[0] = NULL;
	this->ctexcoords[1] = NULL;
}

Object::~Object()
{
	texture::free(this->tex[0]);
	this->tex[0] = NULL;
	texture::free(this->tex[1]);
	this->tex[1] = NULL;
	delete this->ctexcoords[0];
	this->ctexcoords[0] = NULL;
	delete this->ctexcoords[1];
	this->ctexcoords[1] = NULL;
}

void Object::add_vertex(const struct vertex v)
{
	this->vertices.add_end(v);
}

void Object::add_normal(const struct normal n)
{
	this->normals.add_end(n);
}

void Object::add_texcoord(const struct texcoord tc)
{
	this->texcoords.add_end(tc);
}

void Object::add_face(const int a, const int b, const int c)
{
	this->faces.add_end(a);
	this->faces.add_end(b);
	this->faces.add_end(c);
}

void Object::add_face(const int a, const int b, const int c, const int d)
{
	this->faces.add_end(a);
	this->faces.add_end(b);
	this->faces.add_end(c);
	this->faces.add_end(d);
}

void Object::unlock_object()
{
	if (this->tv || this->pure_indices) {
		delete this->tv;
		this->tv = NULL;
		delete[] this->halo_vert;
		this->halo_vert = NULL;
	}
}

void Object::setup_vertex_arrays(bool textures, bool normals)
{
	if (this->pure_indices) {
		glVertexPointer(3, GL_FLOAT, 0, this->vertices.get_array());
	} else {
		glVertexPointer(3, GL_FLOAT, 0, tv->get_array());
	}
	glEnableClientState(GL_VERTEX_ARRAY);

	if (textures) {
		if (this->has_multitexture && this->tex[1]) {
			(*this->glClientActiveTextureARB)(GL_TEXTURE1_ARB);
			if (this->texgen[1] == none) {
				glTexCoordPointer(2, GL_FLOAT, 0, this->texcoords.get_array());
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			} else {
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			(*this->glClientActiveTextureARB)(GL_TEXTURE0_ARB);
		}
		
		if (this->tex[0] && (this->texgen[0] == none)) {
			glTexCoordPointer(2, GL_FLOAT, 0, this->texcoords.get_array());
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	
	if (normals) {
		glNormalPointer(GL_FLOAT, 0, this->normals.get_array());
		glEnableClientState(GL_NORMAL_ARRAY);
	} else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}
}

void Object::draw_object()
{
	this->draw_object((this->texcoords.num_elems() > 0 && this->tex[0]),
			  (this->normals.num_elems() > 0));
}

void Object::draw_object(bool textures, bool normals)
{
	if (stencil_mask != 0x0) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, stencil_mask, stencil_mask);
		glStencilMask(stencil_mask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	if (this->has_multitexture && textures && this->tex[1]) {
		/* unit 0 */
		(*this->glActiveTextureARB)(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
		this->tex[0]->bind();

		/* the texgen0 checking is done a bit later */
		
		/* unit 1 */
		(*this->glActiveTextureARB)(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, this->tu1combine);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, this->tu1s0);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, this->tu1s1);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, this->tu1s2);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, this->tu1o0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, this->tu1o1);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, this->tu1o2);
		this->tex[1]->bind();

		if (this->texgen[1] != none) {
			GLenum mode = (this->texgen[1] == envmap) ? GL_SPHERE_MAP : GL_OBJECT_LINEAR;
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		} else {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
		
		(*this->glActiveTextureARB)(GL_TEXTURE0_ARB);

		/* we don't blend with multitexturing */
		glDisable(GL_BLEND);
		
		/* set the relative texture strengths */
		glColor4f(1.0f, 1.0f, 1.0f, this->texstr0);
	} else {
		/* okay, no single-pass multitexturing */
		if (textures && this->tex[0]) {
			glEnable(GL_TEXTURE_2D);
			this->tex[0]->bind();
		} else {
			glDisable(GL_TEXTURE_2D);
		}
	}

	if (textures) {
		if (this->texgen[0] != none) {
			GLenum mode = (this->texgen[0] == envmap) ? GL_SPHERE_MAP : GL_OBJECT_LINEAR;
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		} else {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}

	if (normals) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}

	GLenum primitive;
	switch (this->vertices_per_face) {
		case 1: primitive = GL_POINTS;	break;
		case 2: primitive = GL_LINES;	break;
		case 3: primitive = GL_TRIANGLES;	break;
		case 4: primitive = GL_QUADS;	break;
		default: primitive = GL_TRIANGLE_STRIP;
	}

	/* 
	 * one might collapse these into fewer ifs, but this
	 * is better for speed... this `main' code branch is done in
	 * three cases:
	 *
	 * 1. no texturing at all
	 * 2. only texmap0/envmap0/linmap0
	 * 3. envmap1/linmap1 and GL_ARB_multitexture
	 */
	const int f = this->faces.num_elems();

	/*
	 * If the vertex array hasn't been made yet (or has been
	 * invalidated by unlock_object()), make it now :-)
	 */
	if (this->tv == NULL && !this->pure_indices) {
		this->tv = new Array<GLfloat>;
		for (int i = 0; i < f; i++) {
			tv->add_end(this->vertices[this->faces[i]].x);
			tv->add_end(this->vertices[this->faces[i]].y);
			tv->add_end(this->vertices[this->faces[i]].z);
		}
	}

	this->setup_vertex_arrays(textures, normals);
	if (this->has_compiled_vertex_array) {
		if (this->pure_indices) {
			(*this->glLockArraysEXT)(0, this->vertices.num_elems());
		} else {
			(*this->glLockArraysEXT)(0, tv->num_elems());
		}
	}
	
	if (!textures || !this->tex[1] || this->has_multitexture) {
		if (this->pure_indices) {
			if (this->has_draw_range_elements) {
				(*this->glDrawRangeElementsEXT)(primitive,
						    		0,
						 		this->vertices.num_elems() - 1,
								this->faces.num_elems(),
								GL_UNSIGNED_INT,
								this->faces.get_array());
			} else {
				glDrawElements(primitive,
					       this->faces.num_elems(),
					       GL_UNSIGNED_INT,
					       this->faces.get_array());
			}
		} else {
			glDrawArrays(primitive, 0, this->faces.num_elems());
		}
	} else {
		/*
		 * texmap1/envmap1/linmap1, and we don't have GL_ARB_multitexture. Multipass
		 * rendering to the rescue!
		 */
		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		for (int p = 0; p < 2; p++) {
			this->tex[p]->bind();
			
			if (this->texgen[p] != none) {
				GLenum mode = (this->texgen[p] == envmap) ? GL_SPHERE_MAP : GL_OBJECT_LINEAR;
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);

				/* causes problems on broken Matrox  cards */
				//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			} else {
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}
				
			if (p == 1) {
				glBlendFunc(this->p2sf, this->p2df);
				glEnable(GL_BLEND);
				glEnable(GL_COLOR_MATERIAL);
				if (!this->p2light)
					glDisable(GL_LIGHTING);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f - this->texstr0);
				glDepthMask(GL_FALSE);
				glDepthFunc(GL_EQUAL);
			}
			
			if (this->pure_indices) {
				if (this->has_draw_range_elements) {
					(*this->glDrawRangeElementsEXT)(primitive,
							    		0,
							 		this->vertices.num_elems()  - 1,
									this->faces.num_elems(),
									GL_UNSIGNED_INT,
									this->faces.get_array());
				} else {
					glDrawElements(primitive,
						       this->faces.num_elems(),
						       GL_UNSIGNED_INT,
						       this->faces.get_array());
				}
			} else {
				glDrawArrays(primitive, 0, this->faces.num_elems());
			}
		}
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
	
	if (this->has_multitexture && textures && this->tex[1]) {
		(*this->glActiveTextureARB)(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
			
		(*this->glActiveTextureARB)(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		(*this->glActiveTextureARB)(GL_TEXTURE0_ARB);
	}

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	/* okay, it _is_ ugly, but still the best place to have it */
	if (this->draw_halo) {
		if (!this->pure_indices)
			throw new FatalException("Halo needs presorted objects!");
	
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0x0, this->stencil_mask);
		glStencilMask(this->stencil_mask);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		
		const float hl = this->halo_length;
		if (this->halo_vert == NULL) {
			this->halo_vert = new vertex[vertices.num_elems()];
			for (int i = 0; i < vertices.num_elems(); i++) {
				halo_vert[i].x = vertices[i].x + this->normals[i].nx * hl;
				halo_vert[i].y = vertices[i].y + this->normals[i].ny * hl;
				halo_vert[i].z = vertices[i].z + this->normals[i].nz * hl;
			}
		}
	
		if (this->has_compiled_vertex_array) {
			(*this->glUnlockArraysEXT)();
		}

		glVertexPointer(3, GL_FLOAT, 0, (GLfloat *)halo_vert);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		if (this->has_draw_range_elements) {
			(*this->glDrawRangeElementsEXT)(primitive,
					    		0,
					 		this->vertices.num_elems() - 1,
							this->faces.num_elems(),
							GL_UNSIGNED_INT,
							this->faces.get_array());
		} else {
			glDrawElements(primitive,
				       this->faces.num_elems(),
				       GL_UNSIGNED_INT,
				       this->faces.get_array());
		}
		glDisable(GL_STENCIL_TEST);
		glStencilMask(0xFFFFFFFF);
	}
	
	glPopMatrix();
	if (this->has_compiled_vertex_array) {
		(*this->glUnlockArraysEXT)();
	}
}

void Object::start_effect()
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Object::setup_drawing(float progress)
{
	/*
	 * add Z sorting later, perhaps?
	 */
	
	if (!this->no_zbuffer) {
		glEnable(GL_DEPTH_TEST);
	}
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	if (this->has_rescale_normal) {
		glEnable(GL_RESCALE_NORMAL);
	} else {
		glEnable(GL_NORMALIZE);
	}

	if (this->additive_blend) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

//	glDisable(GL_COLOR_MATERIAL);

	if (!this->precolor) {
		float r = this->get_val("red", progress);
		float g = this->get_val("green", progress);
		float b = this->get_val("blue", progress);
		float alpha = this->get_val("alpha", progress);

		GLfloat material[] = { r, g, b, alpha };
		glColor4f(r, g, b, alpha);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material);
	}

	if (!this->pretrans) {
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
	}
}

void Object::draw_scene(float progress)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	this->setup_drawing(progress);
	this->draw_object();
	
	glPopMatrix();
}

void Object::end_effect()
{
}

#endif /* DEMOLIB_MAINLOOP */

