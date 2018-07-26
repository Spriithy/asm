#ifndef EMU64_VECTOR_H
#define EMU64_VECTOR_H

#include <stdlib.h>

typedef struct {
    size_t len;
    size_t cap;
    char   buf[];
} __core_vector_header_t;

#define vector_iter(typ, var, vec) for (typ* var = (vec); var != vector_end(vec); var++)

#define vector_reserve(vec, n) (__core_vector_fit((vec), (n)))

#define vector_length(vec) ((vec) ? __core_vector_header(vec)->len : 0)

#define vector_capacity(vec) ((vec) ? __core_vector_header(vec)->cap : 0)

#define vector_end(vec) ((vec) + vector_length(vec))

#define vector_free(vec) ((vec) ? (free(__core_vector_header(vec)), (vec) = NULL) : 0)

#define vector_push(vec, ...) (__core_vector_fit((vec), 1), (vec)[__core_vector_header(vec)->len++] = __VA_ARGS__)

#define vector_pop(vec) (vector_length(vec) > 0 ? (vec)[--__core_vector_header(vec)->len] : NULL)

/* Following declarations should not be used by end users */

#ifndef offsetof
#define offsetof(st, m) ((size_t) & (((st*)0)->m))
#endif

#define __core_vector_header(vec) ((__core_vector_header_t*)((char*)(vec)-offsetof(__core_vector_header_t, buf)))
#define __core_vector_fits(vec, n) (vector_length(vec) + (n) <= vector_capacity(vec))
#define __core_vector_fit(vec, n) (__core_vector_fits((vec), (n)) ? 0 : ((vec) = __core_vector_grow((vec), vector_length(vec) + (n), sizeof(*(vec)))))

void* __core_vector_grow(const void* vector, size_t new_len, size_t elem_size);

#endif /* vector.h */
