#include <assert.h>
#include "util.h"
#include "objvec.h"

unsigned int GCD(unsigned int a, unsigned int b)
{
	if(b) while((a %= b) && (b %= a));
	return a + b;
}

unsigned int LCM(unsigned int a, unsigned int b)
{
	return a * b / GCD(a, b);
}

/* osize: in bytes */
struct objvec *objv_init(unsigned int osize, unsigned int mode, struct objv_obj_ops *oops)
{
	int ret;
	struct objvec *ov = malloc(sizeof(struct objvec));
	if (ov == NULL) return NULL;

	unsigned lcm = LCM(osize, 4);
	unsigned need_slots = (MAX_TRUNK_SIZE-(MIN_NODE_SIZE<<2)) / lcm;
	need_slots = (need_slots*lcm) >> 2;
	if ((ret=cvsp_init(&ov->cp, need_slots, mode)) != 0)
		ERROR("cvsp_init() failure...\n");

	//ov->node_tree = RB_ROOT;

	ov->mode = mode;
	if (mode & OBJV_M_NEEDID) {
		ov->oops = oops;	/* may be null when no significant pointer member in obj */
		struct ser_root_data *srd = msm_get_no_ref();
		ov->pool_id = idr_alloc(&srd->poolidr, ov, POOLID_MIN, POOLID_MID-1);
	}
	ov->size = osize;

	return ov;
}

void objv_exit(struct objvec *ov)
{
	assert(ov != NULL);
	if (ov->cp)
		cvsp_destroy(ov->cp);

	if (ov->mode & OBJV_M_NEEDID) {
		struct ser_root_data *srd = msm_get_no_ref();
		idr_remove(&srd->poolidr, ov->pool_id);
	}

	free(ov);
}

char *objv_alloc(struct objvec *ov, unsigned obj_cnt)
{
	char *objv = cvsp_alloc(ov->cp, ov->size*obj_cnt);

	assert(objv != NULL);

	return objv;
}

/* caller must make sure oldobjs is allocated from our objvec, and
 * has a correct length */
char *objv_realloc(struct objvec *ov, char *oldobjs, unsigned new_cnt,
		   void (*adjust_fn)(char *oldobjs, unsigned oldsize,
				     char *newobjs, unsigned newsize,
				     void *adj_param),
		   void *adjparam)
{
	unsigned int oldlen = get_bnode_mem_len(ov->cp, (char *)oldobjs);

	/* generally expand memory */
	unsigned int newlen = ov->size * new_cnt;
	assert(newlen > oldlen);

	char *newobjs = cvsp_alloc(ov->cp, newlen);

	/* do different copy, and adjust */
	if (adjust_fn)
		adjust_fn(oldobjs, oldlen, newobjs, newlen, adjparam);

	cvsp_free(ov->cp, oldobjs);
	return newobjs;
}

void objv_free(struct objvec *ov, char *objs)
{
	cvsp_free(ov->cp, objs);
}

unsigned int objvec_ser_getlen(struct ser_root_data *srd, void *mpl)
{
	struct objvec *ov = (struct objvec *)mpl;

	return sizeof(struct objvec) + cvspool_ser_getlen(srd, ov->cp);
}

/* ser func used by cvsp_for_each_bnode() while serring objvec
 * p: objs pointer
 * data: objvec pointer
 *
 * mast return 0;
 */
static inline int objs_ser_dtor(void *p, void *data)
{
	struct objvec *ov = (struct objvec *)data;

	assert(ov && ov->oops && ov->oops->ser_dtor);
	ov->oops->ser_dtor(ov, p);

	return 0;
}

char *objvec_ser(struct ser_root_data *srd, void *mpl, char *dst)
{
	void *dv = dst;
	struct objvec *ov = (struct objvec *)mpl;

	dv = msm_ser(dv, (void *)ov, sizeof(struct objvec), true);
	dv = cvspool_ser(srd, ov->cp, dv);

	if (ov->oops && ov->oops->ser_dtor)
		cvsp_for_each_bnode(ov->cp, objs_ser_dtor, ov);

	return dv;
}

static inline int objs_ser_ctor_high(void *p, void *data)
{
	struct objvec *ov = (struct objvec *)data;

	assert(ov && ov->oops && ov->oops->ser_ctor_high);
	ov->oops->ser_ctor_high(ov, p);

	return 0;
}

char *objvec_deser_high(struct ser_root_data *srd, void *mpl, char *src)
{
	void *dv = src;
	assert(mpl != NULL);

	/* deserring and repatching */
	struct objvec *sered_ov = (struct objvec *)src;
	struct objvec *new_ov = (struct objvec *)mpl;
	/* here, we may use msm_deser() to deser pool type, but must note
	 * NOT to corrupt the data in dest pool */
	dv = msm_deser((void *)dv, (void *)mpl, sizeof(struct objvec), false);
	dv = cvspool_deser_high(srd, new_ov->cp, dv);

	/* maintain those desered objs in this objvec pool,
	 * and do extra work through oops->ser_ctor_high() if need */
	if (new_ov->oops && new_ov->oops->ser_dtor)
		cvsp_for_each_bnode(new_ov->cp, objs_ser_ctor_high, new_ov);

	/* NOTE: patching nothing */

	/* bookkeeping pool id mapping */
	int retid = idr_alloc(&srd->ididr, (void *)(new_ov->pool_id),
			      sered_ov->pool_id, sered_ov->pool_id+1);
	assert(retid == sered_ov->pool_id);

	return dv;
}

/* ser func used by idr_for_each while deser objvec
 * p: objs pointer
 * data: objvec pointer
 *
 * mast return 0;
 */
static inline int objs_ser_ctor_low(void *p, void *data)
{
	struct objvec *ov = (struct objvec *)data;

	assert(ov && ov->oops && ov->oops->ser_ctor_low);
	ov->oops->ser_ctor_low(ov, p);

	return 0;
}

void objvec_deser_low(struct ser_root_data *srd, void *mpl)
{
	struct objvec *ov = (struct objvec *)mpl;

	/* just for completion */
	cvspool_deser_low(srd, ov->cp);

	if (ov->oops && ov->oops->ser_ctor_low)
		cvsp_for_each_bnode(ov->cp, objs_ser_ctor_low, ov);
}

