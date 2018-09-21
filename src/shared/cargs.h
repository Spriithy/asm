#ifndef CARGS_H
#define CARGS_H

typedef enum {
    TYPE_FLAG,
    TYPE_INT,
    TYPE_STRING,
    TYPE_BOOL,
} type_t;

typedef struct {
    char*  name;
    char*  alt;
    char*  desc;
    type_t type;
    void*  dest;
} arg_t;

int  parse_args(int argc, char** argv);
void register_arg(type_t type, char* name, char* alt, void* dest, char* desc);

#endif // cargs.h
