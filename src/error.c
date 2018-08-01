#include "error.h"
#include <stdio.h>
#include <stdlib.h>

error_t* errorf(char* fmt, ...)
{
    error_t* err;
    va_list  args;
    va_start(args, fmt);

    vasprintf(&err, fmt, args);

    va_end(args);
    return err;
}

void error_free(error_t** err)
{
    if (*err == NULL) {
        return;
    }

    free(*err);
    *err = NULL;
}