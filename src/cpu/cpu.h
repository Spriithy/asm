#ifndef CPU_H_
#define CPU_H_

#include "mem.h"
#include <errno.h>
#include <string.h>

#define MEMORY_SIZE (1 << 24)

struct cpu {
    int     trap;
    byte_u* rosector;
    byte_u* text;
    byte_u  data[MEMORY_SIZE];
    dword_u gpr[16];
    qword_u xmm[8];
};

extern struct cpu cpu;

#define trace(message) __trace(__func__, message)
void __trace(const char* func, const char* message);

#define tracef(fmt, ...) __tracef(__func__, fmt, __VA_ARGS__)
#define tracep() __trace(__func__, strerror(errno))
void __tracef(const char* func, const char* fmt, ...);

void exception(const char* message);
void exceptionf(const char* fmt, ...);

int load(char* path);

void exec();

void clean();

#endif /* CPU_H_ */
