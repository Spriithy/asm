#include "cpu.h"
#include <stdarg.h>
#include <stdio.h>

void __trace(const char* func, const char* message)
{
    printf("%s: %s\n", func, message);
}

void __tracef(const char* func, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    printf("%s: ", func);
    vprintf(fmt, args);
    putchar('\n');

    va_end(args);
}

void exception(const char* message)
{
    printf("(*) %s\n", message);
    clean();
}

void exceptionf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    printf("(*) ");
    vprintf(fmt, args);
    putchar('\n');

    va_end(args);
    clean();
}
