#include "cargs.h"
#include "vector.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static arg_t* argslist;

#define arg_set(type, value) *(type*)(arg->dest) = (value)

int parse_args(int argc, char** argv)
{
    int parsed = 0;
    for (int i = 1; i < argc; i++) {
        size_t arglen = strlen(argv[i]);
        vector_iter(arg_t, arg, argslist)
        {
            if ((arglen == strlen(arg->name) && strncmp(argv[i], arg->name, arglen) == 0)
                || (arg->alt == NULL ? 0 : arglen == strlen(arg->alt) && strncmp(argv[i], arg->alt, arglen) == 0)) {

                switch (arg->type) {
                case TYPE_FLAG:
                    arg_set(int, 1);
                    parsed++;
                    continue;

                case TYPE_BOOL:
                    if (i + 1 >= argc) {
                        printf("warning: missing 1 positional argument for %s (type: bool)\n", argv[i]);
                        return parsed;
                    }
                    i++;
                    for (size_t j = 0; j < strlen(argv[i]); j++) {
                        argv[i][j] = tolower(argv[i][j]);
                    }
                    if (strcmp(argv[i], "true") == 0 || atoi(argv[i]) == 1) {
                        arg_set(int, 1);
                    } else if (strcmp(argv[i], "false") == 0 || (atoi(argv[i]) == 0 && argv[i][0] != '0')) {
                        arg_set(int, 0);
                    } else {
                        printf("warning: missing 1 positional argument for %s (type: bool)\n", argv[i - 1]);
                        return parsed;
                    }
                    parsed++;
                    continue;

                case TYPE_INT:
                    if (i + 1 >= argc) {
                        printf("warning: missing 1 positional argument for %s (type: int)\n", argv[i]);
                        return parsed;
                    }
                    i++;
                    int val = atoi(argv[i]);
                    if (val == 0 && argv[i][0] != '0') {
                        printf("error: %s expects an integer value\n", argv[i - 1]);
                        i--;
                    } else {
                        arg_set(int, val);
                    }
                    parsed++;
                    continue;

                case TYPE_STRING:
                    arg_set(char*, argv[++i]);
                    parsed++;
                    continue;

                default:
                    break;
                }
            }
        }
    }
    return parsed;
}

void register_arg(type_t type, char* name, char* alt, void* dest, char* desc)
{
    vector_push(argslist, (arg_t){
                              .name = name,
                              .alt = alt,
                              .desc = desc,
                              .type = type,
                              .dest = dest,
                          });
}