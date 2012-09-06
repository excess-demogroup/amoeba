#ifndef _FPSCOUNTER_H
#define _FPSCOUNTER_H

#ifndef WIN32
#include <sys/time.h>
#endif

class FPSCounter {
public:
	FPSCounter();
	~FPSCounter();

	void reset();	
	float getfps();
	void draw();

protected:
	unsigned char digits_bm[128 * 16 * 4];
	unsigned char texbuf[64 * 16 * 4];

#ifdef WIN32
	long tv[10];
#else	
	struct timeval tv[10];
#endif
};
	
#endif /* !defined(_FPSCOUNTER_H) */
