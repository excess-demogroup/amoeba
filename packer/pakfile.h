#ifndef _PAKFILE_H
#define _PAKFILE_H

#include "file.h"

struct pak_direntry {
	char filename[56];
	unsigned int pos;
	unsigned int size;
};

class PakFile : public File {
public:
	PakFile(char *filename);
	~PakFile();

	char *get_data();
	int data_length();

protected:
	char *data;
	int length;
};

#endif /* !_PAKFILE_H */
