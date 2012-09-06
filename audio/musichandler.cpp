/*
 * The general sound system. In theory, this is meant to be flexible enough
 * to support softsynths etc.. Who knows if it will work -- only time will
 * show :-)
 */

#define NOCOMINTERFACE
#define WIN32_LEAN_AND_MEAN
#define CINTERFACE

#include "packer/file.h"
#include "exception.h"
#include "demolib_prefs.h"

#include "audio/musichandler.h"
#include "audio/audio.h"
#include "audio/vorbis.h"

#define DEMOLIB_SOUND_PROVIDER OggVorbisAudioProvider
	
#if __unix__
#include "audio/linux_oss.h"
#define DEMOLIB_SOUND_DRIVER OSSAudioDriver
#else
#include "audio/win32_dsound.h"
#define DEMOLIB_SOUND_DRIVER DirectSoundAudioDriver
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

MusicHandler::MusicHandler(MainLoop *ml, const char *title, const char *elem,
		       Hashtable *attr) :
	Event(ml, title, elem, attr, NULL)
{
	this->musicfile = load_file(attr->get_str("file"));
	if (attr->exists("jump")) {
		this->jump = attr->get_float("jump");
	} else {
		this->jump = 0.0f;
	}
	this->prov = new DEMOLIB_SOUND_PROVIDER(this->musicfile, jump);
	this->ml = ml;
	if (this->ml)
		this->ml->timer = this;
	this->drv = NULL;
}

MusicHandler::~MusicHandler()
{
	delete this->prov;
	this->prov = NULL;
}

void MusicHandler::start_effect()
{
	this->drv = new DEMOLIB_SOUND_DRIVER(this->prov, jump, this->ml);
}

void MusicHandler::end_effect()
{
	delete this->drv;
	this->drv = NULL;
}

void MusicHandler::draw_scene(float progress)
{
	/* ignore the eof info */
	if (this->drv) {
		(void)this->drv->run();
	}
}

float MusicHandler::get_time()
{
	if (this->drv == NULL) {
		return 0.0f;
	} else {
		return this->drv->get_time();
	}
}
