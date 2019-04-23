#ifndef _CVSPOOL_TESTS_
#define _CVSPOOL_TESTS_

#include <stdio.h>
#include "CuTest.h"
#include "vs.h"


CuSuite* cvspool_getsuite();


/* tests */
void test_fuck_macro(CuTest *tc);
void test_init_cvspool(CuTest *tc);
void test_alloc1_cvspool(CuTest *tc);
void test_alloc2_cvspool(CuTest *tc);
void test_free_cvspool(CuTest *tc);



#endif /* _MEMPOOL_TESTS_ */	



