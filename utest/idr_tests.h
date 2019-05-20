#ifndef _IDR_TESTS_
#define _IDR_TESTS_

#include <stdio.h>
#include "CuTest.h"
#include "vs.h"


CuSuite* idr_getsuite();

/* data item for test */
struct item {
	unsigned long index;
	unsigned int order;
};

/* tests */
void test_idr_alloc_free(CuTest *tc);
void test_idr_alloc_cyclic_test();
void test_idr_replace_test(CuTest *tc);
void test_idr_null_test(CuTest *tc);
void test_idr_get_next_test(CuTest *tc);
void test_idr_align_test(CuTest *tc);

void test_ida_conv_user(CuTest *tc);
void test_ida_simple_get_remove(CuTest *tc);



#endif /* _IDR_TESTS_ */



