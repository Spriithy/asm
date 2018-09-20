#ifndef EMU64_BUF_H
#define EMU64_BUF_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t   cap;
    uint8_t* bytes;
} buf_t;

buf_t* buf_alloc(size_t cap);
buf_t* buf_from(uint8_t* data, size_t len);

void*    buf_read(buf_t* buf, size_t at);
uint8_t  buf_read_uint8(buf_t* buf, size_t at);
uint16_t buf_read_uint16(buf_t* buf, size_t at);
uint32_t buf_read_uint32(buf_t* buf, size_t at);
uint64_t buf_read_uint64(buf_t* buf, size_t at);

size_t buf_memcpy(buf_t* buf, size_t at, uint8_t* src, size_t len);

size_t buf_write(buf_t* buf, size_t at, void* x, size_t size);
size_t buf_write_uint8(buf_t* buf, size_t at, uint8_t x);
size_t buf_write_uint16(buf_t* buf, size_t at, uint16_t x);
size_t buf_write_uint32(buf_t* buf, size_t at, uint32_t x);
size_t buf_write_uint64(buf_t* buf, size_t at, uint64_t x);
size_t buf_write_str(buf_t* buf, size_t at, char* str);

void buf_free(buf_t** buf);

#endif // buf.h
