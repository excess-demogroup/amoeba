#ifndef _FILEREADER_H
#define _FILEREADER_H

#ifdef __GNUG__
#include <unistd.h>
#endif
#include <sys/types.h>
#include "packer/file.h"

class FileReader {
public:
	FileReader(File *file);
	~FileReader();

	size_t fread(void *ptr, size_t size, size_t nmemb);
	size_t read(void *buf, size_t count);
	long ftell();
	int fseek(long offset, int whence);
	
protected:
	File *curr_file;
	size_t position;
};

#endif

