#include "buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

static void buf_grow(buf_t* buf, size_t new_len)
{
    size_t new_cap = MAX(1 + 2 * buf->cap, new_len);
    buf->bytes = realloc(buf->bytes, new_cap);
    buf->cap = new_cap;
}

static int buf_fits(buf_t* buf, size_t at, size_t n)
{
    return at + n <= buf->cap;
}

buf_t* buf_alloc(size_t cap)
{
    buf_t* buf = malloc(sizeof(*buf));
    if (buf == NULL) {
        perror("buf_alloc");
        exit(-1);
    }
    buf->cap = cap;
    buf->bytes = malloc(cap);
    if (buf->bytes == NULL) {
        free(buf);
        perror("buf_alloc");
        exit(-1);
    }
    return buf;
}

buf_t* buf_from(uint8_t* data, size_t len)
{
    buf_t* buf = buf_alloc(len);
    if (data != NULL) {
        memcpy(buf->bytes, data, len);
    }
    return buf;
}

void* buf_read(buf_t* buf, size_t at)
{
    return buf->bytes + at;
}

uint8_t buf_read_uint8(buf_t* buf, size_t at)
{
    return buf->bytes[at];
}

uint16_t buf_read_uint16(buf_t* buf, size_t at)
{
    return *(uint16_t*)(buf->bytes + at);
}

uint32_t buf_read_uint32(buf_t* buf, size_t at)
{
    return *(uint32_t*)(buf->bytes + at);
}

uint64_t buf_read_uint64(buf_t* buf, size_t at)
{
    return *(uint64_t*)(buf->bytes + at);
}

char* buf_read_str(buf_t* buf, size_t at)
{
    return (char*)(buf->bytes + at);
}

size_t buf_memcpy(buf_t* buf, size_t at, uint8_t* src, size_t len)
{
    if (!buf_fits(buf, at, len)) {
        buf_grow(buf, buf->cap + len);
    }
    memcpy(buf->bytes + at, src, len);
    return at + len;
}

size_t buf_write(buf_t* buf, size_t at, void* x, size_t size)
{
    return buf_memcpy(buf, at, (uint8_t*)x, size);
}

size_t buf_write_uint8(buf_t* buf, size_t at, uint8_t x)
{
    if (!buf_fits(buf, at, sizeof(x))) {
        buf_grow(buf, buf->cap + sizeof(x));
    }
    buf->bytes[at++] = x;
    return at;
}

size_t buf_write_uint16(buf_t* buf, size_t at, uint16_t x)
{
    if (!buf_fits(buf, at, sizeof(x))) {
        buf_grow(buf, buf->cap + sizeof(x));
    }
    *(uint16_t*)(buf->bytes + at) = x;
    return at + 2;
}

size_t buf_write_uint32(buf_t* buf, size_t at, uint32_t x)
{
    if (!buf_fits(buf, at, sizeof(x))) {
        buf_grow(buf, buf->cap + sizeof(x));
    }
    *(uint32_t*)(buf->bytes + at) = x;
    return at + 4;
}

size_t buf_write_uint64(buf_t* buf, size_t at, uint64_t x)
{
    if (!buf_fits(buf, at, sizeof(x))) {
        buf_grow(buf, buf->cap + sizeof(x));
    }
    *(uint64_t*)(buf->bytes + at) = x;
    return at + 8;
}

size_t buf_write_str(buf_t* buf, size_t at, char* str)
{
    return buf_memcpy(buf, at, (uint8_t*)str, strlen(str));
}

void buf_free(buf_t** buf)
{
    if (buf == NULL) {
        return;
    }

    if (*buf != NULL) {
        free((*buf)->bytes);
        free(*buf);
        *buf = NULL;
    }
}