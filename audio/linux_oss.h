#ifndef _LINUX_OSS_H
#define _LINUX_OSS_H 1

#include "audio/audio.h"
#include "main/mainloop.h"

#include <time.h>
#include <sys/time.h>

class OSSAudioDriver : public AudioDriver {
public:
	OSSAudioDriver(AudioProvider *prv, float jump, MainLoop *ml);
	~OSSAudioDriver();

 	bool run();
	float get_time();

protected:
	int oss_fd;
	struct timeval eof_time;

	char outbuf[65536];
	int in_outbuf;

	unsigned int last_fill, bytes_played;
	bool eof;

private:
	inline void set_ioctl(int command, int value);
};

#endif /* !defined(_LINUX_OSS_H) */
