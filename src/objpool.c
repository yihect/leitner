#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "objpool.h"

/*
 *      slab                slab
 *      +---------+    +--->+---------+
 *      |  *free  |    |    |  *free  |
 *      |  *next  |----+    |  *next  |
 *      +---------+         +---------+ -----------------------
 *      |         |         |         |  /|\            /|\
 *      |         |         |         |   |              |
 *      |         |         |         | obj_size         |
 *      |         |         |         |   |        real_obj_size
 *      |         |         |         |  \|/             |
 *      |         |         +---------+ --------         |
 *      |         |         |  void*  |                 \|/
 *      |         |         +---------+ -----------------------
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         +---------+
 *      |         |         |  void*  |
 *      |         |         +---------+
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         |         |
 *      |         |         +---------+
 *      |         |         |  void*  |
 *      +---------+         +---------+
 *
 */

#define SLAB_CACHE(slab) ((void*)slab + sizeof(slab_t))

const size_t OBJ_NUM = 128;

void set_freepointer(objpool_t *mpl, void *obj, void *fp)
{
	*(void **)(obj + mpl->obj_size) = fp;
}

void *get_freepointer(objpool_t *mpl, void *obj)
{
	return *(void **)(obj + mpl->obj_size);
}

void init_slab(objpool_t *mpl, slab_t *slab)
{
	void *start = SLAB_CACHE(slab);
	void *end = SLAB_CACHE(slab) + mpl->slab_size;

	slab->free = start;
	for (; start < end; start += mpl->real_obj_size) {
		set_freepointer(mpl, start, start + mpl->real_obj_size);
	}

	set_freepointer(mpl, start - mpl->real_obj_size, NULL);
}

slab_t *add_new_slab(objpool_t *mpl)
{
	slab_t *new_slab;

	new_slab = malloc(sizeof(slab_t) + mpl->slab_size);

	init_slab(mpl, new_slab);

	new_slab->next = mpl->slab_head;
	mpl->slab_head = new_slab;

	new_slab->id = mpl->slab_nr++;	/* slab id, count from zero */
	mpl->obj_nr += OBJ_NUM;
	mpl->free_obj_nr += OBJ_NUM;

	return new_slab;
}

void free_slab(slab_t *slab)
{
	free(slab);
}

void objpool_init(objpool_t  *mpl)
{
	assert(mpl != NULL);

	mpl->slab_nr = 0;
	mpl->obj_nr = 0;
	mpl->free_obj_nr = 0;
	mpl->slab_head = NULL;
	mpl->recent_slab = NULL;
}

objpool_t *create_objpool(int obj_size, unsigned int mode, objp_oops_t *oops)
{
	objpool_t *mpl = malloc(sizeof(objpool_t));

	if (obj_size % sizeof(long)) {
		mpl->obj_size = ( obj_size / sizeof(long) + 1 ) * sizeof(long);
	}else {
		mpl->obj_size = obj_size;
	}
	mpl->real_obj_size = mpl->obj_size + sizeof(void*);
	mpl->slab_size = OBJ_NUM * mpl->real_obj_size;
	objpool_init(mpl);

	/* need id when these objpool ptr is saved in objs of another type */
	mpl->mode = mode;
	if (mpl->mode & OBJP_M_NEEDID) {
		mpl->oops = oops;	/* may be null when no significant pointer member in obj */
		struct ser_root_data *srd = msm_get_no_ref();
		mpl->pool_id = idr_alloc(&srd->poolidr, mpl, POOLID_MIN, POOLID_MAX);
		mpl->slab_idr = malloc(sizeof(struct idr));
		idr_init(mpl->slab_idr);
	}

	return mpl;
}

void destroy_objpool(objpool_t *mpl)
{
	slab_t *slab;

	if (mpl) {
		slab = mpl->slab_head;
		while (slab) {
			mpl->slab_head = slab->next;
			free_slab(slab);
			slab = mpl->slab_head;
		}

		if (mpl->mode & OBJP_M_NEEDID) {
			struct ser_root_data *srd = msm_get_no_ref();
			idr_remove(&srd->poolidr, mpl->pool_id);
			idr_destroy(mpl->slab_idr);
			free(mpl->slab_idr);
			mpl->slab_idr = NULL;
		}
		free(mpl);
	}
}

void *objpool_alloc(objpool_t *mpl)
{
	slab_t *found_slab;
	void *object;

	if (mpl->recent_slab && mpl->recent_slab->free) {
		found_slab = mpl->recent_slab;
		goto found;
	}

	found_slab = mpl->slab_head;
	while (found_slab) {
		if (found_slab->free) {
			mpl->recent_slab = found_slab;
			goto found;
		}
		found_slab = found_slab->next;
	}

	found_slab = add_new_slab(mpl);

found:
	object = found_slab->free;
	found_slab->free = get_freepointer(mpl, object);
	set_freepointer(mpl, object, found_slab);

	mpl->free_obj_nr--;

	return object;
}

void *objpool_zalloc(objpool_t *mpl)
{
	void *obj = objpool_alloc(mpl);

	assert(obj != NULL);
	memset(obj, 0x0, mpl->obj_size);

	return obj;
}

void objpool_free(objpool_t *mpl, void *object)
{
	slab_t *slab;

	slab = (slab_t*)get_freepointer(mpl, object);
	set_freepointer(mpl, object, slab->free);
	slab->free = object;

	mpl->free_obj_nr++;
}

unsigned int objpool_ser_getlen(struct ser_root_data *srd, void *mpl)
{
	unsigned int len = 0;
	objpool_t *pl = (objpool_t *)mpl;

	len += sizeof(struct objpool);
	len += ((sizeof(slab_t *)+sizeof(slab_t) + pl->slab_size) * pl->slab_nr);

	return len;
}

/* used only in obj_ser_dtor(), for getting sered version of object at **obj** */
void *get_sered_obj(objpool_t *mpl, void *obj)
{
	assert(mpl->mode & OBJP_M_NEEDID);
	assert(mpl->slab_idr != NULL);

	slab_t *slab = get_freepointer(mpl, obj);
	struct slab_ser_ctx *ssc = idr_find(mpl->slab_idr, slab->id);
	assert(ssc->slab == slab);

	return (char *)obj+((char *)ssc->sered_slab-(char *)ssc->slab);
}

char *objpool_ser(struct ser_root_data *srd, void *mpl, char *dst)
{
	void *dv = dst;
	objpool_t *pl = (objpool_t *)mpl;

	/* 1st, serring all data */
	objpool_t *sered_pl = (objpool_t *)dst;
	dv = msm_ser(dv, (void *)pl, sizeof(objpool_t), true);

	int i=0;
	slab_t *s = pl->slab_head;
	while (s) {
		/* sering an old position, for repaching at load time */
		dv = msm_ser(dv, (void *)&s, sizeof(slab_t *), true);

		/* prepare the context for ser patching if need */
		if (pl->slab_idr != NULL) {
			struct slab_ser_ctx *ssc = malloc(sizeof(struct slab_ser_ctx));
			ssc->slab = s;
			ssc->sered_slab = (struct slab *)dv;
			idr_alloc(pl->slab_idr, ssc, s->id, s->id+1);
		}

		/* sering slab itself */
		dv = msm_ser(dv, (void *)s, sizeof(slab_t)+pl->slab_size, true);

		s = s->next;
	}

	/* 2nd, patching slab&pool
	 * theoretically speaking, we need do some patching here, but for speed,
	 * we do patching at the very least, because there will be an equal amount
	 * of work/repaching when loading.
	 * We just recorde slab # stored at objpool_t:recent_slab, 0th slab is
	 * at objpool_t:slab_head */
	sered_pl->recent_slab = sered_pl->recent_slab->id;
	if (pl->oops && pl->oops->ser_dtor)
		pl->oops->ser_dtor(pl);

	return dv;
}

char *objpool_deser_high(struct ser_root_data *srd, void *mpl, char *src)
{
	void *dv = src;
	assert(mpl != NULL);

	/* deserring and repatching */
	objpool_t *sered_pl = (objpool_t *)src;
	/* here, we may use msm_deser() to deser pool type, but must note
	 * NOT to corrupt the data in dest pool */
	dv = msm_deser((void *)dv, (void *)mpl, sizeof(objpool_t), false);

	slab_t *next_slab=NULL, *new_slab=NULL;
	char *pt, *saved_pt;
	int slab_cnt = sered_pl->slab_nr;
	pt = saved_pt = (char *)dv +
		slab_cnt*(sizeof(slab_t *)+sizeof(slab_t)+sered_pl->slab_size);
	while (slab_cnt-- > 0) {
		pt -= (sizeof(slab_t *)+sizeof(slab_t)+sered_pl->slab_size);
		new_slab = add_new_slab(mpl);
		msm_deser((void *)(pt+sizeof(slab_t*)), (void *)new_slab,
				sizeof(slab_t)+sered_pl->slab_size, true);

		/* repatching */
		signed long deta = (char *)new_slab - *(char **)pt;
		if (new_slab->next)	/* where old value != 0 */
			new_slab->next = next_slab;
		if (new_slab->free)
			new_slab->free += deta;

		char *tmp = (char *)new_slab + sizeof(slab_t) + sered_pl->obj_size;
		for (int i=0; i<OBJ_NUM; i++) {
			if (*(char **)tmp == 0) {		/* null tail */
				tmp += sered_pl->real_obj_size;
				continue;
			}else if (*(char **)tmp == *(char **)pt) {	/* busy obj */
				*(char **)tmp = new_slab;
				if (((struct objpool *)mpl)->oops && ((struct objpool *)mpl)->oops)
					((struct objpool *)mpl)->oops->ser_ctor_high(mpl, tmp-sered_pl->obj_size);
			}
			else	/* free obj */
				*(char **)tmp += deta;

			tmp += sered_pl->real_obj_size;
		}

		next_slab = new_slab;
	}

	/* patching recent_slab field */
	slab_t *s = ((struct objpool *)mpl)->slab_head;
	while (s) {
		if (s->id == (int)sered_pl->recent_slab) {
			((struct objpool *)mpl)->recent_slab = s;
			break;
		}
		s = s->next;
	}
	((struct objpool *)mpl)->free_obj_nr = sered_pl->free_obj_nr;

	/* mapping in ididr from sered pool_id to new pool_id */
	int retid = idr_alloc(&srd->ididr, (void *)((struct objpool *)mpl)->pool_id,
			      sered_pl->pool_id, sered_pl->pool_id+1);
	assert(retid == sered_pl->pool_id);

	return saved_pt;
}

void objpool_deser_low(struct ser_root_data *srd, void *mpl)
{
	struct objpool *p = (struct objpool *)mpl;
	if (p->oops && p->oops->ser_ctor_low)
		p->oops->ser_ctor_low(mpl);
}

void ser_objp_content(objpool_t *mpl, void **p_other_objp)
{
#if 0
	/* obj pointer */
	void *objp = *pointer;
	slab_t *slab = (slab_t*)get_freepointer(mpl, objp);

	union objp_ser_ctx *osc = (union objp_ser_ctx *)pointer;
	//osc->c.objp_id = mpl->
#else
	//*(long *)p_other_objp = d
#endif
}

void deser_objp_content(objpool_t *mpl, void **p_other_objp)
{
}
