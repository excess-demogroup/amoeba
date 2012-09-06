/*
 * Sesse's demolib
 */ 
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <unistd.h>
#endif

#include "demolib_prefs.h"
#include "exception.h"
#include "file.h"
#include "memfile.h"
#include "pakfile.h"

#include "zfile.h"

File::File() {}
File::~File() {}

/*
 * Gives a File (from data/, data.tar or the `demodata' symbol * from the exe).
 */
File *load_file(const char * const filename)
{
	char fname[256];
	sprintf(fname, "data/%s", filename);

#if !DEMOLIB_SILENT
	printf("%s\n", fname);
#endif
	
#if DEMOLIB_DATA_ZLIB
	if (strlen(filename) > 6 &&
	    strncmp(filename, "zlib:", 5) == 0) {
		File *tempfile = load_file(filename + 5);
		ZFile *zf = new ZFile(tempfile);
		delete tempfile;
		return zf;
	}
#endif
	
#if DEMOLIB_DATA_DIR_NONMMAP
	try {
		return new MemFile(fname);
	} catch (NonFatalException *e) {}
#endif

#if DEMOLIB_DATA_PAKFILE
	try {
		return new PakFile(fname);
	} catch (NonFatalException *e) {}
#endif
	
	sprintf(fname, "%s not found", filename);	
	throw new FileNotFoundException(fname);
}
