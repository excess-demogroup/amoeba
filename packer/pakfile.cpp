/*
 * Sesse's demolib
 */ 
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __unix__
#include <unistd.h>
#else
#include <io.h>
#define strncasecmp strnicmp
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <endian.h>

#include "demolib_prefs.h"
#include "file.h"
#include "pakfile.h"
#include "../exception.h"

#if DEMOLIB_DATA_PAKFILE

#if __BYTE_ORDER != __LITTLE_ENDIAN
#define fixendianl(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
#else
#define fixendianl(x) (x)
#endif

PakFile::PakFile(char *filename)
{
	char buf[12];
	int err, dirpos, dirsize;
	unsigned int i;

#if __unix__
	int fd = open("/usr/share/amoeba/demo.dat", O_RDONLY);
#else
	int fd = open("demo.dat", O_RDONLY | O_BINARY);
#endif
	if (fd == -1) throw new FileNotFoundException("demo.dat wasn't found");

	/* first verify that this is indeed a pakfile */
	err = read(fd, buf, 12);
	if (err != 12 || strncasecmp(buf, "PACK", 4) != 0) {
		throw new FatalException("demo.dat is not a valid pakfile");
	}
	
	/* seek to the beginning of the directory */
	dirpos = fixendianl(*(int *)(buf + 4));
	if (lseek(fd, dirpos, SEEK_SET) != dirpos) {
		throw new FatalException("demo.dat is truncated");
	}
	dirsize = fixendianl(*(int *)(buf + 8));
	
	for (i = 0; i < dirsize / sizeof(struct pak_direntry); i++) {
		char *ptr;
		struct pak_direntry d;
		
		if (read(fd, (char *)(&d), sizeof(d)) != sizeof(d)) {
			throw new FatalException("demo.dat is truncated");
		}

		d.pos = fixendianl(d.pos);
		d.size = fixendianl(d.size);

		d.filename[55] = 0;
		if (strcmp(filename, d.filename) != 0) continue;
		
		ptr = (char *)(malloc(d.size));
		if (ptr == NULL) {
			throw new FatalException("Out of memory!");
		}

		if (lseek(fd, d.pos, SEEK_SET) == -1) {
			throw new FatalException("pakfile truncated!");
		}

		unsigned int i = 0;
		while (i < d.size) {
			int num_bytes = d.size - i;
			int err;

			if (num_bytes > 4096) num_bytes = 4096;
			err = read(fd, ptr + i, num_bytes);

			if (err <= 0)
				throw new FatalException("Error on read()", strerror(errno));
			i += err;
		}

		this->data = ptr;
		this->length = d.size;

		close(fd);
		return;
	}
	
	throw new FileNotFoundException("file not found in demo.dat");
}

PakFile::~PakFile()
{
	free(this->data);
}

char *PakFile::get_data()
{
	return this->data;
}

int PakFile::data_length()
{
	return this->length;
}
#endif /* DEMOLIB_DATA_PAKFILE */

