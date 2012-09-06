/*
 * This "effect" is almost totally untimed (ie. it counts frames, not
 * seconds).  Rationale: A faster machine will play the fades faster,
 * but it will also _load_ faster, so it's reasonable that it also
 * loads faster ;-) (If you want, do a s/rationale/excuse for being
 * lazy/ ;-) ) Unfortunately, vsync messes this up ;-)
 */

#include "main/piprecalc.h"
#include <stdio.h>
#include <unistd.h>

#define PI_STRING "3.14159265358979323846264338327950288"

/* ahem ;-) */
#ifndef __linux__
#define usleep(x) Sleep(x)
#endif

PiPrecalc::PiPrecalc(GLWindow *win)
{
	this->win = win;
	this->font = texture::load("loaderfont.png");

	this->font->bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	this->last_status = 0;
}

PiPrecalc::~PiPrecalc()
{
	/*
	 * simple fade to zero
	 * (some drivers don't appear to like that we base ourselves
	 * on the last frame, so we have to redraw every time here)
	 */
	for (int i = 0; i < 50; i++) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
	
        	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	        glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		this->font->bind();

		glColor4f(1.0f, 1.0f, 1.0f, (float)(49-i) / 49.0f);
			
		glBegin(GL_QUADS);
		for (int j = 0; j < 37; j++) {
			char ch = PI_STRING[j];
			int t = (ch == '.') ? 7 : (8 + (ch - '0'));
			
			glTexCoord2f((float)(t) / 32.0f, 0.0f);
			glVertex2f((float)(j+2) / 41.0f, 0.45f);
			
			glTexCoord2f((float)(t+1) / 32.0f, 0.0f);
			glVertex2f((float)(j+3) / 41.0f, 0.45f);
			
			glTexCoord2f((float)(t+1) / 32.0f, 0.9f);
			glVertex2f((float)(j+3) / 41.0f, 0.55f);
			
			glTexCoord2f((float)(t) / 32.0f, 0.9f);
			glVertex2f((float)(j+2) / 41.0f, 0.55f);
		}
		glEnd();

		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		this->win->flip();
		usleep(1);
	}
	
	delete this->font;
	this->font = NULL;
}
	
void PiPrecalc::update(float p)
{
	int target = (int)(p * 37.0f);

	for (int i = this->last_status; i < target; i++) {
		for (int fno = 0; fno < 7; fno++) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			
		        glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		        glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);
			this->font->bind();

			/* first rewrite the text :-) */
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			
			/*
			 * then write every letter that we already wrote,
			 * and last the new letter
			 */
			for (int j = 0; j <= i; j++) {
				char ch = PI_STRING[36 - j];
				int t = (ch == '.') ? 7 : (8 + (ch - '0'));

				if (j == i) {
					glColor4f(1.0f, 1.0f, 1.0f, (float)(fno) / 6.0f);
				}
				
				glBegin(GL_QUADS);
				glTexCoord2f((float)(t) / 32.0f, 0.0f);
				glVertex2f((float)(38-j) / 41.0f, 0.45f);
			
				glTexCoord2f((float)(t+1) / 32.0f, 0.0f);
				glVertex2f((float)(38-j+1) / 41.0f, 0.45f);
			
				glTexCoord2f((float)(t+1) / 32.0f, 0.9f);
				glVertex2f((float)(38-j+1) / 41.0f, 0.55f);
			
				glTexCoord2f((float)(t) / 32.0f, 0.9f);
				glVertex2f((float)(38-j) / 41.0f, 0.55f);
				glEnd();
			}
	
			glPopMatrix();
			
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();

			this->win->flip();
			usleep(1);
		}
	}

	this->last_status = target;
}
