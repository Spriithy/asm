#include "../shared/vector.h"
#include "core.h"
#include <string.h>

func_t* core_func_addr_search(core_t* core, uint64_t addr)
{
    vector_iter(func_t, func, core->funcs)
    {
        if (func->addr == addr) {
            return func;
        }
    }

    return NULL;
}

func_t* core_func_name_search(core_t* core, char* name)
{
    vector_iter(func_t, func, core->funcs)
    {
        if (strcmp(func->name, name)) {
            return func;
        }
    }

    return NULL;
}