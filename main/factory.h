#ifndef _FACTORY_H
#define _FACTORY_H

#include <string.h>

#include "main/mainloop.h"
#include "util/hashtable.h"

class Event;
class MainLoop;

class Factory {
public:
	Factory();
	virtual ~Factory();

	virtual bool can_handle(const char *el) = 0;
	virtual Event *instantiate(MainLoop *ml, const char *title, const char *el, Hashtable *attr) = 0;
	
	virtual char *get_short_effectname() = 0;
	virtual char *get_long_effectname() = 0;
	virtual char *get_display_parameter() = 0;
};

template <class E> class HandlerFactory : public Factory {
private:
	char *elem, *longdesc, *mainparm;

public:
	HandlerFactory(char *element)
	{
		this->elem = strdup(element);
		this->longdesc = NULL;
		this->mainparm = NULL;
	}
	HandlerFactory(char *element, char *longdesc)
	{
		this->elem = strdup(element);
		this->longdesc = strdup(longdesc);
		this->mainparm = NULL;
	}
	HandlerFactory(char *element, char *longdesc, char *mainparm)
	{
		this->elem = strdup(element);
		this->longdesc = strdup(longdesc);
		this->mainparm = strdup(mainparm);
	}
	~HandlerFactory()
	{
		free(this->elem);
		this->elem = NULL;

		free(this->longdesc);
		this->longdesc = NULL;

		free(this->mainparm);
		this->mainparm = NULL;
	}

	bool can_handle(const char *el)
	{
		return (strcmp(this->elem, el) == 0);
	}
	Event *instantiate(MainLoop *ml, const char *title, const char *el, Hashtable *attr)
	{
		return new E(ml, title, el, attr);
	}

	char *get_short_effectname()
	{
		return this->elem;
	}
	
	char *get_long_effectname()
	{
		return this->longdesc;
	}
	
	char *get_display_parameter()
	{
		return this->mainparm;
	}
}; 

#endif /* !defined(_FACTORY_H) */
