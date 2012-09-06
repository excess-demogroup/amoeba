#ifndef _MEMFILE_H
#define _MEMFILE_H

#include "file.h"

class MemFile : public File {
public:
	MemFile(char *filename);
	~MemFile();

	char *get_data();
	int data_length();

protected:
	char *data;
	int length;
};

#endif /* !_MEMFILE_H */
