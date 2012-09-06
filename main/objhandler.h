#ifndef _OBJHANDLER_H
#define _OBJHANDLER_H 1

#include "main/event.h"
#include "main/mainloop.h"
#include "packer/file.h"
#include "main/object.h"
#include "math/array.h"

class ObjHandler : public Object
{
protected:	
	void split_lines(char *data);
	void parse_line(char *data);
	void parse_vertex(char *data);
	void parse_texcoords(char *data);
	void parse_normals(char *data);
	void parse_faces(char *data);

	Array<texcoord> temp_texcoords;
	Array<normal> temp_normals;

public:
	ObjHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~ObjHandler();

	virtual void start_effect();
	virtual void draw_scene(float progress);
	virtual void end_effect();
};

#endif /* !defined(_OBJHANDLER_H) */
