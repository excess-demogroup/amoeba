#include <stdio.h>
#include <string.h>
#include <GL/gl.h>

#include "main/timerhandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP 

TimerHandler::TimerHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "value")
{
	this->digits = texture::load("digitsxp.png");
}

TimerHandler::~TimerHandler()
{
	texture::free(this->digits);
	this->digits = NULL;
}

void TimerHandler::start_effect()
{
}

void TimerHandler::draw_scene(float progress)
{
	char buf[10];
	sprintf(buf, "%6.2f", this->get_val("value", progress));

	const float pix = 1.0f / 127.0f;
	const float vpix = 0.2f / 9.0f;
		
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0f,1.0f,0.0f,1.0f,0.0f,10.0f);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	this->digits->bind();
        glEnable(GL_TEXTURE_2D);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
        for (int i = 0; i < 6; i++) {
                int offs = 0;
                if (buf[i] == ' ') continue;
                switch (buf[i]) {
                        case '0': offs =  0; break;
                        case '1': offs =  6; break;
                        case '2': offs = 12; break;
                        case '3': offs = 18; break;
                        case '4': offs = 24; break;
                        case '5': offs = 30; break;
                        case '6': offs = 36; break;
                        case '7': offs = 42; break;
                        case '8': offs = 48; break;
                        case '9': offs = 54; break;
                        case '.': offs = 60; break;
                        case 'f': offs = 66; break;
                        case 'p': offs = 72; break;
                        case 's': offs = 78; break;
                }

                glTexCoord2f(offs * pix + 0.001f, 1.0f);
                glVertex3f(vpix * i, 0.9f, 0.0f);
                glTexCoord2f((offs+6) * pix, 1.0f);
                glVertex3f(vpix * (i+1), 0.9f, 0.0f);
                glTexCoord2f((offs+6) * pix, 0.0f);
                glVertex3f(vpix * (i+1), 1.0f, 0.0f);
                glTexCoord2f(offs * pix + 0.001f, 0.0f);
                glVertex3f(vpix * i, 1.0f, 0.0f);
	}
	glEnd();
	
	glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

}
void TimerHandler::end_effect() {}

#endif
