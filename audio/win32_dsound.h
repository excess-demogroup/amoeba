#ifndef _WIN32_DSOUND_H
#define _WIN32_DSOUND_H 1

#define NOCOMINTERFACE
#define WIN32_LEAN_AND_MEAN
#define CINTERFACE
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "audio/audio.h"
#include "main/mainloop.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class DirectSoundAudioDriver : public AudioDriver {
public:
        DirectSoundAudioDriver(AudioProvider *prv, float jump, MainLoop *ml);
        ~DirectSoundAudioDriver();

        bool run();
        float get_time();

protected:
        LPDIRECTSOUND dsound;
        LPDIRECTSOUNDBUFFER primary_buf;
        LPDIRECTSOUNDBUFFER stream_buf;
        unsigned int last_played;

        int bytes_played;
	unsigned int last_fill;
	
	bool eof;
	MainLoop *ml;

	int fill_buf_completely(char *buf, int bytes);
};

#endif /* !defined(_WIN32_DSOUND_H) */
