/*
 * generic exception class
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
 
#include "exception.h"

/*
 * Generic exception class
 */
Exception::Exception(const char *msg)
{
	this->str = strdup(msg);
}
Exception::Exception(const char *owner, const char *msg)
{
	this->str = (char *)malloc(strlen(owner) + strlen(msg) + 3);
	if (this->str == NULL) throw new FatalException("Out of memory");	/* beautiful :-P */
	sprintf(this->str, "%s: %s", owner, msg);
}
Exception::~Exception()
{
	free(this->str);
}

char *Exception::get_error()
{
	return this->str;
}
int Exception::get_fatal()
{
	return 0;
}

/*
 * Fatal exceptions
 */
FatalException::FatalException(const char *msg) : Exception(msg)
{
	/* grossss hack */
//	fprintf(stderr, "Unhandled exception: %s\n", msg);
}
FatalException::FatalException(const char *owner, const char *msg) : Exception(owner, msg)
{
	/* grossss hack */
//	fprintf(stderr, "Unhandled exception: %s: %s\n", owner, msg);
}
int FatalException::get_fatal()
{
	return 1;
}

/*
 * Non-fatal exceptions
 */
NonFatalException::NonFatalException(const char *msg) : Exception(msg) {}
NonFatalException::NonFatalException(const char *owner, const char *msg) : Exception(owner, msg) {}
int NonFatalException::get_fatal()
{
	return 1;
}

/*
 * A fatal exception thrown from the Hashtable class when a value
 * requested wasn't found (only from get_int() and get_float() --
 * lookup() returns NULL but get_int() and get_float() are simplifications
 * to get less code duplication, and "not found" would definitely
 * be an error here)
 */
ValueNotSpecifiedException::ValueNotSpecifiedException(const char *msg) :
	FatalException(msg) {}
ValueNotSpecifiedException::ValueNotSpecifiedException
	(const char *owner, const char *msg) : FatalException(owner, msg) {}
	
FileNotFoundException::FileNotFoundException(const char *msg) :
	NonFatalException(msg) {}
FileNotFoundException::FileNotFoundException
	(const char *owner, const char *msg) : NonFatalException(owner, msg) {}
