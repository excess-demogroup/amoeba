/*
 * More or less equal to the interference heightmap, except that it's wrapped around
 * a circle instead of on a plane. :-)
 */

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

#include "main/heightmaptunnelhandler.h"
#include "math/vector.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

HeightmapTunnelHandler::HeightmapTunnelHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Object(ml, title, elem, attr)
{
	this->xlod = attr->get_int("xlod");
	this->ylod = attr->get_int("ylod");
	this->speedlin = attr->get_float("speedlin");
	this->freqlinx = attr->get_float("freqlinx");
	this->freqliny = attr->get_float("freqliny");
	this->ampllin = attr->get_float("ampllin");
	
	this->speedcirc = attr->get_float("speedcirc");
	this->freqcirc = attr->get_float("freqcirc");
	this->amplcirc = attr->get_float("amplcirc");
	
	this->pure_indices = true;

	this->vertices_per_face = 4;

	float xradscale = 2.0f * M_PI / (float)(xlod);
	float xscale = 1.0f / (float)xlod;
	float yscale = 1.0f / (float)ylod;
	
	int x, y;
	float xf, xrf, yf;
	for (y = 0, yf = -0.5f; y < ylod; y++, yf += yscale) {
		for (x = 0, xrf = 0.0f, xf = 0.0f; x < xlod; x++, xf += xscale, xrf += xradscale) {
			struct vertex v = { cos(xrf) * 10.0f, sin(xrf) * 10.0f, yf * 100.0f };
			struct texcoord t = { xf, yf + 0.5f };
			struct normal n = { 0.0f, 1.0f, 0.0f };
			this->vertices.add_end(v);
			this->texcoords.add_end(t);
			this->normals.add_end(n);
		}
	}
	
	for (y = 0; y < ylod-1; y++) {
		for (x = 0; x < xlod-1; x++) {
			this->faces.add_end(y*xlod + x);
			this->faces.add_end((y+1)*xlod + x);
			this->faces.add_end((y+1)*xlod + ((x+1)%xlod));
			this->faces.add_end(y*ylod + ((x+1)%xlod));
		}
	}
}

HeightmapTunnelHandler::~HeightmapTunnelHandler()
{
}

void HeightmapTunnelHandler::start_effect()
{
	Object::start_effect();
}
void HeightmapTunnelHandler::draw_scene(float progress)
{
	this->unlock_object();
	float u1 = this->get_val("user1", progress);
	float u2 = this->get_val("user2", progress);
	float u3 = this->get_val("user3", progress);
	float u4 = this->get_val("user4", progress);
	float xscale = 1.0f / (float)(xlod-1);
	float yscale = 1.0f / (float)ylod;
	float xscale2 = freqlinx * xscale;
	float yscale2 = freqliny * yscale;

	const float p_sl = progress * speedlin;
	const float flx_al = freqlinx * ampllin;
	const float fly_al = freqliny * ampllin;
	
	float xf = -u3, xfs = 0.0f;
	for (int x = 0; x < xlod; x++, xf += xscale, xfs += xscale2) {
		const float xxf = xf * 2.0f * M_PI;
		const float xwrap = sin(xxf);
		const float xwrap2 = sin(xfs * 2.0f * M_PI);
		const float cosrot = cos(xxf);
		const float sinrot = sin(xxf);

		float yf = -u4, yfs = 0.0f;
		for (int y = 0; y < ylod; y++, yf += yscale, yfs += yscale2) {
			/* copied more or less directly from interferenceheightmaphandler.cpp :-) */
			const float dist = hypot(xwrap, yf);
#if __GNUC__ && defined(__i386__)
			float sin_v, cos_v;
			__asm__("fsincos" : "=t" (cos_v), "=u" (sin_v) :
				"0" (dist * freqcirc + progress * speedcirc));
#else
			const float temp = dist * freqcirc + progress * speedcirc;
			const float sin_v = sin(temp);
			const float cos_v = cos(temp);
#endif
			
			const float divval = freqcirc * amplcirc / dist;

			const float z = sin_v * amplcirc +
				(sin(xwrap2 + p_sl + u1) +
			         sin(yfs + p_sl + u2)) * ampllin;
			const float dx = cos_v * xwrap * divval
				+ flx_al * cos(xwrap2 + p_sl + u1);
			const float dy = cos_v * yf * divval
				+ fly_al * cos(yfs + p_sl + u2);
		
			Vector normal(dx * -0.2f, dy * -0.2f, 1.0f);
			normal.normalize();

			vertices[y * xlod + x].x = cosrot * (10.0f - z * 2.0f);
			vertices[y * xlod + x].y = sinrot * (10.0f - z * 2.0f);

			/* also rotate the normal */
			normals[y * xlod + x].nx = normal.x *  cosrot + normal.y * sinrot;
			normals[y * xlod + x].ny = normal.x * -sinrot + normal.y * cosrot;
			normals[y * xlod + x].nz = normal.z;
		}
	}
	Object::draw_scene(progress);
}
void HeightmapTunnelHandler::end_effect()
{
	Object::end_effect();
}

#endif /* DEMOLIB_MAINLOOP */

