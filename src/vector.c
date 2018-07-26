#include "vector.h"

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

void* __core_vector_grow(const void* vec, size_t new_len, size_t elem_size)
{
    size_t new_cap = MAX(1 + 2 * vector_capacity(vec), new_len);
    size_t new_size = offsetof(__core_vector_header_t, buf) + new_cap * elem_size;

    __core_vector_header_t* new_hdr;

    if (vec) {
        new_hdr = realloc(__core_vector_header(vec), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }

    new_hdr->cap = new_cap;

    return new_hdr->buf;
}
