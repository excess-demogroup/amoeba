/*
 * Generic object class -- not to be used directly, but to be subclassed
 * by actual loaders. (For this reason, it's called ObjHandler, even though
 * one might call it an ObjHandlerHandler.) It handles texture loading, but
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

#include "main/object.h"
#include "main/objhandler.h"
#include "math/array.h"
#include "packer/file.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

ObjHandler::ObjHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Object(ml, title, elem, attr)
{		
	File *obj = load_file(attr->get_str("file"));

	/*
	 * Now parse the object, line by line. Note that files are
	 * NOT usually terminated, so for simplicity's sake,
	 * we add a trailing ASCII 0 here.
	 */
	int length = obj->data_length();
	char *data = new char[length + 1];
	memcpy(data, obj->get_data(), length);
	data[length] = '\0';
	
	this->split_lines(data);

	delete[] data;
	delete obj;
}

ObjHandler::~ObjHandler()
{
}

void ObjHandler::split_lines(char *data)
{
	while (*data) {
		/* find \n or \r\n */
		char *ptr = strchr(data, '\n');

		if (ptr) {
			*ptr = '\0';
			if (ptr[-1] == '\r') ptr[-1] = '\0';
		}

		this->parse_line(data);
		if (ptr) {
			data = ptr + 1;
		} else {
			return;
		}
	}
}

void ObjHandler::parse_line(char *data)
{
	/* look at the line type */
	switch (data[0]) {
	case 'v':
		switch (data[1]) {
		case ' ':
			this->parse_vertex(data + 2);
			break;
		case 't':
			this->parse_texcoords(data + 3);
			break;
		case 'n':
			this->parse_normals(data + 3);
			break;
		}
		break;
	case 'f':
		this->parse_faces(data + 2);
		break;
	case 'g':
		if (strncmp(data, "g presorted", 11) == 0) {
			this->pure_indices = true;
		}
		break;
	case '\0':
	case '#':
	default:
		break;              /* unknown, comment, or blank line */
	}
}

void ObjHandler::parse_vertex(char *data)
{
	struct vertex v;
	
	if (sscanf(data, "%f %f %f", &v.x, &v.y, &v.z) != 3)
		throw new FatalException("Error in .obj data");

	this->add_vertex(v);
}

void ObjHandler::parse_texcoords(char *data)
{
	struct texcoord tc;

	if (sscanf(data, "%f %f", &tc.u, &tc.v) != 2)
		throw new FatalException("Error in .obj data");

	if (this->pure_indices)
		this->texcoords.add_end(tc);
	else
		this->temp_texcoords.add_end(tc);
}

void ObjHandler::parse_normals(char *data)
{
	struct normal n;
	
	if (sscanf(data, "%f %f %f", &n.nx, &n.ny, &n.nz) != 3)
		throw new FatalException("Error in .obj data");

	if (this->pure_indices)
		this->normals.add_end(n);
	else
		this->temp_normals.add_end(n);
}

void ObjHandler::parse_faces(char *data)
{
	if (this->vertices_per_face == 0) {
		/* determine how many groups we have */
		int vpf = 1;
		char *ptr = strchr(data, ' ');
		while (ptr) {
			vpf++;
			ptr = strchr(ptr + 1, ' ');
		}
	
		this->vertices_per_face = vpf;
	}

	int v[4], t[4], n[4], i;
	for (i = 0; i < this->vertices_per_face; i++) {
		if (data == NULL)
			throw new FatalException("Error in .obj data");
		v[i] = atoi(data);
		if (strchr(data, '/') == NULL) {
			if (strchr(data, ' ') == NULL) {
				continue;
			} else {
				data = strchr(data, ' ');
			}
		} else {
			data = strchr(data, '/');
		}

		switch (data[1]) {
		case '/':        /* no texcoord, but a normal */
			data += 1;
			break;
		case ' ':        /* only the vertex */
			data += 2;
			continue;
		case '\0':       /* end of line */
			continue;
		default:         /* a texcoord */		
			t[i] = atoi(data + 1);
			char *ptr = strchr(data, ' ');
			data = strchr(data + 1, '/');
			if (ptr != NULL && ptr < data || data == NULL) data = ptr;
			if (data == NULL && ptr == NULL) continue;
			break;
		}	
		
		/* finally read a normal if there is one */
		switch (data[0]) {
		case '/':
			n[i] = atoi(data + 1);
			data = strchr(data, ' ') + 1;
			break;
		}
	}

	/* now add */
	for (i = 0; i < this->vertices_per_face; i++) {
		this->faces.add_end(v[i] - 1);
		if (this->temp_texcoords.num_elems() > 0)
			this->texcoords.add_end(temp_texcoords[t[i] - 1]);
		if (this->temp_normals.num_elems() > 0)
			this->normals.add_end(temp_normals[n[i] - 1]);
	}
}
	
void ObjHandler::start_effect()
{
	Object::start_effect();
}
void ObjHandler::draw_scene(float progress)
{
	Object::draw_scene(progress);
}
void ObjHandler::end_effect()
{
	Object::end_effect();
}

#endif /* DEMOLIB_MAINLOOP */

