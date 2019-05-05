#ifndef _OBJVEC_TESTS_
#define _OBJVEC_TESTS_

#include <stdio.h>
#include "CuTest.h"
#include "vs.h"


CuSuite* objvec_getsuite();

/* adj param for adj_func while calling realloc()
 * insert len objs at the postion where offset of off */
struct adjp {
	unsigned int off;
	unsigned int len;
};

/* tests */
void test_objv_init_alloc_free(CuTest *tc);
void test_objv_realloc(CuTest *tc);



#endif /* _OBJVEC_TESTS_ */



