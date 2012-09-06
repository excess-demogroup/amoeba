/*
 * Some general abstract base classes. The AudioDriver gets input from the
 * AudioProvider and plays it.
 */
#ifndef _AUDIO_H
#define _AUDIO_H 1

#include "packer/file.h"

class AudioProvider {
public:
	virtual ~AudioProvider() {};
	
	virtual int fill_buf(char *buf, int bytes) = 0;

protected:
	AudioProvider() {};
};

class AudioDriver {
public:
	virtual ~AudioDriver() {};

 	virtual bool run() = 0;
	virtual float get_time() = 0;

protected:
	AudioDriver() {};
	AudioProvider *prov;
};

#endif /* !defined(_AUDIO_H) */
