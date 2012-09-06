/* simple wrapper class to port stdio-based code easier */

#include <stdio.h>
#include <string.h>

#include "demolib_prefs.h"
#include "filereader.h"
	
#if DEMOLIB_DATA_FILEREADER
FileReader::FileReader(File *file)
{
	this->curr_file = file;
	this->position = 0;
}

FileReader::~FileReader()
{
	delete this->curr_file;
}

size_t FileReader::fread(void *ptr, size_t size, size_t nmemb)
{
	size_t i;
	for (i = 0; i < nmemb; i++) {
		if ((int)this->position + (int)size > (int)this->curr_file->data_length()) {
			return i;
		}
		memcpy((char *)(ptr) + i * size, this->curr_file->get_data() + this->position, size);
		this->position += size;
	}
	return i;
}

size_t FileReader::read(void *buf, size_t count)
{
	if ((int)this->position + (int)count > (int)this->curr_file->data_length()) {
		count = this->curr_file->data_length() - this->position;
	}
	memcpy(buf, this->curr_file->get_data() + this->position, count);
	return count;
}

long FileReader::ftell()
{
	return this->position;
}

int FileReader::fseek(long offset, int whence)
{
	int real_offset;
	switch (whence) {
	case SEEK_SET:
		real_offset = 0;
		break;
	case SEEK_CUR:
		real_offset = this->position;
		break;
	case SEEK_END:
		real_offset = this->curr_file->data_length();
		break;
	default:
		return -1;
	}

	if (real_offset + offset < 0 ||
            real_offset + offset > this->curr_file->data_length()) {
		return -1;
	}

	this->position = real_offset + offset;
	return 0;
}

#endif
