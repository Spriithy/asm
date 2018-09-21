#include "../shared/disasm.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config_t config = {
    .debug = 0,
    .memory_size = 16 * 0xF4240, // 16 Mb
    .input_file = NULL,
};

static int is_option(char* str, char* lopt, char* sopt)
{
    if (sopt != NULL) {
        return strcmp(str, lopt) == 0 || strcmp(str, sopt) == 0;
    }
    return strcmp(str, lopt);
}

static int starts_with(char* pre, const char* str)
{
    size_t lpre = strlen(pre),
           lstr = strlen(str);
    return lstr < lpre ? false : strncmp(pre, str, lpre) == 0;
}

static int is_assignable_option(char* str, char* lopt, char* sopt)
{
    int len = 0;

    if (sopt != NULL) {
        if (starts_with(lopt, str)) {
            len = strlen(lopt);
        } else if (starts_with(sopt, str)) {
            len = strlen(sopt);
        }

        return len;
    }

    if (starts_with(lopt, str)) {
        len = strlen(lopt);
    }

    return len;
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (is_option(argv[i], "--XDebug", "-D")) {
            config.debug = 1;
            continue;
        }

        size_t len = 0;
        if ((len = is_assignable_option(argv[i], "--XMemSize=", NULL))) {
            if (len == strlen(argv[i])) {
                printf("error: missing XMemSize value\n");
                exit(-1);
            }

            char* value = argv[i] + len;
            int   memory_size = atoi(value);
            if (memory_size == 0 && value[0] != '0') {
                printf("error: XMemSize expects an integer value %s\n", value);
                exit(-1);
            } else if (memory_size <= 0) {
                printf("error: XMemSize expects a positive integer (got %s).\n", value);
                exit(-1);
            }

            config.memory_size = memory_size;
            continue;
        }

        if (config.input_file == NULL) {
            config.input_file = argv[i];
        }
    }

    if (config.input_file == NULL) {
        printf("error: no input file\n");
        exit(-1);
    }

    FILE* input_file = fopen(config.input_file, "rb");
    if (input_file == NULL) {
        printf("error: '%s' file error\n", config.input_file);
        exit(-1);
    }

    core_t core;
    load_file(&core, input_file);
    fclose(input_file);

    core_exec(&core);

    exit(0);
}