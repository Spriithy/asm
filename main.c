#include "src/compiler/scanner.h"
#include "src/error.h"
#include "src/intern.h"
#include "src/jit.h"
#include "src/run/cpu.h"
#include "src/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern jit_t     jit;
extern scanner_t scan;

int main(int argc, char** argv)
{
    int   debug = 0;
    char* in_file = NULL;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
                debug = 1;
            } else {
                in_file = argv[i];
            }
        }
    }

    if (in_file != NULL) {
        scanner_t* scan = scanner_init(in_file);
        scanner_tok(scan);
        while (scan->tok->kind != TOK_EOF) {
            char* str = tok_str(scan->tok);
            printf("%s\n", str);
            free(scan->tok);
            free(str);
            scanner_tok(scan);
        }
        char* str = tok_str(scan->tok);
        printf("%s\n", str);
        free(scan->tok);
        free(str);

        if (scan->err_list != NULL) {
            vector_iter(error_t*, err, scan->err_list)
            {
                printf("%s\n", *err);
            }
        }

        scanner_delete(scan);
    } else {
        printf("error: no input file\n");
        exit(-1);
    }

    free_interns();

    return 0;
}
