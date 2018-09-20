#include "../disasm.h"
#include "../intern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    int   raw = 0;
    char* in_file = NULL;
    char* out_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--raw") == 0 || strcmp(argv[i], "-r")) {
            raw = 1;
        } else if (in_file == NULL) {
            in_file = argv[i];
        } else if (out_file == NULL) {
            out_file = argv[i];
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

    FILE* out = stdout;

    if (out_file != NULL) {
        out = fopen(out_file, "w+");
        if (out == NULL) {
            printf("error: couldn't create output file, redirecting to stdout\n");
            out = stdout;
        }
    }

    disasm(NULL, out, code, size / 4);

    free_interns();
    exit(0);
}
