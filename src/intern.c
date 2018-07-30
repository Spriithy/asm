#include "intern.h"
#include "vector.h"
#include <stdio.h>
#include <string.h>

struct intern {
    size_t      len;
    const char* str;
};

static struct intern* interns;

const char* intern(const char* str)
{
    return intern_len(str, strlen(str));
}

const char* intern_len(const char* start, size_t len)
{
    if (len == 0) {
        return intern("");
    }

    for (struct intern* it = interns; it != vector_end(interns); it++) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }

    char* str = malloc(len + 1);
    memcpy(str, start, len);
    str[len] = '\0';

    vector_push(interns, (struct intern){ len, str });

    return str;
}

const char* intern_range(const char* start, const char* end)
{
    return intern_len(start, end - start);
}

/* O(n) best case, O(n^2 + n) worst case */
const char* interned(char* str)
{
    size_t len = strlen(str);

    /* O(n) direct lookup */
    for (struct intern* it = interns; it != vector_end(interns); it++) {
        if (it->str == str) {
            return it->str;
        }
    }

    /* O(n^2) worst case lookup */
    for (struct intern* it = interns; it != vector_end(interns); it++) {
        if (it->len == len && strncmp(it->str, str, len) == 0) {
            return it->str;
        }
    }

    return NULL;
}

const char* interned_len(char* start, size_t len)
{
    char str[len + 1];

    strncpy(str, start, len);
    str[len] = '\0';

    return interned(str);
}

const char* interned_range(char* start, char* end)
{
    return interned_len(start, end - start);
}

void free_interns()
{
    if (interns == NULL) {
        return;
    }

    /* free all string entries in the table */
    for (struct intern* it = interns; it != vector_end(interns); it++) {
        free((char*)it->str);
    }

    vector_free(interns);
}
