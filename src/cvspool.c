#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "cvspool.h"

/*
 *   cvspool
 * +-----------+
 * |           |
 * |           |       trunk1              trunk2
 * |trunk_list |--->+----------+   +--->+----------+   +--> NULL
 * +-----------+    |          |   |    |          |   |
 *                  |          |   |    |          |   |
 *            +-----|mem       |   |    |mem       |   |
 *            |  +--|free_head |   |    |free_head |   |
 *            |  |  |next      |---+    |next      |---+
 *            |  |  +----------+        +----------+
 *            |  |
 *            +--+->+--------------+
 *               +>>|  1st fnode   |==+    *Note: the fnode list is constructed:
 *               ^  |--------------|  |	    1, take blk_idx/in_blk_offset as
 *               ^  |XXXXXXXXXXXXXX|  |        prev/next pointer;
 *               ^  |XXXXXXXXXXXXXX|  |     2, is a circular list.
 *               +<<|              |<=+     3, max size of fnode is MAX_BLK_SIZE
 *             +>>>>|  2nd fnode   |==+
 *             ^    |XXXXXXXXXXXXXX|  |
 *             ^    |XXXXXXXXXXXXXX|  |
 *             ^    |X BUSY FNODE X|  |
 *             ^    |XXXXXXXXXXXXXX|  |
 *             +<<<<|              |<=+
 *                  |  3rd fnode   |
 *               +>>|              |==+
 *               ^  |XXXXXXXXXXXXXX|  |
 *               ^  |XXXXXXXXXXXXXX|  |
 *               ^  |XXXXXXXXXXXXXX|  |
 *               +<<|              |<=+
 *                  |  4th fnode   |
 *                  +--------------+
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
	t->mem = (char *)malloc(poollen + (MIN_NODE_SIZE<<2));
	if (t->mem == NULL) {
		return -1;
	}

	/* use the first free node as a head to make circle free list.
	 * Never allocate mem from the first free node */
	t->next = NULL;
	t->t_used = 0;
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

void add_new_trunk(cvspool *sp, unsigned int t_slot_size)
{
	cvspool_trunk *ptrunk = NULL;
	assert(sp->trunk_list != NULL);

	ptrunk = (cvspool_trunk *)malloc(sizeof(cvspool_trunk));
	if (ptrunk == NULL) return ptrunk;

	cvsp_init_trunk(ptrunk, t_slot_size);

	ptrunk->next = sp->trunk_list;
	sp->trunk_list = ptrunk;
	sp->total += t_slot_size;
	sp->trunk_cnt++;

	return ptrunk;
}

/* total length of the pool is 4Bytes*slots_num */
int cvsp_init(cvspool **cvsp, unsigned int slots_num, unsigned int mode)
{
	assert(cvsp != 0);
	cvspool *sp = (cvspool *)malloc(sizeof(cvspool));
	if (sp == NULL) goto out1;
	sp->trunk_list = NULL;
	sp->trunk_cnt = 0;
	sp->mode = mode;

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

		if (prev_trunk != NULL)
			prev_trunk->next = ptrunk;
		prev_trunk = ptrunk;

		tremain -= (tsize>>2);
		sp->trunk_cnt++;
	} while (tremain > 0);

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
	assert(need_slots+1 <= (MAX_BLK_SIZE>>2));

	/* no way to alloc buf of size longer than the larger of 1st trunk's size
	 * and max_size of a block. Since the above assertion can exclude the
	 * first case, here we need just to think about the 2nd one.
	 * (take 4Bytes meta of busy node into account) */
	if (need_slots+1 > cvsp->trunk_list->t_total) {
		PINFO("too much allocation.");
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

cvspool_trunk *get_trunk(cvspool *cvsp, char *str)
{
	cvspool_trunk *t = cvsp->trunk_list;
	while (t) {
		assert(t->mem != 0);

		/* found our trunk */
		if ((t->mem<=str) && (str<=(t->mem+MAX_TRUNK_SIZE)))
			break;

		t = t->next;
	}

	assert(t != NULL);
	return t;
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

