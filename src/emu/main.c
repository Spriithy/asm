#include "../shared/disasm.h"
#include "../shared/intern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    char* in_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (in_file == NULL) {
            in_file = argv[i];
            break;
        }
    }

    if (in_file == NULL) {
        printf("error: no input file\n");
        exit(-1);
    }

    FILE* in = fopen(in_file, "rb");
    fseek(in, 0L, SEEK_END);
    size_t size = ftell(in);
    fseek(in, 0L, SEEK_SET);

    if (size % 4 > 0) {
        printf("error: miss-aligned binary\n");
        exit(-1);
    }

    uint32_t* code = malloc(size * sizeof(*code));
    if (code == NULL) {
        printf("error: not enough memory\n");
        exit(-1);
    }

    fread(code, size / 4, 4, in);
    fclose(in);

    free_interns();
    exit(0);
}
