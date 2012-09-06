/*
 * generic exception class
 */

class Exception {
public:
	Exception(const char *msg);
	Exception(const char *owner, const char *msg);
	~Exception();

	char *get_error();
	int get_fatal();

protected:
	char *str;
};

class FatalException : public Exception {
public:
	FatalException(const char *msg);
	FatalException(const char *owner, const char *msg);

//	char *get_error();
	int get_fatal();
};
class NonFatalException : public Exception {
public:
	NonFatalException(const char *msg);
	NonFatalException(const char *owner, const char *msg);

//	char *get_error();
	int get_fatal(); 
};

class ValueNotSpecifiedException : public FatalException {
public:
	ValueNotSpecifiedException(const char *msg);
	ValueNotSpecifiedException(const char *owner, const char *msg);
};

class FileNotFoundException : public NonFatalException {
public:
	FileNotFoundException(const char *msg);
	FileNotFoundException(const char *owner, const char *msg);
};
