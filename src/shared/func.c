#include "func.h"
#include "vector.h"
#include <string.h>

func_t* core_func_addr_search(func_t* funcs, uint64_t addr)
{
    vector_iter(func_t, func, funcs)
    {
        if (func->addr == addr) {
            return func;
        }
    }

    return NULL;
}

func_t* core_func_name_search(func_t* funcs, char* name)
{
    vector_iter(func_t, func, funcs)
    {
        if (strcmp(func->name, name)) {
            return func;
        }
    }

    return NULL;
}
