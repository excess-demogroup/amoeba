#include <stdio.h>
#include <string.h>
#include <expat.h>

#include "event.h"
#include "main/curve.h"
#include "main/linearcurve.h"
#include "main/autosplinecurve.h"
#include "main/linecurve.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

Event::Event(MainLoop *ml, const char *title, const char *elem, Hashtable *attr, const char *curvenames)
{
	int curve_type = CURVE_NONE;

	this->start = -1.0f;
	this->stop = -1.0f;
	this->active = false;
	this->title = strdup(title);

	if (attr->exists("layer")) {
		this->layer = attr->get_float("layer");
	} else {
		this->layer = 0.0f;
	}

	this->curves = NULL;
	if (attr->exists("curve")) {
		const char * const ctstr = (const char * const)(attr->lookup("curve"));
		if (strcmp(ctstr, "linear") == 0) {
			curve_type = CURVE_LINEAR;
		}
		if (strcmp(ctstr, "autospline") == 0) {
			curve_type = CURVE_AUTOSPLINE;
		}
		if (strcmp(ctstr, "line") == 0) {
			curve_type = CURVE_LINE;
		}
		if (curve_type == CURVE_NONE) {
			throw new FatalException(ctstr, "Invalid curve type.");
		}
	}

	if (curvenames == NULL || strcmp(curvenames, "") == 0) return;
	if (curve_type == CURVE_NONE) {
		throw new FatalException(elem, "No curve type specified.");
	}

	this->curves = new Hashtable(true);

	/* now create a set of attribute curves */
	char *cn = strdup(curvenames);
	char *curve = strtok(cn, ":");

	do {
		Curve *c = NULL;

		switch (curve_type) {
		case CURVE_LINEAR:
			c = new LinearCurve();
			break;
		case CURVE_AUTOSPLINE:
			c = new AutoSplineCurve();
			break;
		case CURVE_LINE:
			c = new LineCurve();
			break;
		}

		this->curves->insert(curve, c);
	} while ((curve = strtok(NULL, ":")) != NULL);

	free(cn);

	this->ml = ml;
}

Event::~Event()
{
	free(this->title);

	/* free the hash tables for the curves */
	if (this->curves != NULL) {
		this->curves->destroy_values();
		delete this->curves;
		this->curves = NULL;
	}
}

void Event::add_curvepoint(Hashtable *markers, const char *element, const char **attr)
{
	float timeval = -1.0f;

	/* first we need to grab the time */
	int i = 0;
	while (attr[i]) {
		if (strcmp(attr[i], "time") == 0) {
			timeval = parse_time(markers, attr[i + 1]);
			break;
		}
		i += 2;
	}
	if (timeval == -1.0f) {
		throw new FatalException(element, "Missing time= attribute");
	}

	if (strcmp(element, "start") == 0) {
		if (this->start != -1.0f) {
			throw new FatalException(element, "Multiple <start> tags");
		}
		this->start = timeval;
	}
	if (strcmp(element, "end") == 0) {
		if (this->stop != -1.0f) {
			throw new FatalException(element, "Multiple <end> tags");
		}
		this->stop = timeval;
	}

	i = 0;
	while (attr[i]) {
		if (strcmp(attr[i], "time") == 0) {
			i += 2;
			continue;
		}

		if (this->curves == NULL) {
			throw new FatalException(attr[i], "No such curve!");
		}

		Curve *c = (Curve *)this->curves->lookup(attr[i]);
		if (c == NULL) {
			throw new FatalException(attr[i], "No such curve!");
		}
		c->add_curvepoint(timeval, atof(attr[i + 1]));
		i += 2;
	}
}

void Event::end_curvedata()
{
	if (this->start == -1.0f) {
		throw new FatalException("Missing <start> tag!");
	}
	if (this->stop == -1.0f) {
		throw new FatalException("Missing <end> tag!");
	}

	if (this->curves != NULL) {
		this->curves->finish_curvedata(this->start, this->stop);
	}
}

void Event::start_effect() {}
void Event::draw_scene(float progress) {}
void Event::end_effect() {}

float Event::get_val(const char *attr_name, float progress)
{
	if (this->curves == NULL) {
		throw new FatalException(attr_name, "No curves defined in effect!");
	}
	Curve *c = (Curve *)this->curves->lookup(attr_name);
	if (c == NULL) {
		throw new FatalException(attr_name, "No such curve defined!");
	}

	return c->get_value(progress);
}

float Event::parse_time(Hashtable *markers, const char *timestr)
{
	if (timestr[0] < '0' || timestr[0] > '9') {
		/* a marker name -- does it contain a '-' or '+'? */
		char buf[256];
		float adj;

		strcpy(buf, timestr);
		unsigned int p = strcspn(buf, "-+");

		if (p == strlen(buf)) {
			adj = 0.0f;
		} else {
			if (buf[p] == '+') {
				adj = atof(buf + p + 1);
			} else {
				adj = atof(buf + p);
			}

			buf[p] = '\0';
		}

		void *f = markers->lookup(buf);
		if (f == NULL) {
			throw new FatalException(buf, "No such marker.");
		}
		return *((float *)f) + adj;
	} else {
		return atof(timestr);
	}
}
#endif
