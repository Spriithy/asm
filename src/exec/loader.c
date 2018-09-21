#include "core.h"
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

static void load_segment_sizes(core_t* core, FILE* input_file)
{
    // old position in file
    size_t fpos = ftell(input_file);

    // read data segment size
    fread(&core->data_size, sizeof(core->data_size), 1, input_file);

    // skip the data segment
    fseek(input_file, core->data_size, SEEK_CUR);

    // read text segment size
    fread(&core->text_size, sizeof(core->text_size), 1, input_file);

    // adjust to word count
    core->text_size /= 4;

    // go back to where we were
    fseek(input_file, fpos, SEEK_SET);
}

int load_file(core_t* core, FILE* input_file)
{
    if (!check_file_header(input_file)) {
        printf("error: magic header mismatch\n");
        return 0;
    }

    // read segment sizes
    load_segment_sizes(core, input_file);

    size_t data_size = core->data_size;
    size_t text_size = 4 * core->text_size;

    core->binary_data = malloc(data_size + text_size);
    if (core->binary_data == NULL) {
        printf("error: not enough memory\n");
        return 0;
    }

    // read data segment
    fseek(input_file, sizeof(data_size), SEEK_CUR);
    fread(core->binary_data, 1, data_size, input_file);

    // read text segment
    fseek(input_file, sizeof(text_size), SEEK_CUR);
    fread(core->binary_data + data_size, 1, text_size, input_file);

    return 1;
}
