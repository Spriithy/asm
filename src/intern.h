#ifndef EMU64_INTERN_H
#define EMU64_INTERN_H

#include <stddef.h>

const char* intern(const char* str);
const char* intern_len(const char* str, size_t len);
const char* intern_range(const char* start, const char* end);

const char* interned(char* str);
const char* interned_len(char* str, size_t len);
const char* interned_range(char* start, char* end);

void free_interns();

#endif // intern.h
