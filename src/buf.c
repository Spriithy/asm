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

static int buf_fits(buf_t* buf, size_t n)
{
    return buf->len + n <= buf->cap;
}

buf_t* buf_alloc(size_t cap)
{
    buf_t* buf = malloc(sizeof(*buf));
    if (buf == NULL) {
        perror("buf_alloc");
        exit(-1);
    }
    buf->len = 0;
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

uint8_t buf_read_uint8(buf_t* buf, size_t at)
{
    return buf->bytes[at];
}

uint16_t buf_read_uint16(buf_t* buf, size_t at)
{
    uint16_t hi = buf_read_uint8(buf, at);
    uint16_t lo = buf_read_uint8(buf, at + 1);
    return hi << 8 | lo;
}

uint32_t buf_read_uint32(buf_t* buf, size_t at)
{
    uint32_t hi = buf_read_uint16(buf, at);
    uint32_t lo = buf_read_uint16(buf, at + 2);
    return hi << 16 | lo;
}

uint64_t buf_read_uint64(buf_t* buf, size_t at)
{
    uint64_t hi = buf_read_uint32(buf, at);
    uint64_t lo = buf_read_uint32(buf, at + 4);
    return hi << 32 | lo;
}

char* buf_read_str(buf_t* buf, size_t at)
{
    return (char*)buf->bytes + at;
}

size_t buf_memcpy(buf_t* buf, size_t at, uint8_t* src, size_t len)
{
    if (!buf_fits(buf, len)) {
        buf_grow(buf, buf->len + len);
    }
    memcpy(buf->bytes + at, src, len);
    return at + len;
}

size_t buf_write_uint8(buf_t* buf, size_t at, uint8_t x)
{
    if (!buf_fits(buf, sizeof(x))) {
        buf_grow(buf, buf->len + sizeof(x));
    }
    buf->bytes[at++] = x;
    return at;
}

size_t buf_write_uint16(buf_t* buf, size_t at, uint16_t x)
{
    if (!buf_fits(buf, sizeof(x))) {
        buf_grow(buf, buf->len + sizeof(x));
    }
    at = buf_write_uint8(buf, at, ((x >> 8) & 0xff));
    at = buf_write_uint8(buf, at, ((x >> 0) & 0xff));
    return at;
}

size_t buf_write_uint32(buf_t* buf, size_t at, uint32_t x)
{
    if (!buf_fits(buf, sizeof(x))) {
        buf_grow(buf, buf->len + sizeof(x));
    }
    at = buf_write_uint16(buf, at, ((x >> 16) & 0xffff));
    at = buf_write_uint16(buf, at, ((x >> 0) & 0xffff));
    return at;
}

size_t buf_write_uint64(buf_t* buf, size_t at, uint64_t x)
{
    if (!buf_fits(buf, sizeof(x))) {
        buf_grow(buf, buf->len + sizeof(x));
    }
    at = buf_write_uint32(buf, at, ((x >> 32) & 0xffffffff));
    at = buf_write_uint32(buf, at, ((x >> 0) & 0xffffffff));
    return at;
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