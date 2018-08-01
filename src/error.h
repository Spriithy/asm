#ifndef EMU64_ERROR_H
#define EMU64_ERROR_H

#include <stdarg.h>

typedef char error_t;

error_t* errorf(char* fmt, ...);
void     error_free(error_t** err);

#endif // error.h
