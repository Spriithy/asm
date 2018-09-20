#ifndef EMU64_INTERN_H
#define EMU64_INTERN_H

#include <stddef.h>

char* intern(char* str);
char* intern_len(char* str, size_t len);
char* intern_range(char* start, char* end);

char* interned(char* str);
char* interned_len(char* str, size_t len);
char* interned_range(char* start, char* end);

void free_interns();

#endif // intern.h
