#include "../shared/cargs.h"
#include "../shared/disasm.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config_t config = {
    .debug = 0,
    .memory_size = 16 * 0xF4240, // 16 Mb
    .recursion_limit = 100,
    .input_file = NULL,
};

int main(int argc, char** argv)
{
    register_arg(TYPE_FLAG, "--XDebug", "-d", &config.debug, NULL);
    register_arg(TYPE_INT, "--XMemSize", "-mem", &config.memory_size, NULL);
    register_arg(TYPE_INT, "--XRecursionLimit", NULL, &config.recursion_limit, NULL);
    register_arg(TYPE_STRING, "--", "-", &config.input_file, NULL);
    parse_args(argc, argv);

    if (config.memory_size <= 0) {
        printf("error: XMemSize expects a positive integer value.\n");
        exit(-1);
    }

    if (config.recursion_limit < 0) {
        printf("error: XRecursionLimit expects a positive integer value\n");
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
