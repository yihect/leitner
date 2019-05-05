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
struct objvec *objv_init(unsigned int osize, objctor ctor, void *arg, objdtor dtor)
{
	int ret;
	struct objvec *ov = malloc(sizeof(struct objvec));
	if (ov == NULL) return NULL;

	unsigned lcm = LCM(osize, 4);
	unsigned need_slots = (MAX_TRUNK_SIZE-(MIN_NODE_SIZE<<2)) / lcm;
	need_slots = (need_slots*lcm) >> 2;
	if ((ret=cvsp_init(&ov->cp, need_slots)) != 0)
		ERROR("cvsp_init() failure...\n");

	ov->size = osize;
	ov->ctor = ctor;
	ov->dtor = dtor;
	ov->arg = arg;

	return ov;
}

void objv_exit(struct objvec *ov)
{
	assert(ov != NULL);
	if (ov->cp)
		cvsp_destroy(ov->cp);
	free(ov);
}

char *objv_alloc(struct objvec *ov, unsigned obj_cnt)
{
	return cvsp_alloc(ov->cp, ov->size*obj_cnt);
}

/* caller must make sure oldobjs is allocated from our objvec, and
 * has a correct length */
char *objv_realloc(struct objvec *ov, char *oldobjs, unsigned new_cnt,
		   void (*adjust_fn)(char *oldobjs, char *newobjs, void *adj_param),
		   void *adjparam)
{
	unsigned int memlen = get_bnode_mem_len(ov->cp, (char *)oldobjs);

	/* generally expand memory */
	assert((new_cnt*ov->size) > memlen);
	char *newobjs = cvsp_alloc(ov->cp, ov->size*new_cnt);

	/* simply copy, and adjust */
	memcpy(newobjs, oldobjs, memlen);
	if (adjust_fn)
		adjust_fn(oldobjs, newobjs, adjparam);

	cvsp_free(ov->cp, oldobjs);
	return newobjs;
}

void objv_free(struct objvec *ov, char *objs)
{
	cvsp_free(ov->cp, objs);
}


