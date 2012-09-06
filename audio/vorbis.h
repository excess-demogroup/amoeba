#ifndef _VORBIS_H
#define _VORBIS_H 1

#include "audio/audio.h"
#include "packer/filereader.h"
#include <vorbis/vorbisfile.h>

class MusicBar;

class OggVorbisAudioProvider : public AudioProvider {
public:
	OggVorbisAudioProvider(File *file, float jump);
	~OggVorbisAudioProvider();
        
        int fill_buf(char *buf, int bytes);
	
protected:
	OggVorbis_File *vf;
        FileReader *vfr;

	friend class MusicBar;
};

#endif /* !defined(_VORBIS_H) */
