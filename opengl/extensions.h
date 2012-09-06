#ifndef _EXTENSIONS_H
#define _EXTENSIONS_H

namespace GLExtensions {
	bool has_ext(const char *extname);
	void *func_ptr(const char *function);
}
	
#endif /* !defined(_EXTENSIONS_H) */
