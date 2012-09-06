#ifndef _ZFILE_H
#define _ZFILE_H

#include "file.h"

class ZFile : public File {
public:
	ZFile(File *basefile);
	~ZFile();

	char *get_data();
	int data_length();

protected:
	char *data;
	int length;
};

#endif /* !_ZFILE_H */
