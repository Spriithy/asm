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
    // read text segment size
    fread(&core->text_size, sizeof(size_t), 1, input_file);

    // read data segment size
    fread(&core->data_size, sizeof(size_t), 1, input_file);
}

int load_file(core_t* core, FILE* input_file)
{
    if (!check_file_header(input_file)) {
        printf("error: magic header mismatch\n");
        return 0;
    }

    // read segment sizes
    load_segment_sizes(core, input_file);

    size_t text_size = core->text_size;
    size_t data_size = core->data_size;

    if (config.memory_size <= 2 * (text_size + data_size)) {
        printf("error: insufficient memory (use --XMemSize and use a greater value).\n");
        return 0;
    }

    core->mem = malloc(config.memory_size);
    if (core->mem == NULL) {
        printf("error: not enough memory\n");
        return 0;
    }

    // read text segment
    core->seg_text = (uint32_t*)core->mem;
    fread(core->mem, 1, text_size, input_file);

    // read data segment
    core->seg_data = core->mem + text_size;
    fread(core->seg_data, 1, data_size, input_file);

    return 1;
}
