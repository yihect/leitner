#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned long	idt;

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

typedef __signed__ long long __s64;
typedef unsigned long long __u64;


typedef         __u8            uint8_t;
typedef         __u16           uint16_t;
typedef         __u32           uint32_t;

//#ifndef uint64_t
//typedef         __u64           uint64_t;
//#endif
//typedef         __u64           u_int64_t;
//typedef         __s64           int64_t;


typedef __u64 u64;
typedef __s64 s64;

typedef __u32 u32;
typedef __s32 s32;

typedef __u16 u16;
typedef __s16 s16;

typedef __u8  u8;
typedef __s8  s8;


//#include "bitops.h"
//#define BITS_PER_LONG 64
//#define DECLARE_BITMAP(name,bits) \
          //unsigned long name[BITS_TO_LONGS(bits)]

#endif /* _TYPES_H_ */

