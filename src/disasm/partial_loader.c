#include "../shared/vector.h"
#include "disas.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HDR_MAGIC 0x6865787944414e4f

static int check_file_header(FILE* input_file)
{
    uint64_t hdr_magic = 0;

    if (input_file == NULL) {
        return 0;
    }

    // go back to file start if needed
    fseek(input_file, 0, SEEK_SET);

    // read magic number from file
    fread(&hdr_magic, sizeof(hdr_magic), 1, input_file);

    return hdr_magic == HDR_MAGIC;
}

static void read_segment_sizes(disas_t* disas, FILE* input_file)
{
    // read the number of defined functions
    fread(&disas->nfunc, sizeof(size_t), 1, input_file);

    // read text segment size
    fread(&disas->text_size, sizeof(size_t), 1, input_file);

    // skip data segment size
    fseek(input_file, sizeof(size_t), SEEK_CUR);
}

static void read_functions(disas_t* disas, FILE* input_file)
{
    // load functions
    for (size_t i = 0; i < disas->nfunc; i++) {
        func_t func = {};
        size_t namelen = 0;

        // read function address
        fread(&func.addr, sizeof(uint64_t), 1, input_file);

        // read function's name length
        fread(&namelen, sizeof(size_t), 1, input_file);

        // read the actual name
        func.name = malloc(namelen + 1);
        func.name[namelen] = '\0';
        fread(func.name, 1, namelen, input_file);

        // save to vector
        vector_push(disas->funcs, func);
    }
}

int load_file(disas_t* disas, FILE* input_file)
{
    if (!check_file_header(input_file)) {
        printf("error: magic header mismatch\n");
        return 0;
    }

    // read segment sizes
    read_segment_sizes(disas, input_file);

    // read function definition segment
    read_functions(disas, input_file);

    disas->text = malloc(disas->text_size);
    if (disas->text == NULL) {
        printf("error: not enough memory\n");
        return 0;
    }

    // read text segment
    fread(disas->text, 1, disas->text_size, input_file);

    return 1;
}
