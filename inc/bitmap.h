#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "types.h"

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_PER_BYTE           8
#define BITS_PER_LONG		(sizeof(long ) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, (BITS_PER_BYTE * sizeof(long)))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITOP_WORD(nr)		((nr) / BITS_PER_LONG)

unsigned long __ffs(unsigned long word);
unsigned long __fls(unsigned long word);

#define ffz(x) __ffs( ~(x) )

#define DECLARE_BITMAP(name,bits) \
          unsigned long name[BITS_TO_LONGS(bits)]

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)                                    \
(                                                                       \
        ((nbits) % BITS_PER_LONG) ?                                     \
                (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL               \
)

#define small_const_nbits(nbits) \
  ((nbits) <= BITS_PER_LONG)


#if 0
#define for_each_set_bit(bit, addr, size) \
          for ((bit) = find_first_bit((addr), (size));            \
	                   (bit) < (size);                                    \
	                   (bit) = find_next_bit((addr), (size), (bit) + 1))

/* same as for_each_set_bit() but use bit as value to start with */
#define for_each_set_bit_from(bit, addr, size) \
          for ((bit) = find_next_bit((addr), (size), (bit));      \
	                   (bit) < (size);                                    \
	                   (bit) = find_next_bit((addr), (size), (bit) + 1))

#define for_each_clear_bit(bit, addr, size) \
          for ((bit) = find_first_zero_bit((addr), (size));       \
	                   (bit) < (size);                                    \
	                   (bit) = find_next_zero_bit((addr), (size), (bit) + 1))

/* same as for_each_clear_bit() but use bit as value to start with */
#define for_each_clear_bit_from(bit, addr, size) \
          for ((bit) = find_next_zero_bit((addr), (size), (bit)); \
	                   (bit) < (size);                                    \
	                   (bit) = find_next_zero_bit((addr), (size), (bit) + 1))
#endif


void set_bit(int nr, volatile unsigned long *addr);
void clear_bit(int nr, volatile unsigned long *addr);
void change_bit(int nr, volatile unsigned long *addr);
int test_and_set_bit(int nr, volatile unsigned long *addr);
int test_and_clear_bit(int nr, volatile unsigned long *addr);
int test_and_change_bit(int nr, volatile unsigned long *addr);
int test_bit(int nr, const volatile unsigned long *addr);

void bitmap_zero(unsigned long *dst, unsigned int nbits);
int bitmap_empty(const unsigned long *src, unsigned nbits);
int bitmap_full(const unsigned long *src, unsigned int nbits);
void bitmap_set(unsigned long *map, unsigned int start, int len);
void bitmap_clear(unsigned long *map, unsigned int start, int len);

int bitmap_weight(const unsigned long *src, unsigned int nbits);

int fls64(__u64 x);


/*
 * Find the next set bit in a memory region.
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
    unsigned long offset);


/*
 * This implementation of find_{first,next}_zero_bit was stolen from
 * Linus' asm-alpha/bitops.h.
 */
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
    unsigned long offset);


/*
 * Find the first set bit in a memory region.
 */
unsigned long find_first_bit(const unsigned long *addr, unsigned long size);


/*
 * Find the first cleared bit in a memory region.
 */
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size);




#endif


