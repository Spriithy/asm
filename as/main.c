#include "../emu/exec.h"
#include "../shared/buf.h"
#include "../shared/error.h"
#include "../shared/intern.h"
#include "../shared/vector.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        /*   jit_init();
        jit_set_debug(debug);
        parser_t* pars = parser_init(in_file);
        parser_set_debug(pars, debug);
        parse(pars);
        parser_delete(pars);
        jit_run();*/
    } else {
        printf("error: no input file\n");
        free_interns();
        exit(-1);
    }

    free_interns();
    exit(0);
}
