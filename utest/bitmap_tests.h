#ifndef _BITMAP_TESTS_
#define _BITMAP_TESTS_

#include <stdio.h>
#include "CuTest.h"
#include "vs.h"


CuSuite* bitmap_getsuite();


/* tests */
void test_bitmap_set_clear(CuTest *tc);
void test_bitmap_full_clear(CuTest *tc);
void test_bitmap_for(CuTest *tc);
void test_bitmap_weight(CuTest *tc);



#endif /* _BITMAP_TESTS_ */



