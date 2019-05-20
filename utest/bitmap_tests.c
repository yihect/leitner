#include "CuTest.h"
#include "bitops.h"
#include "bitmap.h"
#include "bitmap_tests.h"
#include "util.h"

CuSuite* bitmap_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_bitmap_set_clear);
	SUITE_ADD_TEST(suite, test_bitmap_full_clear);
	SUITE_ADD_TEST(suite, test_bitmap_for);
	SUITE_ADD_TEST(suite, test_bitmap_weight);
	return suite;
}

void test_bitmap_set_clear(CuTest *tc)
{
	DECLARE_BITMAP(mbits,23);
	*mbits = 0;

	set_bit(3, mbits);
	set_bit(4, mbits);
	set_bit(6, mbits);
	set_bit(8, mbits);
	set_bit(20, mbits);
	set_bit(23, mbits);
	set_bit(26, mbits);
	CuAssertIntEquals(tc, 1, test_bit(3, mbits));
	CuAssertIntEquals(tc, 1, test_bit(4, mbits));
	CuAssertIntEquals(tc, 1, test_bit(6, mbits));
	CuAssertIntEquals(tc, 1, test_bit(8, mbits));
	CuAssertIntEquals(tc, 1, test_bit(20, mbits));
	CuAssertIntEquals(tc, 1, test_bit(23, mbits));
	CuAssertIntEquals(tc, 1, test_bit(26, mbits));

	clear_bit(6, mbits);
	CuAssertIntEquals(tc, 0, test_bit(6, mbits));
	set_bit(8, mbits);
	CuAssertIntEquals(tc, 1, test_bit(8, mbits));

	CuAssertIntEquals(tc, 0, test_and_set_bit(12, mbits));
	CuAssertIntEquals(tc, 1, test_bit(12, mbits));
	CuAssertIntEquals(tc, 0, test_and_set_bit(14, mbits));
	CuAssertIntEquals(tc, 1, test_bit(14, mbits));

	CuAssertIntEquals(tc, 1, test_and_clear_bit(20, mbits));
	CuAssertIntEquals(tc, 0, test_bit(20, mbits));
	CuAssertIntEquals(tc, 1, test_and_clear_bit(23, mbits));
	CuAssertIntEquals(tc, 0, test_bit(23, mbits));
}

void test_bitmap_full_clear(CuTest *tc)
{
	printf("sizeof(int): %d, sizeof(long): %d\n", sizeof(int), sizeof(long));

	struct a {
		unsigned long p1;
		DECLARE_BITMAP(mbits,48);
		unsigned long p2;
	} t;

	memset((char *)&t, 0xff, sizeof(t));

	bitmap_zero(t.mbits, 48); /* 0~47 + 48~63 */
	CuAssertIntEquals(tc, 0, test_bit(0, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(1, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(46, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(47, t.mbits));

	CuAssertIntEquals(tc, 0, test_bit(48, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(49, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(63, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(64, t.mbits));

	CuAssertIntEquals(tc, -1, t.p1); /* bits all set */
	CuAssertIntEquals(tc, -1, t.p2); /* bits all set */

	CuAssertIntEquals(tc, 1, bitmap_full(&t.p1, 64)); /* bits all set */
	CuAssertIntEquals(tc, 1, bitmap_empty(&t.mbits, 64));
	CuAssertIntEquals(tc, 1, bitmap_full(&t.p2, 64)); /* bits all set */

	/* set and clear */
	bitmap_set(t.mbits, 3, 4);
	CuAssertIntEquals(tc, 0, test_bit(2, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(3, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(5, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(6, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(7, t.mbits));

	bitmap_set(t.mbits, 9, 17);
	bitmap_clear(t.mbits, 12, 3);
	CuAssertIntEquals(tc, 0, test_bit(8, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(9, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(11, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(12, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(13, t.mbits));
	CuAssertIntEquals(tc, 0, test_bit(14, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(15, t.mbits));
	CuAssertIntEquals(tc, 1, test_bit(16, t.mbits));

}

#define BITS_LEN  32
void test_bitmap_for(CuTest *tc)
{
	int n;
	unsigned int r = 0;
	DECLARE_BITMAP(mbits, BITS_LEN);

	*((unsigned int *)mbits) = 0x12345678;

	/* test find_first_bit/find_next_bit */
	n = find_first_bit(&mbits, BITS_LEN);
	while (n < BITS_LEN) {
		r |= 1<<n;
		//printf("bits[%d] set in mbits \n", (unsigned long)n);
		n = find_next_bit(&mbits, BITS_LEN, n + 1);
	}
	CuAssertIntEquals(tc, 0x12345678, r);

	/* test find_first_zero_bit/find_next_zero_bit */
	n=-1;
	n = find_first_zero_bit(&mbits, BITS_LEN);
	while (n < BITS_LEN) {
		r |= 1<<n;
		//printf("bits[%d] not set in mbits \n", (unsigned long)n);
		n = find_next_zero_bit(&mbits, BITS_LEN, n + 1);
	}
	CuAssertIntEquals(tc, -1, r);

	/* test for_each_... */
	n=-1; r=0;
	for_each_set_bit(n, mbits, BITS_LEN) {
		r |= 1<<n;
		//printf("bits[%d] set in mbits \n", (unsigned long)n);
	}
	CuAssertIntEquals(tc, 0x12345678, r);
}

void test_bitmap_weight(CuTest *tc)
{
	DECLARE_BITMAP(mbits, 36);

	*((unsigned long *)mbits) = 0x36;	/* 0x36, 4bits set totally */
	CuAssertIntEquals(tc, 4, bitmap_weight(mbits, 36));

	*((unsigned long *)mbits) = 0x123456;
	CuAssertIntEquals(tc, 9, bitmap_weight(mbits, 36));
}




