#ifndef _MEMPOOL_TESTS_
#define _MEMPOOL_TESTS_

#include <stdio.h>
#include "CuTest.h"
#include "vs.h"


CuSuite* mempool_getsuite();


/* tests */
void test_init_mempool(CuTest *tc);
void test_alloc_mempool(CuTest *tc);



#endif /* _MEMPOOL_TESTS_ */	



