#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "cvspool.h"
#include "list.h"

/*
 *  +-----------------------------------------------------------------+
 *  |     cvspool						      |
 *  |  +-----------+                                                  |
 *  |  |           |                                                  |
 *  |  |           |       trunk1              trunk2                 |
 *  |  |trunk_list |--->+----------+   +--->+----------+   +--> NULL  |
 *  +--|btrunk_list|    |          |   |    |          |   |          |
 *     +-----------+    |          |   |    |          |   |          |
 *                      |          |   |    |          |   |          |
 *           +----------|mem       |   |    |mem       |   |          |
 *           |  +-------|free_head |   |    |free_head |   |          |
 *           |  |       |next o<-+ |---+    |next o<-+ |---+          |
 *           |  |   +---|pprev   +-|--------|pprev   +-|--------------+
 *           |  |   |   +----------+        +----------+
 *           |  |   +-----> NULL
 *           +--+------>+--------------+
 *                      |  1st fnode   |
 *                   +>>|  as header   |==+    *Note: the fnode list is constructed:
 *                   ^  |--------------|  |	1, take blk_idx/in_blk_offset as
 *                   ^  |XXXXXXXXXXXXXX|  |        prev/next pointer;
 *                   ^  |XXXXXXXXXXXXXX|  |     2, is a circular list.
 *                   +<<|              |<=+     3, max size of fnode is MAX_BLK_SIZE
 *                 +>>>>|  2nd fnode   |==+
 *                 ^    |XXXXXXXXXXXXXX|  |
 *                 ^    |XXXXXXXXXXXXXX|  |
 *                 ^    |X  BUSY-NODE X|  |
 *                 ^    |XXXXXXXXXXXXXX|  |
 *                 +<<<<|              |<=+
 *                      |  3rd fnode   |
 *                   +>>|              |==+
 *                   ^  |XXXXXXXXXXXXXX|  |
 *                   ^  |XXXXXXXXXXXXXX|  |
 *                   ^  |XXXXXXXXXXXXXX|  |
 *                   +<<|              |<=+
 *                      |  4th fnode   |
 *                      +--------------+
 *
 */

/* save slot count into fnode0/fnode1 */
void set_fnode_len(cvspool_fnode0 *fn0, unsigned int len)
{
	cvspool_fnode1 *fn1 = (cvspool_fnode1 *)((char *)fn0+(len<<2)-sizeof(cvspool_fnode1));

	assert(len >= MIN_NODE_SIZE);
	fn0->len_1 = fn1->len_1 = len-1;
	fn0->pad = FREE_GUARD0;
	fn1->g1 = FREE_GUARD1;
}

int get_fnode_len(cvspool_fnode0 *fn0)
{
	cvspool_fnode1 *fn1 = (cvspool_fnode1 *)((char *)fn0+((fn0->len_1+1)<<2)-sizeof(cvspool_fnode1));

	assert(fn0->len_1 == fn1->len_1);
	return fn0->len_1+1;
}

void set_bnode_len(cvspool_bnode0 *bn0, unsigned int len)
{
	assert(len >= MIN_NODE_SIZE);
	bn0->g0 = BUSY_GUARD0;
	bn0->len_1 = len-1;
}

unsigned int get_bnode_len(cvspool_bnode0 *bn0)
{
	return bn0->len_1+1;
}

/* alloc_trunk */
int cvsp_init_trunk(cvspool_trunk *t, u_int32_t tslots_cnt)
{
	int poollen = tslots_cnt<<2;
	if (poollen < MIN_NODE_SIZE)  return -1;
	t->t_total_mem = poollen + (MIN_NODE_SIZE<<2);
	t->mem = (char *)malloc(t->t_total_mem);
	if (t->mem == NULL) {
		return -1;
	}

	/* use the first free node as a head to make circle free list.
	 * Never allocate mem from the first free node */
	t->next = NULL;
	t->t_used = 0;
	t->pprev = NULL;
	t->t_total = tslots_cnt;
	t->free_head = (cvspool_fnode0 *)t->mem;
	t->free_head->idx = PNBLKIDX(0,0);	// init the head directory
	t->free_head->prev = IBOFF_OF_SLOTABS(0);
	t->free_head->next = IBOFF_OF_SLOTABS(MIN_NODE_SIZE);
	set_fnode_len(t->free_head, MIN_NODE_SIZE);

	/* but all nodes still use offset from t->mem */
	int remained = poollen;
	cvspool_fnode0 *spprevfn, *spfn;
	spfn = (cvspool_fnode0 *)(t->mem+(MIN_NODE_SIZE<<2));
	spfn->idx = PNBLKIDX(0,0);	// store self-slotabs into next/2nd-half of idx
	spfn->prev = IBOFF_OF_SLOTABS(0);
	spfn->next = IBOFF_OF_SLOTABS(MIN_NODE_SIZE);
	set_fnode_len(spfn, MIN(((MAX_BLK_SIZE>>2)-MIN_NODE_SIZE), (remained>>2)));

	spprevfn = spfn;
	remained -= (MAX_BLK_SIZE-(MIN_NODE_SIZE<<2)); //byte count

	int cur_slot = 0;
	while (remained > 0) {
		spfn = (cvspool_fnode0 *)((char *)spprevfn+(get_fnode_len(spprevfn)<<2));
		set_fnode_len(spfn, MIN((MAX_BLK_SIZE>>2), (remained>>2)));

		cur_slot = tslots_cnt-(remained>>2)+MIN_NODE_SIZE;
		spfn->idx = PNBLKIDX(NEXT_BLKIDX(spprevfn->idx),
				     BLKIDX_OF_SLOTABS(cur_slot));
		spfn->prev = spprevfn->next;
		spprevfn->idx = PNBLKIDX(PREV_BLKIDX(spprevfn->idx),
					 BLKIDX_OF_SLOTABS(cur_slot));
		spfn->next = spprevfn->next = IBOFF_OF_SLOTABS(cur_slot);

		spprevfn = spfn;
		remained -= MAX_BLK_SIZE;
	}
	/* modify t->free_head first before setting the last free node */
	t->free_head->idx = PNBLKIDX(NEXT_BLKIDX(spfn->idx), NEXT_BLKIDX(t->free_head->idx));
	t->free_head->prev = spfn->next;
	spfn->next = IBOFF_OF_SLOTABS(0);	// free node list formats as: 0(free_head)<-a=b=c->0(free_head)
	spfn->idx = PNBLKIDX(PREV_BLKIDX(spfn->idx), BLKIDX_OF_SLOTABS(0));

	return 0;
}

unsigned int slots_num_adjust(unsigned int slots_num)
{
	unsigned int st = slots_num, nt = 0;

	/* take care of situation where across trunk border */
	nt++;
	st += MIN_NODE_SIZE;
	while ((st/(MAX_TRUNK_SIZE>>2)) && (st%(MAX_TRUNK_SIZE>>2))) {
		nt++;
		st -= (MAX_TRUNK_SIZE>>2);
		st += MIN_NODE_SIZE;
	}
	if (nt > 1) { /* several trunks? recursion for the last trunk */
		unsigned int slots_in_last_trunk =
			slots_num - ((nt-1) * ((MAX_TRUNK_SIZE>>2) - MIN_NODE_SIZE));
		slots_num -= slots_in_last_trunk;
		slots_num += slots_num_adjust(slots_in_last_trunk);
		return slots_num;
	}

	/* make sure there are MIN_NODE_SIZE slots in a block/trunk */
	if ((slots_num < MIN_NODE_SIZE) && (slots_num % MIN_NODE_SIZE)) {
		slots_num += (MIN_NODE_SIZE - (slots_num % MIN_NODE_SIZE));
		return slots_num;
	}

	/* make sure slot num of the last blk in the last trunk is >= MIN_NODE_SIZE */
	int remain_after_blocked = (slots_num + MIN_NODE_SIZE) % (MAX_BLK_SIZE>>2);
	if ((remain_after_blocked < MIN_NODE_SIZE) && (remain_after_blocked % MIN_NODE_SIZE))
		slots_num += (MIN_NODE_SIZE - (remain_after_blocked % MIN_NODE_SIZE));

	return slots_num;
}

cvspool_trunk *add_new_trunk(cvspool *sp, unsigned int t_slot_size)
{
	cvspool_trunk *ptrunk = NULL;
	assert(sp->trunk_list != NULL);

	ptrunk = (cvspool_trunk *)malloc(sizeof(cvspool_trunk));
	if (ptrunk == NULL) return ptrunk;

	cvsp_init_trunk(ptrunk, t_slot_size);

	sp->trunk_list->pprev = &ptrunk->next;
	ptrunk->next = sp->trunk_list;
	sp->trunk_list = ptrunk;
	sp->total += t_slot_size;
	sp->total_mem += ptrunk->t_total_mem;
	sp->total_mem += sizeof(cvspool_trunk);

	/* trunk id, count from zero, trunk_cnt steps up from trunk_ocnt's value */
	ptrunk->id = sp->trunk_cnt++;

	return ptrunk;
}

/* total length of the pool is 4Bytes*slots_num */
int cvsp_init(cvspool **cvsp, unsigned int slots_num, unsigned int mode)
{
	assert(cvsp != 0);
	cvspool *sp = (cvspool *)malloc(sizeof(cvspool));
	if (sp == NULL) goto out1;

	memset(sp, 0, sizeof(cvspool));
	sp->mode = mode;

	/* need id when cvspool ptr is saved in objs of another type */
	if (sp->mode & CVSP_M_NEEDID) {
		struct ser_root_data *srd = msm_get_no_ref();
		sp->pool_id = idr_alloc(&srd->poolidr, sp, POOLID_MIN, POOLID_MID-1);
		sp->trunk_idr = malloc(sizeof(struct idr));
		idr_init(sp->trunk_idr);
	}

	/* we need make sure there are 3 slots at lease in the last free block */
	slots_num = slots_num_adjust(slots_num);
	sp->total = slots_num;

	int tremain = slots_num;
	cvspool_trunk *ptrunk=NULL, *prev_trunk=NULL;
	do {
		ptrunk = (cvspool_trunk *)malloc(sizeof(cvspool_trunk));
		if (ptrunk == NULL) goto out2;

		if (sp->trunk_list == NULL)
			sp->trunk_list=ptrunk;

		/* adjust for blk0 in every trunk */
		int tsize = MIN((MAX_TRUNK_SIZE-(MIN_NODE_SIZE<<2)), (tremain<<2));
		cvsp_init_trunk(ptrunk, tsize>>2);

		if (prev_trunk != NULL) {
			prev_trunk->next = ptrunk;
			ptrunk->pprev = &prev_trunk->next;
		}
		prev_trunk = ptrunk;

		tremain -= (tsize>>2);
		ptrunk->id = sp->trunk_ocnt++; /* trunk id, count from zero */
		sp->total_mem += ptrunk->t_total_mem;
		sp->total_mem += sizeof(cvspool_trunk);
		sp->btrunk_list = &ptrunk->next;
	} while (tremain > 0);

	sp->trunk_cnt = sp->trunk_ocnt; /* after this, maintain trunk cnt in trunk_cnt */
	*cvsp = sp;
	return 0;

out2:
	/* destory sp */
	cvsp_destroy(sp);
out1:
	return -1;
}

void cvsp_destroy(cvspool *cvsp)
{
	assert(cvsp != 0);
	if (cvsp->used != 0) {
		PINFO("cvspool can't be destroy due to busy.");
		return;
	}

	cvspool_trunk *t = cvsp->trunk_list;
	while (t) {
		free(t->mem);

		cvspool_trunk *pt = t;
		t = pt->next;

		free((void *)pt);
	}

	if (cvsp->mode & CVSP_M_NEEDID) {
		struct ser_root_data *srd = msm_get_no_ref();
		idr_remove(&srd->poolidr, cvsp->pool_id);
		idr_destroy(cvsp->trunk_idr);
		free(cvsp->trunk_idr);
		cvsp->trunk_idr = NULL;
	}
	free((void *)cvsp);

	return;
}

/* size in Bytes */
char *cvsp_alloc(cvspool *cvsp, unsigned int size)
{
	assert(cvsp != 0);
	if (cvsp->used == cvsp->total) {
		PINFO("cvspool is full.");
		if (cvsp->mode & CVSP_M_AUTOEX)
			/* take the 1st trunk as a template */
			add_new_trunk(cvsp, cvsp->trunk_list->t_total);
		else
			return NULL;
	}

	/* There are 4Bytes for meta data in busy node.
	 * And a busy node occupies 3 slots at least. */
	unsigned int need_slots = MAX(((size+4+3)>>2), MIN_NODE_SIZE);
	//assert(need_slots+1 <= (MAX_BLK_SIZE>>2));
	assert(need_slots <= (MAX_BLK_SIZE>>2));

	/* no way to alloc buf of size longer than the larger of 1st trunk's size
	 * and max_size of a block. Since the above assertion can exclude the
	 * first case, here we need just to think about the 2nd one.
	 * (take 4Bytes meta of busy node into account) */
	if (need_slots > cvsp->trunk_list->t_total) {
		PINFO("too much of allocation.");
		return NULL;
	}

	cvspool_fnode0 *pfn=NULL;
	cvspool_trunk *t = NULL;

RESCAN: t = cvsp->trunk_list;
	while (t) {
		assert(t->mem != 0);
		for_each_fnode(t, &pfn) {
			int len = get_fnode_len(pfn);
			if ((len == need_slots) || (len >= need_slots+MIN_NODE_SIZE))
				break;
		}
		if (pfn && (pfn!=t->free_head))	// found it
			break;

		t = t->next;
	}

	if (!pfn || !t) {
		PINFO("there is no free node large enough in cvspool.");
		if (cvsp->mode & CVSP_M_AUTOEX) {
			/* take the 1st trunk as a template */
			add_new_trunk(cvsp, cvsp->trunk_list->t_total);
			goto RESCAN;
		}
		else
			return NULL;
	}

	cvspool_fnode0 *prev_pfn=NULL, *next_pfn=NULL;
	prev_pfn = PREV_FNODE(t, pfn);
	next_pfn = NEXT_FNODE(t, pfn);
	if (get_fnode_len(pfn) == need_slots) {
		prev_pfn->idx = PNBLKIDX(PREV_BLKIDX(prev_pfn->idx), NEXT_BLKIDX(pfn->idx));
		prev_pfn->next = pfn->next;

		next_pfn->idx = PNBLKIDX(PREV_BLKIDX(pfn->idx), NEXT_BLKIDX(next_pfn->idx));
		next_pfn->prev = pfn->prev;
	}else {
		cvspool_fnode0 *new_fn = (cvspool_fnode0 *)((char *)pfn+(need_slots<<2));
		memcpy(new_fn, pfn, sizeof(cvspool_fnode0));
		set_fnode_len(new_fn, (get_fnode_len(pfn)-need_slots));

		prev_pfn->next += need_slots;
		next_pfn->prev += need_slots;
	}

	/* bookkeeping allocated pfn and return */
	cvspool_bnode0 *pbn0 = (cvspool_bnode0 *)pfn;
	set_bnode_len(pbn0, need_slots);

	t->t_used += need_slots;
	cvsp->used += need_slots;
	return (char *)(pbn0+1);
}

/* get trunk ptr of a node in this pool */
cvspool_trunk *get_trunk(cvspool *cvsp, char *str)
{
	/* optimize with some trunk ptr cache */
	cvspool_trunk *t = cvsp->cached_trunk;
	if ((t!=NULL) &&
	   ((t->mem<=str) && (str<(t->mem+t->t_total_mem))))
		return t;

	/* search the trunk */
	t = cvsp->trunk_list;
	while (t) {
		assert(t->mem != 0);

		/* found our trunk */
		if ((t->mem<=str) && (str<(t->mem+MAX_TRUNK_SIZE)))
			break;

		t = t->next;
	}

	assert(t != NULL);
	cvsp->cached_trunk = t;
	return t;
}

cvspool_trunk *get_trunk_by_id(cvspool *cvsp, unsigned tid)
{
	cvspool_trunk *t = cvsp->cached_trunk;
	if ((t!=NULL) && (t->id==tid))
		return t;

	/* search the trunk */
	t = cvsp->trunk_list;
	while (t) {
		/* found our trunk */
		if (t->id == tid)
			break;

		t = t->next;
	}

	assert(t != NULL);
	cvsp->cached_trunk = t;
	return t;
}

unsigned long id_of_node(cvspool *cvsp, char *str)
{
	union node_id nid = {0};
	cvspool_trunk *t = get_trunk(cvsp, str);

	nid.s.pool = cvsp->pool_id;
	nid.s.trunk = t->id;
	nid.s.slot = (str-t->mem)>>2;

	return nid.l;
}

/* Get node from the desered/original cvspool, the poolid in id param is
 * poolid of either desered or sered cvspool. If the id param is of sered pool,
 * this function must be called in the context of *_deser_low() */
char *node_from_id(cvspool *cvsp, unsigned long id)
{
	union node_id nid = {0};

	nid.l = id;

	struct ser_root_data *srd = msm_get_no_ref();
	unsigned desered_poolid = idr_find(&srd->ididr, nid.s.pool);
	assert(cvsp && ((cvsp->pool_id == nid.s.pool)
			||(cvsp->pool_id == desered_poolid)));

	cvspool_trunk *t = get_trunk_by_id(cvsp, nid.s.trunk);
	assert(t != NULL);

	return (char *)(t->mem + (nid.s.slot<<2));
}

/* get a busy node ptr a time, must be called while iteration, such as while or
 * for loop */
char *iter_bnode(cvspool_trunk *t)
{
	static char *tmp = NULL;
	if (t->t_used == 0) return NULL;

	if (tmp == NULL)
		tmp = t->mem;
	else	/* have found a bnode in the last call */
		tmp += ((((cvspool_bnode0 *)tmp)->len_1+1)<<2);
	while ((cvspool_bnode0 *)tmp < (t->mem + (t->t_total<<2))) {
	        if (((cvspool_bnode0 *)tmp)->g0 == BUSY_GUARD0)
			break;
		tmp += ((((cvspool_bnode0 *)tmp)->len_1+1)<<2);
	}

	if ((cvspool_bnode0 *)tmp >= (t->mem + (t->t_total<<2))) {
		tmp = NULL;
		return NULL;
	}

	/* bnode content pointer */
	return tmp+sizeof(cvspool_bnode0);
}

/* do something for every busy node
 * @fn: function to be called for each busy node
 * @data: data passed back to callback function
 */
void cvsp_for_each_bnode(cvspool *cvsp, int (*fn)(void *p, void *data), void *data)
{
	char *pobj = NULL;
	cvspool_trunk *t = NULL;

	assert(fn != NULL);
	for_each_trunk(cvsp, &t) {
		while ((pobj=iter_bnode(cvsp->trunk_list)) != NULL)
			fn(pobj, data);
	}
}

/* get usable memory size of a busy node, in bytes */
unsigned int get_bnode_mem_len(cvspool *cvsp, char *mem)
{
	cvspool_trunk *t = get_trunk(cvsp, mem);
	assert(t != NULL);

	cvspool_bnode0 *pbn = (cvspool_bnode0 *)(mem-sizeof(cvspool_bnode0));
	assert(pbn->g0 == BUSY_GUARD0);

	/* 1 slots for busy meta */
	return (get_bnode_len(pbn)-1) << 2;
}

void cvsp_free(cvspool *cvsp, char *str)
{
	cvspool_trunk *t = get_trunk(cvsp, str);

	cvspool_bnode0 *pbn = (cvspool_bnode0 *)(str-sizeof(cvspool_bnode0));
	assert(pbn->g0 == BUSY_GUARD0);

	/* find the node at prev/next location */
	unsigned prev_free=false, next_free=false;
	cvspool_fnode0 *prevfn0=NULL, *nextfn0=NULL;

	int pbn_len = get_bnode_len(pbn);
	nextfn0 = (cvspool_fnode0 *)((char *)pbn+(pbn_len<<2));
	if (nextfn0->pad == FREE_GUARD0)
		next_free = true;

	cvspool_fnode0 *pfn = (cvspool_fnode0 *)pbn;
	int slotabs = ((char*)pfn-t->mem)>>2;
	cvspool_fnode1 *prevfn1 = (cvspool_fnode1 *)((char *)pbn-sizeof(cvspool_fnode1));
	if (prevfn1->g1 == FREE_GUARD1) {
		prevfn0 = (cvspool_fnode0 *)((char *)pbn-((prevfn1->len_1+1)<<2));
		assert(prevfn0->pad == FREE_GUARD0);
		assert(prevfn0->len_1 == prevfn1->len_1);
		/* don't merge with free_head */
		prev_free = (t->free_head != prevfn0)? true: false;
	}

	if (prev_free && next_free) {
		cvspool_fnode0 *prev_of_next = PREV_FNODE(t, nextfn0);
		cvspool_fnode0 *next_of_next = NEXT_FNODE(t, nextfn0);

		prev_of_next->idx = PNBLKIDX(PREV_BLKIDX(prev_of_next->idx), NEXT_BLKIDX(nextfn0->idx));
		prev_of_next->next = nextfn0->next;
		next_of_next->idx = PNBLKIDX(PREV_BLKIDX(nextfn0->idx), NEXT_BLKIDX(next_of_next->idx));
		next_of_next->prev = nextfn0->prev;

		set_fnode_len(prevfn0, get_fnode_len(prevfn0)+get_bnode_len(pbn)+get_fnode_len(nextfn0));
	}else if (prev_free) {
		set_fnode_len(prevfn0, get_fnode_len(prevfn0)+get_bnode_len(pbn));
	}else if (next_free) {
		cvspool_fnode0 *prev_of_next = PREV_FNODE(t, nextfn0);
		cvspool_fnode0 *next_of_next = NEXT_FNODE(t, nextfn0);

		set_fnode_len(pfn, pbn_len+get_fnode_len(nextfn0));
		pfn->idx = nextfn0->idx;
		pfn->prev = nextfn0->prev;
		pfn->next = nextfn0->next;

		prev_of_next->idx = PNBLKIDX(PREV_BLKIDX(prev_of_next->idx), BLKIDX_OF_SLOTABS(slotabs));
		prev_of_next->next -= pbn_len;
		next_of_next->idx = PNBLKIDX(BLKIDX_OF_SLOTABS(slotabs), NEXT_BLKIDX(next_of_next->idx));
		next_of_next->prev -= pbn_len;
	}else {
		cvspool_fnode0 *ppfn = PREV_FNODE(t, t->free_head);

		set_fnode_len(pfn, pbn_len);
		pfn->idx = PNBLKIDX(PREV_BLKIDX(t->free_head->idx), NEXT_BLKIDX(ppfn->idx));
		pfn->prev = t->free_head->prev;
		pfn->next = ppfn->next;

		ppfn->idx = PNBLKIDX(PREV_BLKIDX(ppfn->idx), BLKIDX_OF_SLOTABS(slotabs));
		ppfn->next = IBOFF_OF_SLOTABS(slotabs);

		t->free_head->idx = PNBLKIDX(BLKIDX_OF_SLOTABS(slotabs), NEXT_BLKIDX(t->free_head->idx));
		t->free_head->prev = IBOFF_OF_SLOTABS(slotabs);
	}
	t->t_used -= pbn_len;
	cvsp->used -= pbn_len;
}

unsigned int cvspool_ser_getlen(struct ser_root_data *srd, void *mpl)
{
	unsigned int len = 0;
	cvspool *cvsp = (cvspool *)mpl;

	len += sizeof(cvspool);
	len += cvsp->total_mem;

	return len;
}

char *cvspool_ser(struct ser_root_data *srd, void *mpl, char *dst)
{
	void *dv = dst;
	cvspool *cvspl = (cvspool *)mpl;

	/* 1st, serring all data */
	cvspool *sered_cvspl = (cvspool *)dst;
	dv = msm_ser(dv, (void *)cvspl, sizeof(cvspool), true);

	int i=0;
	cvspool_trunk *pt = cvspl->trunk_list;
	while (pt) {
		/* for simply desering, we firstly do sering trunk's memory */
		dv = msm_ser(dv, (void *)pt->mem, pt->t_total_mem, true);

		/* and then do sering trunk itself */
		dv = msm_ser(dv, (void *)pt, sizeof(cvspool_trunk), true);

		pt = pt->next;
	}

	return dv;
}

/* get next(aka. the prev trunk) trunk to deser every time.
 * NOTE: because static variable used here, this function shoule be called like:
 *
 * while(!prev_trunk()) {
 *    do_something(...);
 * }
 *
 * Namely, we must iterate all trunks in a pool, so the static pt pointer has a
 * chance to be NULL. */
cvspool_trunk *prev_trunk(cvspool *p)
{
	assert(p != NULL);
	assert(p->btrunk_list != NULL);
	static cvspool_trunk *pt = NULL;

	if (pt == NULL) {
		pt = container_of(p->btrunk_list, cvspool_trunk, next);
	}else {
		if (pt->pprev == NULL) {
			pt = NULL;
			return NULL;
		}
		pt = container_of(pt->pprev, cvspool_trunk, next);
	}

	return pt;
}

char *cvspool_deser_high(struct ser_root_data *srd, void *mpl, char *src)
{
	void *dv = src;
	assert(mpl != NULL);

	/* deserring and repatching */
	cvspool *sered_cvspl = (cvspool *)src;
	/* here, we may use msm_deser() to deser pool type, but must note
	 * NOT to corrupt the data in dest pool */
	dv = msm_deser((void *)dv, (void *)mpl, sizeof(cvspool), false);

	char *pt, *saved_pt;
	cvspool_trunk *next_trunk=NULL, *new_trunk=NULL;

	/* total_mem has include sizeof(cvspool_trunk) */
	pt = saved_pt = (char *)dv + sered_cvspl->total_mem;

	int i=0;
	while (i < sered_cvspl->trunk_cnt) {
		cvspool_trunk *cur_t = (cvspool_trunk *)(pt-sizeof(cvspool_trunk));
		if (i < sered_cvspl->trunk_ocnt)
			new_trunk = prev_trunk((cvspool *)mpl);
		else {
			/* for correct work of next pool, we must do an extra
			 * call of perv_trunk() function here */
			if (i == sered_cvspl->trunk_ocnt) {
				cvspool_trunk *tmp = prev_trunk((cvspool *)mpl);
				assert(tmp == NULL);
			}

			/* take the 1st trunk as a template */
			new_trunk = add_new_trunk((cvspool *)mpl, ((cvspool *)mpl)->trunk_list->t_total);
		}
		assert(new_trunk->t_total_mem == cur_t->t_total_mem);
		msm_deser(cur_t, (void *)new_trunk, sizeof(cvspool_trunk), false);
		new_trunk->t_used = cur_t->t_used;

		pt -= (sizeof(cvspool_trunk) + cur_t->t_total_mem);
		msm_deser(pt, (void *)new_trunk->mem, cur_t->t_total_mem, true);

		i++;
	}

	/* patching */
	((cvspool *)mpl)->used = sered_cvspl->used;

	/* bookkeeping pool id mapping */
	int retid = idr_alloc(&srd->ididr, (void *)(((cvspool *)mpl)->pool_id),
			      sered_cvspl->pool_id, sered_cvspl->pool_id+1);
	assert(retid == sered_cvspl->pool_id);

	return saved_pt;
}

void cvspool_deser_low(struct ser_root_data *srd, void *mpl)
{
	cvspool *p = (cvspool *)mpl;

	/* we are used purly for storing strings, so do nothing here. */
	p;
}
