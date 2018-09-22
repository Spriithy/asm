#ifndef SHARED_FUNC_H
#define SHARED_FUNC_H

#include <stdint.h>

typedef struct {
    uint64_t addr;
    char*    name;
} func_t;

func_t* core_func_addr_search(func_t* funcs, uint64_t addr);
func_t* core_func_name_search(func_t* funcs, char* name);

#endif // shared/func.h
