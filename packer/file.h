/* virtual top class */

#ifndef _FILE_H
#define _FILE_H

class File {
public:
	File();
	virtual ~File();

	virtual char *get_data() = 0;
	virtual int data_length() = 0;
};

File *load_file(const char * const filename);

#endif
