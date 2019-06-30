#include <stdio.h>
#include <assert.h>
#include "CuTest.h"
#include "msm.h"
#include "msm_tests.h"
#include "util.h"
#include "objpool.h"

CuSuite* msm_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_msm_objpool);
	return suite;
}

objpool_t *bar_opl, *foo_opl;
struct bar *bar_array[240]={0}, *bar_array2[240]={0}, *bar_free_array2[240]={0};
struct foo *foo_array[240]={0}, *foo_array2[240]={0};

struct bar {
	int bar_id;
	char bar_desc[12];
};

void bar_ser_ctor_high(objpool_t *mpl, void *obj)
{
	struct bar *pb = (struct bar *)obj;
	bar_array2[pb->bar_id] = pb;
}

static struct objp_obj_ops bar_oops = {
	.ser_ctor_high = bar_ser_ctor_high,
};

struct foo {
	struct bar *pb;
	int foo_id;
	int foo_age;
	char foo_name[12];
};

void foo_ser_ctor_high(objpool_t *mpl, void *obj)
{
	struct foo *pf = (struct foo *)obj;
	foo_array2[pf->foo_id] = pf;
}

void foo_ser_ctor_low(objpool_t *mpl)
{
	for (int i = 0; i < 240; i++) {
		if (foo_array2[i] != NULL) {
			foo_array2[i]->pb = bar_array2[i];
		}
	}
}

void foo_ser_dtor(objpool_t *mpl)
{
	struct foo *sf;
	for (int i = 0; i < 240; i++) {
		if (foo_array[i] != NULL) {
			sf = get_sered_obj(mpl, foo_array[i]);
			sf->pb = (struct bar *)(sf->pb->bar_id);
		}
	}

}

static struct objp_obj_ops foo_oops = {
	.ser_ctor_high = foo_ser_ctor_high,
	.ser_ctor_low = foo_ser_ctor_low,
	.ser_dtor = foo_ser_dtor,
};

static struct ds_node dn_bar = {
	.name		= "bar",
};

static struct ds_node dn_foo = {
	.name		= "foo",
};

struct all_data {
	objpool_t *barpl;
	objpool_t *foopl;
};

struct all_data global_ad = {0};

unsigned int ad_getlen(struct ds_node *dn)
{
	return sizeof(struct all_data);
}

void *ad_enc_detail(struct ds_node *dn, char *dst)
{
	struct all_data *ad = (struct all_data *)dn->pool_pt; /* original ad */
	struct ser_sum_entry *sse = dn->sered_sum;

	/* must set dn->sered_detail and sse->sse_len */
	dn->sered_detail = dst;
	void *dend = msm_ser(dst, ad, sizeof(struct all_data), true);
	sse->sse_len = (char *)dend - dst;

	struct all_data *sered_ad = (struct all_data *)dn->sered_detail;
	sered_ad->barpl = (objpool_t *)(ad->barpl)->pool_id; /* old id */
	sered_ad->foopl = (objpool_t *)(ad->foopl)->pool_id; /* old id */

	return dend;
}

void *ad_dec_detail_high(struct ds_node *dn, char *src)
{
	struct all_data *sered_ad = (struct all_data  *)src;
	struct all_data *ad = (struct all_data *)dn->pool_pt; /* original ad */

	/* just deser firstly, and then do patching in *_detail_low() */
	assert(dn->pool_type>=PT_NOP);
	return msm_deser((void *)src, (void *)ad, sizeof(struct all_data), true);
}

void ad_dec_detail_low(struct ds_node *dn)
{
	struct ser_root_data *srd = dn->srd;
	struct all_data *ad = (struct all_data *)dn->pool_pt; /* original ad */
	struct all_data *sered_ad = (struct all_data *)dn->sered_detail;

	/* get new pool ptr */
	ad->barpl = (objpool_t *)get_new_pool_pt(srd, (int)sered_ad->barpl);
	ad->foopl = (objpool_t *)get_new_pool_pt(srd, (int)sered_ad->foopl);
}

static struct ds_node dn_ad = {
	.name		= "adata",
	.get_len	= ad_getlen,
	.enc_detail	= ad_enc_detail,
	.dec_detail_high	= ad_dec_detail_high,
	.dec_detail_low		= ad_dec_detail_low,
};

void test_msm_objpool(CuTest *tc)
{
	printf("test msm()...\n");

	char *filename = "./test.msm";

	bar_opl = create_objpool(sizeof(struct bar), OBJP_M_NEEDID, &bar_oops);
	foo_opl = create_objpool(sizeof(struct foo), OBJP_M_NEEDID, &foo_oops);

	global_ad.barpl = bar_opl;
	global_ad.foopl = foo_opl;

	struct ser_root_data *srd = msm_get();
	msm_register_ds(srd, &dn_bar, PT_OBJP, (void *)bar_opl);
	msm_register_ds(srd, &dn_foo, PT_OBJP, (void *)foo_opl);
	msm_register_ds(srd, &dn_ad, PT_NOP, (void *)&global_ad);

	int i;
	void *new;

	/* we use a objpool... */
	for (i = 0; i < 240; i++) {
		new = objpool_alloc(bar_opl);
		if (new) {
			((struct bar*)new)->bar_id = i;
			snprintf(((struct bar*)new)->bar_desc, 12, "bar%d", i);
			bar_array[i] = new;
		}
	}
	for (i = 0; i < 240; i++) {
		if (i%13 == 0) {
			snprintf(bar_array[i]->bar_desc, 12, "barfreed", i);
			bar_free_array2[i] = bar_array[i];
			objpool_free(bar_opl, bar_array[i]);
			bar_array[i] = NULL;
		}
	}

	for (i = 0; i < 240; i++) {
		if (i%13 != 0) {
			new = objpool_alloc(foo_opl);
			if (new) {
				foo_array[i] = new;
				((struct foo*)foo_array[i])->pb = bar_array[i];
				((struct foo*)foo_array[i])->foo_id = i;
				((struct foo*)foo_array[i])->foo_age = i*i+3;
				snprintf(((struct foo*)foo_array[i])->foo_name, 12, "foo%d", i);
			}
		}
	}

	int bar_obj_nr = bar_opl->obj_nr;
	int foo_obj_nr = foo_opl->obj_nr;
	int bar_free_obj_nr = bar_opl->free_obj_nr;
	int foo_free_obj_nr = foo_opl->free_obj_nr;

	/* and try to serialize it */
	msm_dump(srd, MSM_FILE, filename);

	for (i = 0; i < 240; i++) {
		if (bar_array[i] != NULL)
			objpool_free(bar_opl, bar_array[i]);
		if (foo_array[i] != NULL)
			objpool_free(foo_opl, foo_array[i]);
	}
	destroy_objpool(bar_opl);
	destroy_objpool(foo_opl);

	/* try to deserialize it */
	objpool_t *bar_opl2 = create_objpool(sizeof(struct bar), OBJP_M_NEEDID, &bar_oops);
	objpool_t *foo_opl2 = create_objpool(sizeof(struct foo), OBJP_M_NEEDID, &foo_oops);
	msm_change_ds_pt(srd, &dn_bar, bar_opl2);
	msm_change_ds_pt(srd, &dn_foo, foo_opl2);
	msm_load(srd, MSM_FILE, filename);

	char buf[16] = {0};
	/* asserting ... */
	for (i = 0; i < 240; i++) {
		if (i%13 != 0) {
			CuAssertPtrNotNull(tc, foo_array2[i]);
			CuAssertPtrEquals(tc, bar_array2[i], (struct foo*)foo_array2[i]->pb);
			CuAssertIntEquals(tc, i, (struct foo*)foo_array2[i]->foo_id);
			CuAssertIntEquals(tc, i*i+3, (struct foo*)foo_array2[i]->foo_age);
			snprintf(buf, 12, "foo%d", i);
			CuAssertStrEquals(tc, buf, (struct foo*)foo_array2[i]->foo_name);

			CuAssertIntEquals(tc, i, (struct bar*)bar_array2[i]->bar_id);
			snprintf(buf, 12, "bar%d", i);
			CuAssertStrEquals(tc, buf, (struct bar*)bar_array2[i]->bar_desc);
			CuAssertPtrNotNull(tc, bar_array2[i]);

		}else {
			CuAssertIntEquals(tc, i, (struct bar*)bar_free_array2[i]->bar_id);
			CuAssertStrEquals(tc, "barfreed", (struct bar*)bar_free_array2[i]->bar_desc);
			CuAssertPtrNull(tc, bar_array2[i]);
		}
	}

	CuAssertIntEquals(tc, 256, foo_opl2->obj_nr);
	CuAssertIntEquals(tc, 256, bar_opl2->obj_nr);

	/* try to alloc some new objs from the desered pools */
	struct bar *pnewb = objpool_alloc(bar_opl2);
	CuAssertPtrNotNull(tc, pnewb);
	struct foo *pnewf = objpool_alloc(foo_opl2);
	CuAssertPtrNotNull(tc, pnewf);

	CuAssertIntEquals(tc, foo_free_obj_nr-1, foo_opl2->free_obj_nr);
	CuAssertIntEquals(tc, bar_free_obj_nr-1, bar_opl2->free_obj_nr);

	objpool_free(bar_opl2, pnewb);
	objpool_free(foo_opl2, pnewf);

	/* test ptr in all_data */
	CuAssertPtrEquals(tc, bar_opl2, global_ad.barpl);
	CuAssertPtrEquals(tc, foo_opl2, global_ad.foopl);

	msm_put(srd);
}


