/*
 * Oh my goodness, how ugly DirectSound is. SOMEBODY PORT OSS/ALSA TO
 * WINDOWS, PLEASE! ;-) Needs DX3 or something. ;-)
 */

#include "audio/win32_dsound.h"
#include "main/win32-config/win32-config.h"
#include "exception.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef HRESULT (WINAPI *DIRECTSOUNDCREATE) (GUID FAR *lpGUID,LPDIRECTSOUND FAR *lplpDS, IUnknown FAR *pUnkOuter);

DirectSoundAudioDriver::DirectSoundAudioDriver(AudioProvider *prv, float jump, MainLoop *ml)
{
	HWND hWnd = ml->win->hWnd;
	DSBUFFERDESC dsbd_p, dsbd_s;
	WAVEFORMATEX format;
	DIRECTSOUNDCREATE dsCreate;
	HMODULE library;

        int err;
	
	this->ml = ml;
        this->eof = false;
	this->bytes_played = (int)(jump * 44100.0f) * 4;
	this->prov = prv;

	library = (HMODULE)LoadLibrary("dsound.dll");
	if (library == NULL)
		throw new FatalException("DirectSound", "Error in loading dsound.dll");

	dsCreate = (DIRECTSOUNDCREATE)GetProcAddress(library, "DirectSoundCreate");
	if (dsCreate == NULL)
		throw new FatalException("DirectSound", "Error in finding DirectSoundCreate");

	if (dsCreate(Win32Config::instance()->get_sound(), &dsound, NULL) != DS_OK)
		throw new FatalException("DirectSound", "DirectSoundCreate");

	err =
		IDirectSound_SetCooperativeLevel(dsound, hWnd,
			DSSCL_EXCLUSIVE |
			DSSCL_PRIORITY);
	if (err != DS_OK)
		throw new FatalException("DirectSound", "SetCooperativeLevel");
	
	/*
	 * Now create any buffers we need, first a primary buffer (to
	 * get the format right and thus eliminate any conversion noise
	 * DirectSound might try to impose on us :-P), then the secondary
	 * buffer, which is used for the actual stream.
	 *
	 * Our buffer is exactly one second of stereo, 16-bit 44100Hz raw
	 * PCM, ie. 44100 * 4 bytes.
	 */
	memset(&dsbd_p, 0, sizeof(DSBUFFERDESC));
	dsbd_p.dwSize = sizeof(DSBUFFERDESC);
	dsbd_p.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STICKYFOCUS;
	dsbd_p.dwBufferBytes = 0;
	dsbd_p.lpwfxFormat = NULL;

	err = IDirectSound_CreateSoundBuffer(dsound, &dsbd_p,
					     &this->primary_buf, NULL);
	if (err != DS_OK)
		throw new FatalException("DirectSound",
					 "Create primary buffer");
	
	memset(&format, 0, sizeof(WAVEFORMATEX));
	format.wFormatTag      = WAVE_FORMAT_PCM;
	format.nChannels       = 2;
	format.nSamplesPerSec  = 44100;
	format.nAvgBytesPerSec = 44100 * 4;
	format.nBlockAlign     = 4;
	format.wBitsPerSample  = 16;

	if (IDirectSoundBuffer_SetFormat(primary_buf, &format) != DS_OK)
		throw new FatalException("DirectSound", "SetFormat");

	/* now for the secondary buffer */
	memset(&dsbd_s, 0, sizeof(DSBUFFERDESC));
	dsbd_s.dwSize = sizeof(DSBUFFERDESC);
	dsbd_s.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	dsbd_s.lpwfxFormat = &format;
	dsbd_s.dwBufferBytes = format.nAvgBytesPerSec;
	
	err = IDirectSound_CreateSoundBuffer(dsound, &dsbd_s,
					     &this->stream_buf, NULL);
	if (err != DS_OK)
		throw new FatalException("DirectSound",
				         "Create secondary buffer");

        /* fill the first buffer */
	char *write_buf;
	DWORD write_len;

	this->last_played = 0;

	IDirectSoundBuffer_Lock(stream_buf, 0, 44100 * 4,
			        (void **)(&write_buf), &write_len,
				NULL, 0, 0);

	memset(write_buf, 0, write_len);
	this->last_fill = this->fill_buf_completely(write_buf, write_len);

	IDirectSoundBuffer_Unlock(stream_buf,
				  (void *)write_buf, write_len,
				  NULL, 0);

	/* now play :-) */
	if (IDirectSoundBuffer_SetCurrentPosition(stream_buf, 0) != DS_OK)
		throw new FatalException ("DirectSound", "SetCurrentPosition");

	if (IDirectSoundBuffer_Play(stream_buf, 0, 0, DSBPLAY_LOOPING) != DS_OK)
		throw new FatalException("DirectSound", "Play");
}

DirectSoundAudioDriver::~DirectSoundAudioDriver()
{
        IDirectSoundBuffer_Release(this->primary_buf);
        IDirectSoundBuffer_Release(this->stream_buf);
        IDirectSound_Release(this->dsound);
}

bool DirectSoundAudioDriver::run()
{
	unsigned char *write_buf1, *write_buf2;
	DWORD write_len1, write_len2, play_pos, junk;
	int err;

	err = IDirectSoundBuffer_GetCurrentPosition(stream_buf,
						    &play_pos, &junk);
	if (err != DS_OK)
		throw new FatalException("DirectSound", "GetCurrentPosition");

	if (play_pos != last_played) {
		int fill, tfill, err, played;
		char outbuf[44100 * 4];

		if (this->last_fill < play_pos) {
			fill = play_pos - last_fill;
		} else {
			fill = 44100 * 4 - (last_fill - play_pos);
		}

		/* very crude error detection */
		while ((err = IDirectSoundBuffer_Lock(stream_buf,
						      this->last_fill, fill,
						      (void **)(&write_buf1), &write_len1,
						      (void **)(&write_buf2), &write_len2,
						      0)) != DS_OK) {
			IDirectSoundBuffer_Restore(stream_buf);
			IDirectSoundBuffer_Play(stream_buf, 0, 0, DSBPLAY_LOOPING);
		}
		
		tfill = this->fill_buf_completely(outbuf, write_len1 + write_len2);
		if ((unsigned int)tfill <= write_len1) {
			memcpy(write_buf1, outbuf, tfill);
		} else {
			memcpy(write_buf1, outbuf, write_len1);
			memcpy(write_buf2, outbuf + write_len1, tfill - write_len1);
		}
		this->last_fill += tfill;
		if (this->last_fill >= 44100 * 4) this->last_fill -= 44100 * 4;
		
		played = play_pos - last_played;
		if (played < 0) played += 44100 * 4;
		
		this->bytes_played += played;
		this->last_played = play_pos;

		IDirectSoundBuffer_Unlock(stream_buf,
				          (void *)(write_buf1), write_len1,
					  (void *)(write_buf2), write_len2);
	}
	return true;
}

float DirectSoundAudioDriver::get_time()
{
        return (float)(this->bytes_played) / 44100.0f / 4.0f;
}

int DirectSoundAudioDriver::fill_buf_completely(char *buf, int bytes)
{
	int junk;
	int ret = 0;

	while (bytes > 4096) {
		int i = this->prov->fill_buf(buf, bytes);
		
		if (i == 0) {
			/* fill with zeroes */
			memset(buf, 0, bytes);
			i = bytes;
		}
		if (i < 0)
			throw new FatalException("Error in Ogg stream");

		ret += i;
		bytes -= i;
		buf += i;
	}
	return ret;
}

