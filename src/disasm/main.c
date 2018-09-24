#include "../shared/cargs.h"
#include "../shared/disasm.h"
#include "../shared/intern.h"
#include "../shared/vector.h"
#include "disas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cleanup()                 \
    {                             \
        vector_free(disas.funcs); \
        free(disas.text);         \
    }

config_t config = {
    .input_file = NULL,
    .output_file = NULL,
};

int main(int argc, char** argv)
{
    register_arg(TYPE_STRING, "--", "-", &config.input_file, NULL);
    register_arg(TYPE_STRING, "--output", "-o", &config.output_file, NULL);
    parse_args(argc, argv);

    if (config.input_file == NULL) {
        printf("error: no input file\n");
        exit(-1);
    }

    FILE* input_file = fopen(config.input_file, "rb+");
    if (input_file == NULL) {
        printf("error: '%s' file error\n", config.input_file);
        exit(-1);
    }

    FILE* output_file = stdout;
    if (config.output_file != NULL) {
        FILE* f = fopen(config.output_file, "w+");
        if (f != NULL) {
            output_file = f;
        }
    }

    disas_t disas;
    load_file(&disas, input_file);
    fclose(input_file);

    func_t* funcs = malloc((disas.text_size / 4) * sizeof(*funcs));
    if (funcs == NULL) {
        printf("error: not enough memory\n");
        cleanup();
        exit(-1);
    }

    for (size_t pc = 0; pc < disas.text_size / 4; pc++) {
        func_t* f = func_addr_search(disas.funcs, 4 * pc);
        if (f == NULL) {
            funcs[pc] = (func_t){ -1, NULL };
        } else {
            funcs[pc] = *f;
        }
    }

    for (size_t pc = 0; pc < disas.text_size; pc += 4) {
        if (funcs[pc / 4].name != NULL) {
            fprintf(output_file, "\n%s:\n", funcs[pc / 4].name);
        }
        fputs("    ", output_file);
        disasm(disas.funcs, (uint32_t*)(disas.text + pc), 1, output_file);
    }

    if (output_file != stdout) {
        fclose(output_file);
    }

    cleanup();
    exit(0);
}
