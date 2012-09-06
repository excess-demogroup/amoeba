#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#include "demolib_prefs.h"
#include "file.h"
#include "memfile.h"
#include "../exception.h"

#if DEMOLIB_DATA_DIR_NONMMAP
MemFile::MemFile(char *filename)
{
	struct stat buf;
	FILE *file;
	int i = 0;
	
	if (stat(filename, &buf) == -1)
		throw new FileNotFoundException("couldn't find file in data/");
	file = fopen(filename, "rb");
	if (file == NULL)
		throw new FatalException("error in opening file in data/");
	
	this->data = (char *)(malloc(buf.st_size));
	if (this->data == NULL) throw new FatalException("Out of memory");

	while (i < buf.st_size) {
		int num_bytes = buf.st_size - i;
		int err;

		if (num_bytes > 4096) num_bytes = 4096;
		err = fread(this->data + i, num_bytes, 1, file);

		if (err <= 0) 
			throw new FatalException("Error on read()");
		i += err * num_bytes;
	}

	this->length = buf.st_size;

	fclose(file);
}

MemFile::~MemFile()
{
	free(this->data);
}

char *MemFile::get_data()
{
	return this->data;
}

int MemFile::data_length()
{
	return this->length;
}
#endif /* DEMOLIB_DATA_DIR_NONMMAP */

