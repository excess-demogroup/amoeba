/*
 * A quite standard OSS (Linux) output driver. Uses fragments and byte
 * counters, so your soundcard driver would better support it :-) I've
 * heard not everybody do, but never had a bug report on it yet, at
 * least :-)
 */

#include "audio/linux_oss.h"
#include "exception.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>
#include <endian.h>

OSSAudioDriver::OSSAudioDriver(AudioProvider *prv, float jump, MainLoop *lp)
{
	this->prov = prv;

	this->oss_fd = open("/dev/sound/dsp", O_WRONLY);
	if (this->oss_fd == -1) {
	        this->oss_fd = open("/dev/dsp", O_WRONLY);
	        if (this->oss_fd == -1)
	                throw new FatalException("/dev/dsp", strerror(errno));
	}

	this->set_ioctl(SNDCTL_DSP_RESET, 1);
	this->set_ioctl(SOUND_PCM_WRITE_RATE, 44100);
	this->set_ioctl(SOUND_PCM_WRITE_CHANNELS, 2);
	this->set_ioctl(SNDCTL_DSP_SETFMT, AFMT_S16_NE);
	this->set_ioctl(SNDCTL_DSP_NONBLOCK, 1);
	this->set_ioctl(SNDCTL_DSP_SETFRAGMENT, 0x7fff000c);
	
	this->bytes_played = (int)(jump * 44100.0f) * 4;
	this->eof = false;
	this->in_outbuf = 0;

	{
		struct audio_buf_info abinfo;
		if (ioctl(this->oss_fd, SNDCTL_DSP_GETOSPACE, &abinfo) == -1)
			throw new FatalException("Couldn't call SNDCTL_DSP_GETOSPACE!");
		this->last_fill = abinfo.bytes;
	}
}

OSSAudioDriver::~OSSAudioDriver()
{
}

inline void OSSAudioDriver::set_ioctl(int command, int value)
{
	ioctl(this->oss_fd, command, &value);
}

bool OSSAudioDriver::run()
{
	/* find out how many fragments are free */
	struct audio_buf_info abinfo;
	if (ioctl(this->oss_fd, SNDCTL_DSP_GETOSPACE, &abinfo) == -1)
		throw new FatalException("Couldn't call SNDCTL_DSP_GETOSPACE!");

	/* record how much was actually played */
	this->bytes_played += abinfo.bytes - last_fill;
	last_fill = abinfo.bytes;

	if (this->eof) {
		struct timeval now;
		gettimeofday(&now, NULL);

		float secs =
			(float)(now.tv_sec  - eof_time.tv_sec) +
			(float)(now.tv_usec - eof_time.tv_usec) * 0.000001f;
		this->bytes_played += (int)(secs * 44100.0f * 4.0f);
		memcpy(&this->eof_time, &now, sizeof(struct timeval));
		return true;
	}

	while (abinfo.fragments > 0
	       && abinfo.fragments * abinfo.fragsize >= 8192) {
		/* whopee, we can output :-) */
		int ret = this->prov->fill_buf(this->outbuf + in_outbuf, 65536 - in_outbuf);

		if (ret == 0) {
			/* switch to software timer */
			this->eof = true;
			gettimeofday(&this->eof_time, NULL);
			return true;
		}
		in_outbuf += ret;

		/* write only whole fragments */
		if (in_outbuf >= abinfo.fragsize) {
			int to_write =
			    in_outbuf - (in_outbuf % abinfo.fragsize);

			to_write = write(this->oss_fd, this->outbuf, to_write);
			in_outbuf -= to_write;

			memmove(outbuf, outbuf + to_write, in_outbuf);

			/* avoid another ioctl */
			abinfo.fragments -= to_write / abinfo.fragsize;
			abinfo.bytes -= to_write;

			last_fill = abinfo.bytes;
		}
	}
	return false;
}

float OSSAudioDriver::get_time()
{
        return (float)(this->bytes_played) / 44100.0f / 4.0f;
}

