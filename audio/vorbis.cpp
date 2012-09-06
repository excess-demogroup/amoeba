/*
 * Plays Ogg Vorbis files using libvorbisfile (could perhaps slash
 * the file size a bit by using only libvorbis+libogg, but it's a
 * bit complicated and I don't think it's worth it). Ogg Vorbis
 * rules, MP3 sucks. Whoa! ;-)
 */

#include <stdlib.h>

#include "audio/vorbis.h"
#include "exception.h"

/*
 * These simply emulate a file to libvorbisfile, using the FileReader
 * class.
 */
size_t mem_readfunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return ((FileReader *) datasource)->fread(ptr, size, nmemb);
}
int mem_seekfunc(void *datasource, ogg_int64_t offset, int whence)
{
//	<Vakor> make sure your seek callback returns -1 if you want to save cpu time,
//	        effort, and so on .
//	(but for jump support, seek is nice)
	return ((FileReader *) datasource)->fseek(offset, whence);
//	return -1;
}
int mem_closefunc(void *datasource)
{
	return 0;
}
long mem_tellfunc(void *datasource)
{
	return ((FileReader *) datasource)->ftell();
}

OggVorbisAudioProvider::OggVorbisAudioProvider(File *file, float jump)
{
	this->vf = (OggVorbis_File *)malloc(sizeof(OggVorbis_File));
	if (this->vf == NULL) {
		throw new FatalException("Out of memory!");
	}

	ov_callbacks mem_callbacks = {
		mem_readfunc,
		mem_seekfunc,
		mem_closefunc,
		mem_tellfunc
	};

	this->vfr = new FileReader(file);
	
	if (ov_open_callbacks(this->vfr, this->vf, NULL, 0, mem_callbacks) < 0)
		throw new FatalException("Couldn't open the Ogg bitstream");
	
	if (jump != 0.0f) {
		ov_time_seek(this->vf, jump);
	}

	/* validate it better later */
}

OggVorbisAudioProvider::~OggVorbisAudioProvider()
{
	ov_clear(this->vf);
	free(this->vf);
	this->vf = NULL;
	
	delete this->vfr;
	this->vfr = NULL;
}

int OggVorbisAudioProvider::fill_buf(char *buf, int bytes)
{
	int junk, ret;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	ret = ov_read(this->vf, buf, bytes, 0, 2, 1, &junk);
#else
	ret = ov_read(this->vf, buf, bytes, 1, 2, 1, &junk);
#endif

	if (ret < 0)
		throw new FatalException("Ogg Vorbis stream error!");

	return ret;
}

