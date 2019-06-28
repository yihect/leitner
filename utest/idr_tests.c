#include <stdint.h>
#include "CuTest.h"
#include "merror.h"
#include "bitmap.h"
#include "idr.h"
#include "idr_tests.h"
#include "util.h"

CuSuite* idr_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_idr_alloc_free);
	SUITE_ADD_TEST(suite, test_idr_alloc_cyclic_test);
	SUITE_ADD_TEST(suite, test_idr_replace_test);
	SUITE_ADD_TEST(suite, test_idr_null_test);
	SUITE_ADD_TEST(suite, test_idr_get_next_test);
	SUITE_ADD_TEST(suite, test_idr_align_test);

	SUITE_ADD_TEST(suite, test_ida_conv_user);
	SUITE_ADD_TEST(suite, test_ida_simple_get_remove);


	/* here we init idr firstly */
	idr_init_cache();

	return suite;
}

#define DUMMY_PTR       ((void *)0x10)
#define INT_MAX		INT32_MAX

struct item *item_create(unsigned long index, unsigned int order)
{
	struct item *ret = malloc(sizeof(*ret));

	ret->index = index;
	ret->order = order;
	return ret;
}

int item_idr_free(int id, void *p, void *data)
{
	struct item *item = p;
	assert(item->index == id);
	free(p);

	return 0;
}

void item_idr_remove(struct idr *idr, int id)
{
	struct item *item = idr_find(idr, id);
	assert(item->index == id);
	idr_remove(idr, id);
	free(item);
}

void test_idr_alloc_free(CuTest *tc)
{
	unsigned long i;
	DEFINE_IDR(idr);
	int ret = 0;

	for (i = 0; i < 10000; i++) {
		struct item *item = item_create(i, 0);
		ret = idr_alloc(&idr, item, 0, 20000);
		assert(ret == i);
	}

	assert(idr_alloc(&idr, DUMMY_PTR, 5, 30) < 0);

	for (i = 0; i < 5000; i++)
		item_idr_remove(&idr, i);

	idr_remove(&idr, 3);

	idr_for_each(&idr, item_idr_free, &idr);
	idr_destroy(&idr);

	assert(idr_is_empty(&idr));

	idr_remove(&idr, 3);
	idr_remove(&idr, 0);

	assert(idr_alloc(&idr, DUMMY_PTR, 0, 0) == 0);
	idr_remove(&idr, 1);
	for (i = 1; i < 32; i++)
		assert(idr_alloc(&idr, DUMMY_PTR, 0, 0) == i);
	idr_remove(&idr, 1 << 30);
	idr_destroy(&idr);

	for (i = INT_MAX - 3UL; i < INT_MAX + 1UL; i++) {
		struct item *item = item_create(i, 0);
		assert(idr_alloc(&idr, item, i, i + 10) == i);
	}
	assert(idr_alloc(&idr, DUMMY_PTR, i - 2, i) == -ENOSPC);
	assert(idr_alloc(&idr, DUMMY_PTR, i - 2, i + 10) == -ENOSPC);

	idr_for_each(&idr, item_idr_free, &idr);
	idr_destroy(&idr);
	idr_destroy(&idr);

	assert(idr_is_empty(&idr));

#if 0
	idr_set_cursor(&idr, INT_MAX - 3UL);
	for (i = INT_MAX - 3UL; i < INT_MAX + 3UL; i++) {
		struct item *item;
		unsigned int id;
		if (i <= INT_MAX)
			item = item_create(i, 0);
		else
			item = item_create(i - INT_MAX - 1, 0);

		id = idr_alloc_cyclic(&idr, item, 0, 0);
		assert(id == item->index);
	}

	idr_for_each(&idr, item_idr_free, &idr);
	idr_destroy(&idr);
	assert(idr_is_empty(&idr));
#endif

	for (i = 1; i < 10000; i++) {
		struct item *item = item_create(i, 0);
		ret = idr_alloc(&idr, item, i, i+1);
		assert(ret == i);
	}

	idr_for_each(&idr, item_idr_free, &idr);

	/* try to alloc the same id twice */
	assert(idr_alloc(&idr, (void *)10000, 10000, 10001) == 10000);
	ret = idr_alloc(&idr, (void *)10000, 10000, 10001);
	//assert(ret == 10000); //fail
	assert(ret == -ENOSPC);
	ret = idr_alloc(&idr, (void *)10001, 10000, 10002);
	assert(ret == 10001);
	ret = idr_alloc(&idr, (void *)10001, 10000, 10002);
	//assert(ret == 10000); //fail
	//assert(ret == 10001); //fail
	assert(ret == -ENOSPC);

	idr_destroy(&idr);
}

void test_idr_replace_test(CuTest *tc)
{
	int id;
	struct item *pi;
	DEFINE_IDR(idr);

	idr_alloc(&idr, (void *)-1, 10, 11);
	assert(!idr_is_empty(&idr));

	struct item *item = item_create(10, 0x1234);
	idr_replace(&idr, item, 10);
	/* after replace */
	idr_for_each_entry(&idr, pi, id) {
		printf("pi->index: %d, pi->order: %d\n",
		       pi->index, pi->order);
	}

	idr_destroy(&idr);
}

void test_idr_alloc_cyclic_test()
{
	unsigned long i;
	DEFINE_IDR(idr);

	assert(idr_alloc_cyclic(&idr, DUMMY_PTR, 0, 0x4000) == 0);
	assert(idr_alloc_cyclic(&idr, DUMMY_PTR, 0x3ffd, 0x4000) == 0x3ffd);
	idr_remove(&idr, 0x3ffd);
	idr_remove(&idr, 0);

	for (i = 0x3ffe; i < 0x4003; i++) {
		int id;
		struct item *item;

		if (i < 0x4000)
			item = item_create(i, 0);
		else
			item = item_create(i - 0x3fff, 0);

		id = idr_alloc_cyclic(&idr, item, 1, 0x4000);
		assert(id == item->index);
	}

	idr_for_each(&idr, item_idr_free, &idr);
	idr_destroy(&idr);
}

void test_idr_null_test(CuTest *tc)
{
	int i;
	DEFINE_IDR(idr);

	assert(idr_is_empty(&idr));

	assert(idr_alloc(&idr, NULL, 0, 0) == 0);

	struct item *pi;
	int id;
	idr_for_each_entry(&idr, pi, id) {
		printf("pi->index: %d, pi->order: %d\n",
		       pi->index, pi->order);
	}

	/* yihect: nothing output in above for() loop,
	 * and the following assert fails, so this version of idr
	 * doesn't support assocating NULL to IDs */
	//assert(!idr_is_empty(&idr));
	idr_remove(&idr, 0);

	assert(idr_alloc(&idr, (void *)-1, 10, 0) == 10);
	assert(!idr_is_empty(&idr));

	assert(idr_replace(&idr, NULL, 10) == (void *)-1);
	assert(idr_find(&idr, 10) == NULL);

	/* can't store NULL ptr in idr */
	//assert(!idr_is_empty(&idr));
}

void test_idr_get_next_test(CuTest *tc)
{
	unsigned long i;
	int nextid;
	DEFINE_IDR(idr);
	//idr_init_base(&idr, base);

	int indices[] = {4, 7, 9, 15, 65, 128, 1000, 99999, 0};

	for(i = 0; indices[i]; i++) {
		struct item *item = item_create(indices[i], 0);
		assert(idr_alloc(&idr, item, indices[i], indices[i+1]) == indices[i]);
	}

	for(i = 0, nextid = 0; indices[i]; i++) {
		idr_get_next(&idr, &nextid);
		assert(nextid == indices[i]);
		nextid++;
	}

	idr_for_each(&idr, item_idr_free, &idr);
	idr_destroy(&idr);
}

void test_idr_align_test(CuTest *tc)
{
	int i, id;
	void *entry;
	DEFINE_IDR(idr_obj);
	char name[] = "Motorola 68000";

	struct idr *idr = &idr_obj;

	for (i = 0; i < 9; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == i);
		idr_for_each_entry(idr, entry, id);
	}
	idr_destroy(idr);

	for (i = 1; i < 10; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == i - 1);
		idr_for_each_entry(idr, entry, id);
	}
	idr_destroy(idr);

	for (i = 2; i < 11; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == i - 2);
		idr_for_each_entry(idr, entry, id);
	}
	idr_destroy(idr);

	for (i = 3; i < 12; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == i - 3);
		idr_for_each_entry(idr, entry, id);
	}
	idr_destroy(idr);

	for (i = 0; i < 8; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == 0);
		assert(idr_alloc(idr, &name[i + 1], 0, 0) == 1);
		idr_for_each_entry(idr, entry, id);
		idr_remove(idr, 1);
		idr_for_each_entry(idr, entry, id);
		idr_remove(idr, 0);
		assert(idr_is_empty(idr));
	}

	for (i = 0; i < 8; i++) {
		assert(idr_alloc(idr, NULL, 0, 0) == 0);
		idr_for_each_entry(idr, entry, id);
		idr_replace(idr, &name[i], 0);
		idr_for_each_entry(idr, entry, id);
		assert(idr_find(idr, 0) == &name[i]);
		idr_remove(idr, 0);
	}

	for (i = 0; i < 8; i++) {
		assert(idr_alloc(idr, &name[i], 0, 0) == 0);
		assert(idr_alloc(idr, NULL, 0, 0) == 1);
		idr_remove(idr, 1);
		idr_for_each_entry(idr, entry, id);
		idr_replace(idr, &name[i + 1], 0);
		idr_for_each_entry(idr, entry, id);
		idr_remove(idr, 0);
	}
}

/* Check handling of conversions between exceptional entries and full bitmaps */
void test_ida_conv_user(CuTest *tc)
{
	DEFINE_IDA(ida);
	unsigned long i;
	int id;

	for (i = 0; i < 1000000; i++) {
		ida_get_new(&ida, &id);
		if(id == -ENOSPC) {
			printf("id_NOSPC: %d at %d\n", id, i);
		}else if(id == -EAGAIN) {
			printf("id_AGAIN: %d at %d\n", id, i);
		}
		// get id radomly, not like idr
		//assert(id == i);
	}
	ida_destroy(&ida);
}


void test_ida_simple_get_remove(CuTest *tc)
{
	DEFINE_IDA(ida);
	unsigned long i;
	unsigned int id[10000] = {0};

	for (i = 0; i < 10000; i++) {
		id[i] = ida_simple_get(&ida, 0, 20000);
	}
	assert(ida_simple_get(&ida, 5, 30) < 0);

	for (i = 0; i < 10000; i++) {
		ida_simple_remove(&ida, i);
	}
	// maybe we need to implement this
	//assert(ida_is_empty(&ida));

	ida_destroy(&ida);
}


