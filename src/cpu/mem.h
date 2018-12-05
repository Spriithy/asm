#ifndef MEM_H_
#define MEM_H_

#include <stdint.h>

typedef int8_t  byte;
typedef int16_t word;
typedef int32_t dword;
typedef int64_t qword;

typedef uint8_t  byte_u;
typedef uint16_t word_u;
typedef uint32_t dword_u;
typedef uint64_t qword_u;

#define rw_addr(t, addr) ((t*)(addr))

#define rw_byte(addr) rw_addr(byte, addr)
#define rw_word(addr) rw_addr(word, addr)
#define rw_dword(addr) rw_addr(dword, addr)
#define rw_qword(addr) rw_addr(qword, addr)

#define rw_byte_u(addr) rw_addr(byte_u, addr)
#define rw_word_u(addr) rw_addr(word_u, addr)
#define rw_dword_u(addr) rw_addr(dword_u, addr)
#define rw_qword_u(addr) rw_addr(qword_u, addr)

#endif /* MEM_H_ */
