#include "../shared/cargs.h"
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

int main(int argc, char** argv)
{
    register_arg(TYPE_FLAG, "--XDebug", "-D", &config.debug, NULL);
    register_arg(TYPE_INT, "--XMemSize", NULL, &config.memory_size, NULL);
    register_arg(TYPE_STRING, "--", "-", &config.input_file, NULL);
    parse_args(argc, argv);

    if (config.memory_size <= 0) {
        printf("error: XMemSize expects a positive integer (got %zu).\n", config.memory_size);
        exit(-1);
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