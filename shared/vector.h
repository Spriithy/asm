#ifndef EMU64_VECTOR_H
#define EMU64_VECTOR_H

#include <stdlib.h>

typedef struct {
    size_t len;
    size_t cap;
    char   buf[];
} __vector_header_t;

#define vector_iter(typ, var, vec) for (typ* var = (vec); var != vector_end(vec); var++)

#define vector_reserve(vec, n) (__vector_fit((vec), (n)))

#define vector_length(vec) ((vec) ? __vector_header(vec)->len : 0)

#define vector_capacity(vec) ((vec) ? __vector_header(vec)->cap : 0)

#define vector_end(vec) ((vec) + vector_length(vec))

#define vector_free(vec) ((vec) ? (free(__vector_header(vec)), (vec) = NULL) : 0)

#define vector_push(vec, ...) (__vector_fit((vec), 1), (vec)[__vector_header(vec)->len++] = __VA_ARGS__)

#define vector_pop(vec) (vector_length(vec) > 0 ? (vec)[--__vector_header(vec)->len] : NULL)

/* Following declarations should not be used by end users */

#ifndef offsetof
#define offsetof(st, m) ((size_t) & (((st*)0)->m))
#endif

#define __vector_header(vec) ((__vector_header_t*)((char*)(vec)-offsetof(__vector_header_t, buf)))
#define __vector_fits(vec, n) (vector_length(vec) + (n) <= vector_capacity(vec))
#define __vector_fit(vec, n) (__vector_fits((vec), (n)) ? 0 : ((vec) = __vector_grow((vec), vector_length(vec) + (n), sizeof(*(vec)))))

void* __vector_grow(const void* vector, size_t new_len, size_t elem_size);

#endif /* vector.h */
