#include "CuTest.h"
#include "objpool.h"
#include "objpool_tests.h"
#include "util.h"

CuSuite* objpool_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_objp_case1);
	SUITE_ADD_TEST(suite, test_objp_case2);
	SUITE_ADD_TEST(suite, test_objp_case3);
	return suite;
}

const int SIZE = 16*1024;
extern size_t OBJ_NUM;

void test_objp_case1(CuTest *tc)
{
	int i;
	void *new;
	void *array[1000];

	objpool_t *opl = create_objpool(SIZE);

	for (i = 0; i < 820; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	CuAssertIntEquals(tc, 820/OBJ_NUM+1, opl->slab_nr);

	for (i = 0; i < 10; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}
	for (i = 10; i < 30; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}
	for (i = 30; i < 820; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 820/OBJ_NUM+1, opl->slab_nr);

	destroy_objpool(opl);
}

void test_objp_case2(CuTest *tc)
{
	int i;
	void *new;
	void *array[1000];

	objpool_t *opl = create_objpool(SIZE);

	/* alloc 200 chunks */
	for (i = 0; i < 200; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	CuAssertIntEquals(tc, 200/OBJ_NUM+1, opl->slab_nr);

	/* free the 80 chunks */
	for (i = 0; i < 80; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 200/OBJ_NUM+1, opl->slab_nr);

	/* alloc 80 chunks */
	for (i = 0; i < 80; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	CuAssertIntEquals(tc, 200/OBJ_NUM+1, opl->slab_nr);

	/* free 200 chunks */
	for (i = 0; i < 200; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 200/OBJ_NUM+1, opl->slab_nr);

	destroy_objpool(opl);
}

void test_objp_case3(CuTest *tc)
{
	int i;
	void *new;
	void *array[1000];

	objpool_t *opl = create_objpool(SIZE);

	/* alloc 128 chunks */
	for (i = 0; i < 128; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	CuAssertIntEquals(tc, 1, opl->slab_nr);
	CuAssertIntEquals(tc, 128, opl->obj_nr);
	CuAssertIntEquals(tc, 0, opl->free_obj_nr);

	/* free the 80 chunks */
	for (i = 0; i < 80; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 1, opl->slab_nr);
	CuAssertIntEquals(tc, 128, opl->obj_nr);
	CuAssertIntEquals(tc, 80, opl->free_obj_nr);

	/* alloc 80 + 128 chunks */
	for (i = 0; i < 80; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	for (i = 128; i < 256; i++) {
		new = objpool_alloc(opl);
		if (new) {
			memset(new, 'a', SIZE);
			array[i] = new;
		}
	}

	CuAssertIntEquals(tc, 2, opl->slab_nr);
	CuAssertIntEquals(tc, 256, opl->obj_nr);
	CuAssertIntEquals(tc, 0, opl->free_obj_nr);

	/* free 56 chunks */
	for (i = 0; i < 56; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 2, opl->slab_nr);
	CuAssertIntEquals(tc, 256, opl->obj_nr);
	CuAssertIntEquals(tc, 56, opl->free_obj_nr);

	/* free 200 chunks */
	for (i = 56; i < 256; i++) {
		if (array[i]) {
			objpool_free(opl, array[i]);
		}
	}

	CuAssertIntEquals(tc, 2, opl->slab_nr);
	CuAssertIntEquals(tc, 256, opl->obj_nr);
	CuAssertIntEquals(tc, 256, opl->free_obj_nr);

	destroy_objpool(opl);
}



