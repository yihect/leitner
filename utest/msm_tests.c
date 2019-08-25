#include <stdio.h>
#include <assert.h>
#include "CuTest.h"
#include "msm.h"
#include "msm_tests.h"
#include "util.h"
#include "objpool.h"
#include "cvspool.h"
#include "objvec.h"

CuSuite* msm_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_msm);
	return suite;
}

char *filename = "./test.msm";

/*******************************************************************
 * bar & foo of objpool
 */
struct bar {
	int bar_id;
	char bar_desc[12];
};

objpool_t *bar_opl, *foo_opl;
struct bar *bar_array[240]={0}, *bar_array2[240]={0}, bar_free_array2[240]={0};
struct foo *foo_array[240]={0}, *foo_array2[240]={0};

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
	char *foo_desc;		// allocated from cp_str cvsp
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

			/* we restor foo_desc into cp_str2 */
			extern cvspool *cp_str2;
			foo_array2[i]->foo_desc =
				node_from_id(cp_str2, (unsigned )foo_array2[i]->foo_desc);
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

			/* we know foo_desc comes from cp_str */
			extern cvspool *cp_str;
			sf->foo_desc = (char *)id_of_node(cp_str, foo_array[i]);
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

/*******************************************************************
 * ad of nopool
 */
struct all_data {
	objpool_t *barpl;
	objpool_t *foopl;
	char *strbuf1, *strbuf2;	/* allocated from cp_str */
	struct list_head lh_head;	/* store all ov_lh objs */
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

	/* strbuf* is from cp_str */
	extern cvspool *cp_str;
	sered_ad->strbuf1 = (char *)id_of_node(cp_str, sered_ad->strbuf1);
	sered_ad->strbuf2 = (char *)id_of_node(cp_str, sered_ad->strbuf2);

	/* lh_head nodes from ov_lh objvec */
	extern struct objvec *ov_lh;
	if (!list_empty(&ad->lh_head)) {
		sered_ad->lh_head.next = id_of_node(ov_lh->cp, ad->lh_head.next);
		sered_ad->lh_head.prev = id_of_node(ov_lh->cp, ad->lh_head.prev);
	}

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

	/* restore strbuf */
	extern cvspool *cp_str2;
	ad->strbuf1 = node_from_id(cp_str2, (unsigned long)ad->strbuf1);
	ad->strbuf2 = node_from_id(cp_str2, (unsigned long)ad->strbuf2);

	/* restore lh_head nodes */
	extern struct objvec *ov_lh2;
	if (sered_ad->lh_head.next == sered_ad->lh_head.prev) {
		INIT_LIST_HEAD(&ad->lh_head);
	} else {
		ad->lh_head.next = node_from_id(ov_lh2->cp, ad->lh_head.next);
		ad->lh_head.prev = node_from_id(ov_lh2->cp, ad->lh_head.prev);
	}
}

static struct ds_node dn_ad = {
	.name		= "adata",
	.get_len	= ad_getlen,
	.enc_detail	= ad_enc_detail,
	.dec_detail_high	= ad_dec_detail_high,
	.dec_detail_low		= ad_dec_detail_low,
};

/*******************************************************************
 * cv_str of cvspool
 */
cvspool *cp_str=NULL;
cvspool *cp_str2 = NULL;

static struct ds_node dn_cp_str = {
	.name		= "cpstr",
};

/*******************************************************************
 * ov_lh of cvspool
 */
struct objvec *ov_lh = NULL;
struct objvec *ov_lh2 = NULL;
static struct ds_node dn_ov_lh = {
	.name		= "ovlh",
};

struct glh {
	unsigned int id;
	char *name;		/* from cp_str pool */
	struct list_head lh;
};

static struct ds_node dn_ovlh = {
	.name		= "ovlh",
};

/*******************************************************************
 */

static inline int objs_check(void *p, void *data)
{
	int *bn_cnt = (int *)data;

	(*bn_cnt)++;

	return 0;
}

/* test function */
void test_msm(CuTest *tc)
{
	printf("test msm objpool ()...\n");

	bar_opl = create_objpool(sizeof(struct bar), OBJP_M_NEEDID, &bar_oops);
	foo_opl = create_objpool(sizeof(struct foo), OBJP_M_NEEDID, &foo_oops);

	global_ad.barpl = bar_opl;
	global_ad.foopl = foo_opl;
	INIT_LIST_HEAD(&global_ad.lh_head);

	/* init a cvspool with 4096 slots */
	int ret=cvsp_init(&cp_str, 4096, CVSP_M_AUTOEX|CVSP_M_NEEDID);
	CuAssertIntEquals(tc, 0, ret);

	/* init an objvec pool */
	ov_lh = objv_init(sizeof(struct glh), CVSP_M_AUTOEX|CVSP_M_NEEDID, NULL);

	struct ser_root_data *srd = msm_get();
	msm_register_ds(srd, &dn_bar, PT_OBJP, (void *)bar_opl);
	msm_register_ds(srd, &dn_foo, PT_OBJP, (void *)foo_opl);
	msm_register_ds(srd, &dn_ad, PT_NOP, (void *)&global_ad);
	msm_register_ds(srd, &dn_cp_str, PT_CVSP, (void *)cp_str);
	msm_register_ds(srd, &dn_ovlh, PT_OBJV, (void *)ov_lh);

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
			snprintf(bar_array[i]->bar_desc, 12, "barfreed");
			//bar_free_array2[i] = bar_array[i];
			memcpy(&bar_free_array2[i], bar_array[i], sizeof(struct bar));
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
				((struct foo*)foo_array[i])->foo_desc = cvsp_alloc(cp_str, 20);
				snprintf(((struct foo*)foo_array[i])->foo_desc, 20, "foo%d_desc", i);
			}
		}
	}

	int bar_obj_nr = bar_opl->obj_nr;
	int foo_obj_nr = foo_opl->obj_nr;
	int bar_free_obj_nr = bar_opl->free_obj_nr;
	int foo_free_obj_nr = foo_opl->free_obj_nr;

	/* use out of cp_str, we can only use 4096-1 slots
	 * use data member in cvspool directly */
	int remain_bytes = (cp_str->total-cp_str->used-1)*4;
	global_ad.strbuf1 = cvsp_alloc(cp_str, remain_bytes);
	CuAssertPtrNotNull(tc, global_ad.strbuf1);

	//CuAssertIntEquals(tc, 4095*4, remain_bytes);
	memset(global_ad.strbuf1, 'T', remain_bytes);

	global_ad.strbuf2 = cvsp_alloc(cp_str, 1024);
	memset(global_ad.strbuf2, 'F', 1024);

	/* test objvec */
	for (i = 0; i < 10; i++) {
		struct glh *gl = objv_alloc(ov_lh, 1);
		gl->name = cvsp_alloc(cp_str, 24);
		snprintf(gl->name, 24, "glhid%d", i);
		gl->id = i;

		list_add(&gl->lh, &global_ad.lh_head);
	}

	/* save a temply cvspool, for assert after deser */
	cvspool tempcvsp = {0};
	memcpy(&tempcvsp, cp_str, sizeof(cvspool));


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
	cvsp_destroy(cp_str);
	cp_str = NULL;

	struct glh *gl = NULL;
	list_for_each_entry(gl, &global_ad.lh_head, lh) {
		list_del(&gl->lh);
		objv_free(ov_lh, gl);
	}
	objv_exit(ov_lh);
	ov_lh = NULL;

	/* try to deserialize it */
	objpool_t *bar_opl2 = create_objpool(sizeof(struct bar), OBJP_M_NEEDID, &bar_oops);
	objpool_t *foo_opl2 = create_objpool(sizeof(struct foo), OBJP_M_NEEDID, &foo_oops);
	msm_change_ds_pt(srd, &dn_bar, bar_opl2);
	msm_change_ds_pt(srd, &dn_foo, foo_opl2);

	/* a new cp_str */
	cvsp_init(&cp_str2, 4096, CVSP_M_AUTOEX|CVSP_M_NEEDID);
	msm_change_ds_pt(srd, &dn_cp_str, cp_str2);

	/* a new objvec */
	ov_lh2 = objv_init(sizeof(struct glh), CVSP_M_AUTOEX|CVSP_M_NEEDID, NULL);
	msm_change_ds_pt(srd, &dn_ovlh, ov_lh2);

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
			CuAssertIntEquals(tc, i, (struct bar*)bar_free_array2[i].bar_id);
			CuAssertStrEquals(tc, "barfreed", (struct bar*)bar_free_array2[i].bar_desc);
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

	/* assert & reuse for cp_str2 */
	CuAssertIntEquals(tc, tempcvsp.total, cp_str2->total);
	CuAssertIntEquals(tc, tempcvsp.used, cp_str2->used);
	CuAssertIntEquals(tc, tempcvsp.total_mem, cp_str2->total_mem);
	CuAssertIntEquals(tc, tempcvsp.mode, cp_str2->mode);
	CuAssertIntEquals(tc, tempcvsp.trunk_cnt, cp_str2->trunk_cnt);
	CuAssertIntEquals(tc, tempcvsp.trunk_ocnt, cp_str2->trunk_ocnt);

	int tcnt = 0;
	cvspool_trunk *t = NULL;
	for_each_trunk(cp_str2, &t) {
		tcnt++;
	}
	CuAssertIntEquals(tc, tcnt, cp_str2->trunk_cnt);

	/* test free and alloc for cp_str2 */
	char *strbuf3 = NULL;
	strbuf3 = cvsp_alloc(cp_str2, 512);
	CuAssertPtrNotNull(tc, strbuf3);
	CuAssertIntEquals(tc, tempcvsp.used+((512+4)>>2), cp_str2->used);
	memset(strbuf3, 'G', 512);

	cvsp_free(cp_str2, global_ad.strbuf1);
	global_ad.strbuf1 = NULL;

	cvsp_free(cp_str2, global_ad.strbuf2);
	global_ad.strbuf2 = NULL;

	/* test objvec */
	for (int i=101; i<103; i++) {
		gl = objv_alloc(ov_lh2, 10);
		gl->name = cvsp_alloc(cp_str2, 24);
		snprintf(gl->name, 24, "glhid%d", i);
		gl->id = i;
		list_add(&gl->lh, &global_ad.lh_head);
	}

	int gl_cnt = 0;
	gl = NULL;
	list_for_each_entry(gl, &global_ad.lh_head, lh) {
		gl_cnt++;
		list_del(&gl->lh);
		objv_free(ov_lh2, gl);
	}
	CuAssertIntEquals(tc, 10+2, gl_cnt);

	gl_cnt = 0;
	cvsp_for_each_bnode(ov_lh2->cp, objs_check, &gl_cnt);
	CuAssertIntEquals(tc, 0, gl_cnt);

	msm_put(srd);
}


