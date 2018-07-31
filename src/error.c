#include "error.h"
#include <stdio.h>
#include <stdlib.h>

error_t* errorf(char* fmt, ...)
{
    error_t* err = malloc(sizeof(*err));
    if (err == NULL) {
        perror("malloc: ");
        exit(-1);
    }

    va_list args;
    va_start(args, fmt);

    asprintf(&err->message, fmt, args);

    va_end(args);
    return err;
}

void error_free(error_t** err)
{
    if (*err == NULL) {
        return;
    }

    // should we free (*err)->message ?
    //  - could be malloc'd string
    //  - could be data segment string
    //
    // free((*err)->message);

    free(*err);
    *err = NULL;
}