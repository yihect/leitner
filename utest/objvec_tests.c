#include "CuTest.h"
#include "objvec.h"
#include "objvec_tests.h"
#include "util.h"

CuSuite* objvec_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_objv_init_alloc_free);
	SUITE_ADD_TEST(suite, test_objv_realloc);
	return suite;
}

void test_objv_init_alloc_free(CuTest *tc)
{
	printf("test test_objv_init_alloc_free()...\n");

	/* obj size: 16Bytes, 2 Pointers in 64bit machine */
	struct objvec *ov = objv_init(16, CVSP_M_AUTOEX|CVSP_M_NEEDID, NULL);

	char *ov15 = objv_alloc(ov, 15);

	memset(ov15, 0x22, 16*15);
	CuAssertIntEquals(tc, (16*15), get_bnode_mem_len(ov->cp, ov15));

	char *ov270 = objv_alloc(ov, 270);
	CuAssertIntEquals(tc, (16*270), get_bnode_mem_len(ov->cp, ov270));

	/* in 64bits machine, an obj of 16Bytes occupies 4 slots internally
	 * and there is 1 slot for every busy node, namely for every allocation */
	CuAssertIntEquals(tc, (15*4+1 + 270*4+1), ov->cp->used);

	objv_free(ov, ov270);
	objv_free(ov, ov15);
	objv_exit(ov);
}

void our_adjust_fn(char *oldobjs, unsigned oldsize,
		   char *newobjs, unsigned newsize, void *adj_param)
{
	struct adjp *pap = (struct adjp *)adj_param;

	/* firstly, copy content */
	memcpy(newobjs, oldobjs, oldsize);

	/* and then, do adjust of these content */
	char *pobj = NULL, *pdest = NULL;
	for (int i=6, j=31; i>=pap->off; i--,j--) {
		pobj = newobjs + i*16;
		pdest = newobjs + j*16;
		memcpy(pdest, pobj, 16);
	}

	for (int i=0; i<pap->len; i++)
		*(int *)(newobjs+(pap->off+i)*16) = i+7;
}

void test_objv_realloc(CuTest *tc)
{
	printf("test test_objv_realloc()...\n");

	/* obj size: 16Bytes, 2 Pointers in 64bit machine */
	struct objvec *ov = objv_init(16, CVSP_M_AUTOEX|CVSP_M_NEEDID, NULL);

	char *ov7 = objv_alloc(ov, 7);
	CuAssertIntEquals(tc, (7*4+1), ov->cp->used);

	memset(ov7, 0x0, 7*16);
	for (int i=0; i<7; i++)
		*(int *)(ov7+i*16) = i;

	struct adjp ap = {2, 25};	/* expand 25 objs at offset 2 */

	/* 7 is too small, realloc to 32 objs */
	ov7 = objv_realloc(ov, ov7, 32, our_adjust_fn, &ap, true);
	CuAssertIntEquals(tc, (16*32), get_bnode_mem_len(ov->cp, ov7));
	CuAssertIntEquals(tc, (32*4+1), ov->cp->used);

	for (int i=0; i<2; i++)
		CuAssertIntEquals(tc, i, *(int *)(ov7+i*16));
	for (int i=2; i<27; i++)
		CuAssertIntEquals(tc, i-2+7, *(int *)(ov7+i*16));
	for (int i=27; i<32; i++)
		CuAssertIntEquals(tc, i-27+2, *(int *)(ov7+i*16));

	objv_free(ov, ov7);
	objv_exit(ov);
}

