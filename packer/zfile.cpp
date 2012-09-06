#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include "demolib_prefs.h"
#include "file.h"
#include "zfile.h"
#include "../exception.h"

#if DEMOLIB_DATA_ZLIB

ZFile::ZFile(File *basefile)
{
	z_stream zs;

	unsigned char *file_data = (unsigned char *)(basefile->get_data());

	/* read the file length first */
	this->length = file_data[0] | ((int)(file_data[1]) << 8) |
		((int)(file_data[2]) << 16) | ((int)(file_data[3]) << 24);
	this->data = (char *)(malloc(this->length));

	if (this->data == NULL) {
		throw new FatalException("Out of memory!");
	}

        memset(&zs, 0, sizeof(zs));
	zs.next_in = (Bytef *)(file_data + 4);
	zs.avail_in = basefile->data_length() - 4;
	zs.next_out = (Bytef *)(this->data);
	zs.avail_out = this->length;
	
	if (inflateInit(&zs) != Z_OK ||
	    inflate(&zs, Z_FINISH) != Z_STREAM_END) {
		throw new FatalException("zlib", zs.msg);
	}
	inflateEnd(&zs);
}

ZFile::~ZFile()
{
	free(this->data);
}

char *ZFile::get_data()
{
	return this->data;
}

int ZFile::data_length()
{
	return this->length;
}
#endif /* DEMOLIB_DATA_ZLIB */

