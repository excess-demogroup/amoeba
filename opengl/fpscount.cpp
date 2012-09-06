#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


#include "fpscount.h"
#include "../demolib_prefs.h"
#include "../image/image.h"
#include "../exception.h"

#if DEMOLIB_FPSCOUNTER

#include <GL/gl.h>

FPSCounter::FPSCounter()
{
	Image *digits = load_image("digitsxp.png");
	memcpy(this->digits_bm, digits->get_pixel_data(), 128 * 16 * 4);
	delete digits;
	this->reset();
}

FPSCounter::~FPSCounter()
{
}

void FPSCounter::reset()
{
#ifdef WIN32
	this->tv[0] = GetTickCount();	
	this->tv[1] = GetTickCount();	
	this->tv[2] = GetTickCount();	
	this->tv[3] = GetTickCount();	
	this->tv[4] = GetTickCount();	
	this->tv[5] = GetTickCount();	
	this->tv[6] = GetTickCount();	
	this->tv[7] = GetTickCount();	
	this->tv[8] = GetTickCount();	
	this->tv[9] = GetTickCount();	
#else	
	gettimeofday(&(this->tv[0]), NULL);
	gettimeofday(&(this->tv[1]), NULL);
	gettimeofday(&(this->tv[2]), NULL);
	gettimeofday(&(this->tv[3]), NULL);
	gettimeofday(&(this->tv[4]), NULL);
	gettimeofday(&(this->tv[5]), NULL);
	gettimeofday(&(this->tv[6]), NULL);
	gettimeofday(&(this->tv[7]), NULL);
	gettimeofday(&(this->tv[8]), NULL);
	gettimeofday(&(this->tv[9]), NULL);
#endif
}

float FPSCounter::getfps()
{
	float fps;
	
#ifdef WIN32
	long now;
//	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	now = GetTickCount();	
#else
	struct timeval now;
	gettimeofday(&now, NULL);
#endif

#ifdef WIN32
	fps = 10000.0f / (float)((now - this->tv[0]));
#else
	fps = 10000000.0f / 
		(float)((now.tv_sec  - this->tv[0].tv_sec) * 1000000 +
			(now.tv_usec - this->tv[0].tv_usec));
#endif

	tv[0] = tv[1];
	tv[1] = tv[2];
	tv[2] = tv[3];
	tv[3] = tv[4];
	tv[4] = tv[5];
	tv[5] = tv[6];
	tv[6] = tv[7];
	tv[7] = tv[8];
	tv[8] = tv[9];
	tv[9] = now;

	return fps;
}

void FPSCounter::draw()
{
	char fpsstring[10];
	int i;
//	GLuint texnum;

	float fps = this->getfps();

	sprintf(fpsstring, "%6.2ffps", fps);

	memset(texbuf, 0x00, 64*16*4);

	for (i = 0; i <= 8; i++) {
		int offs = 0;
		if (fpsstring[i] == ' ') continue;
		switch (fpsstring[i]) {
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
		for (int y = 0; y < 10; y++) {
			memcpy(this->texbuf + y*64*4 + i*6*4, this->digits_bm + y*128*4 + offs*4, 24);
		}
	}

	/*glGenTextures(1, &texnum); */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 64, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->texbuf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

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
	glEnable(GL_TEXTURE_2D);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
       
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(0.8f,0.9f,0.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(1.0f,0.9f,0.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(1.0f,1.0f,0.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(0.8f,1.0f,0.0f);
	glEnd();

	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

//	glDeleteTextures(1, &texnum);
}
#endif
