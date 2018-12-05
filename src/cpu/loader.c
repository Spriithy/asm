#include "cpu.h"
#include <stdio.h>

#define HDR_MAGIC 0x6865787944414e4f

static FILE* file;

static int check_header()
{
    uint64_t hdr_magic = 0;

    // fseek(file, 0, SEEK_SET);
    fread(&hdr_magic, sizeof(hdr_magic), 1, file);

    return hdr_magic == HDR_MAGIC;
}

int load(char* path)
{
    trace(path);

    file = fopen(path, "r");

    if (file == NULL) {
        tracep();
        return -1;
    }

    if (!check_header()) {
        trace("unrecognized file");
        return -1;
    }

    size_t text_len;
    if (fread(&text_len, sizeof(text_len), 1, file) != 1) {
        trace("corrupted file");
        fclose(file);
        return -1;
    }
    cpu.rosector = &cpu.data[text_len];

    size_t total_len;
    if (fread(&total_len, sizeof(text_len), 1, file) != 1) {
        trace("corrupted file");
        fclose(file);
        return -1;
    }

    if (fread(cpu.data, 1, total_len, file) != total_len) {
        trace("corrupted file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}
