#ifndef MEM_H_
#define MEM_H_

#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define rw_addr(t, addr) ((t*)(addr))

#define rw_i8(addr) rw_addr(i8, addr)
#define rw_i16(addr) rw_addr(i16, addr)
#define rw_i32(addr) rw_addr(i32, addr)
#define rw_i64(addr) rw_addr(i64, addr)

#define rw_u8(addr) rw_addr(u8, addr)
#define rw_u16(addr) rw_addr(u16, addr)
#define rw_u32(addr) rw_addr(u32, addr)
#define rw_u64(addr) rw_addr(u64, addr)

#endif /* MEM_H_ */
